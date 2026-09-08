/****************************** Cron configurations ***************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 7th Sep 2026
******************************************************************************/

#ifndef _CRON_CONFIG_H_
#define _CRON_CONFIG_H_

#include "Common.h"

#ifndef CRONTAB_FILE_PATH
#define CRONTAB_FILE_PATH "/etc/crontab"
#endif

#ifndef CRON_MAX_ENTRIES
#define CRON_MAX_ENTRIES 8            /* rows taken from the table, later ones refused */
#endif

#define CRONTAB_FILE_PERMS 0600       /* jobs run as root, so only root may add one */

#define CRON_COMMENT_CHAR LINE_COMMENT_CHAR
#define CRON_ANY_CHAR WILDCARD_CHAR
#define CRON_RANGE_CHAR RANGE_SEPARATOR_CHAR
#define CRON_STEP_CHAR STEP_SEPARATOR_CHAR
#define CRON_LIST_CHAR LIST_SEPARATOR_CHAR

#define CRON_FIELD_COUNT 5
#define CRON_FIELD_MINUTE 0
#define CRON_FIELD_HOUR 1
#define CRON_FIELD_DOM 2
#define CRON_FIELD_MONTH 3
#define CRON_FIELD_DOW 4

#define CRONTAB_FILE_HEADER \
    "# PDI cron table, one job per line" TERMINAL_NEW_LINE \
    "# min hour dom mon dow  command" TERMINAL_NEW_LINE

#endif
