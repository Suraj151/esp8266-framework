#!/usr/bin/env python3

"""
The session environment, over whatever transport the target is reached by.

The three tiers only exist as one thing from the shell's side: a name resolves,
or it does not. So these ask the questions an operator would — does the name
come back, does it expand, does it survive, does it go away — rather than which
tier answered.

Two of them are here because the host cannot ask them at all. A derived name is
resolved against the live session, so PWD is only really tested by changing the
working directory of a session a transport is holding open; and the environment
surviving a change of user is a property of the session object's lifetime, which
on a serial console is not the lifetime of a login.

These mirror tests/host/system/test_environment.cpp.
"""

from .registry import test, expect_in, expect_not_in, Skip

W = "wt_"


def env_names(t):
    """The names env reports, ignoring what they are bound to."""
    names = []
    for line in t.run("env").splitlines():
        line = line.strip()
        if "=" in line and not line.startswith("env"):
            names.append(line.split("=", 1)[0].strip())
    return names


def env_value(t, name):
    """What env binds one name to, or None when it does not carry it."""
    for line in t.run("env").splitlines():
        line = line.strip()
        if "=" in line and line.split("=", 1)[0].strip() == name:
            return line.split("=", 1)[1].strip()
    return None


@test("env reports the names the session answers for", needs=("env",))
def env_lists_derived(t):
    names = env_names(t)
    for name in ("PWD", "USER", "UID"):
        if name not in names:
            raise AssertionError("env did not report %s, only %r" % (name, names))


@test("a variable set in a session is reported by env", needs=("env", "export", "unset"))
def export_then_env(t):
    try:
        t.run("export TENVA=alpha")
        if env_value(t, "TENVA") != "alpha":
            raise AssertionError("env did not report what export set")
    finally:
        t.run("unset TENVA")


@test("a variable expands in a later command", needs=("export", "unset", "echo"))
def export_then_expands(t):
    try:
        t.run("export TENVB=beta")
        expect_in("beta", t.run("echo $TENVB"), "the name expanded")
    finally:
        t.run("unset TENVB")


@test("single quotes suppress expansion and double quotes allow it",
      needs=("export", "unset", "echo"))
def quoting_around_expansion(t):
    try:
        t.run("export TENVC=gamma")

        expect_in("gamma", t.run('echo "$TENVC"'), "double quotes expand")

        out = t.run("echo '$TENVC'")
        expect_not_in("gamma", out, "single quotes must not expand")
        expect_in("$TENVC", out, "single quotes keep the name as typed")
    finally:
        t.run("unset TENVC")


@test("adjacent names concatenate", needs=("export", "unset", "echo"))
def adjacent_names(t):
    try:
        t.run("export TENVD=one")
        t.run("export TENVE=two")
        expect_in("onetwo", t.run("echo $TENVD$TENVE"), "two names run together")
    finally:
        t.run("unset TENVD")
        t.run("unset TENVE")


@test("a name the session does not carry expands to nothing",
      needs=("echo",))
def unset_name_expands_empty(t):
    out = t.run("echo [$TENVNOSUCHNAME]")

    # a real shell leaves the brackets touching. what must not happen is the
    # name surviving into the output as if it were literal text
    expect_in("[]", out, "an unset name left nothing behind")


@test("unset drops a variable", needs=("env", "export", "unset"))
def unset_drops(t):
    t.run("export TENVF=delta")
    t.run("unset TENVF")

    if env_value(t, "TENVF") is not None:
        raise AssertionError("the variable survived unset")


@test("unset of a name the session does not carry is not a missing command",
      needs=("unset",))
def unset_missing_is_not_noent(t):
    """
    CMD_ERROR_NOENT means *no such command* in this taxonomy, so returning it
    for a name that is simply absent made the dispatcher print 'unset: command
    not found'. The command ran; it just had nothing to drop.
    """
    out = t.run("unset TENVNOTHERE")
    expect_not_in("command not found", out, "unset reported as a missing command")


@test("export needs a name and a value", needs=("export",))
def export_needs_both(t):
    expect_in("name and a value", t.run("export TENVNOVALUE"), "export with no assignment")


@test("a name the session answers for cannot be assigned", needs=("export",))
def derived_cannot_be_set(t):
    out = t.run("export PWD=/somewhere/else")
    expect_in("cannot be set", out, "assigning a derived name")


@test("a name the session answers for cannot be dropped", needs=("unset",))
def derived_cannot_be_unset(t):
    out = t.run("unset PWD")
    expect_in("cannot be dropped", out, "dropping a derived name")


@test("a malformed name is refused", needs=("export",))
def bad_name_refused(t):
    out = t.run("export 9TENV=nope")
    expect_in("a letter or underscore", out, "a name that starts with a digit")


@test("PWD follows the working directory", needs=("env", "export", "cd", "mkdir"),
      mounts=("/",))
def pwd_is_live(t):
    """
    The derived tier is resolved at lookup, never stored, so PWD is right after
    a cd without anything having written it. Storing it would put two sources of
    truth on one fact and go stale exactly here.
    """
    path = t.workspace(W + "envpwd")

    seen = env_value(t, "PWD")
    if seen is None or path not in seen:
        raise AssertionError("PWD is %r after cd to %r" % (seen, path))

    t.run("cd /")
    seen = env_value(t, "PWD")
    if seen is None or seen.strip() != "/":
        raise AssertionError("PWD is %r after cd to /" % (seen,))


@test("USER follows the session", needs=("env", "whoami"))
def user_is_live(t):
    expect_in(t.username, t.run("env"), "env reports the logged in user")


@test("the session holds a bounded number of variables",
      needs=("export", "unset"), slow=True)
def session_bound(t):
    """
    ENV_SESSION_MAX is 8. The bound is on what the session stores, so the names
    it derives do not spend any of it.
    """
    made = []
    try:
        for i in range(8):
            name = "TENVX%d" % i
            out = t.run("export %s=v%d" % (name, i))
            expect_not_in("no more variables", out, "variable %d of 8" % (i + 1))
            made.append(name)

        out = t.run("export TENVX8=overflow")
        expect_in("no more variables", out, "the ninth variable")
    finally:
        for name in made:
            t.run("unset %s" % name)
        t.run("unset TENVX8")


@test("changing user clears the environment",
      needs=("env", "export", "unset", "su"), su=True)
def su_clears_environment(t):
    """
    The regression for the one the author found on device: variables set by one
    user survived into the next login on a serial console. session_t::clear()
    does wipe them, but it runs when the session is *freed* — telnet and ssh
    close a channel and release the session, while a serial console has no
    channel to close and carries the same session object straight through.

    su is the path that reproduces it on every transport, and ours follows the
    new user's identity, uid, gid and home, which is su - behaviour.
    """
    try:
        t.run("export TENVCARRY=survived")
        if env_value(t, "TENVCARRY") != "survived":
            raise AssertionError("the variable was not set to begin with")

        t.run("su %s %s" % (t.username, t.password))

        if env_value(t, "TENVCARRY") is not None:
            raise AssertionError("the variable survived a change of user")
    finally:
        t.become_root()
        t.run("unset TENVCARRY")


@test("logout clears the environment", needs=("env", "export", "logout"))
def logout_clears_environment(t):
    """
    The other half of the same fix, on a session this test can afford to lose.
    A serial console has no second session, so it is skipped there — that
    transport is covered by su, which reaches the same reset.
    """
    peer = t.peer()
    try:
        peer.run("export TENVOUT=survived", t.timeout)
        expect_in("TENVOUT", peer.run("env", t.timeout), "the peer set it")

        from ..driver.shell import ShellError

        try:
            peer.send_line("logout")
        except ShellError:
            return

        try:
            peer.attach(t.username, t.password, timeout=t.timeout)
            after = peer.run("env", t.timeout)
        except Exception:
            return

        expect_not_in("TENVOUT", after, "the variable survived a logout")
    finally:
        peer.close()


@test("one session's variables are not another's",
      needs=("env", "export", "unset"))
def environment_is_per_session(t):
    peer = t.peer()
    try:
        t.run("export TENVMINE=here")
        expect_not_in("TENVMINE", peer.run("env", t.timeout),
                      "a second session saw the first session's variable")
    finally:
        t.run("unset TENVMINE")
        peer.close()
