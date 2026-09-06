#!/usr/bin/env python3

"""
The test command, over whatever transport the target is reached by.

test answers a question rather than doing work, so what matters as much as the
answer is how it reports one: silently, through the status, so a false result
can sit in a script or a chain without printing anything. Nothing else in the
framework signals a boolean through $? — commands return success or a fault —
which is why this is what a condition will be built on.
"""

from .registry import test, expect_in, expect_not_in

W = "wt_"


def status(t):
    """
    The exact status of the last command, bracketed so a match cannot be a
    substring of some other number. Bare $? would let expect_in("0") pass on
    any code that merely contains a zero.
    """
    return t.run("echo [$?]")


def expect_true(t, what):
    expect_in("[0]", status(t), what)


def expect_false(t, what):
    seen = status(t)
    expect_not_in("[0]", seen, what)
    if "[" not in seen:
        raise AssertionError("%s: no status was printed at all:\n%s" % (what, seen))


@test("test compares two strings", needs=("test", "echo"))
def string_compare(t):
    t.run("test a = a")
    expect_true(t, "equal strings are true")

    t.run("test a = b")
    expect_false(t, "unequal strings are false")


@test("test compares two strings for difference", needs=("test", "echo"))
def string_not_equal(t):
    t.run("test a != b")
    expect_true(t, "different strings are true for !=")

    t.run("test a != a")
    expect_false(t, "identical strings are false for !=")


@test("a false test prints nothing", needs=("test",))
def false_is_silent(t):
    """
    A false answer is not a fault. If it printed CmdErr the way an error does,
    a loop testing a condition would print a line per iteration.
    """
    out = t.run("test a = b")
    expect_not_in("CmdErr", out, "a false test reported itself as an error")
    expect_not_in("usage", out, "a false test printed its usage")


@test("a true test prints nothing either", needs=("test",))
def true_is_silent(t):
    out = t.run("test a = a")
    expect_not_in("CmdErr", out, "a true test printed something")


@test("test compares numbers", needs=("test", "echo"))
def numeric_compare(t):
    for expr in ("2 -eq 2", "2 -ne 3", "2 -lt 3", "2 -le 2", "3 -gt 2", "3 -ge 3"):
        t.run("test %s" % expr)
        expect_true(t, "%s is true" % expr)

    for expr in ("2 -eq 3", "2 -ne 2", "3 -lt 2", "3 -le 2", "2 -gt 3", "2 -ge 3"):
        t.run("test %s" % expr)
        expect_false(t, "%s is false" % expr)

    # a negative number is a number
    t.run("test -5 -lt 0")
    expect_true(t, "-5 is less than 0")


@test("a numeric comparison of words is refused, not answered",
      needs=("test",))
def numeric_needs_numbers(t):
    """
    Comparing a word numerically is a malformed test rather than a false one,
    so it reports instead of quietly answering no.
    """
    out = t.run("test apple -eq 2")
    expect_in("usage", out, "a word given to -eq")


@test("test answers on an empty or non-empty string", needs=("test", "echo"))
def string_emptiness(t):
    t.run("test -n something")
    expect_true(t, "-n on a non-empty string")

    t.run("test -z something")
    expect_false(t, "-z on a non-empty string")


@test("test answers on a path", needs=("test", "echo", "touch", "mkdir", "rm"),
      mounts=("/",))
def path_tests(t):
    base = t.workspace(W + "tst")
    t.run("touch afile")
    t.run("mkdir adir")
    try:
        t.run("test -e %s/afile" % base)
        expect_true(t, "-e on a file that exists")

        t.run("test -f %s/afile" % base)
        expect_true(t, "-f on a file")

        t.run("test -d %s/adir" % base)
        expect_true(t, "-d on a directory")

        t.run("test -f %s/adir" % base)
        expect_false(t, "-f on a directory")

        t.run("test -d %s/afile" % base)
        expect_false(t, "-d on a plain file")

        t.run("test -e %s/nothinghere" % base)
        expect_false(t, "-e on a missing path")
    finally:
        t.run("rm afile")
        t.run("rm adir")
        t.run("cd /")


@test("test gates a following command", needs=("test", "echo"))
def gates_a_chain(t):
    """
    The whole point: && and || read PDI_OK, so test is what lets a chain
    branch on a value rather than on whether a command malfunctioned.
    """
    out = t.run("test a = a && echo ranwhentrue")
    expect_in("ranwhentrue", out, "&& after a true test")

    out = t.run("test a = b && echo mustnotrun")
    expect_not_in("mustnotrun", out, "&& after a false test")

    out = t.run("test a = b || echo ranwhenfalse")
    expect_in("ranwhenfalse", out, "|| after a false test")


@test("test reads a variable", needs=("test", "export", "unset", "echo"))
def compares_a_variable(t):
    """
    Variables are the reason a condition is wanted at all, and nothing else in
    the framework can branch on one.
    """
    try:
        t.run("export TMODE=debug")

        out = t.run("test $TMODE = debug && echo matchedthevariable")
        expect_in("matchedthevariable", out, "a variable compared equal")

        out = t.run("test $TMODE = release && echo mustnotmatch")
        expect_not_in("mustnotmatch", out, "a variable compared unequal")
    finally:
        t.run("unset TMODE")


@test("test with nothing to compare is refused", needs=("test",))
def needs_arguments(t):
    expect_in("usage", t.run("test"), "test with no arguments")


@test("an unknown operator is refused", needs=("test",))
def unknown_operator(t):
    expect_in("usage", t.run("test a -wat b"), "an operator test does not know")
