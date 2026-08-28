#!/usr/bin/env python3

"""
The shell grammar itself, over whatever transport the target is reached by.

Redirection and pipelines belong to the shell, not to any one command: the
command being redirected does not know it is being redirected, and the command
downstream of a pipe does not know its input was not a file. These check that
on a real board, where the output an operator sees is the only evidence.

These mirror tests/host/system/test_shell_redirect.cpp.
"""

import re

from .registry import test, expect_in, expect_not_in, Skip

W = "wt_"

FREE_HEAP = re.compile(r"(\d+)\s+bytes free heap")


def counts(text):
    """The three numbers wc prints, or None when it printed something else."""
    for line in text.splitlines():
        found = re.findall(r"\d+", line)
        if len(found) >= 3:
            return [int(n) for n in found[:3]]
    return None


def free_heap(t):
    """What the target says it has left, or a skip when it does not say."""
    found = FREE_HEAP.search(t.run("ps"))
    if found is None:
        raise Skip("this target does not report free heap")
    return int(found.group(1))


@test("a command with no redirect handling of its own is captured", needs=("pwd", "cat"))
def redirect_captures_any_command(t):
    t.workspace(W + "shredir")

    # pwd contains no redirect code at all; the shell supplies every bit of it
    out = t.run("pwd > where.txt")
    expect_not_in("shredir", out, "the terminal saw the output")

    expect_in("shredir", t.run("cat where.txt"), "the file holds it instead")


@test("a redirected file ends its lines the way a file should", needs=("echo", "wc"))
def redirect_line_ending(t):
    t.workspace(W + "shend")
    t.run("echo abc > ended.txt")

    # three characters and one terminator. a carriage return is what a terminal
    # needs, not what a file holds, so a fourth byte here and no more
    expect_in("4", t.run("wc ended.txt"), "byte count of a redirected line")


@test("the terminal is given back after a redirect", needs=("echo", "cat"))
def redirect_hands_back(t):
    t.workspace(W + "shback")
    t.run("echo away > gone.txt")

    expect_in("still here", t.run("echo still here"), "echo after a redirect")


@test("a redirect to an unwritable path is refused", needs=("echo",))
def redirect_bad_target(t):
    t.workspace(W + "shbad")

    out = t.run("echo hi > /nosuchmount/nope.txt")
    expect_not_in("hi", out, "nothing was printed as if it had worked")


@test("a dangling redirect is a syntax error", needs=("echo",))
def redirect_dangling(t):
    expect_in("syntax error", t.run("echo hi >"), "a target that was never named")


@test("output of one command becomes input of the next", needs=("echo", "wc"))
def pipeline_two_stages(t):
    t.workspace(W + "shpipe")

    # wc counts what echo produced, not a file it was handed
    out = t.run("echo hello world | wc")
    expect_in("1", out, "line count through a pipe")
    expect_in("2", out, "word count through a pipe")


@test("a pipeline carries through three stages", needs=("echo", "grep", "wc"))
def pipeline_three_stages(t):
    t.workspace(W + "shpipe3")

    expect_in("1", t.run("echo alpha | grep alpha | wc"), "count after a filter")


@test("grep filters what it is piped", needs=("echo", "grep"))
def pipeline_grep_filters(t):
    t.workspace(W + "shgrep")

    expect_in("needle", t.run("echo needle | grep needle"), "a match through a pipe")
    expect_not_in("needle", t.run("echo needle | grep absent"), "a miss through a pipe")


@test("a file can be piped into a filter", needs=("echo", "cat", "grep"))
def pipeline_from_a_file(t):
    t.workspace(W + "shcatpipe")
    t.run("echo findme here > src.txt")

    expect_in("findme", t.run("cat src.txt | grep findme"), "a file through a pipe")


@test("the last stage of a pipeline can still be redirected", needs=("echo", "wc", "cat"))
def pipeline_then_redirect(t):
    t.workspace(W + "shboth")

    out = t.run("echo hello world | wc > counted.txt")
    expect_not_in("hello world", out, "the terminal saw the payload")

    expect_in("2", t.run("cat counted.txt"), "the count landed in the file")


@test("an empty pipeline stage is a syntax error", needs=("echo", "wc"))
def pipeline_empty_stage(t):
    expect_in("syntax error", t.run("echo hi |"), "a pipe with nothing after it")


@test("cat passes its input straight through", needs=("echo", "cat"))
def pipeline_cat_passthrough(t):
    t.workspace(W + "shcat")

    expect_in("through", t.run("echo through | cat"), "cat as a filter")


@test("head stops at the count it was piped", needs=("echo", "cat", "head"))
def pipeline_head_count(t):
    path = t.workspace(W + "shhead") + "/lines.txt"
    t.run("echo one > %s" % path)
    t.run("echo two >> %s" % path)
    t.run("echo three >> %s" % path)

    # with a pipe there is no file to name, so the lone argument is the count
    out = t.run("cat %s | head 2" % path)
    expect_in("one", out, "the first line")
    expect_not_in("three", out, "head stopped before the last line")


@test("tail keeps the last lines it was piped", needs=("echo", "cat", "tail"))
def pipeline_tail_count(t):
    path = t.workspace(W + "shtail") + "/lines.txt"
    t.run("echo one > %s" % path)
    t.run("echo two >> %s" % path)
    t.run("echo three >> %s" % path)

    out = t.run("cat %s | tail 2" % path)
    expect_in("three", out, "the last line")
    expect_not_in("one", out, "tail started after the first line")


@test("a file argument still wins when nothing is piped", needs=("echo", "head"))
def head_file_argument_unchanged(t):
    path = t.workspace(W + "shfarg") + "/lines.txt"
    t.run("echo one > %s" % path)
    t.run("echo two >> %s" % path)
    t.run("echo three >> %s" % path)

    # no pipe, so the first positional is the path it has always been
    out = t.run("head %s 1" % path)
    expect_in("one", out, "the first line")
    expect_not_in("three", out, "head stopped before the last line")


@test("a command given neither a file nor a pipe is unaffected", needs=("wc",))
def pipeline_no_input(t):
    t.workspace(W + "shnoin")

    # wc still wants an argument when nothing has claimed its input
    expect_not_in("0 0 0", t.run("wc"), "wc invented a count from nowhere")


@test("an appending redirect keeps what was already there", needs=("echo", "cat", "wc"))
def redirect_append_through_the_shell(t):
    t.workspace(W + "shapp")

    t.run("echo first > note.txt")
    t.run("echo second >> note.txt")

    out = t.run("cat note.txt")
    expect_in("first", out, "the original line survived")
    expect_in("second", out, "the appended line is there")

    got = counts(t.run("wc note.txt"))
    if got is not None and got[0] < 2:
        raise AssertionError("an append left %d line(s), not 2" % got[0])


@test("output larger than the write buffer keeps every byte", needs=("help", "wc", "cat"))
def redirect_spans_the_write_buffer(t):
    """
    A file stream gathers bytes and commits a block at a time, so anything
    longer than one buffer exercises the boundary between blocks. `help` lists
    every registered command, which is far more than one buffer's worth.
    """
    t.workspace(W + "shbig")

    printed = t.run("help", timeout=max(t.timeout, 30))
    t.run("help > big.txt", timeout=max(t.timeout, 30))

    got = counts(t.run("wc big.txt", timeout=max(t.timeout, 30)))
    if got is None:
        raise Skip("wc did not report a count for the redirected file")

    # every command name that reached the terminal must be in the file too
    body = t.run("cat big.txt", timeout=max(t.timeout, 30))
    for token in ("help", "echo", "cat"):
        expect_in(token, body, "%s survived a multi-block redirect" % token)

    if got[2] < 200:
        raise AssertionError("a multi-block redirect wrote only %d bytes:\n%s"
                             % (got[2], printed[:200]))


@test("a pipeline bigger than the pipe does not wedge the shell", needs=("help", "wc"))
def pipeline_beyond_capacity_is_survivable(t):
    """
    A pipe is fixed capacity, as a linux one is, so a stage can outrun it. What
    must never happen is the shell being left unusable by it.
    """
    out = t.run("help | wc", timeout=max(t.timeout, 30))

    if counts(out) is None:
        raise AssertionError("a large pipeline printed no counts:\n%s" % out[:200])

    expect_in("/", t.run("pwd"), "the shell still answers after a large pipeline")


@test("two sessions can redirect at the same time", needs=("echo", "cat"))
def redirect_is_per_session(t):
    """
    Descriptors belong to a session, not to the board. Two sessions redirecting
    at once must not land in each other's file or each other's terminal.
    """
    peer = t.peer()
    try:
        t.run("echo fromfirst > /shared_a.txt")
        peer.run("echo fromsecond > /shared_b.txt", t.timeout)

        expect_in("fromfirst", t.run("cat /shared_a.txt"), "the first session's file")
        expect_not_in("fromsecond", t.run("cat /shared_a.txt"), "the other session's text")

        expect_in("fromsecond", peer.run("cat /shared_b.txt", t.timeout),
                  "the second session's file")
    finally:
        peer.close()
        t.run("rm /shared_a.txt")
        t.run("rm /shared_b.txt")


@test("redirecting many times does not leak", needs=("echo", "ps", "rm"), slow=True)
def redirect_releases_everything(t):
    """
    Every redirect allocates a descriptor table, an adapter and a file stream,
    and must give all three back. A leak shows as heap that never returns.
    """
    t.workspace(W + "shleak")

    # settle first: the first redirect of a session allocates what later ones reuse
    for _ in range(3):
        t.run("echo settle > leak.txt")

    before = free_heap(t)

    for _ in range(20):
        t.run("echo repeated > leak.txt")

    after = free_heap(t)

    # a per-redirect leak of even 64 bytes would show as 1280 across 20 rounds
    if before - after > 512:
        raise AssertionError("heap fell %d bytes over 20 redirects (%d -> %d)"
                             % (before - after, before, after))


@test("piping many times does not leak", needs=("echo", "wc", "ps"), slow=True)
def pipeline_releases_everything(t):
    """A pipe is allocated per stage and freed once the next stage has read it."""
    for _ in range(3):
        t.run("echo settle | wc")

    before = free_heap(t)

    for _ in range(15):
        t.run("echo repeated here | wc")

    after = free_heap(t)

    if before - after > 512:
        raise AssertionError("heap fell %d bytes over 15 pipelines (%d -> %d)"
                             % (before - after, before, after))


@test("a redirect works on a memory filesystem too", needs=("echo", "cat"), mounts=("/tmp",))
def redirect_into_tmpfs(t):
    t.run("echo intmpfs > /tmp/redir.txt")

    expect_in("intmpfs", t.run("cat /tmp/redir.txt"), "a redirect into tmpfs")
    t.run("rm /tmp/redir.txt")


@test("a redirect operator typed into a waiting command is not a redirect",
      needs=("fedit", "cat", "rm"), mounts=("/",), slow=True)
def operator_inside_an_interactive_command(t):
    """
    The shell parses a line only when it is starting a command. A command that
    is already waiting for input gets the line raw, so an editor can hold a '>'
    without the shell stealing it into a file.
    """
    from ..driver.shell import PROMPT

    path = "/wt_shedit.txt"
    body = "a > b | c"
    t.run("rm %s" % path)

    t.shell.send_line("fedit %s" % path)
    t.shell.expect("ESC", t.timeout)
    t.shell.drain(0.5)

    t.shell.send_raw(body)
    t.shell.drain(0.5)
    t.shell.send_raw("\r")
    t.shell.drain(0.5)

    t.shell.send_raw("\x03")
    t.shell.expect("save", t.timeout)
    t.shell.send_line("!w")
    t.shell.expect("saved", t.timeout)
    t.shell.expect(PROMPT, t.timeout)

    try:
        # the operators are text the editor held, not a redirect the shell took
        expect_in(">", t.run("cat %s" % path), "the editor kept the operator")
        expect_in("|", t.run("cat %s" % path), "the editor kept the pipe")
    finally:
        t.run("rm %s" % path)


@test("a command reads the file it is given as a source", needs=("echo", "wc"))
def source_feeds_a_command(t):
    path = t.workspace(W + "shsrc") + "/in.txt"
    t.run("echo one > %s" % path)
    t.run("echo two >> %s" % path)

    got = counts(t.run("wc < %s" % path))
    if got is None:
        raise AssertionError("wc printed no counts when given a source")
    if got[0] != 2:
        raise AssertionError("wc counted %d lines from a source, not 2" % got[0])


@test("a source feeds the first stage of a pipeline", needs=("echo", "cat", "grep"))
def source_feeds_a_pipeline(t):
    path = t.workspace(W + "shsrcpipe") + "/in.txt"
    t.run("echo alpha > %s" % path)
    t.run("echo beta >> %s" % path)

    out = t.run("cat < %s | grep alpha" % path)
    expect_in("alpha", out, "the matching line came through the pipe")
    expect_not_in("beta", out, "the other line was filtered out")


@test("a source and a target work on one line", needs=("echo", "wc", "cat"))
def source_and_target_together(t):
    base = t.workspace(W + "shboth2")
    t.run("echo one > in.txt")
    t.run("echo two >> in.txt")

    out = t.run("wc < in.txt > out.txt")
    expect_not_in("2", out, "the terminal saw the count")

    expect_in("2", t.run("cat out.txt"), "the count landed in the target")


@test("a missing source is refused and the terminal comes back", needs=("wc", "echo"))
def source_missing_is_refused(t):
    t.workspace(W + "shsrcmiss")

    expect_in("cannot open", t.run("wc < absent.txt"), "a source that is not there")
    expect_in("still here", t.run("echo still here"), "the shell after a refused source")


@test("a dangling source operator is a syntax error", needs=("wc",))
def source_dangling(t):
    expect_in("syntax error", t.run("wc <"), "a source that was never named")
