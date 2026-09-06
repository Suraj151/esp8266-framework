/**************************** Service configurations **************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 30th Aug 2026
******************************************************************************/

#ifndef _SERVICE_CONFIG_H_
#define _SERVICE_CONFIG_H_

#include "Common.h"

#define SERVICE_CONFIG_DIR_ROOT "/etc/"
#define SERVICE_CONFIG_FILE_SUFFIX ".conf"
#define SERVICE_ENABLE_CONFIG_FILE "/etc/service.conf"
#define SERVICE_ENABLE_CONFIG_HEADER \
    "# PDI service enable state, one line per service" TERMINAL_NEW_LINE \
    "# <service> yes|no" TERMINAL_NEW_LINE

#ifndef SERVICE_NAME_MAX
#define SERVICE_NAME_MAX 16
#endif

#ifndef TIME_SERVICE_TICK_MS
#define TIME_SERVICE_TICK_MS 10
#endif

#endif
