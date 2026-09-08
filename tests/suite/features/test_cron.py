#!/usr/bin/env python3

"""
Cron, over whatever transport the target is reached by.

The table is read from the file every minute and nothing is cached, so what is
worth checking on a board is that the file really is the source of truth: a row
written now is listed now, and a row removed is gone.

One test waits for a real minute boundary and checks a job left something
behind. That is the only way to know the event actually reaches the service and
that a job runs without a session to run in — neither of which the host tier
can show, since it has no clock ticking and no console to refuse.
"""

import time

from .registry import test, expect_in, expect_not_in, Skip

W = "wt_"

TABLE = "/etc/crontab"


def table(t, lines):
    """Write the crontab with the given lines, and hand back a restore hook."""
    t.run("rm %s" % TABLE)

    op = ">"
    for line in lines:
        t.run("echo '%s' %s %s" % (line, op, TABLE))
        op = ">>"


def clear(t):
    t.run("rm %s" % TABLE)


def clock_is_valid(t):
    """Skip rather than fail where the board has no trustworthy time."""
    out = t.run("crontab")
    if "clock not valid" in out:
        raise Skip("the board's clock is not valid, cron holds every job")
    return out


@test("crontab says so when there is no table", needs=("crontab", "rm"))
def crontab_without_a_table(t):
    clear(t)
    expect_in("no crontab", t.run("crontab"), "crontab with no file")


@test("crontab lists the rows the table holds", needs=("crontab", "echo", "rm"))
def crontab_lists_rows(t):
    try:
        table(t, ["*/5 * * * * echo aaa", "0 3 * * * echo bbb"])
        out = t.run("crontab")
        expect_in("echo aaa", out, "the first job")
        expect_in("echo bbb", out, "the second job")
        expect_in("*/5", out, "the first job's minute field")
    finally:
        clear(t)


@test("a comment and a blank line are not jobs", needs=("crontab", "echo", "rm"))
def crontab_skips_comments(t):
    try:
        table(t, ["# min hour dom mon dow  command", "", "* * * * * echo realjob"])
        out = t.run("crontab")
        expect_in("echo realjob", out, "the one real job")
        expect_not_in("min hour dom", out, "the comment line")
    finally:
        clear(t)


@test("a row that is not five fields and a command is not a job",
      needs=("crontab", "echo", "rm"))
def crontab_skips_malformed(t):
    try:
        table(t, ["* * * * *", "* * * echo tooshort", "* * * * * echo goodrow"])
        out = t.run("crontab")
        expect_in("echo goodrow", out, "the well formed job")
        expect_not_in("tooshort", out, "the row with too few fields")
    finally:
        clear(t)


@test("a job with every field open is due this minute",
      needs=("crontab", "echo", "rm"))
def crontab_reports_due(t):
    try:
        table(t, ["* * * * * echo alwaysdue"])
        clock_is_valid(t)
        out = t.run("crontab")
        expect_in("due this minute", out, "the due heading")
        expect_in("echo alwaysdue", out, "the job listed as due")
    finally:
        clear(t)


@test("a job whose minute has not come round is not due",
      needs=("crontab", "echo", "rm", "date"))
def crontab_reports_not_due(t):
    try:
        # a job pinned to one minute of one hour of one day cannot be due now
        # unless the clock is exactly there, and february 30th never is
        table(t, ["0 0 30 2 * echo neverdue"])
        clock_is_valid(t)
        out = t.run("crontab")
        expect_in("echo neverdue", out, "the job is listed")
        expect_in("none", out, "nothing is due")
    finally:
        clear(t)


@test("the table is re-read, so a removed job stops being listed",
      needs=("crontab", "echo", "rm"))
def crontab_rereads_the_table(t):
    try:
        table(t, ["* * * * * echo firstversion"])
        expect_in("firstversion", t.run("crontab"), "the first table")

        table(t, ["* * * * * echo secondversion"])
        out = t.run("crontab")
        expect_in("secondversion", out, "the rewritten table")
        expect_not_in("firstversion", out, "the replaced job")
    finally:
        clear(t)


@test("a job runs on its minute", needs=("crontab", "echo", "rm", "cat"),
      mounts=("/tmp",), slow=True)
def a_job_runs_on_its_minute(t):
    marker = "/tmp/%scron_ran" % W

    try:
        t.run("rm %s" % marker)
        table(t, ["* * * * * echo cronfired > %s" % marker])
        clock_is_valid(t)

        # the job fires on the next minute boundary, so allow one to pass
        for _ in range(20):
            time.sleep(5.0)
            out = t.run("cat %s" % marker)
            if "cronfired" in out:
                return

        raise AssertionError("the job left nothing behind after a minute passed")
    finally:
        t.run("rm %s" % marker)
        clear(t)


@test("the cron service is listed and active", needs=("service",), services=("Cron",))
def cron_service_is_active(t):
    expect_in("Cron", t.run("service list"), "cron in the service list")
    expect_in("active", t.run("service status Cron", timeout=max(t.timeout, 30)),
              "the cron service state")


@test("the service leaves a table to edit when none was there",
      needs=("crontab", "cat", "rm"), services=("Cron",), slow=True)
def the_service_seeds_its_table(t):
    # the table is written at init, so it comes back on the next service start
    t.run("rm %s" % TABLE)
    t.run("service restart Cron", timeout=max(t.timeout, 20))

    out = t.run("cat %s" % TABLE)
    expect_in("min hour dom mon dow", out, "the seeded table's column names")

    expect_in("no jobs", t.run("crontab"), "a seeded table holds no job")


@test("cron can be stopped and started again", needs=("service",),
      services=("Cron",), slow=True)
def cron_stops_and_starts(t):
    try:
        t.run("service stop Cron", timeout=max(t.timeout, 15))
        expect_in("inactive", t.run("service status Cron", timeout=max(t.timeout, 15)),
                  "cron after a stop")
    finally:
        t.run("service start Cron", timeout=max(t.timeout, 15))

    expect_in("active", t.run("service status Cron", timeout=max(t.timeout, 15)),
              "cron after being started again")


@test("a job does not run while the service is stopped", needs=("service", "cat", "rm"),
      services=("Cron",), mounts=("/tmp",), slow=True)
def a_stopped_service_runs_no_job(t):
    marker = "/tmp/%scron_stopped" % W

    try:
        t.run("service stop Cron", timeout=max(t.timeout, 15))
        t.run("rm %s" % marker)
        table(t, ["* * * * * echo ranwhilestopped > %s" % marker])
        clock_is_valid(t)

        # a job set to every minute would have fired inside this window
        time.sleep(70.0)

        expect_not_in("ranwhilestopped", t.run("cat %s" % marker),
                      "a job that ran with the service stopped")
    finally:
        clear(t)
        t.run("rm %s" % marker)
        t.run("service start Cron", timeout=max(t.timeout, 15))
