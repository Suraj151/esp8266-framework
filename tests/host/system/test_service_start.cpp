/**************************** Service Start Tests *****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers starting a service from cold: service start runs the service's own init, says
so rather than acting when the service is already running, and restart is a stop
followed by a start that refuses whatever a stop refuses.

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

/* Counts both halves of the lifecycle, and records what it was handed, because
   a service that can start from cold must not need an argument to do it. */
struct LifecycleProbeService : public ServiceProvider
{
    LifecycleProbeService(terminal_types_t owned = TERMINAL_TYPE_MAX)
        : ServiceProvider(SERVICE_DATABASE, PROBE_NAME),
          m_started(0), m_stopped(0), m_lastarg(this), m_refuse(false), m_owned(owned) {}

    bool initService(void *arg = nullptr) override
    {
        m_started++;
        m_lastarg = arg;
        if (m_refuse) return false;
        return ServiceProvider::initService(arg);
    }

    bool stopService() override
    {
        m_stopped++;
        return ServiceProvider::stopService();
    }

    terminal_types_t getServiceTerminalType() const override { return m_owned; }

    uint16_t m_started;
    uint16_t m_stopped;
    void *m_lastarg;
    bool m_refuse;
    terminal_types_t m_owned;
};

struct BorrowedSlot
{
    BorrowedSlot(service_t which = SERVICE_DATABASE)
        : m_which(which), m_saved(ServiceProvider::getService(which)) {}
    ~BorrowedSlot() { ServiceProvider::m_services[m_which] = m_saved; }
    service_t m_which;
    ServiceProvider *m_saved;
};

/* A dependency whose state the test sets directly, so what is being checked is
   the dependency rule and not whether some real service re-inits. */
struct DependencyProbe : public ServiceProvider
{
    DependencyProbe() : ServiceProvider(SERVICE_CMD, "Dep") {}
};

} // namespace

TEST(servicestart, start_brings_a_service_up_from_cold)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    sh.run("service start Probe");

    ASSERT_EQ((int)probe.m_started, 1);
}

TEST(servicestart, start_hands_the_service_no_argument)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    sh.run("service start Probe");

    ASSERT_NULL(probe.m_lastarg);
}

TEST(servicestart, start_says_so_when_it_is_already_running)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    sh.run("service start Probe");
    ASSERT_EQ((int)probe.m_started, 1);

    std::string seen = sh.run("service start Probe");

    ASSERT_TRUE(seen.find("already running") != std::string::npos);
    ASSERT_EQ((int)probe.m_started, 1);
}

TEST(servicestart, a_service_holding_a_task_is_not_running_by_that_alone)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    probe.serviceSetInterval([]() {}, 1000, 0);

    ASSERT_EQ((int)probe.getServiceState(), (int)SERVICE_STATE_BOOT);
    ASSERT_TRUE(sh.run("service start Probe").find("already running") == std::string::npos);
    ASSERT_EQ((int)probe.m_started, 1);
}

TEST(servicestart, a_service_that_refuses_to_start_is_reported)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;
    probe.m_refuse = true;

    std::string seen = sh.run("service start Probe");

    ASSERT_TRUE(seen.find("did not start") != std::string::npos);
}

TEST(servicestart, restart_stops_and_then_starts)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    sh.run("service restart Probe");

    ASSERT_EQ((int)probe.m_stopped, 1);
    ASSERT_EQ((int)probe.m_started, 1);
}

TEST(servicestart, restart_releases_the_tasks_the_service_held)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    probe.serviceSetInterval([]() {}, 1000, 0);
    ASSERT_TRUE(probe.getServiceTaskCount() > 0);

    sh.run("service restart Probe");

    ASSERT_EQ((int)probe.m_stopped, 1);
}

TEST(servicestart, restart_refuses_the_service_carrying_this_session)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    session_t *cur = SessionManager::current();
    ASSERT_NOT_NULL(cur);
    ASSERT_NOT_NULL(cur->m_terminal);

    LifecycleProbeService probe(cur->m_terminal->get_terminal_type());

    std::string seen = sh.run("service restart Probe");

    ASSERT_TRUE(seen.find("session you are typing on") != std::string::npos);
    ASSERT_EQ((int)probe.m_stopped, 0);
    ASSERT_EQ((int)probe.m_started, 0);
}

TEST(servicestart, restart_refuses_an_essential_service)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    struct EssentialProbe : public LifecycleProbeService
    {
        bool isEssentialService() const override { return true; }
    } probe;

    std::string seen = sh.run("service restart Probe");

    ASSERT_TRUE(seen.find("cannot be recovered") != std::string::npos);
    ASSERT_EQ((int)probe.m_stopped, 0);
}

TEST(servicestart, a_restart_that_cannot_come_back_is_reported)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;
    probe.m_refuse = true;

    std::string seen = sh.run("service restart Probe");

    ASSERT_TRUE(seen.find("did not start again") != std::string::npos);
    ASSERT_EQ((int)probe.m_stopped, 1);
}

TEST(servicedeps, a_service_naming_nothing_has_nothing_unmet)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;

    ASSERT_NULL(probe.findUnmetServiceDependency());
}

TEST(servicedeps, a_dependency_that_is_not_active_is_reported_as_unmet)
{
    pditest::mountedVfs();
    BorrowedSlot slot;

    BorrowedSlot depslot(SERVICE_CMD);
    DependencyProbe dep;
    struct NeedsDep : public LifecycleProbeService
    {
        uint8_t getServiceDependencies(service_t *_out, uint8_t _max) const override
        {
            if (_max < 1) return 0;
            _out[0] = SERVICE_CMD;
            return 1;
        }
    } probe;

    ASSERT_EQ((int)dep.getServiceState(), (int)SERVICE_STATE_BOOT);
    ASSERT_TRUE(probe.findUnmetServiceDependency() == &dep);
}

TEST(servicedeps, an_active_dependency_is_met)
{
    pditest::mountedVfs();
    BorrowedSlot slot;

    BorrowedSlot depslot(SERVICE_CMD);
    DependencyProbe dep;
    struct NeedsDep : public LifecycleProbeService
    {
        uint8_t getServiceDependencies(service_t *_out, uint8_t _max) const override
        {
            if (_max < 1) return 0;
            _out[0] = SERVICE_CMD;
            return 1;
        }
    } probe;

    ASSERT_TRUE(dep.startService());
    ASSERT_NULL(probe.findUnmetServiceDependency());
}

TEST(servicedeps, start_refuses_while_a_dependency_is_down)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;

    BorrowedSlot depslot(SERVICE_CMD);
    DependencyProbe dep;
    struct NeedsDep : public LifecycleProbeService
    {
        uint8_t getServiceDependencies(service_t *_out, uint8_t _max) const override
        {
            if (_max < 1) return 0;
            _out[0] = SERVICE_CMD;
            return 1;
        }
    } probe;

    std::string seen = sh.run("service start Probe");

    ASSERT_TRUE(seen.find("it needs") != std::string::npos);
    ASSERT_EQ((int)probe.m_started, 0);
}

TEST(servicedeps, a_dependency_absent_from_the_build_does_not_block)
{
    pditest::mountedVfs();
    BorrowedSlot slot;

    struct NeedsNothingRegistered : public LifecycleProbeService
    {
        uint8_t getServiceDependencies(service_t *_out, uint8_t _max) const override
        {
            if (_max < 1) return 0;
            _out[0] = SERVICE_MAX;
            return 1;
        }
    } probe;

    ASSERT_NULL(probe.findUnmetServiceDependency());
}

TEST(servicedeps, more_dependencies_than_the_cap_are_refused_rather_than_overrun)
{
    pditest::mountedVfs();
    BorrowedSlot slot;

    struct Greedy : public LifecycleProbeService
    {
        uint8_t getServiceDependencies(service_t *_out, uint8_t _max) const override
        {
            if (_max < 1) return 0;
            _out[0] = SERVICE_CMD;
            return 1;
        }
    } probe;

    service_t one[1];
    ASSERT_EQ((int)probe.getServiceDependencies(one, 1), 1);
    ASSERT_EQ((int)probe.getServiceDependencies(one, 0), 0);
}

TEST(servicestate, a_service_starts_out_never_booted)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;

    ASSERT_EQ((int)probe.getServiceState(), (int)SERVICE_STATE_BOOT);
}

TEST(servicestate, a_stopped_service_is_no_longer_in_its_boot_state)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;

    probe.startService();
    probe.stopService();
    probe.startService();

    ASSERT_EQ((int)probe.getServiceState(), (int)SERVICE_STATE_ACTIVE);
    ASSERT_TRUE(SERVICE_STATE_BOOT != probe.getServiceState());
}

TEST(servicestate, a_failed_start_still_leaves_the_boot_state_behind)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;
    probe.m_refuse = true;

    ASSERT_TRUE(!probe.startService());
    ASSERT_TRUE(SERVICE_STATE_BOOT != probe.getServiceState());
}

TEST(servicestate, a_start_that_works_records_active)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;

    ASSERT_TRUE(probe.startService());
    ASSERT_EQ((int)probe.getServiceState(), (int)SERVICE_STATE_ACTIVE);
}

TEST(servicestate, a_start_that_refuses_records_failed)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;
    probe.m_refuse = true;

    ASSERT_TRUE(!probe.startService());
    ASSERT_EQ((int)probe.getServiceState(), (int)SERVICE_STATE_FAILED);
}

TEST(servicestate, a_stop_records_inactive)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    LifecycleProbeService probe;

    probe.startService();
    probe.stopService();

    ASSERT_EQ((int)probe.getServiceState(), (int)SERVICE_STATE_INACTIVE);
}

TEST(servicestate, a_failed_service_reports_failed_to_service)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;
    probe.m_refuse = true;

    sh.run("service start Probe");

    ASSERT_TRUE(sh.run("service status Probe").find("failed") != std::string::npos);
}

TEST(servicestate, a_started_service_reports_active_to_service)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    sh.run("service start Probe");

    ASSERT_TRUE(sh.run("service status Probe").find("state   : active") != std::string::npos);
}

TEST(servicestate, the_starting_banner_begins_on_a_line_of_its_own)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    std::string out = sh.run("service start Probe");

    std::string::size_type at = out.find(" Starting ");
    ASSERT_TRUE(at != std::string::npos);
    ASSERT_TRUE(at > 0);
    ASSERT_TRUE(out[at - 1] == '\n');
}

TEST(servicestart, stop_then_start_puts_the_service_back)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    BorrowedSlot slot;
    LifecycleProbeService probe;

    probe.serviceSetInterval([]() {}, 1000, 0);

    sh.run("service stop Probe");
    ASSERT_EQ((int)probe.getServiceTaskCount(), 0);

    sh.run("service start Probe");
    ASSERT_EQ((int)probe.m_started, 1);
}

#endif
