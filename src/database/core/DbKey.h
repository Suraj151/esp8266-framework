/**************************** Database Key ************************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

The key that seals the records holding credentials. It is kept at the top of the
eeprom, above the capacity the store reports, so records can never be placed
over it and so the key never reaches the container on storage. Nothing in the
shell can read the eeprom, which is what keeps a sealed container sealed to
anyone who copies it off the device.

Author          : Suraj I.
created Date    : 22nd Aug 2026
******************************************************************************/

#ifndef _DATABASE_KEY_H_
#define _DATABASE_KEY_H_

#include <config/Config.h>

#ifdef ENABLE_DB_SEALING

#include <interface/pdi.h>

/**
 * @class DbKey
 * @brief Holds the sealing key and puts one in the eeprom when none is there.
 */
class DbKey
{

public:
    DbKey();
    ~DbKey();

    /**
     * @brief Read the key from the eeprom, creating one when none is stored.
     *
     * The eeprom must be open. The key is kept in ram afterwards, so the eeprom
     * can be closed again while the device runs on the container.
     *
     * @return PDI_OK when a key is available.
     */
    pdi_err_t load();

    /**
     * @brief Forget the key held in ram.
     */
    void unload();

    /**
     * @brief Whether a key is available to seal with.
     */
    bool is_ready() const { return m_ready; }

    /**
     * @brief The sealing key, DB_KEY_BYTES long.
     */
    const uint8_t *key() const { return m_key; }

private:
    pdi_err_t create();

    uint8_t m_key[DB_KEY_BYTES];
    bool m_ready;
};

extern DbKey __db_key;

#endif

#endif
