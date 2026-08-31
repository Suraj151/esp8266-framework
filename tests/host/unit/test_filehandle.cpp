/**************************** File Handle Tests *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Covers the handle based file calls added beside the path based ones: opening,
reading, writing, seeking and closing, the mount a handle is routed back to, and
what a backend without handles answers.

Author          : Suraj I.
created Date    : 28th Aug 2026
******************************************************************************/

#include <MountedStack.h>
#include <pditest.h>

#include <interface/pdi.h>

#ifdef ENABLE_STORAGE_SERVICE

TEST(filehandle, a_file_opens_and_reads_back_what_was_written)
{
    pditest::mountedVfs();
    const char *path = "/handle_rw.txt";
    __i_fs.deleteFile(path);

    pdi_fhandle_t fh = __i_fs.openFile(path, FILE_OPEN_WRITE | FILE_OPEN_CREATE | FILE_OPEN_TRUNCATE);
    ASSERT_TRUE(fh >= 0);
    ASSERT_EQ(__i_fs.writeFileHandle(fh, "hello world", 11), 11);
    ASSERT_EQ((int)__i_fs.closeFile(fh), 0);

    ASSERT_EQ((int)__i_fs.getFileSize(path), 11);

    char buf[16] = {0};
    fh = __i_fs.openFile(path, FILE_OPEN_READ);
    ASSERT_TRUE(fh >= 0);
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, sizeof(buf) - 1), 11);
    ASSERT_STREQ(buf, "hello world");
    ASSERT_EQ((int)__i_fs.closeFile(fh), 0);

    __i_fs.deleteFile(path);
}

TEST(filehandle, a_handle_carries_its_position_across_reads)
{
    pditest::mountedVfs();
    const char *path = "/handle_pos.txt";
    __i_fs.deleteFile(path);
    __i_fs.writeFile(path, "abcdefghij", 10, false);

    pdi_fhandle_t fh = __i_fs.openFile(path, FILE_OPEN_READ);
    ASSERT_TRUE(fh >= 0);

    char buf[8] = {0};
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 4), 4);
    ASSERT_EQ(memcmp(buf, "abcd", 4), 0);

    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 4), 4);
    ASSERT_EQ(memcmp(buf, "efgh", 4), 0);

    // only two bytes are left, so a larger ask is short rather than refused
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 4), 2);
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 4), 0);

    __i_fs.closeFile(fh);
    __i_fs.deleteFile(path);
}

TEST(filehandle, seek_moves_the_position_from_each_reference_point)
{
    pditest::mountedVfs();
    const char *path = "/handle_seek.txt";
    __i_fs.deleteFile(path);
    __i_fs.writeFile(path, "0123456789", 10, false);

    pdi_fhandle_t fh = __i_fs.openFile(path, FILE_OPEN_READ);
    ASSERT_TRUE(fh >= 0);

    char buf[8] = {0};
    ASSERT_EQ((int)__i_fs.seekFile(fh, 4, FILE_SEEK_SET), 4);
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 2), 2);
    ASSERT_EQ(memcmp(buf, "45", 2), 0);

    ASSERT_EQ((int)__i_fs.seekFile(fh, 1, FILE_SEEK_CUR), 7);
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 1), 1);
    ASSERT_EQ(buf[0], '7');

    ASSERT_EQ((int)__i_fs.seekFile(fh, -2, FILE_SEEK_END), 8);
    ASSERT_EQ(__i_fs.readFileHandle(fh, buf, 4), 2);
    ASSERT_EQ(memcmp(buf, "89", 2), 0);

    __i_fs.closeFile(fh);
    __i_fs.deleteFile(path);
}

TEST(filehandle, an_append_open_keeps_what_the_file_already_held)
{
    pditest::mountedVfs();
    const char *path = "/handle_append.txt";
    __i_fs.deleteFile(path);
    __i_fs.writeFile(path, "head", 4, false);

    pdi_fhandle_t fh = __i_fs.openFile(path, FILE_OPEN_WRITE | FILE_OPEN_CREATE | FILE_OPEN_APPEND);
    ASSERT_TRUE(fh >= 0);
    ASSERT_EQ(__i_fs.writeFileHandle(fh, "tail", 4), 4);
    __i_fs.closeFile(fh);

    ASSERT_EQ((int)__i_fs.getFileSize(path), 8);
    __i_fs.deleteFile(path);
}

TEST(filehandle, a_truncating_open_discards_what_the_file_held)
{
    pditest::mountedVfs();
    const char *path = "/handle_trunc.txt";
    __i_fs.deleteFile(path);
    __i_fs.writeFile(path, "0123456789", 10, false);

    pdi_fhandle_t fh = __i_fs.openFile(path, FILE_OPEN_WRITE | FILE_OPEN_TRUNCATE);
    ASSERT_TRUE(fh >= 0);
    ASSERT_EQ(__i_fs.writeFileHandle(fh, "ab", 2), 2);
    __i_fs.closeFile(fh);

    ASSERT_EQ((int)__i_fs.getFileSize(path), 2);
    __i_fs.deleteFile(path);
}

TEST(filehandle, a_sync_makes_a_written_block_visible_without_closing)
{
    pditest::mountedVfs();
    const char *path = "/handle_sync.txt";
    __i_fs.deleteFile(path);

    pdi_fhandle_t fh = __i_fs.openFile(path, FILE_OPEN_WRITE | FILE_OPEN_CREATE | FILE_OPEN_TRUNCATE);
    ASSERT_TRUE(fh >= 0);
    ASSERT_EQ(__i_fs.writeFileHandle(fh, "visible", 7), 7);

    ASSERT_EQ((int)__i_fs.syncFile(fh), 0);
    ASSERT_EQ((int)__i_fs.getFileSize(path), 7);

    __i_fs.closeFile(fh);
    __i_fs.deleteFile(path);
}

TEST(filehandle, opening_a_missing_file_without_create_is_refused)
{
    pditest::mountedVfs();
    const char *path = "/handle_absent.txt";
    __i_fs.deleteFile(path);

    ASSERT_TRUE(__i_fs.openFile(path, FILE_OPEN_READ) < 0);
}

TEST(filehandle, a_handle_that_was_never_opened_is_refused)
{
    pditest::mountedVfs();

    char buf[4] = {0};
    ASSERT_TRUE(__i_fs.readFileHandle(4242, buf, sizeof(buf)) < 0);
    ASSERT_TRUE(__i_fs.writeFileHandle(4242, "x", 1) < 0);
    ASSERT_TRUE(__i_fs.seekFile(4242, 0, FILE_SEEK_SET) < 0);
    ASSERT_TRUE((int)__i_fs.closeFile(4242) < 0);
    ASSERT_TRUE((int)__i_fs.closeFile(-1) < 0);
}

TEST(filehandle, the_pool_refuses_more_than_it_holds_and_recovers_on_close)
{
    pditest::mountedVfs();
    const char *path = "/handle_pool.txt";
    __i_fs.deleteFile(path);
    __i_fs.writeFile(path, "pool", 4, false);

    pdi_fhandle_t open[VFS_MAX_OPEN_FILES] = {0};
    for (uint8_t i = 0; i < VFS_MAX_OPEN_FILES; i++) {
        open[i] = __i_fs.openFile(path, FILE_OPEN_READ);
        ASSERT_TRUE(open[i] >= 0);
    }

    ASSERT_TRUE(__i_fs.openFile(path, FILE_OPEN_READ) < 0);

    ASSERT_EQ((int)__i_fs.closeFile(open[0]), 0);

    pdi_fhandle_t again = __i_fs.openFile(path, FILE_OPEN_READ);
    ASSERT_TRUE(again >= 0);
    __i_fs.closeFile(again);

    for (uint8_t i = 1; i < VFS_MAX_OPEN_FILES; i++) {
        __i_fs.closeFile(open[i]);
    }

    __i_fs.deleteFile(path);
}

#ifdef ENABLE_PROCFS
TEST(filehandle, a_backend_without_handles_says_so_rather_than_pretending)
{
    pditest::mountedVfs();

    // procfs generates its files, so it has nothing to hold open; the caller is
    // told to use the path calls instead of being handed a fake handle
    pdi_fhandle_t fh = __i_fs.openFile("/proc/uptime", FILE_OPEN_READ);
    ASSERT_EQ((int)fh, (int)PDI_ERR_NOT_SUPPORTED);
}
#endif

TEST(filehandle, two_handles_on_one_file_hold_their_own_positions)
{
    pditest::mountedVfs();
    const char *path = "/handle_two.txt";
    __i_fs.deleteFile(path);
    __i_fs.writeFile(path, "abcdefgh", 8, false);

    pdi_fhandle_t a = __i_fs.openFile(path, FILE_OPEN_READ);
    pdi_fhandle_t b = __i_fs.openFile(path, FILE_OPEN_READ);
    ASSERT_TRUE(a >= 0 && b >= 0);
    ASSERT_TRUE(a != b);

    char buf[8] = {0};
    ASSERT_EQ(__i_fs.readFileHandle(a, buf, 4), 4);
    ASSERT_EQ(memcmp(buf, "abcd", 4), 0);

    ASSERT_EQ(__i_fs.readFileHandle(b, buf, 2), 2);
    ASSERT_EQ(memcmp(buf, "ab", 2), 0);

    __i_fs.closeFile(a);
    __i_fs.closeFile(b);
    __i_fs.deleteFile(path);
}

#endif
