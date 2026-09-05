/*************************** Session Config page ******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 18th July 2026
******************************************************************************/
#ifndef _SESSION_CONFIG_H_
#define _SESSION_CONFIG_H_

#include "Common.h"
#include "TelnetConfig.h"
#include "SshConfig.h"

/**
 * Slots the remote transports can ask for. A pool larger than the table it
 * draws from starves whichever transport arrives second, so the two are sized
 * together rather than by coincidence.
 */
#ifdef ENABLE_TELNET_SERVICE
#define PDI_TELNET_SESSION_DEMAND TELNET_MAX_SESSIONS
#else
#define PDI_TELNET_SESSION_DEMAND 0
#endif

#ifdef ENABLE_SSH_SERVICE
#define PDI_SSH_SESSION_DEMAND SSH_MAX_SESSIONS
#else
#define PDI_SSH_SESSION_DEMAND 0
#endif

/**
 * One for the console the board always has, plus every slot a remote transport
 * can fill at once. A board with no network keeps a spare for the login that
 * arrives while a stale session is still being reclaimed.
 */
#define PDI_SESSION_DEMAND (1 + PDI_TELNET_SESSION_DEMAND + PDI_SSH_SESSION_DEMAND)

#ifndef PDI_MAX_SESSIONS
#if PDI_SESSION_DEMAND < 2
#define PDI_MAX_SESSIONS 2
#else
#define PDI_MAX_SESSIONS PDI_SESSION_DEMAND
#endif
#endif

#if PDI_MAX_SESSIONS < PDI_SESSION_DEMAND
#error "PDI_MAX_SESSIONS is smaller than the transports can fill; raise it or shrink TELNET_MAX_SESSIONS / SSH_MAX_SESSIONS"
#endif

/**
 * Bytes a pipe carries between two stages. A pipe is fixed capacity the way a
 * linux one is; a stage that outruns it marks the pipe overflowed rather than
 * growing without bound.
 */
#ifndef PDI_PIPE_CAPACITY
#define PDI_PIPE_CAPACITY 1024
#endif

/**
 * Bytes a file stream gathers before it touches storage. Writing through per
 * byte would cost one open and close per byte on the filesystem.
 */
#ifndef PDI_FILE_STREAM_BUFFER
#define PDI_FILE_STREAM_BUFFER 128
#endif

/**
 * Lines tail will hold back while a stream drains. A stream cannot be rewound,
 * so asking for more than this would buy nothing but the memory to hold it.
 */
#ifndef PDI_STREAM_TAIL_LINES_MAX
#define PDI_STREAM_TAIL_LINES_MAX 32
#endif

#ifndef ENV_FILE_PATH
#define ENV_FILE_PATH "/.env"
#endif

#ifndef ENV_NAME_MAX
#define ENV_NAME_MAX 32
#endif

#ifndef ENV_VALUE_MAX
#define ENV_VALUE_MAX 128
#endif

#ifndef ENV_SESSION_MAX
#define ENV_SESSION_MAX 8
#endif

#define ENV_KEY_HOME "HOME"
#define ENV_KEY_PWD "PWD"
#define ENV_KEY_USER "USER"
#define ENV_KEY_UID "UID"
#define ENV_KEY_HOSTNAME "HOSTNAME"

#endif
