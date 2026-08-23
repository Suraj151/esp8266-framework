/**************************** Database Key ************************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 22nd Aug 2026
******************************************************************************/

#include "DbKey.h"

#ifdef ENABLE_DB_SEALING

#include <utility/Checksum.h>

static const uint8_t DB_KEY_MAGIC[2] = {'P', 'K'};

/**
 * first byte of the key region, kept above what the store hands out.
 */
static uint32_t db_key_region_start()
{
    return __i_db.getMaxDBSize() - DB_KEY_REGION_BYTES;
}

/**
 * Constructor
 */
DbKey::DbKey() : m_ready(false)
{
    memset(m_key, 0, DB_KEY_BYTES);
}

/**
 * Destructor
 */
DbKey::~DbKey()
{
    memset(m_key, 0, DB_KEY_BYTES);
}

/**
 * read the key from the eeprom, creating one when none is stored.
 *
 * @return  pdi_err_t
 */
pdi_err_t DbKey::load()
{
    uint32_t _at = db_key_region_start();
    uint8_t _stored[2 + DB_KEY_BYTES + 2];

    for (uint8_t i = 0; i < sizeof(_stored); i++)
    {
        _stored[i] = __i_db.readByte(_at + i);
    }

    bool _valid = _stored[0] == DB_KEY_MAGIC[0] && _stored[1] == DB_KEY_MAGIC[1];

    if (_valid)
    {
        uint16_t _crc = (uint16_t)_stored[2 + DB_KEY_BYTES] | (uint16_t)((uint16_t)_stored[3 + DB_KEY_BYTES] << 8);
        _valid = _crc == crc16Ccitt(&_stored[2], DB_KEY_BYTES);
    }

    if (!_valid)
    {
        return this->create();
    }

    memcpy(m_key, &_stored[2], DB_KEY_BYTES);
    m_ready = true;

    return PDI_OK;
}

/**
 * put a fresh key in the eeprom and take it into use.
 *
 * @return  pdi_err_t
 */
pdi_err_t DbKey::create()
{
    for (uint8_t i = 0; i < DB_KEY_BYTES; i += 4)
    {
        uint32_t _r = __i_dvc_ctrl.random_now();

        for (uint8_t b = 0; b < 4 && (i + b) < DB_KEY_BYTES; b++)
        {
            m_key[i + b] = (uint8_t)((_r >> (b * 8)) & 0xFF);
        }
    }

    uint32_t _at = db_key_region_start();
    uint16_t _crc = crc16Ccitt(m_key, DB_KEY_BYTES);

    __i_db.writeByte(_at, DB_KEY_MAGIC[0]);
    __i_db.writeByte(_at + 1, DB_KEY_MAGIC[1]);

    for (uint8_t i = 0; i < DB_KEY_BYTES; i++)
    {
        __i_db.writeByte(_at + 2 + i, m_key[i]);
    }

    __i_db.writeByte(_at + 2 + DB_KEY_BYTES, (uint8_t)(_crc & 0xFF));
    __i_db.writeByte(_at + 3 + DB_KEY_BYTES, (uint8_t)((_crc >> 8) & 0xFF));
    __i_db.commitConfigs();

    m_ready = true;

    return PDI_OK;
}

/**
 * forget the key held in ram.
 */
void DbKey::unload()
{
    memset(m_key, 0, DB_KEY_BYTES);
    m_ready = false;
}

DbKey __db_key;

#endif
