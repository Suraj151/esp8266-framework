#!/usr/bin/env python3

"""
The script runner, over whatever transport the target is reached by.

A script here is not a second grammar. Every line goes through the shell a
typed line goes through, so what these check is the part the runner adds on
top: which lines it skips, that a line's effects land in the session that
asked for the script, and that a line which stops for input hands the terminal
back rather than swallowing the line after it.

The suspend and resume pair is the reason this suite is worth running on a
board at all. Where a script has got to lives on the session, so answering a
prompt has to reach the waiting command by the ordinary route and the script
has to pick itself up afterwards.
"""

from .registry import test, expect_in, expect_not_in, Skip

W = "wt_"


def script(t, name, lines):
    """Write a script of the given lines and hand back its path."""
    path = "/%sscr_%s" % (W, name)
    t.run("rm %s" % path)

    op = ">"
    for line in lines:
        # single quoted so a $name in the script is written, not expanded now
        t.run("echo '%s' %s %s" % (line, op, path))
        op = ">>"

    return path


@test("source runs every line of a file", needs=("source", "echo", "rm"))
def source_runs_lines(t):
    path = script(t, "run", ["echo scriptlineone", "echo scriptlinetwo"])
    try:
        out = t.run("source %s" % path)
        expect_in("scriptlineone", out, "the first line ran")
        expect_in("scriptlinetwo", out, "the second line ran")
    finally:
        t.run("rm %s" % path)


@test("comments and blank lines are passed over", needs=("source", "echo", "rm"))
def source_skips_comments(t):
    path = script(t, "cmt", ["# a note to the reader", "", "echo pastthecomment"])
    try:
        out = t.run("source %s" % path)
        expect_in("pastthecomment", out, "the line after a comment ran")
        expect_not_in("not found", out, "a comment was run as a command")
    finally:
        t.run("rm %s" % path)


@test("a variable a script sets is still set afterwards",
      needs=("source", "export", "unset", "echo", "rm"))
def source_export_persists(t):
    """
    source runs in the caller's session rather than a subshell, so what the
    script exports is the caller's variable once it returns.
    """
    path = script(t, "exp", ["export TSCRIPTVAR=fromthescript"])
    try:
        t.run("source %s" % path)
        expect_in("fromthescript", t.run("echo $TSCRIPTVAR"),
                  "the script's variable outlived it")
    finally:
        t.run("unset TSCRIPTVAR")
        t.run("rm %s" % path)


@test("a script sees the variables the session already had",
      needs=("source", "export", "unset", "echo", "rm"))
def source_reads_session_vars(t):
    path = script(t, "rd", ["echo saw$TOUTERVAR"])
    try:
        t.run("export TOUTERVAR=theouter")
        expect_in("sawtheouter", t.run("source %s" % path),
                  "the script expanded a variable set before it ran")
    finally:
        t.run("unset TOUTERVAR")
        t.run("rm %s" % path)


@test("a file that is not there is refused as a file", needs=("source",))
def source_missing_file(t):
    """
    CMD_ERROR_NOENT means *no such command* in this taxonomy, so a missing
    script must not come back reported as a missing command.
    """
    out = t.run("source /nosuchscriptanywhere")
    expect_not_in("command not found", out, "a missing file named as a missing command")


@test("source needs a file to run", needs=("source",))
def source_needs_an_argument(t):
    expect_in("give it a file", t.run("source"), "source with no argument")


@test("a script can run another script", needs=("source", "echo", "rm"))
def source_nested(t):
    inner = script(t, "inner", ["echo theinnerscript"])
    outer = script(t, "outer", ["echo theouterscript", "source %s" % inner])
    try:
        out = t.run("source %s" % outer)
        expect_in("theouterscript", out, "the outer script ran")
        expect_in("theinnerscript", out, "the inner script ran")
    finally:
        t.run("rm %s" % inner)
        t.run("rm %s" % outer)


@test("a script that runs itself is stopped by the depth limit",
      needs=("source", "echo", "pwd", "rm"), slow=True)
def source_self_recursion_bounded(t):
    """
    A script that sources itself would otherwise recurse until the stack or
    the heap gave out. The depth cap is what stops it, and what matters as
    much as the bound is that the shell is still usable afterwards.
    """
    path = "/%sscr_self" % W
    t.run("rm %s" % path)
    t.run("echo 'echo selfdepthmark' > %s" % path)
    t.run("echo 'source %s' >> %s" % (path, path))
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))

        seen = out.count("selfdepthmark")
        if seen < 2:
            raise AssertionError("the script did not recurse at all, saw %d" % seen)
        if seen > 16:
            raise AssertionError("the depth limit did not stop it, saw %d" % seen)

        expect_in("/", t.run("pwd"), "the shell still answers after the recursion")
    finally:
        t.run("rm %s" % path)


@test("a line that fails does not stop the script", needs=("source", "echo", "rm"))
def source_continues_after_failure(t):
    """
    rc.local behaviour: a script runs to the end rather than stopping at the
    first failure, and && is there for a line that must gate the next.
    """
    path = script(t, "fail", ["nosuchcommandinscript", "echo ranafterthefailure"])
    try:
        expect_in("ranafterthefailure", t.run("source %s" % path),
                  "the script stopped at a failing line")
    finally:
        t.run("rm %s" % path)


@test("a script line carries the whole shell grammar",
      needs=("source", "echo", "wc", "grep", "rm"))
def source_line_is_a_shell_line(t):
    """
    The runner adds no grammar of its own, so a pipeline and an && inside a
    script are the shell's, not something the runner had to learn.
    """
    path = script(t, "gram", ["echo alpha beta | wc",
                              "echo gated && echo aftergate",
                              "echo needle | grep needle"])
    try:
        out = t.run("source %s" % path)
        expect_in("2", out, "the pipeline counted two words")
        expect_in("aftergate", out, "&& ran the second command")
        expect_in("needle", out, "grep matched through the pipe")
    finally:
        t.run("rm %s" % path)


@test("a redirect inside a script writes the file",
      needs=("source", "echo", "cat", "rm"))
def source_redirect(t):
    path = script(t, "redir", ["echo intothefile > /%sscr_out" % W])
    try:
        out = t.run("source %s" % path)
        expect_not_in("intothefile", out, "the terminal saw redirected output")
        expect_in("intothefile", t.run("cat /%sscr_out" % W), "the file holds it")
    finally:
        t.run("rm /%sscr_out" % W)
        t.run("rm %s" % path)


@test("a cd in a script is still in force afterwards",
      needs=("source", "cd", "pwd", "mkdir", "rm"), mounts=("/",))
def source_cd_persists(t):
    """
    The other half of running in the caller's session: the working directory
    the script left behind is the caller's working directory.
    """
    base = t.workspace(W + "scd")
    t.run("mkdir inner")

    path = "/%sscr_cd" % W
    t.run("rm %s" % path)
    t.run("echo 'cd %s/inner' > %s" % (base, path))
    try:
        t.run("source %s" % path)
        expect_in("inner", t.run("pwd"), "the script's cd outlived it")
    finally:
        t.run("cd /")
        t.run("rm %s" % path)


@test("an empty script is not an error", needs=("source", "touch", "rm"))
def source_empty_file(t):
    path = "/%sscr_empty" % W
    t.run("touch %s" % path)
    try:
        out = t.run("source %s" % path)
        expect_not_in("not found", out, "an empty script reported a missing command")
        expect_not_in("cannot run", out, "an empty script was refused")
    finally:
        t.run("rm %s" % path)


@test("a directory is not a script", needs=("source", "mkdir", "rm"))
def source_directory(t):
    path = "/%sscr_dir" % W
    t.run("mkdir %s" % path)
    try:
        out = t.run("source %s" % path)
        expect_not_in("command not found", out, "a directory named as a missing command")
        expect_in("/", t.run("pwd"), "the shell still answers")
    finally:
        t.run("rm %s" % path)


@test("a script named by a relative path runs", needs=("source", "echo", "cd", "rm"),
      mounts=("/",))
def source_relative_path(t):
    t.workspace(W + "srel")
    t.run("rm relscript")
    t.run("echo 'echo ranfromrelative' > relscript")
    try:
        expect_in("ranfromrelative", t.run("source relscript"),
                  "a script named without a leading slash")
    finally:
        t.run("rm relscript")
        t.run("cd /")


@test("a script's output can be piped", needs=("source", "echo", "grep", "rm"))
def source_output_pipes(t):
    """
    source is an ordinary command as far as the shell is concerned, so what
    its lines print is what the next stage of a pipeline reads.
    """
    path = script(t, "pipeout", ["echo keepthisline", "echo dropthisline"])
    try:
        out = t.run("source %s | grep keepthisline" % path)
        expect_in("keepthisline", out, "the matching line came through")
        expect_not_in("dropthisline", out, "grep let a non-matching line through")
    finally:
        t.run("rm %s" % path)


@test("a script the session cannot read is refused, not silently skipped",
      needs=("source", "echo", "chmod", "chown", "su", "rm"), su=True)
def source_unreadable(t):
    """
    Reaching the end of a file is how a script finishes, so a file that could
    not be read at all must not look the same as one that ran and did nothing.
    """
    path = "/%sscr_locked" % W
    t.run("rm %s" % path)
    t.run("echo 'echo shouldnotbereadable' > %s" % path)
    t.run("chown 0:0 %s" % path)
    t.run("chmod 600 %s" % path)

    name = None
    try:
        out = t.run("useradd u=tscriptro p=tscriptropass")
        if "root required" in out or "not found" in out:
            raise Skip("this target cannot add a user to test with")
        name = "tscriptro"

        t.run("su %s tscriptropass" % name)

        out = t.run("source %s" % path)
        expect_not_in("shouldnotbereadable", out, "an unreadable script ran anyway")
    finally:
        t.become_root()
        if name is not None:
            t.run("userdel u=%s" % name)
        t.run("rm %s" % path)


@test("a script carries on after a line stops for input",
      needs=("source", "echo", "su", "rm"), su=True)
def source_resumes_after_prompt(t):
    """
    The one the host cannot ask. su with no arguments prompts, so the script
    suspends with its position on the session; the answer reaches su by the
    ordinary waiting-command route, and the runner picks the script up at the
    line after it. Wrong credentials are deliberate — su fails, the session is
    unchanged, and what is under test is that the script resumed at all.
    """
    path = script(t, "ask", ["su", "echo carriedonafterprompt"])
    try:
        t.shell.send_line("source %s" % path)

        t.shell.expect("user: ", t.timeout)
        t.shell.send_line("nosuchuserhere")

        t.shell.expect("Pass : ", t.timeout)
        t.shell.send_line("notthepassword")

        out = t.shell.expect_count("carriedonafterprompt", 1, max(t.timeout, 20.0))
        expect_in("carriedonafterprompt", out, "the script resumed after the prompt")
    finally:
        t.resync()
        t.become_root()
        t.run("rm %s" % path)


@test("a script suspended in one session is not resumed by another",
      needs=("source", "echo", "su", "rm"))
def pending_script_is_per_session(t):
    """
    Where a script has got to lives on the session, so a second session
    running commands must neither resume it nor be handed its output. Both
    directions matter: the resume hook fires in the epilogue of every command
    on every transport, and it reads the *current* session to decide.
    """
    peer = t.peer()
    path = script(t, "cross", ["su", "echo crosssessionmark"])
    try:
        peer.send_line("source %s" % path)
        peer.expect("user: ", t.timeout)

        # this session runs commands while the peer's script sits suspended
        out = t.run("echo mainsessionmark")
        expect_in("mainsessionmark", out, "this session ran its own command")
        expect_not_in("crosssessionmark", out,
                      "another session's script resumed into this one")

        # the peer answers its own prompt and its own script carries on
        peer.send_line("nosuchuserhere")
        peer.expect("Pass : ", t.timeout)
        peer.send_line("notthepassword")

        seen = peer.expect_count("crosssessionmark", 1, max(t.timeout, 20.0))
        expect_in("crosssessionmark", seen, "the peer's script did not resume for the peer")
    finally:
        try:
            peer.close()
        except Exception:
            pass
        t.resync()
        t.run("rm %s" % path)


@test("a session running a script does not disturb another session",
      needs=("source", "echo", "rm"))
def script_does_not_disturb_peer(t):
    """
    A script runs its lines back to back with a yield between them. Anything
    that changes which session is current during that yield would send the
    next line somewhere else, so the peer must see its own output and only
    its own.
    """
    peer = t.peer()
    path = script(t, "busy", ["echo busyone", "echo busytwo", "echo busythree"])
    try:
        t.shell.send_line("source %s" % path)

        out = peer.run("echo peerownmark", t.timeout)
        expect_in("peerownmark", out, "the peer ran its own command")
        expect_not_in("busyone", out, "the script's output reached the peer")

        seen = t.shell.expect_count("busythree", 1, max(t.timeout, 20.0))
        expect_in("busythree", seen, "the script finished in its own session")
    finally:
        try:
            peer.close()
        except Exception:
            pass
        t.resync()
        t.run("rm %s" % path)


@test("a script does not survive the session it was running in",
      needs=("source", "echo", "su", "rm"), su=True)
def pending_script_cleared_on_su(t):
    """
    A script suspended at a prompt must not be picked up again under whoever
    logs in next. On a serial console the session object outlives the login,
    so this is reset explicitly rather than left to the session being freed —
    the same lesson the environment carry bug taught, but here it would run
    the previous user's remaining lines under the new user's identity.
    """
    path = script(t, "carry", ["su", "echo mustnotrunafterswitch"])
    try:
        t.shell.send_line("source %s" % path)
        t.shell.expect("user: ", t.timeout)

        # abandon the prompt, then change user
        t.shell.send_raw("\x03")
        t.resync()
        t.run("su %s %s" % (t.username, t.password))

        out = t.run("echo afterswitchmarker")
        expect_not_in("mustnotrunafterswitch", out,
                      "a pending script resumed after the user changed")
    finally:
        t.resync()
        t.become_root()
        t.run("rm %s" % path)


@test("if runs its body when the condition is true",
      needs=("source", "test", "echo", "rm"))
def if_true(t):
    path = script(t, "iftrue", ["if test a = a", "echo insidetheif", "fi",
                                "echo afterthefi"])
    try:
        out = t.run("source %s" % path)
        expect_in("insidetheif", out, "the body of a true if")
        expect_in("afterthefi", out, "the line after fi")
    finally:
        t.run("rm %s" % path)


@test("if passes over its body when the condition is false",
      needs=("source", "test", "echo", "rm"))
def if_false(t):
    path = script(t, "iffalse", ["if test a = b", "echo mustnotrun", "fi",
                                 "echo afterthefi"])
    try:
        out = t.run("source %s" % path)
        expect_not_in("mustnotrun", out, "the body of a false if ran")
        expect_in("afterthefi", out, "the script carried on after fi")
    finally:
        t.run("rm %s" % path)


@test("else runs when the condition is false", needs=("source", "test", "echo", "rm"))
def if_else(t):
    path = script(t, "ifelse", ["if test a = b", "echo thethenbranch", "else",
                                "echo theelsebranch", "fi"])
    try:
        out = t.run("source %s" % path)
        expect_in("theelsebranch", out, "the else branch")
        expect_not_in("thethenbranch", out, "the then branch of a false if ran")
    finally:
        t.run("rm %s" % path)


@test("else is passed over when the condition is true",
      needs=("source", "test", "echo", "rm"))
def if_else_not_taken(t):
    path = script(t, "ifelse2", ["if test a = a", "echo thethenbranch", "else",
                                 "echo mustnotrun", "fi"])
    try:
        out = t.run("source %s" % path)
        expect_in("thethenbranch", out, "the then branch")
        expect_not_in("mustnotrun", out, "the else branch of a true if ran")
    finally:
        t.run("rm %s" % path)


@test("an if inside a skipped if is passed over whole",
      needs=("source", "test", "echo", "rm"))
def nested_if_skipped(t):
    """
    A block being skipped still has to be read, because its own nested blocks
    decide which fi closes it. Getting that wrong ends the outer block early
    and runs lines that should not have run.
    """
    path = script(t, "ifnest", ["if test a = b",
                                "if test a = a", "echo innermustnotrun", "fi",
                                "echo outermustnotrun",
                                "fi",
                                "echo afterbothfi"])
    try:
        out = t.run("source %s" % path)
        expect_not_in("innermustnotrun", out, "a nested if inside a skipped if ran")
        expect_not_in("outermustnotrun", out, "the outer body resumed at the inner fi")
        expect_in("afterbothfi", out, "the script carried on after both")
    finally:
        t.run("rm %s" % path)


@test("while repeats until its condition goes false",
      needs=("source", "test", "export", "unset", "echo", "rm"), slow=True)
def while_loop(t):
    # a -> b on the first pass, b -> c on the second, then the condition
    # goes false, so the body runs exactly twice
    path = script(t, "while", ["export N=a",
                               "while test $N != c",
                               "echo loopran",
                               "if test $N = a",
                               "export N=b",
                               "else",
                               "export N=c",
                               "fi",
                               "done",
                               "echo afterthedone"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_in("loopran", out, "the loop body")
        expect_in("afterthedone", out, "the line after done")

        seen = out.count("loopran")
        if seen != 2:
            raise AssertionError("expected two passes, saw %d" % seen)
    finally:
        t.run("unset N")
        t.run("rm %s" % path)


@test("a while whose condition starts false never runs its body",
      needs=("source", "test", "echo", "rm"))
def while_never(t):
    path = script(t, "while0", ["while test a = b", "echo mustnotrun", "done",
                                "echo afterthedone"])
    try:
        out = t.run("source %s" % path)
        expect_not_in("mustnotrun", out, "the body of a false while ran")
        expect_in("afterthedone", out, "the script carried on after done")
    finally:
        t.run("rm %s" % path)


@test("a block left open is reported rather than passing",
      needs=("source", "test", "echo", "rm"))
def unclosed_block(t):
    """
    A script whose if is never closed has not run as written, so it must not
    look the same as one that finished.
    """
    path = script(t, "unclosed", ["if test a = a", "echo insidetheif"])
    try:
        t.run("source %s" % path)
        expect_not_in("[0]", t.run("echo [$?]"), "an unclosed block reported success")
    finally:
        t.run("rm %s" % path)


@test("a loop that never ends is stopped rather than holding the session",
      needs=("source", "test", "echo", "pwd", "rm"), slow=True)
def while_runaway_capped(t):
    """
    A condition that never goes false would hold the session for good, and in
    rc.local there is nobody to interrupt it. The cap is what makes an infinite
    loop a failed script instead of a wedged board.
    """
    path = script(t, "runaway", ["while test a = a", "echo spin", "done"])
    try:
        t.run("source %s" % path, timeout=max(t.timeout, 60.0))
        expect_in("/", t.run("pwd"), "the shell still answers after a runaway loop")
    finally:
        t.run("rm %s" % path)


@test("for runs its body once for each word",
      needs=("source", "echo", "unset", "rm"), slow=True)
def for_each_word(t):
    path = script(t, "foreach", ["for W in alpha beta gamma",
                                 "echo [$W]",
                                 "done",
                                 "echo afterthedone"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        for word in ("[alpha]", "[beta]", "[gamma]"):
            expect_in(word, out, "the pass binding %s" % word)
        expect_in("afterthedone", out, "the line after done")

        # three words is three passes, and the trailer carries no bracket
        seen = out.count("[")
        if seen != 3:
            raise AssertionError("expected three passes, saw %d" % seen)

        # the order of the list is the order of the passes, not just the set
        if not out.index("[alpha]") < out.index("[beta]") < out.index("[gamma]"):
            raise AssertionError("the words did not bind in the order they were written")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("the loop variable keeps its last word after done",
      needs=("source", "echo", "unset", "rm"), slow=True)
def for_last_word(t):
    path = script(t, "forlast", ["for W in alpha beta gamma", "echo pass", "done"])
    try:
        t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_in("gamma", t.run("echo [$W]"), "the name the loop bound")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("for over an empty list never runs its body",
      needs=("source", "echo", "unset", "rm"))
def for_empty(t):
    path = script(t, "forempty", ["for W in", "echo mustnotrun", "done",
                                  "echo afterthedone"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_not_in("mustnotrun", out, "the body of an empty for ran")
        expect_in("afterthedone", out, "the script carried on after done")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("a quoted word carrying a blank is one pass",
      needs=("source", "echo", "unset", "rm"), slow=True)
def for_quoted_word(t):
    path = script(t, "forquote", ['for W in "one two" three', "echo [$W]", "done"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_in("[one two]", out, "the quoted word kept whole")
        expect_in("[three]", out, "the word after it")

        seen = out.count("[")
        if seen != 2:
            raise AssertionError("quoting did not decide the word count, saw %d passes" % seen)
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("an unquoted value holding several words is several passes",
      needs=("source", "echo", "export", "unset", "rm"), slow=True)
def for_splits_unquoted(t):
    path = script(t, "forsplit", ["for W in $LIST", "echo [$W]", "done"])
    try:
        t.run('export LIST="red green blue"')
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))

        for word in ("[red]", "[green]", "[blue]"):
            expect_in(word, out, "the pass binding %s" % word)
        expect_not_in("[red green blue]", out, "the value was taken as one word")
    finally:
        t.run("unset LIST")
        t.run("unset W")
        t.run("rm %s" % path)


@test("a for inside an if that is taken runs every pass",
      needs=("source", "test", "echo", "unset", "rm"), slow=True)
def for_inside_taken_if(t):
    path = script(t, "forinif", ["if test 1 -eq 1",
                                 "for W in alpha beta",
                                 "echo [$W]",
                                 "done",
                                 "fi",
                                 "echo afterthefi"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_in("[alpha]", out, "the first pass")
        expect_in("[beta]", out, "the second pass")
        expect_in("afterthefi", out, "the line after fi")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("a for inside a skipped if is passed over whole",
      needs=("source", "test", "echo", "unset", "rm"), slow=True)
def for_inside_skipped_if(t):
    path = script(t, "forskip", ["if test 1 -eq 2",
                                 "for W in alpha beta",
                                 "echo mustnotrun",
                                 "done",
                                 "fi",
                                 "echo afterthefi"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        # the done inside the skipped if must not be read as closing the if,
        # or the line after fi would be skipped with it
        expect_not_in("mustnotrun", out, "the body of a skipped for ran")
        expect_in("afterthefi", out, "the line after fi")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("a for nested in a for runs the product of the two",
      needs=("source", "echo", "unset", "rm"), slow=True)
def for_nested(t):
    path = script(t, "fornest", ["for W in a b",
                                 "for V in x y",
                                 "echo [$W$V]",
                                 "done",
                                 "done"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 60.0))
        for pair in ("[ax]", "[ay]", "[bx]", "[by]"):
            expect_in(pair, out, "the pass binding %s" % pair)
    finally:
        t.run("unset W")
        t.run("unset V")
        t.run("rm %s" % path)


@test("a for that names no list is reported rather than passing",
      needs=("source", "echo", "unset", "rm"))
def for_malformed(t):
    path = script(t, "forbad", ["for W alpha beta", "echo mustnotrun", "done"])
    try:
        out = t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_not_in("mustnotrun", out, "a malformed for ran its body")
        expect_not_in("[0]", t.run("echo [$?]"), "a malformed for reported success")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("a for left open is reported rather than passing",
      needs=("source", "echo", "unset", "rm"), slow=True)
def for_unclosed(t):
    path = script(t, "foropen", ["for W in alpha beta", "echo insidetheloop"])
    try:
        t.run("source %s" % path, timeout=max(t.timeout, 30.0))
        expect_not_in("[0]", t.run("echo [$?]"), "an unclosed for reported success")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)


@test("a close that does not match what is open is reported",
      needs=("source", "test", "echo", "unset", "rm"), slow=True)
def mismatched_close(t):
    """
    fi closes an if and done closes a loop. A for closed by fi would run one
    pass and stop, which reads as a loop that simply had one word in it, so a
    mismatch has to be reported rather than quietly meaning something else.
    """
    bad_fi = script(t, "forfi", ["for W in alpha beta", "echo insidetheloop", "fi"])
    bad_done = script(t, "ifdone", ["if test 1 -eq 1", "echo insidetheif", "done"])
    try:
        t.run("source %s" % bad_fi, timeout=max(t.timeout, 30.0))
        expect_not_in("[0]", t.run("echo [$?]"), "a for closed by fi reported success")

        t.run("source %s" % bad_done, timeout=max(t.timeout, 30.0))
        expect_not_in("[0]", t.run("echo [$?]"), "an if closed by done reported success")
    finally:
        t.run("unset W")
        t.run("rm %s" % bad_fi)
        t.run("rm %s" % bad_done)


@test("a for is stopped by the same cap a runaway while is",
      needs=("source", "test", "echo", "unset", "pwd", "rm"), slow=True)
def for_capped_like_while(t):
    """
    A for over a fixed list ends itself, so what is worth proving on a board is
    that a loop the runner cannot see the end of leaves the session usable. The
    self-extending list that reaches the cap is exercised on the host, where a
    thousand passes building a long value costs nothing.
    """
    path = script(t, "forcap", ["for W in a b c",
                                "while test 1 -eq 1",
                                "echo spin",
                                "done",
                                "done"])
    try:
        t.run("source %s" % path, timeout=max(t.timeout, 90.0))
        expect_in("/", t.run("pwd"), "the shell still answers after a capped loop")
    finally:
        t.run("unset W")
        t.run("rm %s" % path)
