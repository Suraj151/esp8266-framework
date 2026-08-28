/**************************** Stream Sink Tests *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers the two things a descriptor slot can hold: a pipe carrying bytes from one
stage to the next, and a file taking a redirect. Both are terminals, so a
command writes to them without knowing which it has.

Author          : Suraj I.
created Date    : 25th Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <StringTerminal.h>
#include <pditest.h>

#include <service_provider/session/PipeStream.h>
#include <service_provider/session/FileWriteStream.h>
#include <service_provider/session/SessionManager.h>

#ifdef ENABLE_CMD_SERVICE

TEST(streams, a_pipe_returns_what_was_written_in_order)
{
    PipeStream pipe(64);
    ASSERT_TRUE(pipe.isValid());

    pipe.write("hello ");
    pipe.write("world");

    ASSERT_EQ(pipe.available(), 11);

    uint8_t out[16] = {0};
    ASSERT_EQ(pipe.read(out, 11), 11);
    ASSERT_STREQ((const char *)out, "hello world");
    ASSERT_EQ(pipe.available(), 0);
}

TEST(streams, a_pipe_reads_a_byte_at_a_time)
{
    PipeStream pipe(8);
    pipe.write('a');
    pipe.write('b');

    ASSERT_EQ((int)pipe.read(), (int)'a');
    ASSERT_EQ((int)pipe.read(), (int)'b');
    ASSERT_EQ(pipe.available(), 0);
}

TEST(streams, a_pipe_refuses_what_will_not_fit_and_says_so)
{
    PipeStream pipe(4);
    ASSERT_FALSE(pipe.overflowed());

    ASSERT_EQ(pipe.write("abcdefgh"), 4);
    ASSERT_TRUE(pipe.overflowed());
    ASSERT_EQ(pipe.available(), 4);
}

TEST(streams, a_pipe_reports_room_before_a_write)
{
    PipeStream pipe(8);

    ASSERT_TRUE(pipe.availableforwrite(8));
    pipe.write("abcd");
    ASSERT_TRUE(pipe.availableforwrite(4));
    ASSERT_FALSE(pipe.availableforwrite(5));
}

TEST(streams, a_pipe_wraps_around_its_buffer)
{
    PipeStream pipe(4);

    pipe.write("abcd");
    uint8_t out[4] = {0};
    pipe.read(out, 4);

    // the write pointer has rolled over, so this exercises the wrap rather
    // than a fresh buffer
    ASSERT_EQ(pipe.write("wxyz"), 4);
    ASSERT_EQ(pipe.read(out, 4), 4);
    ASSERT_EQ((int)out[0], (int)'w');
    ASSERT_EQ((int)out[3], (int)'z');
}

TEST(streams, a_receive_flush_discards_an_unread_pipe)
{
    PipeStream pipe(16);
    pipe.write("dropped");
    ASSERT_EQ(pipe.available(), 7);

    pipe.flush(FLUSH_TX);
    ASSERT_EQ(pipe.available(), 7);

    pipe.flush(FLUSH_RX);
    ASSERT_EQ(pipe.available(), 0);
}

TEST(streams, a_pipe_writes_a_read_only_string)
{
    PipeStream pipe(64);

    // longer than the copy window, so the chunked path runs
    pipe.write_ro(RODT_ATTR("the quick brown fox jumps over the lazy dog"));
    ASSERT_EQ(pipe.available(), 43);

    uint8_t out[64] = {0};
    pipe.read(out, 43);
    ASSERT_STREQ((const char *)out, "the quick brown fox jumps over the lazy dog");
}

#ifdef ENABLE_STORAGE_SERVICE

TEST(streams, a_file_stream_lands_its_bytes_on_close)
{
    pditest::mountedVfs();
    const char *path = "/streams_basic.txt";
    __i_fs.deleteFile(path);

    {
        FileWriteStream fs(path, false);
        ASSERT_TRUE(fs.isValid());
        fs.write("written by a redirect");
    }

    ASSERT_TRUE(__i_fs.isFileExist(path));
    ASSERT_EQ((int)__i_fs.getFileSize(path), 21);
    __i_fs.deleteFile(path);
}

TEST(streams, a_file_stream_spanning_the_buffer_keeps_every_byte)
{
    pditest::mountedVfs();
    const char *path = "/streams_big.txt";
    __i_fs.deleteFile(path);

    const uint32_t total = PDI_FILE_STREAM_BUFFER * 3 + 7;

    {
        FileWriteStream fs(path, false);
        for (uint32_t i = 0; i < total; i++)
        {
            fs.write((uint8_t)('a' + (i % 26)));
        }
        ASSERT_FALSE(fs.failed());
    }

    // every block appended rather than overwriting the one before it
    ASSERT_EQ((int)__i_fs.getFileSize(path), (int)total);
    __i_fs.deleteFile(path);
}

TEST(streams, a_truncating_stream_replaces_what_was_there)
{
    pditest::mountedVfs();
    const char *path = "/streams_trunc.txt";
    __i_fs.deleteFile(path);
    __i_fs.createFile(path, "0123456789", 10);

    {
        FileWriteStream fs(path, false);
        fs.write("new");
    }

    ASSERT_EQ((int)__i_fs.getFileSize(path), 3);
    __i_fs.deleteFile(path);
}

TEST(streams, an_appending_stream_keeps_what_was_there)
{
    pditest::mountedVfs();
    const char *path = "/streams_append.txt";
    __i_fs.deleteFile(path);
    __i_fs.createFile(path, "head", 4);

    {
        FileWriteStream fs(path, true);
        fs.write("tail");
    }

    ASSERT_EQ((int)__i_fs.getFileSize(path), 8);
    __i_fs.deleteFile(path);
}

TEST(streams, a_transmit_flush_commits_without_closing)
{
    pditest::mountedVfs();
    const char *path = "/streams_flush.txt";
    __i_fs.deleteFile(path);

    FileWriteStream fs(path, false);
    fs.write("early");
    ASSERT_EQ((int)fs.written(), 0);

    fs.flush(FLUSH_TX);
    ASSERT_EQ((int)fs.written(), 5);
    ASSERT_EQ((int)__i_fs.getFileSize(path), 5);

    __i_fs.deleteFile(path);
}

TEST(streams, a_redirect_through_the_descriptor_table_reaches_a_file)
{
    pditest::mountedVfs();
    const char *path = "/streams_fd.txt";
    __i_fs.deleteFile(path);

    pditest::StringTerminal terminal;
    session_t *s = SessionManager::attach(&terminal);
    ASSERT_NOT_NULL(s);

    // this is the shape a shell redirect takes: an owned sink in slot 1, the
    // command writing through the adapter, then stdio put back
    SessionManager::setFd(PDI_FD_STDOUT, new FileWriteStream(path, false), true, s);

    SessionStdio *io = SessionManager::stdioFor(s);
    ASSERT_NOT_NULL(io);
    io->write("captured");

    SessionManager::resetStdio(s);

    ASSERT_NULL(s->m_fdtable);
    ASSERT_EQ((int)__i_fs.getFileSize(path), 8);
    ASSERT_STREQ(terminal.captured().c_str(), "");

    __i_fs.deleteFile(path);
    SessionManager::detach(&terminal);
}

#endif

#endif
