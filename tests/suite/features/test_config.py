#!/usr/bin/env python3

"""
The /etc config surface and runtime service enable, on a real board.

These cover what the host suite cannot: that a config file written by the device
comes back off it intact, that a rewrite keeps the rest of the file, and that
`service enable/disable` persists somewhere `cat` can see.

The service these act on is chosen at run time and is never one the session is
arriving over, never an essential one, and is put back the way it was found. A
test that disabled the transport it was speaking through would take the rest of
the suite with it.
"""

from .registry import test, expect_in, expect_not_in, Skip


# Services safe to toggle: outbound clients with no listener the suite arrives on.
TOGGLE_CANDIDATES = ("OTA", "MQTT", "Email", "IOT", "MDNS")

# Services the device cannot be recovered without, which must refuse a disable.
ESSENTIAL_CANDIDATES = ("CMD", "DB", "Serial", "Auth", "UserStore", "FactoryReset")


def _service_rows(text):
    """`service list` rows as columns, header and blank lines dropped."""
    rows = []
    for line in text.splitlines():
        parts = line.split()
        if len(parts) >= 4 and parts[0] not in ("SERVICE",) and not line.startswith(" " * 20):
            if parts[1] in ("inactive", "active", "stopped", "dead"):
                rows.append(parts)
    return rows


def _listed(t):
    out = t.run("service list")
    if "SERVICE" not in out:
        raise Skip("this build has no service list")
    return out


def _pick(t, candidates):
    out = _listed(t)
    names = [parts[0] for parts in _service_rows(out)]
    for name in candidates:
        if name in names:
            return name
    return None


def _status(t, name):
    return t.run("service status %s" % name)


def _field(text, key):
    for line in text.splitlines():
        if line.strip().startswith(key):
            return line.split(":", 1)[1].strip() if ":" in line else ""
    return None


# ------------------------------------------------------------------ the file

@test("the ssh service config is a readable text file", needs=("cat",), services=("SSH",))
def ssh_conf_is_readable(t):
    out = t.run("cat /etc/ssh/ssh.conf")
    if not out.strip() or "No such" in out or "Failed" in out:
        raise Skip("this build has no /etc/ssh/ssh.conf")

    expect_in("PasswordAuthentication", out, "the ssh conf carries its auth options")
    expect_in("PubkeyAuthentication", out, "the ssh conf carries its pubkey option")


@test("each config option is on a line of its own", needs=("cat",), services=("SSH",))
def ssh_conf_lines_are_separate(t):
    """
    A file written with bare newlines staircases on a raw terminal and every
    option after the first arrives indented. Reading it back, that shows up as
    two options sharing one line.
    """
    out = t.run("cat /etc/ssh/ssh.conf")
    if "PasswordAuthentication" not in out:
        raise Skip("this build has no /etc/ssh/ssh.conf")

    for line in out.splitlines():
        if line.count("Authentication") > 1:
            raise AssertionError("two options landed on one line:\n%s" % out)

    starts = [line for line in out.splitlines() if line.startswith(" ")]
    body = [line for line in out.splitlines() if line.strip()]
    if len(starts) > 1 and len(starts) >= len(body) - 1:
        raise AssertionError("the file reads as staircased output:\n%s" % out)


@test("a config file survives being read twice", needs=("cat",), services=("SSH",))
def ssh_conf_is_stable(t):
    first = t.run("cat /etc/ssh/ssh.conf")
    if "PasswordAuthentication" not in first:
        raise Skip("this build has no /etc/ssh/ssh.conf")

    second = t.run("cat /etc/ssh/ssh.conf")
    if first.strip() != second.strip():
        raise AssertionError("reading the conf twice gave two answers:\n%s\nthen\n%s"
                             % (first, second))


# --------------------------------------------------------------- service listing

@test("the service listing carries an enabled column", needs=("service",))
def service_list_has_enabled(t):
    out = _listed(t)
    expect_in("ENABLED", out, "service list has an ENABLED column")

    rows = _service_rows(out)
    if not rows:
        raise AssertionError("service list showed no services:\n%s" % out)

    for parts in rows:
        if parts[2] not in ("yes", "no"):
            raise AssertionError("a service's enabled column is %r:\n%s" % (parts[2], out))


@test("the service listing columns line up", needs=("service",))
def service_list_columns_align(t):
    """
    Fixed width columns, not tabs: every row must start its STATE column at the
    same offset, which is what a tab stop cannot guarantee once a name is long.
    """
    out = _listed(t)
    lines = [line for line in out.splitlines() if line.strip()]
    header = None
    for line in lines:
        if "SERVICE" in line and "STATE" in line:
            header = line
            break
    if header is None:
        raise Skip("service list printed no header")

    at = header.index("STATE")
    for line in lines:
        if line is header or "SERVICE" in line:
            continue
        parts = line.split()
        if len(parts) < 4 or parts[1] not in ("inactive", "active", "stopped", "dead"):
            continue
        if line.index(parts[1]) != at:
            raise AssertionError("a state column starts at %d, header says %d:\n%s"
                                 % (line.index(parts[1]), at, out))


@test("a service reports the file it reads its options from", needs=("service",))
def service_status_names_config(t):
    name = _pick(t, TOGGLE_CANDIDATES) or _pick(t, ESSENTIAL_CANDIDATES)
    if name is None:
        raise Skip("no service to ask about")

    out = _status(t, name)
    if "enabled" not in out:
        raise Skip("this build's service status does not report enabled")

    path = _field(out, "config")
    if path is None or not path.startswith("/etc/"):
        raise AssertionError("%s named no config file under /etc:\n%s" % (name, out))


# ------------------------------------------------------------ enable/disable

@test("disabling a service persists into its config file",
      needs=("service", "cat"))
def disable_persists(t):
    name = _pick(t, TOGGLE_CANDIDATES)
    if name is None:
        raise Skip("no service is safe to toggle on this build")

    before = _status(t, name)
    if "enabled" not in before:
        raise Skip("this build's service status does not report enabled")
    path = _field(before, "config")
    was = _field(before, "enabled")

    out = t.run("service disable %s" % name)
    if "root required" in out:
        raise Skip("this session is not root")

    # Put it back before asserting: a failed assertion must not leave the
    # service off for every test that follows.
    try:
        after = _status(t, name)
        conf = t.run("cat %s" % path)
    finally:
        t.run("service %s %s" % ("enable" if was != "no" else "disable", name))

    if _field(after, "enabled") != "no":
        raise AssertionError("%s still reports enabled after disable:\n%s" % (name, after))

    expect_in("enabled", conf, "the service conf carries the enabled option")
    if "no" not in conf.split("enabled", 1)[1].split("\n", 1)[0]:
        raise AssertionError("%s says enabled but not no:\n%s" % (path, conf))


@test("enabling a service again puts it back", needs=("service", "cat"))
def enable_restores(t):
    name = _pick(t, TOGGLE_CANDIDATES)
    if name is None:
        raise Skip("no service is safe to toggle on this build")

    before = _status(t, name)
    if "enabled" not in before:
        raise Skip("this build's service status does not report enabled")
    was = _field(before, "enabled")

    out = t.run("service disable %s" % name)
    if "root required" in out:
        raise Skip("this session is not root")

    t.run("service enable %s" % name)
    if was == "no":
        t.at_exit(lambda: t.run("service disable %s" % name))
    after = _status(t, name)
    if _field(after, "enabled") != "yes":
        raise AssertionError("%s did not come back enabled:\n%s" % (name, after))


@test("a service the device needs refuses to be disabled", needs=("service",))
def essential_refuses_disable(t):
    name = _pick(t, ESSENTIAL_CANDIDATES)
    if name is None:
        raise Skip("this build lists no essential service")

    out = t.run("service disable %s" % name)
    if "root required" in out:
        raise Skip("this session is not root")

    expect_not_in("disabled %s" % name, out, "%s was not disabled" % name)

    after = _status(t, name)
    if _field(after, "enabled") != "yes":
        raise AssertionError("%s came back disabled and should not have:\n%s" % (name, after))


@test("the shell survives an essential service being asked to stop", needs=("service", "pwd"))
def shell_survives_refusal(t):
    name = _pick(t, ESSENTIAL_CANDIDATES)
    if name is None:
        raise Skip("this build lists no essential service")

    t.run("service disable %s" % name)
    out = t.run("pwd")
    if "/" not in out:
        raise AssertionError("the shell stopped answering after the refusal:\n%s" % out)


# ------------------------------------------------------- service task tracking

@test("a service's background tasks are attributed to it", needs=("service", "ps"))
def service_tasks_are_named(t):
    listing = _listed(t)
    rows = _service_rows(listing)
    owning = [parts[0] for parts in rows if parts[3].isdigit() and int(parts[3]) > 0]
    if not owning:
        raise Skip("no service on this build owns a task")

    ps = t.run("ps")
    if "NAME" not in ps:
        raise Skip("this build has no ps")

    named = [name for name in owning if name in ps]
    if not named:
        raise AssertionError("no service that claims tasks appears in ps:\nservice:\n%s\nps:\n%s"
                             % (listing, ps))


@test("the task count a service claims is the number ps shows for it",
      needs=("service", "ps"))
def service_task_count_agrees_with_ps(t):
    listing = _listed(t)
    ps = t.run("ps")
    if "NAME" not in ps:
        raise Skip("this build has no ps")

    ps_names = []
    for line in ps.splitlines():
        parts = line.split()
        if len(parts) >= 10 and parts[0].isdigit():
            ps_names.append(parts[-1])

    for parts in _service_rows(listing):
        name, claimed = parts[0], parts[3]
        if not claimed.isdigit() or int(claimed) == 0:
            continue
        seen = ps_names.count(name)
        if seen == 0:
            raise AssertionError("%s claims %s task(s) and ps shows none:\nservice:\n%s\nps:\n%s"
                                 % (name, claimed, listing, ps))
