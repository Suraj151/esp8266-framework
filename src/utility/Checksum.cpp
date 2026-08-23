/******************************* Checksum *************************************
This file is part of the PDI stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 22nd Aug 2026
******************************************************************************/

#include "Checksum.h"

/**
 * continue a crc16 ccitt-false over a further byte range.
 *
 * @param   uint16_t        _crc
 * @param   const uint8_t*  _buf
 * @param   uint32_t        _len
 * @return  uint16_t
 */
uint16_t crc16CcittUpdate(uint16_t _crc, const uint8_t *_buf, uint32_t _len)
{
    for (uint32_t i = 0; i < _len; i++)
    {
        _crc ^= (uint16_t)((uint16_t)_buf[i] << 8);

        for (uint8_t b = 0; b < 8; b++)
        {
            _crc = (_crc & 0x8000) ? (uint16_t)((uint16_t)(_crc << 1) ^ 0x1021) : (uint16_t)(_crc << 1);
        }
    }

    return _crc;
}

/**
 * crc16 ccitt-false over a byte range.
 *
 * @param   const uint8_t*  _buf
 * @param   uint32_t        _len
 * @return  uint16_t
 */
uint16_t crc16Ccitt(const uint8_t *_buf, uint32_t _len)
{
    return crc16CcittUpdate(0xFFFF, _buf, _len);
}
