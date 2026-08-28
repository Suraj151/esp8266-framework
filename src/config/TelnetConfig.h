/************************** Telnet Config page *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 19th August 2026
******************************************************************************/
#ifndef _TELNET_CONFIG_H_
#define _TELNET_CONFIG_H_

#include "Common.h"

#define TELNET_DEFAULT_PORT 23

#ifndef TELNET_SHELL_IDLE_MS
#define TELNET_SHELL_IDLE_MS 180000
#endif

#ifndef TELNET_MAX_SESSIONS
#define TELNET_MAX_SESSIONS 2
#endif

#ifndef TELNET_POOL_FULL_GRACE_MS
#define TELNET_POOL_FULL_GRACE_MS 5000
#endif

#endif
