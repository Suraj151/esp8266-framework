/*************************** Exit Status Tests ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers the result a session keeps of the last command it ran to completion: it
follows what the command answered, and a command that has not finished leaves it
where it was.

Author          : Suraj I.
created Date    : 28th Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <ShellHarness.h>
#include <pditest.h>

#include <service_provider/session/SessionManager.h>

#ifdef ENABLE_CMD_SERVICE

TEST(exitstatus, a_fresh_session_starts_at_success)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    ASSERT_EQ((int)SessionManager::getLastExit(), (int)PDI_OK);
}

TEST(exitstatus, a_command_that_succeeds_leaves_success)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("pwd");
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)PDI_OK);
}

TEST(exitstatus, an_unknown_command_leaves_its_code)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("nosuchcommand");
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)CMD_ERROR_NOENT);
}

TEST(exitstatus, a_failure_replaces_an_earlier_success)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("pwd");
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)PDI_OK);

    sh.run("nosuchcommand");
    ASSERT_TRUE(SessionManager::getLastExit() != PDI_OK);
}

TEST(exitstatus, a_success_clears_an_earlier_failure)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("nosuchcommand");
    ASSERT_TRUE(SessionManager::getLastExit() != PDI_OK);

    sh.run("pwd");
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)PDI_OK);
}

TEST(exitstatus, a_command_still_running_does_not_overwrite_the_last_result)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("nosuchcommand");
    pdi_err_t before = SessionManager::getLastExit();

    // neither of these is an outcome, so neither may land on the record
    SessionManager::setLastExit(CMD_ERROR_AGAIN);
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)before);

    SessionManager::setLastExit(CMD_ERROR_HOLD_BUFFER);
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)before);
}

TEST(exitstatus, each_session_keeps_its_own_result)
{
    pditest::mountedVfs();
    pditest::Shell first;
    first.run("nosuchcommand");
    pdi_err_t firstExit = SessionManager::getLastExit();
    ASSERT_TRUE(firstExit != PDI_OK);

    {
        pditest::Shell second;
        second.run("pwd");
        ASSERT_EQ((int)SessionManager::getLastExit(), (int)PDI_OK);
    }

    first.run("pwd");
    ASSERT_EQ((int)SessionManager::getLastExit(), (int)PDI_OK);
}

#endif
