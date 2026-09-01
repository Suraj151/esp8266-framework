/**************************** Service Stop Tests ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers what stopping a service means: service stop runs the service's own teardown
rather than freezing its clockwork, it refuses the service carrying the session
it was typed on, and enable/disable answer for the next boot instead.

Author          : Suraj I.
created Date    : 31st Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <ShellHarness.h>
#include <pditest.h>

#include <service_provider/ServiceProvider.h>
#include <service_provider/session/SessionManager.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

namespace
{

const char *PROBE_NAME = "Probe";

/* Records whether the real teardown ran, which is the whole question: freezing
   tasks and releasing resources look identical from the task table. */
struct StopProbeService : public ServiceProvider
{
    StopProbeService(terminal_types_t owned = TERMINAL_TYPE_MAX)
        : ServiceProvider(SERVICE_DATABASE, PROBE_NAME), m_stopped(0), m_owned(owned) {}

    bool stopService() override
    {
        m_stopped++;
        return ServiceProvider::stopService();
    }

    terminal_types_t getServiceTerminalType() const override { return m_owned; }

    uint16_t m_stopped;
    terminal_types_t m_owned;
};

struct BorrowedSlot
{
    BorrowedSlot() : m_saved(ServiceProvider::getService(SERVICE_DATABASE)) {}
    ~BorrowedSlot() { ServiceProvider::m_services[SERVICE_DATABASE] = m_saved; }
    ServiceProvider *m_saved;
};

} // namespace

TEST(servicestop, stop_runs_the_service_teardown)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    StopProbeService probe;

    sh.run("service stop Probe");

    ASSERT_EQ((int)probe.m_stopped, 1);
}

TEST(servicestop, stop_releases_the_tracked_tasks)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    StopProbeService probe;

    probe.serviceSetInterval([]() {}, 1000, 0);
    ASSERT_TRUE(probe.getServiceTaskCount() > 0);

    sh.run("service stop Probe");

    ASSERT_EQ((int)probe.getServiceTaskCount(), 0);
}

TEST(servicestop, stop_reports_what_it_released)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    StopProbeService probe;

    probe.serviceSetInterval([]() {}, 1000, 0);

    std::string seen = sh.run("service stop Probe");

    ASSERT_TRUE(seen.find("stopped") != std::string::npos);
    ASSERT_TRUE(seen.find("Probe") != std::string::npos);
}

TEST(servicestop, stop_refuses_the_service_carrying_this_session)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    session_t *cur = SessionManager::current();
    ASSERT_NOT_NULL(cur);
    ASSERT_NOT_NULL(cur->m_terminal);

    StopProbeService probe(cur->m_terminal->get_terminal_type());

    std::string seen = sh.run("service stop Probe");

    ASSERT_TRUE(seen.find("session you are typing on") != std::string::npos);
    ASSERT_EQ((int)probe.m_stopped, 0);
}

TEST(servicestop, stop_of_a_service_on_another_transport_is_allowed)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    session_t *cur = SessionManager::current();
    ASSERT_NOT_NULL(cur);
    ASSERT_NOT_NULL(cur->m_terminal);

    terminal_types_t mine = cur->m_terminal->get_terminal_type();
    terminal_types_t other = (TERMINAL_TYPE_SERIAL == mine) ? TERMINAL_TYPE_SSH : TERMINAL_TYPE_SERIAL;

    StopProbeService probe(other);

    sh.run("service stop Probe");

    ASSERT_EQ((int)probe.m_stopped, 1);
}

TEST(servicestop, an_essential_service_refuses_to_stop)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    struct EssentialProbe : public StopProbeService
    {
        bool isEssentialService() const override { return true; }
    } probe;

    std::string seen = sh.run("service stop Probe");

    ASSERT_TRUE(seen.find("cannot be recovered") != std::string::npos);
    ASSERT_EQ((int)probe.m_stopped, 0);
}

TEST(servicestop, the_shell_still_answers_after_a_refused_stop)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    struct EssentialProbe : public StopProbeService
    {
        bool isEssentialService() const override { return true; }
    } probe;

    sh.run("service stop Probe");

    ASSERT_TRUE(sh.run("pwd").find("/") != std::string::npos);
}

TEST(servicestop, disable_does_not_tear_the_service_down)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    StopProbeService probe;

    probe.serviceSetInterval([]() {}, 1000, 0);

    sh.run("service disable Probe");

    ASSERT_EQ((int)probe.m_stopped, 0);
    ASSERT_TRUE(probe.getServiceTaskCount() > 0);
}

TEST(servicestop, disable_says_when_it_takes_effect)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    StopProbeService probe;

    std::string seen = sh.run("service disable Probe");

    ASSERT_TRUE(seen.find("next boot") != std::string::npos);
}

TEST(servicestop, a_service_carrying_no_transport_names_none)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    StopProbeService probe;

    ASSERT_EQ((int)probe.getServiceTerminalType(), (int)TERMINAL_TYPE_MAX);
}

/* ------------------------------------------------------- state on restart */

/*
   A service whose run latches a flag, the shape that makes a restarted service
   look started and do nothing: the guard that skipped the work stays true.
*/
struct LatchProbeService : public ServiceProvider
{
    LatchProbeService()
        : ServiceProvider(SERVICE_DATABASE, PROBE_NAME), m_validated(false), m_work(0) {}

    bool initService(void *arg = nullptr) override
    {
        doWork();
        return ServiceProvider::initService(arg);
    }

    void resetServiceState() override { m_validated = false; }

    void doWork()
    {
        if (m_validated)
        {
            return;
        }
        m_work++;
        m_validated = true;
    }

    bool m_validated;
    uint16_t m_work;
};

TEST(servicestate, a_stop_clears_a_flag_the_run_latched)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LatchProbeService probe;

    probe.startService();
    ASSERT_TRUE(probe.m_validated);

    probe.stopService();

    ASSERT_FALSE(probe.m_validated);
}

TEST(servicestate, a_restarted_service_runs_its_workflow_again)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LatchProbeService probe;

    probe.startService();
    ASSERT_EQ((int)probe.m_work, 1);

    probe.stopService();
    probe.startService();

    ASSERT_EQ((int)probe.m_work, 2);
}

TEST(servicestate, the_base_reset_leaves_a_service_that_needs_none_alone)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    StopProbeService probe;

    probe.startService();
    probe.stopService();

    ASSERT_EQ((int)probe.m_stopped, 1);
}

#endif
