/************************ Session Descriptor Table Tests **********************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers the per session descriptor table: the standard three resolving to the
session terminal with nothing claimed, a claim allocating the table and a
release dropping it again, ownership deciding what gets deleted, and the stdio
adapter splitting reads from writes so a redirect never moves the prompt.

Author          : Suraj I.
created Date    : 25th Aug 2026
******************************************************************************/

#include <StringTerminal.h>
#include <pditest.h>

#include <service_provider/session/SessionManager.h>
#include <service_provider/session/SessionStdio.h>

#ifdef ENABLE_CMD_SERVICE

namespace
{
    /**
     * @brief A terminal that counts which side of a flush reached it.
     */
    class FlushRecorder : public pditest::StringTerminal
    {
    public:
        FlushRecorder() : m_tx(0), m_rx(0) {}

        void flush(int16_t flushtype = FLUSH_TX) override
        {
            if (IsFlushTx(flushtype)) m_tx++;
            if (IsFlushRx(flushtype)) m_rx++;
        }

        uint16_t m_tx;
        uint16_t m_rx;
    };

    /**
     * @brief Detach every terminal a test attached, so one test cannot leave a
     *        session or a descriptor behind for the next.
     */
    struct SessionScope
    {
        SessionScope() { reset(); }
        ~SessionScope() { reset(); }

        static void reset()
        {
            for (uint8_t i = 0; i < SessionManager::maxSessions(); i++)
            {
                session_t *s = SessionManager::getByIndex(i);
                if (nullptr != s && SESSION_STATE_FREE != s->m_state)
                {
                    SessionManager::detach(s->m_terminal);
                }
            }
        }
    };
}

TEST(fdtable, standard_descriptors_resolve_to_the_terminal_before_any_claim)
{
    SessionScope scope;
    pditest::StringTerminal terminal;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    ASSERT_EQ(SessionManager::getFd(PDI_FD_STDIN, s), (iTerminalInterface *)&terminal);
    ASSERT_EQ(SessionManager::getFd(PDI_FD_STDOUT, s), (iTerminalInterface *)&terminal);
    ASSERT_EQ(SessionManager::getFd(PDI_FD_STDERR, s), (iTerminalInterface *)&terminal);
}

TEST(fdtable, a_session_that_never_redirects_allocates_no_table)
{
    SessionScope scope;
    pditest::StringTerminal terminal;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    SessionManager::getFd(PDI_FD_STDOUT, s);
    SessionManager::resetStdio(s);

    ASSERT_NULL(s->m_fdtable);
    ASSERT_NULL(SessionManager::stdioFor(s));
}

TEST(fdtable, a_spare_descriptor_is_unclaimed_until_something_takes_it)
{
    SessionScope scope;
    pditest::StringTerminal terminal;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    ASSERT_NULL(SessionManager::getFd(PDI_FD_STDERR + 1, s));
}

TEST(fdtable, claiming_stdout_allocates_the_table_and_redirects_reads_nowhere)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    ASSERT_TRUE(SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s));
    ASSERT_NOT_NULL(s->m_fdtable);

    ASSERT_EQ(SessionManager::getFd(PDI_FD_STDOUT, s), (iTerminalInterface *)&sink);
    ASSERT_EQ(SessionManager::getFd(PDI_FD_STDIN, s), (iTerminalInterface *)&terminal);
}

TEST(fdtable, releasing_the_last_claim_drops_the_table)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);
    ASSERT_NOT_NULL(s->m_fdtable);

    SessionManager::resetStdio(s);
    ASSERT_NULL(s->m_fdtable);
    ASSERT_EQ(SessionManager::getFd(PDI_FD_STDOUT, s), (iTerminalInterface *)&terminal);
}

TEST(fdtable, an_unowned_stream_survives_its_slot_being_cleared)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);
    SessionManager::closeFd(PDI_FD_STDOUT, s);

    sink.write("still alive");
    ASSERT_STREQ(sink.captured().c_str(), "still alive");
}

TEST(fdtable, an_owned_stream_is_deleted_when_its_slot_is_reassigned)
{
    SessionScope scope;
    pditest::StringTerminal terminal;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    // the sanitizer is what proves this: a leak or a double free here fails the
    // run rather than passing quietly
    SessionManager::setFd(PDI_FD_STDOUT, new pditest::StringTerminal(), true, s);
    SessionManager::setFd(PDI_FD_STDOUT, new pditest::StringTerminal(), true, s);
    SessionManager::resetStdio(s);

    ASSERT_NULL(s->m_fdtable);
}

TEST(fdtable, allocfd_hands_out_the_first_free_spare_slot)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal a;
    pditest::StringTerminal b;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    int8_t first = SessionManager::allocFd(&a, false, s);
    int8_t second = SessionManager::allocFd(&b, false, s);

    ASSERT_EQ((int)first, PDI_FD_STDERR + 1);
    ASSERT_EQ((int)second, PDI_FD_STDERR + 2);
    ASSERT_EQ(SessionManager::getFd((uint8_t)first, s), (iTerminalInterface *)&a);
    ASSERT_EQ(SessionManager::getFd((uint8_t)second, s), (iTerminalInterface *)&b);
}

TEST(fdtable, detaching_a_session_releases_every_owned_descriptor)
{
    SessionScope scope;
    pditest::StringTerminal terminal;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    SessionManager::setFd(PDI_FD_STDOUT, new pditest::StringTerminal(), true, s);
    SessionManager::allocFd(new pditest::StringTerminal(), true, s);

    SessionManager::detach(&terminal);
    ASSERT_NULL(s->m_fdtable);
}

TEST(fdtable, the_stdio_adapter_writes_to_stdout_and_reads_from_stdin)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    terminal.feed("typed");
    io->write("printed");

    // output went to the sink and the terminal saw none of it
    ASSERT_STREQ(sink.captured().c_str(), "printed");
    ASSERT_STREQ(terminal.captured().c_str(), "");

    // input still comes from the terminal
    ASSERT_EQ((int)io->read(), (int)'t');
}

TEST(fdtable, a_redirected_line_ending_loses_its_carriage_return)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    io->write("one\r\ntwo\r\n");

    // a file holds what linux would hold, not what a terminal needs
    ASSERT_STREQ(sink.captured().c_str(), "one\ntwo\n");
}

TEST(fdtable, a_line_ending_split_across_two_writes_still_collapses)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    io->write((const uint8_t *)"one\r", 4);
    io->write((const uint8_t *)"\ntwo", 4);

    ASSERT_STREQ(sink.captured().c_str(), "one\ntwo");
}

TEST(fdtable, a_carriage_return_on_its_own_is_kept)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    session_t *s = SessionManager::attach(&terminal);
    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    // only the carriage return of a line ending is presentation
    io->write((const uint8_t *)"one\rtwo", 7);

    ASSERT_STREQ(sink.captured().c_str(), "one\rtwo");
}

TEST(fdtable, output_left_on_the_terminal_keeps_its_line_endings)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal spare;

    session_t *s = SessionManager::attach(&terminal);

    // a table exists, but stdout still resolves to the terminal itself
    SessionManager::setFd(PDI_FD_STDOUT, &terminal, false, s);
    SessionManager::setFd(PDI_FD_STDERR, &spare, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    io->write("one\r\ntwo\r\n");

    ASSERT_STREQ(terminal.captured().c_str(), "one\r\ntwo\r\n");
}

TEST(fdtable, a_split_flush_all_reaches_both_sides)
{
    SessionScope scope;
    FlushRecorder terminal;
    FlushRecorder sink;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);
    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    io->flush(FLUSH_ALL);

    // output side reached the sink, input side reached the terminal, and
    // neither picked up the half that was not theirs
    ASSERT_EQ((int)sink.m_tx, 1);
    ASSERT_EQ((int)sink.m_rx, 0);
    ASSERT_EQ((int)terminal.m_rx, 1);
    ASSERT_EQ((int)terminal.m_tx, 0);
}

TEST(fdtable, a_split_flush_of_one_side_leaves_the_other_alone)
{
    SessionScope scope;
    FlushRecorder terminal;
    FlushRecorder sink;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);
    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    io->flush(FLUSH_TX);
    ASSERT_EQ((int)sink.m_tx, 1);
    ASSERT_EQ((int)terminal.m_rx, 0);

    io->flush(FLUSH_RX);
    ASSERT_EQ((int)sink.m_tx, 1);
    ASSERT_EQ((int)terminal.m_rx, 1);
}

TEST(fdtable, an_unsplit_flush_passes_the_type_through_once)
{
    SessionScope scope;
    FlushRecorder terminal;
    FlushRecorder spare;

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    // claim a spare slot so the table exists while stdin and stdout both still
    // resolve to the terminal
    SessionManager::allocFd(&spare, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    io->flush(FLUSH_ALL);
    ASSERT_EQ((int)terminal.m_tx, 1);
    ASSERT_EQ((int)terminal.m_rx, 1);
}

TEST(fdtable, the_stdio_adapter_mirrors_the_terminal_geometry)
{
    SessionScope scope;
    pditest::StringTerminal terminal;
    pditest::StringTerminal sink;

    terminal.set_terminal_type(TERMINAL_TYPE_SSH);
    terminal.set_column_width(132);
    terminal.set_row_count(50);

    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);
    SessionManager::setFd(PDI_FD_STDOUT, &sink, false, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);

    ASSERT_EQ((int)io->get_terminal_type(), (int)TERMINAL_TYPE_SSH);
    ASSERT_EQ((int)io->get_column_width(), 132);
    ASSERT_EQ((int)io->get_row_count(), 50);
}

#endif
