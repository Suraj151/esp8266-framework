/*************************** Service Enable Tests *****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers what a service is meant to do across a restart: the choice is persisted in
the service's own config file, it is read back on the next load, and a service the
device cannot be recovered without stays out of reach of a disable.

Author          : Suraj I.
created Date    : 30th Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <pditest.h>

#include <service_provider/ServiceProvider.h>
#include <helpers/ConfigHelper.h>
#ifdef ENABLE_HTTP_SERVER
#include <webserver/WebServer.h>
#endif

#ifdef ENABLE_STORAGE_SERVICE

namespace
{

const char *PROBE_NAME = "Probe";
const char *PROBE_CONF = "/etc/probe/probe.conf";
const char *PROBE_DIR = "/etc/probe";

struct ProbeService : public ServiceProvider
{
    ProbeService(bool essential) : ServiceProvider(SERVICE_DATABASE, PROBE_NAME), m_essential(essential) {}
    bool isEssentialService() const override { return m_essential; }
    bool m_essential;
};

/* The registry slot is one per service type, so a probe borrows one and hands
   it back rather than leaving the real service unreachable. */
struct BorrowedSlot
{
    BorrowedSlot(service_t which = SERVICE_DATABASE)
        : m_which(which), m_saved(ServiceProvider::getService(which)) {}
    ~BorrowedSlot() { ServiceProvider::m_services[m_which] = m_saved; }
    service_t m_which;
    ServiceProvider *m_saved;
};

void clearProbeConf()
{
    VfsDispatcher *fs = pditest::mountedVfs();
    fs->beginPrivileged();
    if (fs->isFileExist(PROBE_CONF))
    {
        fs->deleteFile(PROBE_CONF);
    }
    if (fs->isDirExist(PROBE_DIR))
    {
        fs->deleteDirectory(PROBE_DIR);
    }
    if (fs->isFileExist(SERVICE_ENABLE_CONFIG_FILE))
    {
        fs->deleteFile(SERVICE_ENABLE_CONFIG_FILE);
    }
    fs->endPrivileged();
}

} // namespace

TEST(serviceenable, a_service_names_its_own_config_file)
{
    pditest::mountedVfs();
    BorrowedSlot slot;
    ProbeService probe(false);

    pdiutil::string path;
    probe.getServiceConfigPath(path);
    ASSERT_STREQ(path.c_str(), PROBE_CONF);
}

TEST(serviceenable, a_service_with_no_config_runs)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;
    ProbeService probe(false);

    ASSERT_TRUE(probe.loadServiceEnabled());
    ASSERT_TRUE(probe.isServiceEnabled());
}

TEST(serviceenable, disabling_a_service_writes_its_config)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;
    ProbeService probe(false);

    ASSERT_TRUE(probe.setServiceEnabled(false));
    ASSERT_FALSE(probe.isServiceEnabled());

    pdiutil::string value;
    ASSERT_TRUE(getConfigValue(SERVICE_ENABLE_CONFIG_FILE, "probe", value));
    ASSERT_STREQ(value.c_str(), CONFIG_BOOL_NO);

    clearProbeConf();
}

TEST(serviceenable, the_enable_state_is_not_kept_in_the_feature_config)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;
    ProbeService probe(false);

    ASSERT_TRUE(probe.setServiceEnabled(false));

    VfsDispatcher *fs = pditest::mountedVfs();
    ASSERT_FALSE(fs->isFileExist(PROBE_CONF));

    clearProbeConf();
}

TEST(serviceenable, a_disabled_service_is_still_disabled_after_a_reload)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;

    {
        ProbeService probe(false);
        ASSERT_TRUE(probe.setServiceEnabled(false));
    }

    ProbeService reloaded(false);
    ASSERT_FALSE(reloaded.loadServiceEnabled());
    ASSERT_FALSE(reloaded.isServiceEnabled());

    clearProbeConf();
}

TEST(serviceenable, enabling_a_service_again_puts_it_back)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;

    {
        ProbeService probe(false);
        ASSERT_TRUE(probe.setServiceEnabled(false));
        ASSERT_TRUE(probe.setServiceEnabled(true));
    }

    ProbeService reloaded(false);
    ASSERT_TRUE(reloaded.loadServiceEnabled());

    clearProbeConf();
}

TEST(serviceenable, an_essential_service_refuses_to_be_disabled)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;
    ProbeService probe(true);

    ASSERT_FALSE(probe.setServiceEnabled(false));
    ASSERT_TRUE(probe.isServiceEnabled());
    ASSERT_FALSE(pditest::mountedVfs()->isFileExist(PROBE_CONF));
}

TEST(serviceenable, an_essential_service_runs_even_when_its_config_says_not_to)
{
    pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;

    {
        ProbeService writable(false);
        ASSERT_TRUE(writable.setServiceEnabled(false));
    }

    ProbeService essential(true);
    ASSERT_TRUE(essential.loadServiceEnabled());
    ASSERT_TRUE(essential.isServiceEnabled());

    clearProbeConf();
}

TEST(serviceenable, a_config_written_by_hand_is_honoured)
{
    VfsDispatcher *fs = pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;

    fs->createFile(SERVICE_ENABLE_CONFIG_FILE, "# hand written\r\nprobe off\r\n");

    ProbeService probe(false);
    ASSERT_FALSE(probe.loadServiceEnabled());

    clearProbeConf();
}

TEST(serviceenable, an_unreadable_value_leaves_the_service_running)
{
    VfsDispatcher *fs = pditest::mountedVfs();
    clearProbeConf();
    BorrowedSlot slot;

    fs->createFile(SERVICE_ENABLE_CONFIG_FILE, "probe perhaps\r\n");

    ProbeService probe(false);
    ASSERT_TRUE(probe.loadServiceEnabled());

    clearProbeConf();
}

#ifdef ENABLE_HTTP_SERVER

/**
 * The main loop serves web clients on every pass whether the service started
 * or not, so a disabled one has to survive being asked.
 */
TEST(serviceenable, serving_clients_is_safe_while_the_http_service_is_down)
{
    pditest::mountedVfs();
    BorrowedSlot slot(SERVICE_HTTP_SERVER);

    // never started, so it holds no listener at all
    HttpServer fresh;
    fresh.handle_clients();

    ASSERT_TRUE(SERVICE_STATE_ACTIVE != fresh.getServiceState());
}

#endif

#endif
