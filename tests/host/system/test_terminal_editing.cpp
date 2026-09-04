/*********************** Terminal Editing Tests *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The line editor between the keyboard and the command: echo, cursor movement,
erasing, history, completion and the interrupt keys.

Author          : Suraj I.
created Date    : 16th Aug 2026
******************************************************************************/

#include <ShellHarness.h>
#include <pditest.h>

using pditest::saw;

#define KEY_ENTER "\n"
#define KEY_BACKSPACE "\b"
#define KEY_DELETE "\x7F"
#define KEY_TAB "\t"
#define KEY_CTRL_C "\x03"
#define KEY_CTRL_Z "\x1A"
#define KEY_LEFT "\x1B[D"
#define KEY_RIGHT "\x1B[C"
#define KEY_UP "\x1B[A"
#define KEY_DOWN "\x1B[B"
#define KEY_HOME "\x1B[H"
#define KEY_END "\x1B[F"

/* ------------------------------------------------------------------- echo */

TEST(termedit, a_typed_character_is_echoed)
{
    pditest::Shell shell;
    std::string out = shell.type("abc");

    ASSERT_TRUE(saw(out, "abc"));
    ASSERT_STREQ(shell.lineBuffer().c_str(), "abc");
}

TEST(termedit, a_line_is_held_until_enter)
{
    pditest::Shell shell;

    shell.type("pwd");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "pwd");
    ASSERT_EQ(shell.result(), CMD_ERROR_AGAIN);

    std::string out = shell.type(KEY_ENTER);
    ASSERT_TRUE(saw(out, "/"));
}

TEST(termedit, the_line_buffer_is_cleared_after_a_command_runs)
{
    pditest::Shell shell;

    shell.type("pwd" KEY_ENTER);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, a_whole_command_typed_at_once_runs)
{
    pditest::Shell shell;
    std::string out = shell.type("echo typed through the editor" KEY_ENTER);

    ASSERT_TRUE(saw(out, "typed through the editor"));
}

/* --------------------------------------------------------------- erasing */

TEST(termedit, backspace_removes_the_last_character)
{
    pditest::Shell shell;

    shell.type("pwdx");
    shell.type(KEY_BACKSPACE);
    ASSERT_STREQ(shell.lineBuffer().c_str(), "pwd");

    ASSERT_TRUE(saw(shell.type(KEY_ENTER), "/"));
}

TEST(termedit, delete_at_the_end_behaves_like_backspace)
{
    pditest::Shell shell;

    shell.type("lsx");
    shell.type(KEY_DELETE);
    ASSERT_STREQ(shell.lineBuffer().c_str(), "ls");
}

TEST(termedit, backspace_on_an_empty_line_does_nothing)
{
    pditest::Shell shell;

    shell.type(KEY_BACKSPACE);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, every_character_can_be_erased)
{
    pditest::Shell shell;

    shell.type("abcd");
    shell.type(KEY_BACKSPACE KEY_BACKSPACE KEY_BACKSPACE KEY_BACKSPACE);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

/* --------------------------------------------------------- cursor movement */

TEST(termedit, a_character_can_be_inserted_in_the_middle)
{
    pditest::Shell shell;

    shell.type("pd");
    shell.type(KEY_LEFT);
    shell.type("w");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "pwd");

    ASSERT_TRUE(saw(shell.type(KEY_ENTER), "/"));
}

TEST(termedit, the_cursor_stops_at_the_start_of_the_line)
{
    pditest::Shell shell;

    shell.type("ab");
    shell.type(KEY_LEFT KEY_LEFT KEY_LEFT KEY_LEFT);
    shell.type("X");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "Xab");
}

TEST(termedit, the_cursor_stops_at_the_end_of_the_line)
{
    pditest::Shell shell;

    shell.type("ab");
    shell.type(KEY_RIGHT KEY_RIGHT);
    shell.type("X");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "abX");
}

TEST(termedit, home_and_end_jump_to_the_ends)
{
    pditest::Shell shell;

    shell.type("bc");
    shell.type(KEY_HOME);
    shell.type("a");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "abc");

    shell.type(KEY_END);
    shell.type("d");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "abcd");
}

TEST(termedit, backspace_in_the_middle_removes_the_character_before_the_cursor)
{
    pditest::Shell shell;

    shell.type("axbc");
    shell.type(KEY_LEFT KEY_LEFT);
    shell.type(KEY_BACKSPACE);
    ASSERT_STREQ(shell.lineBuffer().c_str(), "abc");
}

/* -------------------------------------------------------------- interrupt */

TEST(termedit, ctrl_c_abandons_the_line)
{
    pditest::Shell shell;

    shell.type("half typed");
    shell.type(KEY_CTRL_C);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, ctrl_z_abandons_the_line)
{
    pditest::Shell shell;

    shell.type("half typed");
    shell.type(KEY_CTRL_Z);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, a_line_can_be_retyped_after_an_interrupt)
{
    pditest::Shell shell;

    shell.type("wrong");
    shell.type(KEY_CTRL_C);
    ASSERT_TRUE(saw(shell.type("pwd" KEY_ENTER), "/"));
}

/* ---------------------------------------------------------------- history */

TEST(termedit, the_previous_command_comes_back_on_the_up_arrow)
{
    pditest::Shell shell;
    shell.type("pwd" KEY_ENTER);

    shell.type(KEY_UP);
    ASSERT_STREQ(shell.lineBuffer().c_str(), "pwd");
}

TEST(termedit, the_recalled_command_runs)
{
    pditest::Shell shell;
    shell.type("pwd" KEY_ENTER);

    shell.type(KEY_UP);
    ASSERT_TRUE(saw(shell.type(KEY_ENTER), "/"));
}

TEST(termedit, the_down_arrow_walks_back_towards_the_newest)
{
    pditest::Shell shell;
    shell.type("pwd" KEY_ENTER);
    shell.type("uptime" KEY_ENTER);

    shell.type(KEY_UP);
    std::string first = shell.lineBuffer();
    shell.type(KEY_UP);
    std::string second = shell.lineBuffer();
    shell.type(KEY_DOWN);
    std::string back = shell.lineBuffer();

    ASSERT_STRNE(first.c_str(), second.c_str());
    ASSERT_STREQ(back.c_str(), first.c_str());

    shell.type(KEY_CTRL_C);
}

TEST(termedit, history_survives_into_a_new_session)
{
    {
        pditest::Shell first;
        first.type("uptime" KEY_ENTER);
    }

    pditest::Shell second;
    second.type(KEY_UP);
    ASSERT_TRUE(second.lineBuffer().length() > 0);

    second.type(KEY_CTRL_C);
}

/* ------------------------------------------------------------- completion */

TEST(termedit, tab_completes_a_command_prefix)
{
    pditest::Shell shell;

    shell.type("upt");
    shell.type(KEY_TAB);

    // the one match is finished off and a space follows it, so the next word
    // can be typed straight away
    ASSERT_STREQ(shell.lineBuffer().c_str(), "uptime ");

    shell.type(KEY_CTRL_C);
}

/**
 * Several matches share a start, so the shared part is typed and no more. A
 * second ask has nothing left to add and shows what is on offer instead.
 */
TEST(termedit, tab_types_what_every_match_agrees_on_then_lists_them)
{
    pditest::Shell shell;

    shell.type("k");
    shell.type(KEY_TAB);

    std::string completed = shell.lineBuffer();
    ASSERT_STREQ(completed.c_str(), "kill");

    shell.terminal().forget();
    shell.type(KEY_TAB);

    // the line is left exactly as it was, and the candidates are written below
    ASSERT_STREQ(shell.lineBuffer().c_str(), "kill");
    ASSERT_TRUE(shell.terminal().captured().find("killall") != std::string::npos);

    shell.type(KEY_CTRL_C);
}

/**
 * Asking for a completion must never run the line, whether or not anything
 * matched.
 */
TEST(termedit, tab_on_a_prefix_that_matches_nothing_leaves_the_line_alone)
{
    pditest::Shell shell;

    shell.type("zzz");
    shell.terminal().forget();
    shell.type(KEY_TAB);

    ASSERT_EQ(shell.result(), CMD_ERROR_HOLD_BUFFER);
    ASSERT_STREQ(shell.lineBuffer().c_str(), "zzz");
    ASSERT_EQ(shell.terminal().captured().length(), 0u);

    shell.type(KEY_CTRL_C);
}

TEST(termedit, tab_on_an_empty_line_does_nothing)
{
    pditest::Shell shell;
    shell.terminal().forget();

    shell.type(KEY_TAB);

    ASSERT_EQ(shell.result(), CMD_ERROR_HOLD_BUFFER);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
    ASSERT_EQ(shell.terminal().captured().length(), 0u);
}

/**
 * Builds a directory whose entries share a start, so a completion has
 * something to agree on and something to stop at.
 */
static void makeCompletionDir()
{
    if (!__i_fs.isDirectory("/tabdir"))
    {
        __i_fs.createDirectory("/tabdir");
    }
    __i_fs.writeFile("/tabdir/alpha.txt", "a", 1, false);
    __i_fs.writeFile("/tabdir/alphabet.txt", "a", 1, false);
}

/**
 * A path is completed one component at a time, so the leading directories are
 * followed rather than searched for in the working directory.
 */
TEST(termedit, tab_descends_into_the_directory_the_argument_names)
{
    pditest::Shell shell;
    makeCompletionDir();

    shell.type("cat /tabdir/al");
    shell.type(KEY_TAB);

    // both entries start "alpha" and disagree after it, so that is as far as
    // the completion can go
    ASSERT_STREQ(shell.lineBuffer().c_str(), "cat /tabdir/alpha");

    shell.type(KEY_CTRL_C);
}

/**
 * Completion works on the argument the cursor is in, not only on the first
 * one, and what is already typed before it survives.
 */
TEST(termedit, tab_completes_a_later_argument_and_keeps_the_earlier_one)
{
    pditest::Shell shell;
    makeCompletionDir();

    shell.type("cp /tabdir/alpha.txt /tabdir/al");
    shell.type(KEY_TAB);

    ASSERT_STREQ(shell.lineBuffer().c_str(), "cp /tabdir/alpha.txt /tabdir/alpha");

    shell.type(KEY_CTRL_C);
}

/**
 * A command says what its arguments name, so the service command offers
 * services where another command would have offered files.
 */
TEST(termedit, tab_offers_services_to_the_service_command)
{
    pditest::Shell shell;

    // whichever service the build registered first is the one to ask for
    pdiutil::string name;
    for (uint8_t i = 0; i < SERVICE_MAX && name.empty(); i++)
    {
        ServiceProvider *s = ServiceProvider::getService((service_t)i);
        if (nullptr == s || nullptr == s->m_service_name) continue;

        char buf[SERVICE_NAME_MAX];
        memset(buf, 0, SERVICE_NAME_MAX);
        uint32_t len = strlen_ro(s->m_service_name);
        if (len > SERVICE_NAME_MAX - 1) len = SERVICE_NAME_MAX - 1;
        memcpy_ro(buf, s->m_service_name, len);
        name = buf;
    }
    ASSERT_TRUE(!name.empty());

    shell.type("service start ");
    shell.type(name.substr(0, 1).c_str());
    shell.terminal().forget();
    shell.type(KEY_TAB);

    // a path completion would have searched the working directory for that
    // letter, found nothing, and left the line untouched
    std::string typed = std::string("service start ") + name[0];
    ASSERT_TRUE(shell.lineBuffer().find(typed) == 0);
    ASSERT_TRUE(shell.lineBuffer().length() > typed.length() ||
                shell.terminal().captured().find(name.c_str()) != std::string::npos);

    shell.type(KEY_CTRL_C);
}

TEST(termedit, tab_after_an_argument_that_matches_no_file_keeps_the_line)
{
    pditest::Shell shell;

    shell.type("cat /nosuchdir/nosuchfile");
    shell.terminal().forget();
    shell.type(KEY_TAB);

    ASSERT_STREQ(shell.lineBuffer().c_str(), "cat /nosuchdir/nosuchfile");

    shell.type(KEY_CTRL_C);
}

/**
 * A command waiting for input owns the key, exactly as it owns a typed line.
 */
TEST(termedit, tab_still_reaches_a_command_that_is_waiting)
{
    pditest::Shell shell;

    shell.type("su" KEY_ENTER);
    shell.terminal().forget();
    shell.type(KEY_TAB);

    ASSERT_EQ(shell.result(), CMD_ERROR_AGAIN);
    ASSERT_TRUE(saw(shell.terminal().captured(), "user"));

    shell.type(KEY_CTRL_C);
}

TEST(termedit, a_completed_command_runs)
{
    pditest::Shell shell;

    shell.type("pw");
    shell.type(KEY_TAB);
    ASSERT_TRUE(saw(shell.type(KEY_ENTER), "/"));
}

/* ------------------------------------------------------- masked and prompts */

TEST(termedit, a_command_waiting_for_input_takes_the_next_line)
{
    pditest::Shell shell;

    std::string prompt = shell.type("su" KEY_ENTER);
    ASSERT_TRUE(saw(prompt, "user"));

    shell.type("nobodyhere" KEY_ENTER);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, a_masked_prompt_does_not_echo_what_is_typed)
{
    pditest::Shell shell;

    shell.type("su" KEY_ENTER);
    shell.type("someuser" KEY_ENTER);

    std::string out = shell.type("secretpassword");
    ASSERT_FALSE(saw(out, "secretpassword"));

    shell.type(KEY_CTRL_C);
}

/* --------------------------------------------------------- line endings */

static size_t promptsIn(const std::string &out)
{
    size_t seen = 0;
    for (size_t at = out.find('@'); at != std::string::npos; at = out.find('@', at + 1))
    {
        seen++;
    }
    return seen;
}

TEST(termedit, a_lone_line_feed_is_one_enter)
{
    pditest::Shell shell;
    std::string out = shell.type("pwd\n");

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(promptsIn(out), 1u);
}

TEST(termedit, a_lone_carriage_return_is_one_enter)
{
    pditest::Shell shell;
    std::string out = shell.type("pwd\r");

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(promptsIn(out), 1u);
}

TEST(termedit, a_carriage_return_line_feed_pair_is_one_enter)
{
    pditest::Shell shell;
    std::string out = shell.type("pwd\r\n");

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(promptsIn(out), 1u);
}

TEST(termedit, a_line_feed_carriage_return_pair_is_one_enter)
{
    pditest::Shell shell;
    std::string out = shell.type("pwd\n\r");

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(promptsIn(out), 1u);
}

TEST(termedit, the_pair_is_split_across_reads_and_still_one_enter)
{
    pditest::Shell shell;

    std::string first = shell.type("pwd\r");
    std::string second = shell.type("\n");

    ASSERT_TRUE(saw(first, "/"));
    ASSERT_EQ(promptsIn(first) + promptsIn(second), 1u);
}

TEST(termedit, two_of_the_same_ending_stay_two_enters)
{
    pditest::Shell shell;
    std::string out = shell.type("pwd\n\n");

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(promptsIn(out), 2u);
}

/* ------------------------------------------------------------- stray bytes */

TEST(termedit, a_stray_ff_byte_is_not_a_typed_character)
{
    pditest::Shell shell;

    shell.type("\xFF");
    ASSERT_EQ(shell.lineBuffer().length(), 0u);

    shell.type("pwd");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "pwd");
}

TEST(termedit, a_stray_byte_before_a_command_leaves_the_command_alone)
{
    pditest::Shell shell;

    std::string out = shell.type("\xFF" "pwd" KEY_ENTER);

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, an_unhandled_control_byte_is_dropped)
{
    pditest::Shell shell;

    shell.type("a\x01\x02" "b");
    ASSERT_STREQ(shell.lineBuffer().c_str(), "ab");
}

TEST(termedit, a_carriage_return_nul_pair_is_one_enter)
{
    pditest::Shell shell;

    static const uint8_t keys[] = { 'p', 'w', 'd', '\r', 0x00 };
    std::string out = shell.type(keys, sizeof(keys));

    ASSERT_TRUE(saw(out, "/"));
    ASSERT_EQ(promptsIn(out), 1u);
    ASSERT_EQ(shell.lineBuffer().length(), 0u);
}

TEST(termedit, the_nul_after_a_carriage_return_starts_no_line)
{
    pditest::Shell shell;

    static const uint8_t keys[] = { '\r', 0x00, 'p', 'w', 'd' };
    shell.type(keys, sizeof(keys));

    ASSERT_STREQ(shell.lineBuffer().c_str(), "pwd");
}
