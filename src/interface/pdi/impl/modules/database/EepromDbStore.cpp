/**************************** Eeprom Db Store *********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 22nd Aug 2026
******************************************************************************/

#include "EepromDbStore.h"
#include <interface/pdi.h>

/**
 * Constructor
 */
EepromDbStore::EepromDbStore() : m_dirty(false)
{
}

/**
 * Destructor
 */
EepromDbStore::~EepromDbStore()
{
}

/**
 * open the eeprom for the whole space it offers.
 *
 * @return  pdi_err_t
 */
pdi_err_t EepromDbStore::init()
{
    __i_db.beginConfigs(__i_db.getMaxDBSize());
    return PDI_OK;
}

/**
 * close the eeprom, releasing whatever it holds in ram.
 *
 * @return  pdi_err_t
 */
pdi_err_t EepromDbStore::deinit()
{
    __i_db.endConfigs();
    m_dirty = false;
    return PDI_OK;
}

/**
 * copy a byte range out of the store.
 *
 * @param   uint32_t  offset
 * @param   uint8_t*  buf
 * @param   uint32_t  len
 * @return  bool
 */
bool EepromDbStore::read(uint32_t offset, uint8_t *buf, uint32_t len)
{
    if (offset + len > capacity())
    {
        return false;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        buf[i] = __i_db.readByte(offset + i);
    }

    return true;
}

/**
 * stage a byte range into the store, only the bytes that actually differ.
 *
 * @param   uint32_t        offset
 * @param   const uint8_t*  buf
 * @param   uint32_t        len
 * @return  bool
 */
bool EepromDbStore::write(uint32_t offset, const uint8_t *buf, uint32_t len)
{
    if (offset + len > capacity())
    {
        return false;
    }

    for (uint32_t i = 0; i < len; i++)
    {
        if (__i_db.readByte(offset + i) != buf[i])
        {
            __i_db.writeByte(offset + i, buf[i]);
            m_dirty = true;
        }
    }

    return true;
}

/**
 * persist everything staged since the last flush.
 *
 * @return  bool
 */
bool EepromDbStore::flush()
{
    if (m_dirty)
    {
        __i_db.commitConfigs();
        m_dirty = false;
    }

    return true;
}

/**
 * total bytes the store holds.
 *
 * @return  uint32_t
 */
uint32_t EepromDbStore::capacity()
{
#ifdef ENABLE_DB_SEALING
    // the sealing key sits above this, so a record can never be placed over it
    return __i_db.getMaxDBSize() - DB_KEY_REGION_BYTES;
#else
    return __i_db.getMaxDBSize();
#endif
}

EepromDbStore __i_eeprom_dbstore;
