/**************************** Task Scheduler Tests ****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 16th Aug 2026
******************************************************************************/

#include <FakeClock.h>
#include <pditest.h>
#include <utility/TaskScheduler.h>

static int s_counter_a = 0;
static int s_counter_b = 0;
static int s_order[8];
static int s_order_len = 0;

static void resetCounters()
{
    s_counter_a = 0;
    s_counter_b = 0;
    s_order_len = 0;
    memset(s_order, 0, sizeof(s_order));
}

static void bumpA()
{
    s_counter_a++;
    if (s_order_len < 8)
    {
        s_order[s_order_len++] = 1;
    }
}

static void bumpB()
{
    s_counter_b++;
    if (s_order_len < 8)
    {
        s_order[s_order_len++] = 2;
    }
}

/**
 * Drive the scheduler forward in whole millisecond steps so every due task gets
 * a tick, the way a real serve loop would.
 */
static void runFor(TaskScheduler &scheduler, pditest::FakeClock &clock, uint32_t milliseconds)
{
    for (uint32_t i = 0; i < milliseconds; i++)
    {
        clock.advance(1);
        scheduler.handle_tasks();
    }
}

TEST(scheduler, registers_and_reports_a_task)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t id = scheduler.setInterval(bumpA, 10, clock.millis_now());
    ASSERT_GT(id, 0);
    ASSERT_EQ(scheduler.getTaskCount(), (uint16_t)1);
    ASSERT_GE(scheduler.is_registered_task(id), 0);
}

TEST(scheduler, interval_task_repeats)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    scheduler.setInterval(bumpA, 10, clock.millis_now());
    runFor(scheduler, clock, 100);

    ASSERT_GE(s_counter_a, 9);
    ASSERT_LE(s_counter_a, 11);
}

TEST(scheduler, interval_task_does_not_run_before_it_is_due)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    scheduler.setInterval(bumpA, 50, clock.millis_now());
    runFor(scheduler, clock, 40);

    ASSERT_EQ(s_counter_a, 0);
}

TEST(scheduler, timeout_task_runs_once)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    scheduler.setTimeout(bumpA, 20, clock.millis_now());
    runFor(scheduler, clock, 200);

    ASSERT_EQ(s_counter_a, 1);
}

TEST(scheduler, timeout_task_is_reaped_after_firing)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t id = scheduler.setTimeout(bumpA, 20, clock.millis_now());
    runFor(scheduler, clock, 200);
    scheduler.remove_expired_tasks();

    ASSERT_EQ(scheduler.is_registered_task(id), (int16_t)-1);
}

TEST(scheduler, clear_interval_stops_a_task)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t id = scheduler.setInterval(bumpA, 10, clock.millis_now());
    runFor(scheduler, clock, 50);
    int before = s_counter_a;

    ASSERT_TRUE(scheduler.clearInterval(id));
    scheduler.remove_expired_tasks();
    runFor(scheduler, clock, 50);

    ASSERT_EQ(s_counter_a, before);
}

TEST(scheduler, clear_interval_rejects_an_unknown_id)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);

    ASSERT_FALSE(scheduler.clearInterval(9999));
}

TEST(scheduler, update_interval_changes_the_period)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t id = scheduler.setInterval(bumpA, 100, clock.millis_now());
    runFor(scheduler, clock, 50);
    ASSERT_EQ(s_counter_a, 0);

    scheduler.updateInterval(id, bumpA, 10, DEFAULT_TASK_PRIORITY, clock.millis_now());
    runFor(scheduler, clock, 100);

    ASSERT_GT(s_counter_a, 5);
}

TEST(scheduler, update_interval_registers_an_unknown_id_as_a_new_task)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t id = scheduler.updateInterval(4242, bumpA, 10, DEFAULT_TASK_PRIORITY,
                                                     clock.millis_now());
    ASSERT_NE(id, (pdiutil::task_id_t)4242);
    ASSERT_EQ(scheduler.getTaskCount(), (uint16_t)1);
}

TEST(scheduler, two_tasks_both_run)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    scheduler.setInterval(bumpA, 10, clock.millis_now());
    scheduler.setInterval(bumpB, 10, clock.millis_now());
    runFor(scheduler, clock, 50);

    ASSERT_GT(s_counter_a, 0);
    ASSERT_GT(s_counter_b, 0);
}

TEST(scheduler, remove_task_drops_it_from_the_table)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t id = scheduler.setInterval(bumpA, 10, clock.millis_now());
    ASSERT_TRUE(scheduler.remove_task(id));
    scheduler.remove_expired_tasks();

    ASSERT_EQ(scheduler.is_registered_task(id), (int16_t)-1);
}

TEST(scheduler, task_name_and_owner_round_trip)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);

    pdiutil::task_id_t id = scheduler.setInterval(bumpA, 10, clock.millis_now(),
                                                  DEFAULT_TASK_PRIORITY, "worker", 3);
    ASSERT_EQ(scheduler.getTaskOwner(id), (uint8_t)3);

    ASSERT_TRUE(scheduler.setTaskOwner(id, 7));
    ASSERT_EQ(scheduler.getTaskOwner(id), (uint8_t)7);

    ASSERT_TRUE(scheduler.setTaskName(id, "renamed"));
    task_t *task = scheduler.get_task(id);
    ASSERT_NOT_NULL(task);
}

TEST(scheduler, unique_ids_do_not_collide_while_tasks_are_live)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);

    pdiutil::task_id_t first = scheduler.setInterval(bumpA, 10, clock.millis_now());
    pdiutil::task_id_t second = scheduler.setInterval(bumpB, 10, clock.millis_now());
    pdiutil::task_id_t third = scheduler.setInterval(bumpA, 10, clock.millis_now());

    ASSERT_NE(first, second);
    ASSERT_NE(second, third);
    ASSERT_NE(first, third);
}

/**
 * The id of a reaped task is not handed to the next task registered, so a
 * caller still holding it cannot reach whatever came after it.
 */
TEST(scheduler, id_of_a_reaped_task_is_not_handed_out_again)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t oneshot = scheduler.setTimeout(bumpA, 10, clock.millis_now());
    runFor(scheduler, clock, 50);
    scheduler.remove_expired_tasks();
    ASSERT_EQ(scheduler.is_registered_task(oneshot), (int16_t)-1);

    pdiutil::task_id_t replacement = scheduler.setInterval(bumpB, 10, clock.millis_now());
    ASSERT_NE(replacement, oneshot);
}

/**
 * A caller holding the id of a task that has gone gets a task of its own, and
 * the task registered after it keeps the callback it was given.
 */
TEST(scheduler, a_stale_id_does_not_retarget_the_task_that_followed_it)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    pdiutil::task_id_t staleid = scheduler.setTimeout(bumpA, 10, clock.millis_now());
    runFor(scheduler, clock, 50);
    scheduler.remove_expired_tasks();

    scheduler.setInterval(bumpB, 10, clock.millis_now());
    resetCounters();
    runFor(scheduler, clock, 50);
    ASSERT_GT(s_counter_b, 0);

    scheduler.updateInterval(staleid, bumpA, 10, DEFAULT_TASK_PRIORITY, clock.millis_now());
    resetCounters();
    runFor(scheduler, clock, 50);

    ASSERT_GT(s_counter_a, 0);
    ASSERT_GT(s_counter_b, 0);
}

/**
 * Ids run out eventually and start again from one. A task still holding a low
 * id must be stepped over rather than duplicated.
 */
TEST(scheduler, an_id_still_in_use_is_skipped_when_ids_start_again)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);

    pdiutil::task_id_t keeper = scheduler.setInterval(bumpA, 1000, clock.millis_now());
    ASSERT_EQ(keeper, (pdiutil::task_id_t)1);

    bool wrapped = false;
    pdiutil::task_id_t previous = keeper;

    for (int32_t i = 0; i < MAX_TASK_ID + 4; i++)
    {
        pdiutil::task_id_t id = scheduler.setInterval(bumpB, 1000, clock.millis_now());
        ASSERT_TRUE(id > 0);
        ASSERT_NE(id, keeper);

        if (id < previous) wrapped = true;
        previous = id;

        scheduler.remove_task(id);
        scheduler.remove_expired_tasks();
    }

    ASSERT_TRUE(wrapped);
    ASSERT_EQ(scheduler.is_registered_task(keeper) >= 0, true);
}

TEST(scheduler, a_finalizer_runs_with_the_task_when_it_is_reaped)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    static pdiutil::task_id_t reaped_id;
    static int reaped_count;
    reaped_id = -1;
    reaped_count = 0;

    pdiutil::task_id_t id = scheduler.setTimeout(bumpA, 10, clock.millis_now());
    ASSERT_GT(id, 0);

    ASSERT_FALSE(scheduler.hasTaskFinalizer(id));
    ASSERT_TRUE(scheduler.setTaskFinalizer(id, [](void *arg) {
        task_t *task = reinterpret_cast<task_t *>(arg);
        if (nullptr != task) reaped_id = task->m_task_id;
        reaped_count++;
    }));
    ASSERT_TRUE(scheduler.hasTaskFinalizer(id));

    // the timeout fires, then the slot is released on a later pass
    runFor(scheduler, clock, 60);

    ASSERT_EQ(reaped_count, 1);
    ASSERT_EQ((int)reaped_id, (int)id);
    ASSERT_LT(scheduler.is_registered_task(id), 0);
}

TEST(scheduler, a_finalizer_is_not_run_twice)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    static int calls;
    calls = 0;

    pdiutil::task_id_t id = scheduler.setTimeout(bumpA, 10, clock.millis_now());
    scheduler.setTaskFinalizer(id, [](void *) { calls++; });

    runFor(scheduler, clock, 200);

    ASSERT_EQ(calls, 1);
}

TEST(scheduler, a_cleared_task_still_runs_its_finalizer)
{
    TaskScheduler scheduler;
    pditest::FakeClock clock;
    scheduler.setUtilityInterface(&clock);
    resetCounters();

    static int calls;
    calls = 0;

    pdiutil::task_id_t id = scheduler.setInterval(bumpA, 10, clock.millis_now());
    scheduler.setTaskFinalizer(id, [](void *) { calls++; });

    scheduler.clearInterval(id);
    runFor(scheduler, clock, 30);

    ASSERT_EQ(calls, 1);
    ASSERT_LT(scheduler.is_registered_task(id), 0);
}
