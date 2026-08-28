/****************************** Fuzz Config ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The reader behind every /etc file the framework keeps: ssh policy, hosts, and
the per feature conf files. A file that an upload, an interrupted write or an
older firmware left behind arrives here unchecked.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include <FuzzCommon.h>

#ifdef ENABLE_STORAGE_SERVICE

#include <MountedStack.h>
#include <helpers/ConfigHelper.h>

namespace
{

    const char *FUZZ_CONF_PATH = "/tmp/fuzz.conf";

    /**
     * @brief The mounted filesystem the config file is written on.
     */
    void readyStorage()
    {
        static bool done = false;
        if (!done)
        {
            done = true;
            pdifuzz::useFreeClock();
            pditest::mountedVfs();
        }
    }

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    readyStorage();

    if (0 >= __i_fs.writeFile(FUZZ_CONF_PATH, (const char *)data, (uint32_t)size))
    {
        return 0;
    }

    pdiutil::vector<config_kv_t> pairs;
    loadConfigFile(FUZZ_CONF_PATH, pairs);

    __i_fs.deleteFile(FUZZ_CONF_PATH);

    return 0;
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}

#endif
