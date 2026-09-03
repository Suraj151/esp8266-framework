#!/usr/bin/env python3

"""
The generated filesystems, read through the shell.

/proc and /sys hold nothing on the flash: every node is rendered when it is
read. These mirror tests/host/system/test_vfs.cpp, but ask the questions a host
cannot answer — whether the rendering survives a real heap, a real radio and a
scheduler that is genuinely busy, and whether what a node says agrees with what
the command that owns the same data says.

Read-only throughout. The one write here is aimed at a node that must refuse
it, and is checked by the value not moving rather than by the shell reporting
an error.
"""

import re

from .registry import test, expect_in, expect_not_in, expect_any, Skip

IPV4 = re.compile(r"\b(\d{1,3}(?:\.\d{1,3}){3})\b")


def _names(text):
    """The entry names of a listing, whatever columns it carries."""
    found = []
    for line in text.splitlines():
        parts = line.split()
        if not parts:
            continue
        name = parts[-1].rstrip("/")
        if name and not name.startswith("/") and ":" not in name:
            found.append(name)
    return found


def _a_task(t):
    """A live task as (pid, name), or a skip when ps lists none."""
    for line in t.run("ps").splitlines():
        parts = line.split()
        if len(parts) >= 10 and parts[0].isdigit():
            return parts[0], parts[-1]

    raise Skip("ps listed no task to look up in /proc")


def _fields(text):
    """A rendered key/value node as a dict, split on the first colon."""
    out = {}
    for line in text.splitlines():
        if ":" in line:
            key, _, value = line.partition(":")
            out[key.strip()] = value.strip()
    return out


@test("the proc directory lists its nodes", needs=("ls",), mounts=("/proc",))
def proc_lists_nodes(t):
    out = t.run("ls /proc")

    for node in ("uptime", "version", "meminfo", "mounts", "stat"):
        expect_in(node, out, "/proc holds %s" % node)


@test("meminfo agrees with the heap ps reports", needs=("cat", "ps"), mounts=("/proc",))
def meminfo_agrees_with_ps(t):
    """
    Both read the same allocator, so they must not disagree by more than the
    shell's own churn between the two reads. A wide tolerance is deliberate:
    this is here to catch a node reading the wrong counter or the wrong unit,
    not to measure fragmentation.
    """
    out = t.run("cat /proc/meminfo")
    expect_in("MemFree", out, "meminfo names the free heap")
    expect_in("MemMaxBlock", out, "meminfo names the largest block")

    free = None
    for line in out.splitlines():
        if line.startswith("MemFree"):
            digits = [int(part) for part in line.split() if part.isdigit()]
            free = digits[0] if digits else None
    if free is None:
        raise AssertionError("MemFree carried no number:\n%s" % out)

    reported = None
    for line in t.run("ps").splitlines():
        # the summary line also carries an uptime and a task count, so the
        # figure is taken by the unit that follows it rather than by position
        if "free heap" in line:
            parts = line.split()
            for index, part in enumerate(parts):
                if part == "bytes" and index and parts[index - 1].isdigit():
                    reported = int(parts[index - 1])
    if reported is None:
        raise Skip("ps does not report the free heap on this target")

    if abs(free - reported) > max(4096, reported // 4):
        raise AssertionError(
            "meminfo says %d bytes free, ps says %d" % (free, reported))


@test("mounts lists the same filesystems the mount command does",
      needs=("cat", "mount"), mounts=("/proc",))
def mounts_agrees_with_mount(t):
    rendered = t.run("cat /proc/mounts")
    listed = t.run("mount")

    prefixes = []
    for line in rendered.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1].startswith("/"):
            prefixes.append(parts[1])

    if not prefixes:
        raise AssertionError("/proc/mounts named no mount points:\n%s" % rendered)

    for prefix in prefixes:
        expect_in(prefix, listed, "the mount command knows %s" % prefix)

    if "/proc" not in prefixes:
        raise AssertionError("/proc/mounts left out /proc itself:\n%s" % rendered)


@test("stat carries the scheduler counters", needs=("cat",), mounts=("/proc",))
def stat_carries_counters(t):
    out = t.run("cat /proc/stat")

    for key in ("cpu", "ctxt", "processes", "procs_running", "btime"):
        expect_in(key, out, "/proc/stat carries %s" % key)

    for line in out.splitlines():
        if line.startswith("cpu "):
            parts = line.split()
            if len(parts) < 5:
                raise AssertionError("the cpu line is short:\n%s" % line)
            busy, idle = int(parts[3]), int(parts[4])
            if busy == 0 and idle == 0:
                raise AssertionError("a running board reported no cpu time:\n%s" % line)
            return

    raise AssertionError("/proc/stat had no cpu line:\n%s" % out)


@test("stat counts the tasks ps lists", needs=("cat", "ps"), mounts=("/proc",))
def stat_counts_agree_with_ps(t):
    counted = 0
    for line in t.run("ps").splitlines():
        parts = line.split()
        if len(parts) >= 10 and parts[0].isdigit():
            counted += 1
    if counted == 0:
        raise Skip("ps listed no tasks to count")

    processes = None
    for line in t.run("cat /proc/stat").splitlines():
        if line.startswith("processes"):
            digits = [int(part) for part in line.split() if part.isdigit()]
            processes = digits[0] if digits else None

    if processes is None:
        raise AssertionError("/proc/stat carried no process count")

    # a task can start or be reaped between the two reads, so the counts are
    # required to be close rather than equal
    if abs(processes - counted) > 2:
        raise AssertionError(
            "/proc/stat counts %d tasks, ps lists %d" % (processes, counted))


@test("uptime advances", needs=("cat", "ping"), mounts=("/proc",), slow=True)
def proc_uptime_advances(t):
    def seconds(text):
        parts = text.split()
        return float(parts[0]) if parts and parts[0].replace(".", "").isdigit() else None

    first = seconds(t.run("cat /proc/uptime"))
    if first is None:
        raise AssertionError("/proc/uptime did not lead with a number")

    t.run("ping 192.0.2.1 2", timeout=40)

    second = seconds(t.run("cat /proc/uptime"))
    if second is None or second <= first:
        raise AssertionError("/proc/uptime did not advance: %s then %s" % (first, second))


@test("version names the release", needs=("cat",), mounts=("/proc",))
def proc_version(t):
    expect_in("PDI Stack version", t.run("cat /proc/version"), "/proc/version")


@test("a running task has a directory of its own", needs=("ls", "ps"), mounts=("/proc",))
def task_has_a_directory(t):
    pid, _ = _a_task(t)
    out = t.run("ls /proc/%s" % pid)

    for leaf in ("stat", "cmdline", "status"):
        expect_in(leaf, out, "/proc/%s holds %s" % (pid, leaf))


@test("a task's status agrees with ps", needs=("cat", "ps"), mounts=("/proc",))
def task_status_agrees_with_ps(t):
    pid, name = _a_task(t)
    fields = _fields(t.run("cat /proc/%s/status" % pid))

    if fields.get("Pid") != pid:
        raise AssertionError(
            "/proc/%s/status reports pid %r" % (pid, fields.get("Pid")))
    if fields.get("Name") != name:
        raise AssertionError(
            "ps calls the task %r, status calls it %r" % (name, fields.get("Name")))


@test("a task's stat leads with the pid and the name", needs=("cat", "ps"), mounts=("/proc",))
def task_stat_leads_with_pid(t):
    pid, name = _a_task(t)
    out = t.run("cat /proc/%s/stat" % pid).strip()

    body = None
    for line in out.splitlines():
        if line.split() and line.split()[0] == pid:
            body = line
    if body is None:
        raise AssertionError("/proc/%s/stat did not lead with the pid:\n%s" % (pid, out))

    expect_in("(%s)" % name, body, "the stat line carries the task name")


@test("cmdline names the task", needs=("cat", "ps"), mounts=("/proc",))
def task_cmdline(t):
    pid, name = _a_task(t)
    expect_in(name, t.run("cat /proc/%s/cmdline" % pid), "/proc/%s/cmdline" % pid)


@test("a pid that never ran has no directory", needs=("ls",), mounts=("/proc",))
def absent_pid(t):
    out = t.run("ls /proc/60000")
    expect_not_in("stat", out, "a pid that is not there listed a stat node")


@test("a proc node refuses to be written", needs=("cat", "echo"), mounts=("/proc",))
def proc_is_read_only(t):
    """
    Judged by the value not moving. The shell does not yet report a failed
    write through a redirect (defect BD), so the node refusing the write is
    what is provable here, and it is the part that matters.
    """
    before = t.run("cat /proc/version").strip()
    t.run("echo overwritten > /proc/version")
    after = t.run("cat /proc/version").strip()

    if before != after:
        raise AssertionError("/proc/version took a write:\n%s\nbecame\n%s" % (before, after))


@test("the net directory holds route and dev", needs=("ls",), mounts=("/proc",))
def proc_net_lists(t):
    out = t.run("ls /proc/net")
    if not out.strip():
        raise Skip("this build has no /proc/net")

    expect_in("route", out, "/proc/net holds route")
    expect_in("dev", out, "/proc/net holds dev")


@test("route names the interface the board is really on", needs=("cat",), mounts=("/proc",))
def proc_net_route(t):
    out = t.run("cat /proc/net/route")
    if "Iface" not in out:
        raise Skip("this build has no /proc/net/route")

    rows = [line.split() for line in out.splitlines()
            if line.strip() and "Iface" not in line]
    rows = [parts for parts in rows if len(parts) >= 5]
    if not rows:
        raise Skip("no interface is up to route through")

    onlink = [parts for parts in rows if parts[4] == "U"]
    if not onlink:
        raise AssertionError("no interface offered an on-link route:\n%s" % out)

    # an on-link row reaches its network directly, so it names no gateway and
    # its destination is the interface address masked by the netmask
    for parts in onlink:
        if parts[2] != "0.0.0.0":
            raise AssertionError("an on-link route named a gateway:\n%s" % out)
        if parts[3] == "0.0.0.0":
            raise AssertionError("an on-link route carried no netmask:\n%s" % out)

        ip = t.run("cat /sys/class/net/%s/ip" % parts[0]).strip()
        mask = t.run("cat /sys/class/net/%s/netmask" % parts[0]).strip()
        if IPV4.match(ip) and IPV4.match(mask):
            expected = ".".join(str(int(a) & int(b))
                                for a, b in zip(ip.split("."), mask.split(".")))
            if parts[1] != expected:
                raise AssertionError(
                    "%s routes %s but its address masks to %s:\n%s"
                    % (parts[0], parts[1], expected, out))

    # a default route is the one row that names a gateway, and only an
    # interface that has one may carry it
    for parts in [p for p in rows if p[4] == "UG"]:
        if parts[1] != "0.0.0.0" or parts[3] != "0.0.0.0":
            raise AssertionError("a default route was not 0.0.0.0/0:\n%s" % out)
        if parts[2] == "0.0.0.0":
            raise AssertionError("a default route named no gateway:\n%s" % out)


@test("dev lists only interfaces that can count", needs=("cat",), mounts=("/proc",))
def proc_net_dev(t):
    out = t.run("cat /proc/net/dev")
    if "Iface" not in out:
        raise Skip("this build has no /proc/net/dev")

    for line in out.splitlines():
        if not line.strip() or "Iface" in line:
            continue
        parts = line.split()
        if len(parts) != 7:
            raise AssertionError("a dev row is not seven columns:\n%s" % line)


@test("the network class lists the radio", needs=("ls",), mounts=("/sys",))
def sys_class_net_lists(t):
    out = t.run("ls /sys/class/net")
    if not out.strip():
        raise Skip("this build has no /sys/class/net")

    expect_any(("wlan0", "ap0"), out, "/sys/class/net names an interface")


@test("an interface directory holds its attributes", needs=("ls",), mounts=("/sys",))
def sys_net_interface_dir(t):
    listing = t.run("ls /sys/class/net")
    if "wlan0" not in listing:
        raise Skip("this target has no station interface")

    out = t.run("ls /sys/class/net/wlan0")
    for leaf in ("address", "operstate", "ip", "netmask", "gateway"):
        expect_in(leaf, out, "wlan0 holds %s" % leaf)


@test("the station address agrees with the shell", needs=("cat",), mounts=("/sys",))
def sys_net_ip_agrees(t):
    state = t.run("cat /sys/class/net/wlan0/operstate").strip()
    if "up" not in state:
        raise Skip("the station interface is not up")

    address = t.run("cat /sys/class/net/wlan0/ip").strip()
    found = [item for item in IPV4.findall(address) if item != "0.0.0.0"]
    if not found:
        raise Skip("the station interface holds no address")

    expect_in(found[0], t.run("net ip"), "the shell reports the same station address")


@test("the station reports a mac and an association", needs=("cat",), mounts=("/sys",))
def sys_net_mac_and_ssid(t):
    mac = t.run("cat /sys/class/net/wlan0/address").strip()
    if mac.count(":") != 5:
        raise AssertionError("wlan0 reported no mac address:\n%s" % mac)

    state = t.run("cat /sys/class/net/wlan0/operstate").strip()
    if "up" not in state:
        raise Skip("the station interface is not up")

    ssid = t.run("cat /sys/class/net/wlan0/ssid").strip()
    if not ssid:
        raise AssertionError("an associated station reported no ssid")


@test("an access point has no association to report", needs=("ls",), mounts=("/sys",))
def sys_net_ap_stops_early(t):
    listing = t.run("ls /sys/class/net")
    if "ap0" not in listing:
        raise Skip("this target has no access point interface")

    out = t.run("ls /sys/class/net/ap0")
    expect_in("address", out, "ap0 holds address")
    expect_not_in("ssid", out, "an access point listed an ssid")
    expect_not_in("rssi", out, "an access point listed an rssi")


@test("a network attribute refuses to be written", needs=("cat", "echo"), mounts=("/sys",))
def sys_net_is_read_only(t):
    listing = t.run("ls /sys/class/net")
    if "wlan0" not in listing:
        raise Skip("this target has no station interface")

    before = t.run("cat /sys/class/net/wlan0/ip").strip()
    t.run("echo 10.0.0.1 > /sys/class/net/wlan0/ip")
    after = t.run("cat /sys/class/net/wlan0/ip").strip()

    if before != after:
        raise AssertionError(
            "a network attribute took a write: %s became %s" % (before, after))
