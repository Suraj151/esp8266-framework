/**************************** Eeprom Db Store *********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Db store over the device eeprom, reached through the device database interface.
Bytes are compared before being staged so an unchanged record never dirties the
store, and a flush with nothing pending never reaches the device.

Author          : Suraj I.
Created Date    : 22nd Aug 2026
******************************************************************************/

#ifndef _EEPROM_DBSTORE_H_
#define _EEPROM_DBSTORE_H_

#include <interface/pdi/modules/database/iDbStoreInterface.h>

/**
 * @class EepromDbStore
 * @brief Byte device backed by the device eeprom.
 */
class EepromDbStore : public iDbStoreInterface {
public:

    /**
     * @brief Constructor to initialize the EepromDbStore.
     */
    EepromDbStore();

    /**
     * @brief Destructor for the EepromDbStore.
     */
    virtual ~EepromDbStore();

    pdi_err_t init() override;
    pdi_err_t deinit() override;
    bool read(uint32_t offset, uint8_t *buf, uint32_t len) override;
    bool write(uint32_t offset, const uint8_t *buf, uint32_t len) override;
    bool flush() override;
    uint32_t capacity() override;

private:
    bool m_dirty;
};

extern EepromDbStore __i_eeprom_dbstore;

#endif
