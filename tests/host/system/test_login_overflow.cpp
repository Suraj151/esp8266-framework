/************************ Login Overflow Repro ********************************
Drives the interactive login the way a telnet client does: the command, then a
username at its prompt, then a password at its prompt.
******************************************************************************/

#include <ShellHarness.h>
#include <pditest.h>

TEST(loginof, a_long_username_at_the_prompt_does_not_smash_the_stack)
{
    pditest::Shell shell;
    shell.run("logout");

    shell.type("login\n");
    shell.type("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
    shell.type("bbbb\n");

    ASSERT_FALSE(__auth_service.getAuthorized());
}

TEST(loginof, a_long_password_at_the_prompt_does_not_smash_the_stack)
{
    pditest::Shell shell;
    shell.run("logout");

    shell.type("login\n");
    shell.type("pdiStack\n");
    shell.type("cccccccccccccccccccccccccccccccccccccccc\n");

    ASSERT_FALSE(__auth_service.getAuthorized());
}

TEST(loginof, an_empty_answer_at_each_prompt_is_survivable)
{
    pditest::Shell shell;
    shell.run("logout");

    shell.type("login\n");
    shell.type("\n");
    shell.type("\n");
    shell.type("\n");

    ASSERT_FALSE(__auth_service.getAuthorized());
}

TEST(loginof, long_inline_credentials_do_not_smash_the_stack)
{
    pditest::Shell shell;
    shell.run("logout");

    shell.type("login u=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,p=bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");

    ASSERT_FALSE(__auth_service.getAuthorized());
}
