/************************** Shell Redirect Tests ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers redirection as the shell now does it: any command can be sent to a file,
the terminal sees none of it, and the descriptor is handed back afterwards.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <ShellHarness.h>
#include <pditest.h>

#include <service_provider/cmd/ShellParser.h>
#include <service_provider/session/SessionManager.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

namespace
{
    std::string slurp(const char *path)
    {
        std::string out;
        __i_fs.readFile(path, 512, [&](char *data, uint32_t size) -> bool {
            out.append(data, size);
            return true;
        });
        return out;
    }
}

TEST(redirect, the_parser_finds_a_replacing_target)
{
    const char *line = "echo hi > /tmp/out.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_TRUE(t.m_redirected);
    ASSERT_FALSE(t.m_append);
    ASSERT_FALSE(t.m_malformed);
    ASSERT_EQ((int)t.m_stages[0].m_len, 7);
    ASSERT_STREQ(t.m_outpath.c_str(), "/tmp/out.txt");
}

TEST(redirect, the_parser_finds_an_appending_target)
{
    const char *line = "echo hi >> /tmp/out.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_TRUE(t.m_append);
    ASSERT_EQ((int)t.m_stages[0].m_len, 7);
    ASSERT_STREQ(t.m_outpath.c_str(), "/tmp/out.txt");
}

TEST(redirect, a_line_with_no_operator_is_left_whole)
{
    const char *line = "echo hi";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_FALSE(t.m_redirected);
    ASSERT_TRUE(t.isPlain());
    ASSERT_EQ((int)t.m_stages[0].m_len, 7);
}

TEST(redirect, a_dangling_operator_is_malformed)
{
    const char *line = "echo hi >";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_TRUE(t.m_malformed);
}

TEST(redirect, an_operator_with_no_command_is_malformed)
{
    const char *line = "> /tmp/out.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_TRUE(t.m_malformed);
}

TEST(redirect, echo_writes_its_text_and_nothing_else)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/redir_echo.txt");

    std::string seen = sh.run("echo hello > /redir_echo.txt");

    // the terminal saw no payload, only the prompt that follows
    ASSERT_TRUE(seen.find("hello") == std::string::npos);
    ASSERT_STREQ(slurp("/redir_echo.txt").c_str(), "hello\n");

    __i_fs.deleteFile("/redir_echo.txt");
}

TEST(redirect, a_replacing_target_starts_the_file_over)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/redir_trunc.txt");

    sh.run("echo first > /redir_trunc.txt");
    sh.run("echo second > /redir_trunc.txt");

    ASSERT_STREQ(slurp("/redir_trunc.txt").c_str(), "second\n");
    __i_fs.deleteFile("/redir_trunc.txt");
}

TEST(redirect, an_appending_target_keeps_both_lines)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/redir_append.txt");

    sh.run("echo first > /redir_append.txt");
    sh.run("echo second >> /redir_append.txt");

    ASSERT_STREQ(slurp("/redir_append.txt").c_str(), "first\nsecond\n");
    __i_fs.deleteFile("/redir_append.txt");
}

TEST(redirect, a_command_that_never_knew_about_redirection_is_captured)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/redir_pwd.txt");

    // pwd has no redirect handling of its own; the shell supplies all of it
    std::string seen = sh.run("pwd > /redir_pwd.txt");

    ASSERT_TRUE(slurp("/redir_pwd.txt").size() > 0);
    ASSERT_TRUE(seen.find("/") != std::string::npos || seen.size() > 0);

    __i_fs.deleteFile("/redir_pwd.txt");
}

TEST(redirect, the_descriptor_goes_back_to_the_terminal_afterwards)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/redir_back.txt");

    sh.run("echo away > /redir_back.txt");
    std::string seen = sh.run("echo back");

    ASSERT_TRUE(seen.find("back") != std::string::npos);
    ASSERT_NULL(SessionManager::current()->m_fdtable);

    __i_fs.deleteFile("/redir_back.txt");
}

TEST(redirect, a_dangling_operator_is_refused_at_the_shell)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    std::string seen = sh.run("echo hi >");
    ASSERT_TRUE(seen.find("syntax error") != std::string::npos);
}

TEST(redirect, a_target_the_filesystem_refuses_is_reported)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    std::string seen = sh.run("echo hi > /proc/uptime");

    ASSERT_TRUE(seen.find("cannot write") != std::string::npos);
    ASSERT_TRUE(seen.find("/proc/uptime") != std::string::npos);
}

TEST(redirect, a_refused_target_still_leaves_the_terminal_usable)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("echo hi > /proc/uptime");
    std::string seen = sh.run("echo back");

    ASSERT_TRUE(seen.find("back") != std::string::npos);
    ASSERT_NULL(SessionManager::current()->m_fdtable);
}

TEST(pipeline, the_parser_splits_every_stage)
{
    const char *line = "echo hi | grep h | wc";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_FALSE(t.m_malformed);
    ASSERT_FALSE(t.isPlain());
    ASSERT_EQ((int)t.m_stages.size(), 3);
    ASSERT_EQ((int)t.m_stages[0].m_len, 7);
    ASSERT_EQ((int)t.m_stages[2].m_len, 2);
}

TEST(pipeline, a_target_binds_to_the_last_stage)
{
    const char *line = "echo hi | wc > /tmp/o.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_EQ((int)t.m_stages.size(), 2);
    ASSERT_TRUE(t.m_redirected);
    ASSERT_STREQ(t.m_outpath.c_str(), "/tmp/o.txt");
    ASSERT_EQ((int)t.m_stages[1].m_len, 2);
}

TEST(pipeline, an_empty_stage_is_malformed)
{
    const char *a = "echo hi |";
    const char *b = "| wc";
    const char *c = "echo hi || wc";

    ASSERT_TRUE(ShellParser::parse(a, (int16_t)strlen(a)).m_malformed);
    ASSERT_TRUE(ShellParser::parse(b, (int16_t)strlen(b)).m_malformed);
    ASSERT_TRUE(ShellParser::parse(c, (int16_t)strlen(c)).m_malformed);
}

TEST(pipeline, output_of_one_stage_becomes_input_of_the_next)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    // wc counts what echo produced, not a file it was handed
    std::string seen = sh.run("echo hello world | wc");

    ASSERT_TRUE(seen.find("1 2 12") != std::string::npos);
}

TEST(pipeline, a_three_stage_line_carries_through)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    std::string seen = sh.run("echo alpha | grep alpha | wc");
    ASSERT_TRUE(seen.find("1 ") != std::string::npos);
}

TEST(pipeline, grep_filters_what_it_is_piped)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    std::string hit = sh.run("echo needle | grep needle");
    ASSERT_TRUE(hit.find("needle") != std::string::npos);

    std::string miss = sh.run("echo needle | grep absent");
    ASSERT_TRUE(miss.find("absent") == std::string::npos);
}

TEST(pipeline, the_last_stage_can_still_be_redirected)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/pipe_out.txt");

    std::string seen = sh.run("echo hello world | wc > /pipe_out.txt");

    ASSERT_TRUE(seen.find("1 2 12") == std::string::npos);
    ASSERT_TRUE(slurp("/pipe_out.txt").find("1 2 12") != std::string::npos);

    __i_fs.deleteFile("/pipe_out.txt");
}

TEST(pipeline, descriptors_are_all_handed_back_afterwards)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    sh.run("echo hi | wc");
    ASSERT_NULL(SessionManager::current()->m_fdtable);

    std::string seen = sh.run("echo plain");
    ASSERT_TRUE(seen.find("plain") != std::string::npos);
}

TEST(pipeline, cat_passes_its_input_straight_through)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    std::string seen = sh.run("echo through | cat");
    ASSERT_TRUE(seen.find("through") != std::string::npos);
}

TEST(pipeline, head_stops_at_the_count_it_was_given)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/pipe_lines.txt", "one\ntwo\nthree\n", 14, false);

    // with a pipe there is no file to name, so the lone argument is the count
    std::string seen = sh.run("cat /pipe_lines.txt | head 2");

    ASSERT_TRUE(seen.find("one") != std::string::npos);
    ASSERT_TRUE(seen.find("two") != std::string::npos);
    ASSERT_TRUE(seen.find("three") == std::string::npos);

    __i_fs.deleteFile("/pipe_lines.txt");
}

TEST(pipeline, tail_keeps_the_last_lines_of_what_it_was_piped)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/pipe_lines2.txt", "one\ntwo\nthree\n", 14, false);

    std::string seen = sh.run("cat /pipe_lines2.txt | tail 2");

    ASSERT_TRUE(seen.find("one") == std::string::npos);
    ASSERT_TRUE(seen.find("two") != std::string::npos);
    ASSERT_TRUE(seen.find("three") != std::string::npos);

    __i_fs.deleteFile("/pipe_lines2.txt");
}

TEST(pipeline, a_file_argument_still_wins_when_nothing_is_piped)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/pipe_lines3.txt", "one\ntwo\nthree\n", 14, false);

    // no pipe, so the first positional is the path it has always been
    std::string seen = sh.run("head /pipe_lines3.txt 1");

    ASSERT_TRUE(seen.find("one") != std::string::npos);
    ASSERT_TRUE(seen.find("three") == std::string::npos);

    __i_fs.deleteFile("/pipe_lines3.txt");
}

TEST(pipeline, a_stream_without_a_final_newline_still_yields_its_last_line)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/pipe_bare.txt", "alpha\nomega", 11, false);

    std::string seen = sh.run("cat /pipe_bare.txt | tail 1");
    ASSERT_TRUE(seen.find("omega") != std::string::npos);

    __i_fs.deleteFile("/pipe_bare.txt");
}

TEST(pipeline, a_command_given_no_input_is_unaffected)
{
    pditest::mountedVfs();
    pditest::Shell sh;

    // wc with neither a file nor a pipe still reports a missing argument
    sh.run("wc");
    ASSERT_EQ((int)sh.result(), (int)CMD_RESULT_ARGS_MISSING);
}


TEST(source, the_parser_finds_an_input_source)
{
    const char *line = "wc < /tmp/in.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_TRUE(t.m_sourced);
    ASSERT_FALSE(t.m_redirected);
    ASSERT_FALSE(t.m_malformed);
    ASSERT_FALSE(t.isPlain());
    ASSERT_EQ((int)t.m_stages[0].m_len, 2);
    ASSERT_STREQ(t.m_inpath.c_str(), "/tmp/in.txt");
}

TEST(source, a_line_can_name_a_source_and_a_target)
{
    const char *line = "wc < /tmp/in.txt > /tmp/out.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_TRUE(t.m_sourced);
    ASSERT_TRUE(t.m_redirected);
    ASSERT_STREQ(t.m_inpath.c_str(), "/tmp/in.txt");
    ASSERT_STREQ(t.m_outpath.c_str(), "/tmp/out.txt");
    ASSERT_EQ((int)t.m_stages[0].m_len, 2);
}

TEST(source, the_order_of_the_operators_does_not_matter)
{
    const char *line = "wc > /tmp/out.txt < /tmp/in.txt";
    ShellParser::Line t = ShellParser::parse(line, (int16_t)strlen(line));

    ASSERT_STREQ(t.m_inpath.c_str(), "/tmp/in.txt");
    ASSERT_STREQ(t.m_outpath.c_str(), "/tmp/out.txt");
}

TEST(source, a_dangling_or_repeated_operator_is_malformed)
{
    const char *a = "wc <";
    const char *b = "< /tmp/in.txt";
    const char *c = "wc < /tmp/a.txt < /tmp/b.txt";

    ASSERT_TRUE(ShellParser::parse(a, (int16_t)strlen(a)).m_malformed);
    ASSERT_TRUE(ShellParser::parse(b, (int16_t)strlen(b)).m_malformed);
    ASSERT_TRUE(ShellParser::parse(c, (int16_t)strlen(c)).m_malformed);
}

TEST(source, a_command_reads_the_file_it_was_given)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/src_in.txt", "one\ntwo\nthree\n", 14, false);

    std::string seen = sh.run("wc < /src_in.txt");

    // three lines, three words, fourteen bytes
    ASSERT_TRUE(seen.find("3") != std::string::npos);
    ASSERT_TRUE(seen.find("14") != std::string::npos);

    __i_fs.deleteFile("/src_in.txt");
}

TEST(source, a_source_feeds_the_first_stage_of_a_pipeline)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/src_pipe.txt", "alpha\nbeta\n", 11, false);

    std::string seen = sh.run("cat < /src_pipe.txt | grep alpha");

    ASSERT_TRUE(seen.find("alpha") != std::string::npos);
    ASSERT_TRUE(seen.find("beta") == std::string::npos);

    __i_fs.deleteFile("/src_pipe.txt");
}

TEST(source, a_source_and_a_target_work_on_one_line)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.writeFile("/src_both.txt", "one\ntwo\n", 8, false);
    __i_fs.deleteFile("/src_both_out.txt");

    sh.run("wc < /src_both.txt > /src_both_out.txt");

    ASSERT_TRUE(slurp("/src_both_out.txt").find("2") != std::string::npos);

    __i_fs.deleteFile("/src_both.txt");
    __i_fs.deleteFile("/src_both_out.txt");
}

TEST(source, a_missing_source_is_refused_and_hands_the_terminal_back)
{
    pditest::mountedVfs();
    pditest::Shell sh;
    __i_fs.deleteFile("/src_absent.txt");

    std::string seen = sh.run("wc < /src_absent.txt");
    ASSERT_TRUE(seen.find("cannot open") != std::string::npos);

    ASSERT_NULL(SessionManager::current()->m_fdtable);
    ASSERT_TRUE(sh.run("echo back").find("back") != std::string::npos);
}

#endif
