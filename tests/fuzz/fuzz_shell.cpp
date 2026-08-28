/******************************* Fuzz Shell ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The terminal reader every transport shares. Bytes arrive here straight off a
telnet socket, before a password has been accepted, and go through the escape
decoding, the line editor and the login prompt on the way.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include <FuzzCommon.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

#include <ShellHarness.h>
#include <StringTerminal.h>

namespace
{

    /**
     * @brief Bring up the parts of the stack the reader expects to find.
     */
    void readyStack()
    {
        static bool done = false;
        if (!done)
        {
            done = true;
            pdifuzz::useFreeClock();
            pditest::mountedVfs();
            pditest::seedRootAccount();
            pditest::readyScheduler();
        }
    }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    readyStack();

    pditest::StringTerminal terminal;
    if (!__cmd_service.useTerminal(&terminal))
    {
        return 0;
    }

    terminal.feed(data, size);

    // the reader stops at every line ending, so it is pumped until the input
    // is drained or it stops consuming
    int32_t left = terminal.available();
    while (left > 0)
    {
        __cmd_service.processTerminalInput(&terminal);

        int32_t now = terminal.available();
        if (now >= left)
        {
            break;
        }
        left = now;
    }

    pdiutil::string interrupt;
    __cmd_service.executeCommand(&interrupt, CMD_TERM_INSEQ_CTRL_C);
    __cmd_service.useTerminal(nullptr);

    return 0;
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}

#endif
