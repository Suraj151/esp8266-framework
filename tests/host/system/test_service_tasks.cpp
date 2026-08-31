/*************************** Service Task Tests *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers the invariant service control rests on: a task a service owns is registered
through the service task API, so srvc start/stop/disable reaches it. A task
registered straight on the scheduler still runs, and would silently be beyond the
reach of every one of those commands.

Author          : Suraj I.
created Date    : 30th Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <pditest.h>

#include <service_provider/ServiceProvider.h>

namespace
{

struct ProbeTaskService : public ServiceProvider
{
    ProbeTaskService() : ServiceProvider(SERVICE_DATABASE, "TaskProbe") {}
};

/* The scheduler marks a cleared task a zombie and frees the slot only when it is
   reaped, and the table is a fixed 25 shared with everything the stack started. A
   probe that does not reap leaves the next test with nowhere to register. */
struct BorrowedTaskSlot
{
    BorrowedTaskSlot() : m_saved(ServiceProvider::getService(SERVICE_DATABASE)) {}
    ~BorrowedTaskSlot()
    {
        ServiceProvider::m_services[SERVICE_DATABASE] = m_saved;
        __task_scheduler.remove_expired_tasks();
    }
    ServiceProvider *m_saved;
};

} // namespace

TEST(servicetasks, a_task_scheduled_through_the_service_is_tracked)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    pdiutil::task_id_t id = probe.serviceSetTimeout([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_TRUE(id > 0);
    ASSERT_TRUE(probe.isServiceTaskTracked(id));
    ASSERT_EQ((uint32_t)probe.getServiceTaskCount(), 1u);

    __task_scheduler.clearTimeout(id);
}

TEST(servicetasks, a_tracked_task_carries_the_service_name)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    pdiutil::task_id_t id = probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_TRUE(id > 0);

    task_t *t = __task_scheduler.get_task(id);
    ASSERT_TRUE(nullptr != t);
    ASSERT_TRUE(nullptr != t->m_name);
    ASSERT_STREQ(t->m_name, "TaskProbe");

    __task_scheduler.clearInterval(id);
}

TEST(servicetasks, stopping_the_service_reaps_every_tracked_task)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());
    probe.serviceSetTimeout([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_EQ((uint32_t)probe.getServiceTaskCount(), 2u);

    probe.stopService();
    ASSERT_EQ((uint32_t)probe.getServiceTaskCount(), 0u);
}

TEST(servicetasks, a_signal_reaches_every_tracked_task)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());
    probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());

    ASSERT_EQ((uint32_t)probe.signalAllServiceTasks(SIG_STOP), 2u);

    probe.stopService();
}

TEST(servicetasks, a_task_registered_straight_on_the_scheduler_is_out_of_reach)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    pdiutil::task_id_t loose = __task_scheduler.setTimeout([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_TRUE(loose > 0);

    ASSERT_FALSE(probe.isServiceTaskTracked(loose));
    ASSERT_EQ((uint32_t)probe.signalAllServiceTasks(SIG_STOP), 0u);

    probe.stopService();
    ASSERT_TRUE(nullptr != __task_scheduler.get_task(loose));

    __task_scheduler.clearTimeout(loose);
}

TEST(servicetasks, handing_a_loose_task_to_the_service_brings_it_back_in_reach)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    pdiutil::task_id_t loose = __task_scheduler.setTimeout([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_TRUE(probe.trackServiceTask(loose));
    ASSERT_TRUE(probe.isServiceTaskTracked(loose));
    ASSERT_EQ((uint32_t)probe.signalAllServiceTasks(SIG_STOP), 1u);

    probe.stopService();
}

TEST(servicetasks, the_tracked_list_refuses_rather_than_overflowing_quietly)
{
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    pdiutil::task_id_t ids[MAX_SERVICE_TASKS];
    for (uint8_t i = 0; i < MAX_SERVICE_TASKS; i++)
    {
        ids[i] = probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());
        ASSERT_TRUE(ids[i] > 0);
    }
    ASSERT_EQ((uint32_t)probe.getServiceTaskCount(), (uint32_t)MAX_SERVICE_TASKS);

    pdiutil::task_id_t overflow = probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_TRUE(overflow > 0);
    ASSERT_FALSE(probe.isServiceTaskTracked(overflow));

    __task_scheduler.clearInterval(overflow);
    probe.stopService();
}

/* ------------------------------------------------- the whole stack, at boot */

TEST(servicetasks, every_running_task_named_for_a_service_is_tracked_by_it)
{
    pditest::mountedVfs();

    for (uint16_t idx = 0; idx < __task_scheduler.getTaskSlots(); idx++)
    {
        task_t *t = __task_scheduler.getTaskByIndex(idx);
        if (nullptr == t || nullptr == t->m_name)
        {
            continue;
        }

        for (uint8_t i = 0; i < SERVICE_MAX; i++)
        {
            ServiceProvider *s = ServiceProvider::getService((service_t)i);
            if (nullptr == s || nullptr == s->m_service_name)
            {
                continue;
            }

            if (0 == strcmp_ro(t->m_name, s->m_service_name))
            {
                ASSERT_TRUE(s->isServiceTaskTracked(t->m_task_id));
            }
        }
    }
}

TEST(servicetasks, the_boot_scan_actually_sees_service_tasks)
{
    pditest::mountedVfs();
    BorrowedTaskSlot slot;
    ProbeTaskService probe;

    pdiutil::task_id_t id = probe.serviceSetInterval([]() {}, 10000, __i_dvc_ctrl.millis_now());
    ASSERT_TRUE(id > 0);

    uint16_t named = 0;
    for (uint16_t idx = 0; idx < __task_scheduler.getTaskSlots(); idx++)
    {
        task_t *t = __task_scheduler.getTaskByIndex(idx);
        if (nullptr == t || nullptr == t->m_name) continue;

        for (uint8_t i = 0; i < SERVICE_MAX; i++)
        {
            ServiceProvider *s = ServiceProvider::getService((service_t)i);
            if (nullptr == s || nullptr == s->m_service_name) continue;
            if (0 == strcmp_ro(t->m_name, s->m_service_name)) named++;
        }
    }

    ASSERT_TRUE(named > 0);

    probe.stopService();
}
