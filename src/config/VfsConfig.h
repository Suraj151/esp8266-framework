/****************************** VFS Config page *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 20th July 2026
******************************************************************************/
#ifndef _VFS_CONFIG_H_
#define _VFS_CONFIG_H_

#include "Common.h"

#ifndef VFS_MAX_MOUNTS
#define VFS_MAX_MOUNTS 5
#endif

#ifndef VFS_MOUNT_PREFIX_MAX
#define VFS_MOUNT_PREFIX_MAX 15
#endif

#ifndef VFS_MOUNT_NAME_MAX
#define VFS_MOUNT_NAME_MAX 11
#endif

// Per-backend enable flags. Each backend adds a mount slot at boot, so keep
// VFS_MAX_MOUNTS ≥ 1 + (number of enabled synthetic backends). Override to
// undef in DeviceConfig.h to opt out on tight-RAM ports.
#ifndef ENABLE_PROCFS
#define ENABLE_PROCFS
#endif

#ifndef PROC_MOUNT_PREFIX
#define PROC_MOUNT_PREFIX "/proc"
#endif

#ifndef ENABLE_SYSFS
#define ENABLE_SYSFS
#endif

#ifndef SYS_MOUNT_PREFIX
#define SYS_MOUNT_PREFIX "/sys"
#endif

#ifndef ENABLE_DEVFS
#define ENABLE_DEVFS
#endif

#ifndef DEV_MOUNT_PREFIX
#define DEV_MOUNT_PREFIX "/dev"
#endif

// RAM-backed scratch filesystem. Holds real file content in the heap, so keep
// it off (undef in DeviceConfig.h) on tight-RAM ports like Arduino UNO.
#ifndef ENABLE_TMPFS
#define ENABLE_TMPFS
#endif

#ifndef TMP_MOUNT_PREFIX
#define TMP_MOUNT_PREFIX "/tmp"
#endif

// Files one backend can keep open at once. An open file on the root filesystem
// costs an lfs_file_t plus one cache_size buffer, so lower this on tight-RAM
// ports rather than raising it by habit.
#ifndef VFS_MAX_OPEN_FILES
#define VFS_MAX_OPEN_FILES 4
#endif

// A handle returned by the dispatcher carries the mount it belongs to in its
// top byte, so routing a read or a close back to the right backend needs no
// table. The remaining 24 bits are the backend's own handle. The stored mount
// id is the index plus one, and stays under 0x80 so the handle never goes
// negative and collides with an error code.
#define VFS_HANDLE_MOUNT_SHIFT 24
#define VFS_HANDLE_BACKEND_MASK 0x00FFFFFF
#define VFS_HANDLE_MOUNT_MAX 0x7F

#if VFS_MAX_MOUNTS >= VFS_HANDLE_MOUNT_MAX
#error "VFS_MAX_MOUNTS does not fit in the mount byte of a positive pdi_fhandle_t"
#endif

// Byte count an unbounded devfs node (/dev/zero, /dev/random, /dev/urandom)
// yields per read call. MCU-safe cap so `cat /dev/zero` cannot spin forever.
#ifndef DEVFS_STREAM_READ_MAX
#define DEVFS_STREAM_READ_MAX 64
#endif

#endif
