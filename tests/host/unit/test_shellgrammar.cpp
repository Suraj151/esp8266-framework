/************************** Shell Grammar Tests *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers the tokenizer and the grammar over it: quoting and escapes, the operators
and how tightly each binds, and the redirections that belong to one command
rather than to the line.

Author          : Suraj I.
created Date    : 28th Aug 2026
******************************************************************************/

#include <pditest.h>

#include <service_provider/cmd/ShellParser.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

namespace
{
    ShellParser::Line parseOf(const char *line)
    {
        return ShellParser::parse(line, (int16_t)strlen(line));
    }

    std::string textOf(const ShellParser::Line &p, const char *line, int16_t index)
    {
        pdiutil::string out;
        p.commandText(line, index, out);
        return std::string(out.c_str());
    }

    std::string spanOf(const char *line, int16_t start, int16_t len)
    {
        return std::string(line + start, (size_t)len);
    }
}

// ---------------------------------------------------------------- tokenizer

TEST(shellgrammar, words_split_on_blanks)
{
    pdiutil::vector<ShellParser::Token> tokens;
    const char *line = "ls  -l   /tmp";

    ASSERT_TRUE(ShellParser::tokenize(line, (int16_t)strlen(line), tokens));
    ASSERT_EQ((int)tokens.size(), 3);
    ASSERT_STREQ(spanOf(line, tokens[1].m_start, tokens[1].m_len).c_str(), "-l");
}

TEST(shellgrammar, each_operator_is_its_own_token)
{
    pdiutil::vector<ShellParser::Token> tokens;
    const char *line = "a | b && c || d ; e > f >> g < h";

    ASSERT_TRUE(ShellParser::tokenize(line, (int16_t)strlen(line), tokens));

    int pipes = 0, andif = 0, orif = 0, semi = 0, gt = 0, dgt = 0, lt = 0;
    for (uint16_t i = 0; i < tokens.size(); i++) {
        switch (tokens[i].m_type) {
            case ShellParser::TOKEN_PIPE: pipes++; break;
            case ShellParser::TOKEN_AND_IF: andif++; break;
            case ShellParser::TOKEN_OR_IF: orif++; break;
            case ShellParser::TOKEN_SEMI: semi++; break;
            case ShellParser::TOKEN_GT: gt++; break;
            case ShellParser::TOKEN_DGT: dgt++; break;
            case ShellParser::TOKEN_LT: lt++; break;
            default: break;
        }
    }

    ASSERT_EQ(pipes, 1);
    ASSERT_EQ(andif, 1);
    ASSERT_EQ(orif, 1);
    ASSERT_EQ(semi, 1);
    ASSERT_EQ(gt, 1);
    ASSERT_EQ(dgt, 1);
    ASSERT_EQ(lt, 1);
}

TEST(shellgrammar, an_operator_needs_no_space_around_it)
{
    pdiutil::vector<ShellParser::Token> tokens;
    const char *line = "echo hi>out.txt";

    ASSERT_TRUE(ShellParser::tokenize(line, (int16_t)strlen(line), tokens));
    ASSERT_EQ((int)tokens.size(), 4);
    ASSERT_EQ((int)tokens[2].m_type, (int)ShellParser::TOKEN_GT);
}

TEST(shellgrammar, a_quoted_run_is_one_word)
{
    pdiutil::vector<ShellParser::Token> tokens;
    const char *line = "echo \"a b c\"";

    ASSERT_TRUE(ShellParser::tokenize(line, (int16_t)strlen(line), tokens));
    ASSERT_EQ((int)tokens.size(), 2);
    ASSERT_STREQ(spanOf(line, tokens[1].m_start, tokens[1].m_len).c_str(), "\"a b c\"");
}

TEST(shellgrammar, an_unterminated_quote_is_refused)
{
    pdiutil::vector<ShellParser::Token> tokens;
    const char *line = "echo \"never closed";

    ASSERT_TRUE(!ShellParser::tokenize(line, (int16_t)strlen(line), tokens));
}

TEST(shellgrammar, a_backslash_keeps_the_next_character)
{
    pdiutil::vector<ShellParser::Token> tokens;
    const char *line = "echo a\\ b";

    ASSERT_TRUE(ShellParser::tokenize(line, (int16_t)strlen(line), tokens));
    ASSERT_EQ((int)tokens.size(), 2);
}

// ------------------------------------------------------ the fixed defects

TEST(shellgrammar, an_operator_inside_quotes_is_text)
{
    // all three used to be read as operators
    ShellParser::Line redirect = parseOf("echo \"a > b\"");
    ASSERT_TRUE(!redirect.m_malformed);
    ASSERT_EQ((int)redirect.m_redirects.size(), 0);

    ShellParser::Line andif = parseOf("echo \"a && b\"");
    ASSERT_TRUE(!andif.m_malformed);
    ASSERT_EQ((int)andif.m_pipelines.size(), 1);

    ShellParser::Line pipe = parseOf("echo \"a | b\"");
    ASSERT_TRUE(!pipe.m_malformed);
    ASSERT_EQ((int)pipe.m_commands.size(), 1);
}

TEST(shellgrammar, a_pipe_after_a_redirect_target_still_pipes)
{
    // the target used to swallow the rest of the line as part of its name
    const char *line = "echo a > f | b";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_EQ((int)p.m_commands.size(), 2);

    const ShellParser::Redirect *out = p.findRedirect(0, ShellParser::REDIRECT_OUT);
    ASSERT_TRUE(nullptr != out);
    ASSERT_STREQ(spanOf(line, out->m_start, out->m_len).c_str(), "f");
    ASSERT_STREQ(textOf(p, line, 1).c_str(), "b");
}

TEST(shellgrammar, a_redirect_belongs_to_its_own_command)
{
    const char *line = "cat a > one | grep x > two";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_EQ((int)p.m_commands.size(), 2);

    const ShellParser::Redirect *first = p.findRedirect(0, ShellParser::REDIRECT_OUT);
    const ShellParser::Redirect *second = p.findRedirect(1, ShellParser::REDIRECT_OUT);

    ASSERT_TRUE(nullptr != first && nullptr != second);
    ASSERT_STREQ(spanOf(line, first->m_start, first->m_len).c_str(), "one");
    ASSERT_STREQ(spanOf(line, second->m_start, second->m_len).c_str(), "two");
}

// -------------------------------------------------------------- structure

TEST(shellgrammar, a_plain_line_is_one_command)
{
    const char *line = "ls /tmp";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_TRUE(p.isPlain());
    ASSERT_EQ((int)p.m_pipelines.size(), 1);
    ASSERT_STREQ(textOf(p, line, 0).c_str(), "ls /tmp");
}

TEST(shellgrammar, a_pipe_binds_tighter_than_the_separators)
{
    const char *line = "cat f | grep x && echo done";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_EQ((int)p.m_pipelines.size(), 2);
    ASSERT_EQ((int)p.m_pipelines[0].m_command_count, 2);
    ASSERT_EQ((int)p.m_pipelines[1].m_command_count, 1);
    ASSERT_EQ((int)p.m_pipelines[1].m_join, (int)ShellParser::JOIN_ON_SUCCESS);
}

TEST(shellgrammar, each_separator_carries_its_condition)
{
    ShellParser::Line p = parseOf("a ; b && c || d");

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_EQ((int)p.m_pipelines.size(), 4);
    ASSERT_EQ((int)p.m_pipelines[0].m_join, (int)ShellParser::JOIN_FIRST);
    ASSERT_EQ((int)p.m_pipelines[1].m_join, (int)ShellParser::JOIN_ALWAYS);
    ASSERT_EQ((int)p.m_pipelines[2].m_join, (int)ShellParser::JOIN_ON_SUCCESS);
    ASSERT_EQ((int)p.m_pipelines[3].m_join, (int)ShellParser::JOIN_ON_FAILURE);
}

TEST(shellgrammar, a_redirect_is_kept_out_of_the_command_text)
{
    const char *line = "echo hello > /tmp/x";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_STREQ(textOf(p, line, 0).c_str(), "echo hello");
}

TEST(shellgrammar, a_redirect_in_the_middle_still_leaves_the_words)
{
    const char *line = "echo a > f b";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_STREQ(textOf(p, line, 0).c_str(), "echo a b");
}

TEST(shellgrammar, both_directions_can_appear_on_one_command)
{
    const char *line = "grep x < in > out";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_TRUE(nullptr != p.findRedirect(0, ShellParser::REDIRECT_IN));
    ASSERT_TRUE(nullptr != p.findRedirect(0, ShellParser::REDIRECT_OUT));
    ASSERT_STREQ(textOf(p, line, 0).c_str(), "grep x");
}

TEST(shellgrammar, append_is_told_apart_from_replace)
{
    ASSERT_TRUE(nullptr != parseOf("echo a >> f").findRedirect(0, ShellParser::REDIRECT_APPEND));
    ASSERT_TRUE(nullptr == parseOf("echo a >> f").findRedirect(0, ShellParser::REDIRECT_OUT));
    ASSERT_TRUE(nullptr != parseOf("echo a > f").findRedirect(0, ShellParser::REDIRECT_OUT));
}

TEST(shellgrammar, a_trailing_semicolon_just_ends_the_line)
{
    ShellParser::Line p = parseOf("echo one ;");

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_EQ((int)p.m_pipelines.size(), 1);
}

TEST(shellgrammar, a_lone_ampersand_stays_part_of_the_command)
{
    const char *line = "echo a & b";
    ShellParser::Line p = parseOf(line);

    ASSERT_TRUE(!p.m_malformed);
    ASSERT_EQ((int)p.m_pipelines.size(), 1);
    ASSERT_STREQ(textOf(p, line, 0).c_str(), "echo a & b");
}

TEST(shellgrammar, a_dangling_operator_is_malformed)
{
    ASSERT_TRUE(parseOf("&& echo").m_malformed);
    ASSERT_TRUE(parseOf("echo &&").m_malformed);
    ASSERT_TRUE(parseOf("echo ||").m_malformed);
    ASSERT_TRUE(parseOf("| grep x").m_malformed);
    ASSERT_TRUE(parseOf("echo a |").m_malformed);
    ASSERT_TRUE(parseOf("a ;; b").m_malformed);
}

TEST(shellgrammar, a_redirect_without_a_target_is_malformed)
{
    ASSERT_TRUE(parseOf("echo a >").m_malformed);
    ASSERT_TRUE(parseOf("echo a > | b").m_malformed);
    ASSERT_TRUE(parseOf("cat <").m_malformed);
}

TEST(shellgrammar, two_targets_of_the_same_direction_are_malformed)
{
    ASSERT_TRUE(parseOf("echo a > one > two").m_malformed);
    ASSERT_TRUE(parseOf("echo a > one >> two").m_malformed);
    ASSERT_TRUE(parseOf("cat < one < two").m_malformed);
}

TEST(shellgrammar, an_empty_line_is_malformed)
{
    ASSERT_TRUE(parseOf("").m_malformed);
    ASSERT_TRUE(parseOf("   ").m_malformed);
    ASSERT_TRUE(ShellParser::parse(nullptr, 0).m_malformed);
}

TEST(shellgrammar, the_run_rule_follows_the_previous_result)
{
    ASSERT_TRUE(ShellParser::shouldRun(ShellParser::JOIN_FIRST, CMD_ERROR_FAILED));
    ASSERT_TRUE(ShellParser::shouldRun(ShellParser::JOIN_ALWAYS, CMD_ERROR_FAILED));

    ASSERT_TRUE(ShellParser::shouldRun(ShellParser::JOIN_ON_SUCCESS, PDI_OK));
    ASSERT_TRUE(!ShellParser::shouldRun(ShellParser::JOIN_ON_SUCCESS, CMD_ERROR_FAILED));

    ASSERT_TRUE(!ShellParser::shouldRun(ShellParser::JOIN_ON_FAILURE, PDI_OK));
    ASSERT_TRUE(ShellParser::shouldRun(ShellParser::JOIN_ON_FAILURE, CMD_ERROR_FAILED));
}

// -------------------------------------------------------------- expansion

TEST(shellgrammar, expansion_replaces_the_last_result)
{
    pdiutil::string out;
    ShellParser::expand("$?", 2, (pdi_err_t)PDI_OK, out);
    ASSERT_STREQ(out.c_str(), "0");

    out.clear();
    ShellParser::expand("status=$?", 9, (pdi_err_t)PDI_OK, out);
    ASSERT_STREQ(out.c_str(), "status=0");
}

TEST(shellgrammar, a_single_quoted_run_is_literal)
{
    pdiutil::string out;
    const char *src = "'$?'";

    ShellParser::expand(src, (int16_t)strlen(src), (pdi_err_t)PDI_OK, out);
    ASSERT_STREQ(out.c_str(), "'$?'");
}

TEST(shellgrammar, a_double_quoted_run_still_expands)
{
    pdiutil::string out;
    const char *src = "\"$?\"";

    ShellParser::expand(src, (int16_t)strlen(src), (pdi_err_t)PDI_OK, out);
    ASSERT_STREQ(out.c_str(), "\"0\"");
}

TEST(shellgrammar, a_quote_inside_the_other_quote_does_not_toggle)
{
    pdiutil::string out;
    const char *src = "\"it's $?\"";

    ShellParser::expand(src, (int16_t)strlen(src), (pdi_err_t)PDI_OK, out);
    ASSERT_STREQ(out.c_str(), "\"it's 0\"");
}

TEST(shellgrammar, an_escaped_dollar_is_left_alone)
{
    pdiutil::string out;
    const char *src = "\\$?";

    ShellParser::expand(src, (int16_t)strlen(src), (pdi_err_t)PDI_OK, out);
    ASSERT_STREQ(out.c_str(), "\\$?");
}

TEST(shellgrammar, a_negative_result_expands_with_its_sign)
{
    pdiutil::string out;
    ShellParser::expand("$?", 2, (pdi_err_t)CMD_ERROR_NOENT, out);
    ASSERT_STREQ(out.c_str(), "-3603");
}

TEST(shellgrammar, expandable_spots_only_what_needs_work)
{
    ASSERT_TRUE(ShellParser::expandable("echo $?", 7));
    ASSERT_TRUE(!ShellParser::expandable("echo hi", 7));
}

TEST(shellgrammar, a_command_expands_when_its_text_is_built)
{
    const char *line = "echo $?";
    ShellParser::Line p = parseOf(line);

    pdiutil::string out;
    p.commandText(line, 0, out, (pdi_err_t)PDI_OK);
    ASSERT_STREQ(out.c_str(), "echo 0");
}

#endif
