/***************************** Task Record Tests ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The line a task node reads as, and the parse back out of it. A display and a
generated node both go through this, so a field read at the wrong index shows up
here rather than as a column of nonsense on a board.

Author          : Suraj I.
created Date    : 27th Aug 2026
******************************************************************************/

#include <pditest.h>
#include <utility/TaskRecord.h>

namespace
{
    /**
     * @brief A task filled with values distinct enough to catch a field swap.
     */
    task_t sampleTask()
    {
        task_t task;
        task.m_task_id = 7;
        task.m_name = "watch";
        task.m_state = TASK_STATE_SLEEPING;
        task.m_owner = 3;
        task.m_task_priority = 11;
        task.m_nice = -4;
        task.m_run_count = 22;
        task.m_total_exec_us = 3300;
        task.m_created_ms = 44000;
        task.m_duration = 550;
        task.m_task_policy = TASK_POLICY_DEADLINE;
        task.m_task_mode = TASK_MODE_INLINE;
        return task;
    }
}

TEST(taskrecord, a_state_reads_as_the_letter_ps_shows)
{
    ASSERT_EQ('r', taskStateLetter(TASK_STATE_READY));
    ASSERT_EQ('R', taskStateLetter(TASK_STATE_RUNNING));
    ASSERT_EQ('S', taskStateLetter(TASK_STATE_SLEEPING));
    ASSERT_EQ('T', taskStateLetter(TASK_STATE_STOPPED));
    ASSERT_EQ('Z', taskStateLetter(TASK_STATE_ZOMBIE));
}

TEST(taskrecord, a_policy_reads_as_its_letter)
{
    ASSERT_EQ('F', taskPolicyLetter(TASK_POLICY_FIFO));
    ASSERT_EQ('R', taskPolicyLetter(TASK_POLICY_ROUNDROBIN));
    ASSERT_EQ('D', taskPolicyLetter(TASK_POLICY_DEADLINE));
    ASSERT_EQ('S', taskPolicyLetter(TASK_POLICY_FAIRSHARE));
}

TEST(taskrecord, an_unnamed_task_still_has_a_name_to_show)
{
    task_t task;
    task.m_name = nullptr;

    ASSERT_TRUE(pdiutil::string("-") == taskDisplayName(&task));
}

TEST(taskrecord, the_line_leads_with_the_pid_the_name_and_the_state)
{
    task_t task = sampleTask();
    pdiutil::string line;
    taskStatLine(&task, line);

    pdiutil::string field;

    ASSERT_TRUE(taskStatField(line, 0, field));
    ASSERT_TRUE(pdiutil::string("7") == field);

    ASSERT_TRUE(taskStatField(line, 1, field));
    ASSERT_TRUE(pdiutil::string("watch") == field);

    ASSERT_TRUE(taskStatField(line, 2, field));
    ASSERT_TRUE(pdiutil::string("S") == field);
}

TEST(taskrecord, every_field_survives_the_round_trip)
{
    task_t task = sampleTask();
    pdiutil::string line;
    taskStatLine(&task, line);

    ASSERT_EQ(7, (int)taskStatNumber(line, 0));
    ASSERT_EQ(3, (int)taskStatNumber(line, 3));
    ASSERT_EQ(11, (int)taskStatNumber(line, 4));
    ASSERT_EQ(-4, (int)taskStatNumber(line, 5));
    ASSERT_EQ(22, (int)taskStatNumber(line, 6));
    ASSERT_EQ(3300, (int)taskStatNumber(line, 7));
    ASSERT_EQ(44000, (int)taskStatNumber(line, 8));
    ASSERT_EQ(550, (int)taskStatNumber(line, 9));

    pdiutil::string field;
    ASSERT_TRUE(taskStatField(line, 10, field));
    ASSERT_TRUE(pdiutil::string("D") == field);
}

TEST(taskrecord, a_negative_nice_keeps_its_sign)
{
    task_t task = sampleTask();
    task.m_nice = -20;
    pdiutil::string line;
    taskStatLine(&task, line);

    ASSERT_EQ(-20, (int)taskStatNumber(line, 5));

    task.m_nice = 19;
    taskStatLine(&task, line);
    ASSERT_EQ(19, (int)taskStatNumber(line, 5));
}

TEST(taskrecord, a_name_with_a_space_stays_one_field)
{
    task_t task = sampleTask();
    task.m_name = "two words";
    pdiutil::string line;
    taskStatLine(&task, line);

    pdiutil::string field;
    ASSERT_TRUE(taskStatField(line, 1, field));
    ASSERT_TRUE(pdiutil::string("two words") == field);

    // the fields after the name must not have shifted along with it
    ASSERT_EQ(3, (int)taskStatNumber(line, 3));
    ASSERT_EQ(550, (int)taskStatNumber(line, 9));
}

TEST(taskrecord, a_field_past_the_end_is_refused)
{
    task_t task = sampleTask();
    pdiutil::string line;
    taskStatLine(&task, line);

    pdiutil::string field;
    ASSERT_FALSE(taskStatField(line, 40, field));
    ASSERT_EQ(0, (int)taskStatNumber(line, 40));
}

TEST(taskrecord, a_line_that_is_not_one_yields_nothing)
{
    pdiutil::string field;
    pdiutil::string empty;

    ASSERT_FALSE(taskStatField(empty, 0, field));
    ASSERT_EQ(0, (int)taskStatNumber(empty, 0));
}

TEST(taskrecord, a_null_task_renders_an_empty_line)
{
    pdiutil::string line("stale");
    taskStatLine(nullptr, line);

    ASSERT_TRUE(line.empty());
}

TEST(taskrecord, cpu_share_is_a_ratio_of_run_time_to_lifetime)
{
    // half of a second of work over a second of life reads as 50.00 percent
    ASSERT_EQ(5000u, taskCpuShare(500000ULL, 0ULL, 1000ULL));
    ASSERT_EQ(0u, taskCpuShare(0ULL, 0ULL, 1000ULL));
}

TEST(taskrecord, cpu_share_is_capped_rather_than_wrapping)
{
    ASSERT_EQ(99999u, taskCpuShare(0xFFFFFFFFULL, 0ULL, 1ULL));
}

TEST(taskrecord, a_task_younger_than_the_clock_does_not_divide_by_zero)
{
    ASSERT_EQ(0u, taskCpuShare(0ULL, 5000ULL, 1000ULL));
}
