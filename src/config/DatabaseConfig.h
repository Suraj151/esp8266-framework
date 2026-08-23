/*************************** Database Config page *****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 22nd Aug 2026
******************************************************************************/
#ifndef _DATABASE_CONFIG_H_
#define _DATABASE_CONFIG_H_

#include "Common.h"

/**
 * Where the live database is kept when the device has storage. The eeprom
 * holds the defaults the device falls back to, this holds what it is running.
 */
#ifndef DB_STORE_DIR
#define DB_STORE_DIR "/etc/database"
#endif

#ifndef DB_STORE_FILE
#define DB_STORE_FILE "/etc/database/pdi.db"
#endif

/**
 * The database carries credentials, so only its owner may reach it.
 */
#ifndef DB_STORE_DIR_PERMS
#define DB_STORE_DIR_PERMS 0700
#endif

#ifndef DB_STORE_FILE_PERMS
#define DB_STORE_FILE_PERMS 0600
#endif

/**
 * Bytes the container reserves. One littlefs block holds it, and keeping it
 * equal to the eeprom size lets a tier be copied to the other as it stands.
 */
#ifndef DB_CONTAINER_BYTES
#define DB_CONTAINER_BYTES 4096
#endif

/**
 * Bytes moved per filesystem call. Reads land on a stack buffer of this size,
 * so it stays small enough for the device that has the least stack.
 */
#ifndef DB_FS_IO_CHUNK_BYTES
#define DB_FS_IO_CHUNK_BYTES 64
#endif

/**
 * Records holding a credential are sealed, and the key that seals them is kept
 * in the eeprom, which the shell and sftp cannot reach. Reading the container
 * off the filesystem therefore yields ciphertext. Physical extraction of the
 * flash is not covered by this.
 */
#ifndef DB_KEY_BYTES
#define DB_KEY_BYTES 16
#endif

/**
 * Bytes reserved at the top of the eeprom for the key. The store keeps its
 * capacity below this, so records can never be placed over it.
 */
#ifndef DB_KEY_REGION_BYTES
#define DB_KEY_REGION_BYTES 32
#endif

/**
 * Bytes a sealed record carries on top of its payload, the nonce ahead of the
 * ciphertext and the tag behind it.
 */
#define DB_SEAL_IV_BYTES 16
#define DB_SEAL_TAG_BYTES 16
#define DB_SEAL_OVERHEAD (DB_SEAL_IV_BYTES + DB_SEAL_TAG_BYTES)

/**
 * Bytes sealed per pass. A multiple of the cipher block keeps the counter
 * running across passes, so a record is never held in memory whole.
 */
#ifndef DB_SEAL_CHUNK_BYTES
#define DB_SEAL_CHUNK_BYTES 64
#endif

#endif
