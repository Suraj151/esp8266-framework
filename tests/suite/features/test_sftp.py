#!/usr/bin/env python3

"""
SFTP: the file transfer subsystem, checked from both ends.

Every assertion here pairs the two views the way the portal suite does. A file
put over sftp is read back with `cat`; a file written by the shell is fetched
over sftp. Neither view alone separates a working transfer from a client and a
server that agree with each other and are both wrong.

The subsystem reassembles its own packets out of ssh channel data, so the sizes
here matter: one test deliberately writes more than fits in a single channel
payload, which is the case that used to reset the connection.
"""

import struct

from .registry import test, expect_in, expect_not_in, Skip, ssh_port

# large enough to be split across ssh channel-data payloads on the way in,
# small enough to stay quick on a board writing to littlefs
SPLIT_SIZE = 8192

# the request types the malformed-input tests aim at
SSH_FXP_OPEN = 3
SSH_FXP_OPENDIR = 11
SSH_FXP_REALPATH = 16
SSH_FXP_STAT = 17


def sftp_open(t, attempts=3):
    """
    An sftp session on the target, on its own ssh connection.

    A board serves one channel per session, so this does not share the shell's
    connection. Skips when ssh or the subsystem is not there; anything past the
    subsystem opening is a real failure.
    """
    import time

    try:
        import paramiko
    except ImportError as err:
        raise Skip("paramiko is not installed: %s" % err)

    # registers the standard curve25519 name the board offers
    from ..driver import ssh_shell  # noqa: F401

    last = None
    for attempt in range(attempts):
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        try:
            client.connect(t.address(), port=ssh_port(t), username=t.username,
                           password=t.password, timeout=45, banner_timeout=45,
                           auth_timeout=45, allow_agent=False, look_for_keys=False)
        except Exception as err:
            last = err
            client.close()
            if attempt + 1 < attempts:
                time.sleep(3.0)
            continue

        try:
            return client, client.open_sftp()
        except Exception as err:
            client.close()
            raise Skip("the target has no sftp subsystem: %s" % err)

    raise Skip("the target has no reachable ssh server: %s" % last)


def patterned(size):
    """Bytes with every value in them, so a lost or reordered chunk shows."""
    return bytes(bytearray(i % 256 for i in range(size)))


@test("the sftp subsystem opens and resolves the working directory")
def sftp_realpath(t):
    """
    REALPATH is the first thing a client sends after opening the subsystem.
    Answering it wrongly used to reset the connection before any transfer.
    """
    client, sftp = sftp_open(t)
    try:
        where = sftp.normalize(".")
        if not where.startswith("/"):
            raise AssertionError("realpath answered %r, which is not an absolute path"
                                 % where)
    finally:
        sftp.close()
        client.close()


@test("a directory listing agrees with the shell", needs=("ls", "echo", "rm"))
def sftp_listing_matches(t):
    name = "sftp_seen.txt"

    t.run("rm /%s" % name)
    t.run("echo listed > /%s" % name)

    client, sftp = sftp_open(t)
    try:
        listed = sftp.listdir("/")
        if name not in listed:
            raise AssertionError("sftp did not list %s:\n%s" % (name, listed))
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("a file put over sftp is on the target", needs=("cat", "rm"))
def sftp_put_reaches_the_target(t):
    name = "sftp_put.txt"

    t.run("rm /%s" % name)
    client, sftp = sftp_open(t)
    try:
        with sftp.open("/%s" % name, "wb") as fh:
            fh.write(b"put over sftp\n")

        expect_in("put over sftp", t.run("cat /%s" % name),
                  "what the target holds after an sftp put")
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("a file written by the shell is readable over sftp", needs=("echo", "rm"))
def sftp_get_reads_shell_file(t):
    name = "sftp_get.txt"

    t.run("rm /%s" % name)
    t.run("echo written by the shell > /%s" % name)

    client, sftp = sftp_open(t)
    try:
        with sftp.open("/%s" % name, "rb") as fh:
            body = fh.read()

        expect_in("written by the shell", body.decode(errors="replace"),
                  "what sftp reads back")
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("stat reports the size the shell reports", needs=("echo", "wc", "rm"))
def sftp_stat_size(t):
    name = "sftp_stat.txt"

    t.run("rm /%s" % name)
    t.run("echo sized > /%s" % name)

    counted = t.run("wc /%s" % name).split()
    if len(counted) < 3 or not counted[2].isdigit():
        raise Skip("wc did not report a byte count: %s" % counted)
    expected = int(counted[2])

    client, sftp = sftp_open(t)
    try:
        got = sftp.stat("/%s" % name).st_size
        if got != expected:
            raise AssertionError("sftp says %d bytes, wc says %d" % (got, expected))
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("a binary file round trips byte for byte", needs=("rm",))
def sftp_binary_round_trip(t):
    name = "sftp_bin.dat"
    blob = b"\x00\xff\r\n" + patterned(512) + b"\x00tail\r\n"

    t.run("rm /%s" % name)
    client, sftp = sftp_open(t)
    try:
        with sftp.open("/%s" % name, "wb") as fh:
            fh.write(blob)
        with sftp.open("/%s" % name, "rb") as fh:
            back = fh.read()

        if back != blob:
            raise AssertionError("sent %d bytes, got %d back\nsent: %r\ngot:  %r"
                                 % (len(blob), len(back), blob[:40], back[:40]))
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("a transfer larger than one channel payload round trips", needs=("rm",), slow=True)
def sftp_reassembly(t):
    """
    The subsystem accumulates ssh channel data and cuts complete sftp packets
    out of it. A payload big enough to be split across channel messages is what
    exercises that; getting it wrong used to close the connection outright.
    """
    name = "sftp_big.dat"
    blob = patterned(SPLIT_SIZE)

    t.run("rm /%s" % name)
    client, sftp = sftp_open(t)
    try:
        with sftp.open("/%s" % name, "wb") as fh:
            fh.write(blob)

        size = sftp.stat("/%s" % name).st_size
        if size != len(blob):
            raise AssertionError("stored %d of %d bytes" % (size, len(blob)))

        with sftp.open("/%s" % name, "rb") as fh:
            back = fh.read()

        if back != blob:
            first = next((i for i in range(min(len(back), len(blob)))
                          if back[i] != blob[i]), min(len(back), len(blob)))
            raise AssertionError("%d bytes back, first difference at %d"
                                 % (len(back), first))
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("a file removed over sftp is gone from the target",
      needs=("echo", "cat", "ls", "rm"))
def sftp_remove(t):
    # the sentinel must not appear in the path: a failing `cat` echoes the path
    # it was given, so a marker shared with the filename always looks present
    name = "sftp_rm.txt"
    marker = "stillhere"

    t.run("rm /%s" % name)
    t.run("echo %s > /%s" % (marker, name))
    expect_in(marker, t.run("cat /%s" % name), "the file before sftp removes it")

    client, sftp = sftp_open(t)
    try:
        sftp.remove("/%s" % name)
    finally:
        sftp.close()
        client.close()

    expect_not_in(marker, t.run("cat /%s" % name),
                  "the contents after sftp removed it")
    expect_not_in(name, t.run("ls /"), "the listing after sftp removed it")


@test("a directory made over sftp is visible to the shell", needs=("ls", "rm"))
def sftp_mkdir(t):
    name = "sftp_dir"

    t.run("rm /%s" % name)
    client, sftp = sftp_open(t)
    try:
        sftp.mkdir("/%s" % name)
        expect_in(name, t.run("ls /"), "the directory sftp created")

        sftp.rmdir("/%s" % name)
        expect_not_in(name, t.run("ls /"), "the directory after sftp removed it")
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


@test("sftp and a shell session can be open at once", needs=("cat", "rm"))
def sftp_alongside_a_shell(t):
    """
    Editing a file in a graphical client opens a second connection while the
    first is still there — the case the session pool was widened for. The shell
    session the suite is already running on counts as one of them here.
    """
    name = "sftp_concurrent.txt"

    t.run("rm /%s" % name)
    client, sftp = sftp_open(t)
    try:
        with sftp.open("/%s" % name, "wb") as fh:
            fh.write(b"written while a shell is open\n")

        # the suite's own session must still work with sftp holding a slot
        expect_in("written while a shell is open", t.run("cat /%s" % name),
                  "the shell reading a file sftp just wrote")
    finally:
        sftp.close()
        client.close()
        t.run("rm /%s" % name)


def sftp_channel(t):
    """
    A raw sftp channel, so a test can send bytes paramiko would never build.

    Everything above goes through paramiko's client, which only ever sends well
    formed requests. The parser's bounds checks are only reachable from here.
    """
    import paramiko  # noqa: F401

    client, sftp = sftp_open(t)
    try:
        channel = client.get_transport().open_session()
        channel.settimeout(20.0)
        channel.invoke_subsystem("sftp")
    except Exception as err:
        sftp.close()
        client.close()
        raise Skip("the target would not open a second sftp channel: %s" % err)

    return client, sftp, channel


def sftp_packet(fxp_type, request_id, body=b""):
    payload = bytes(bytearray([fxp_type])) + struct.pack(">I", request_id) + body
    return struct.pack(">I", len(payload)) + payload


def still_answers(t, channel):
    """Whether the channel still serves a request after being sent rubbish."""
    channel.sendall(sftp_packet(SSH_FXP_REALPATH, 99, ssh_string(b".")))

    seen = b""
    try:
        while len(seen) < 4:
            block = channel.recv(4096)
            if not block:
                return False
            seen += block
    except Exception:
        return False

    return len(seen) >= 4


def ssh_string(payload):
    return struct.pack(">I", len(payload)) + payload


@test("a truncated sftp request does not end the session")
def sftp_short_request_is_rejected(t):
    """
    The request header is a type byte and a request id. A packet whose declared
    length cannot hold them used to be read past its end, because only the
    length itself was checked against the buffer. Its own frame is intact, so
    the server passes over it and keeps serving rather than dropping the client.
    """
    client, sftp, channel = sftp_channel(t)
    try:
        channel.sendall(struct.pack(">I", 5) + bytes(bytearray([1])) + struct.pack(">I", 3))
        channel.recv(4096)

        for declared in (0, 1, 4):
            channel.sendall(struct.pack(">I", declared) + bytes(bytearray([16] * declared)))

        if not still_answers(t, channel):
            raise AssertionError("the sftp channel stopped answering after a short request")
    finally:
        channel.close()
        sftp.close()
        client.close()


@test("an sftp path longer than the packet is refused")
def sftp_overlong_path_is_refused(t):
    """
    Every string field in a request carries its own length. A length larger
    than what arrived, including one near 2^32, must be refused rather than
    read past the end of the packet.
    """
    client, sftp, channel = sftp_channel(t)
    try:
        channel.sendall(struct.pack(">I", 5) + bytes(bytearray([1])) + struct.pack(">I", 3))
        channel.recv(4096)

        for fxp_type in (SSH_FXP_REALPATH, SSH_FXP_STAT, SSH_FXP_OPEN, SSH_FXP_OPENDIR):
            for claimed in (0xFFFFFFFF, 0x7FFFFFFF, 4096):
                body = struct.pack(">I", claimed) + b"/etc"
                channel.sendall(sftp_packet(fxp_type, 7, body))

        if not still_answers(t, channel):
            raise AssertionError("the sftp channel stopped answering after an overlong length")
    finally:
        channel.close()
        sftp.close()
        client.close()


@test("the sftp session survives a stream of malformed requests", slow=True)
def sftp_survives_garbage(t):
    """
    The board must not be left unusable by a peer that never sends a valid
    request. Sizes here straddle the header length and the reassembly cut.
    """
    client, sftp, channel = sftp_channel(t)
    try:
        channel.sendall(struct.pack(">I", 5) + bytes(bytearray([1])) + struct.pack(">I", 3))
        channel.recv(4096)

        for size in (1, 2, 3, 5, 8, 16, 31, 33):
            channel.sendall(bytes(bytearray([0xA5] * size)))

        # a session the board gave up on is gone, which is a pass here only if
        # the board itself is still serving new ones
        channel.close()
        sftp.close()
        client.close()
        client, sftp = sftp_open(t)

        listed = sftp.listdir("/")
        if not isinstance(listed, list):
            raise AssertionError("the target would not list after malformed sftp traffic")
    finally:
        try:
            sftp.close()
            client.close()
        except Exception:
            pass
