/****************************** Fuzz Crontab **********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The table reader behind /etc/crontab: the row split into five field specs and a
command, and the field grammar of stars, numbers, ranges and steps. The file is
re-read every minute and whatever it says runs with the scheduler's privileges,
so a row that got there by an upload is read the same as one that was typed.

Author          : Suraj I.
created Date    : 11th Sep 2026
******************************************************************************/

#include <FuzzCommon.h>

#ifdef ENABLE_CRON_SERVICE

#include <service_provider/cron/CronServiceProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    pdifuzz::FuzzInput input(data, size);

    uint8_t value = input.pick(80);
    uint8_t lo = input.pick(80);
    uint8_t hi = input.pick(80);

    pdiutil::string line((const char *)input.rest(), input.remaining());

    pdiutil::string fields[CRON_FIELD_COUNT];
    pdiutil::string command;

    if (CronServiceProvider::parseRow(line, fields, command))
    {
        for (uint8_t at = 0; at < CRON_FIELD_COUNT; at++)
        {
            CronServiceProvider::fieldMatches(fields[at], value, lo, hi);
        }
    }

    // the whole line as one spec as well, so the field grammar is reached on
    // inputs the row split refuses
    CronServiceProvider::fieldMatches(line, value, lo, hi);

    uint32_t number = 0;
    CronServiceProvider::parseNumber(line, number);

    return 0;
}

#else

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    return 0;
}

#endif
