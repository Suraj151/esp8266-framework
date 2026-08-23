/***************************** Database Interface *****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st Jan 2024
******************************************************************************/

#include "DatabaseInterface.h"

/**
 * begin eeprom configs.
 *
 * @param uint16_t  _size
 */
void DatabaseInterface::beginConfigs(uint32_t _size)
{
  EEPROM.begin(_size);
}

/**
 * close the eeprom, committing anything staged and releasing its buffer.
 */
void DatabaseInterface::endConfigs()
{
  EEPROM.end();
}

/**
 * clear eeprom by writing zero ot its all locations.
 */
void DatabaseInterface::cleanAllConfigs(void)
{
  for (uint16_t i = 0; i < DATABASE_MAX_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.end();
}

/**
 * maximum database size can be stored
 *
 * @return max db size
 */
uint32_t DatabaseInterface::getMaxDBSize()
{
    return DATABASE_MAX_SIZE;
}

/**
 * read one byte from the config store.
 *
 * @param   uint32_t  _address
 * @return  byte at the address
 */
uint8_t DatabaseInterface::readByte(uint32_t _address)
{
  return EEPROM.read(_address);
}

/**
 * stage one byte into the config store, commitConfigs() persists it.
 *
 * @param   uint32_t  _address
 * @param   uint8_t   _value
 */
void DatabaseInterface::writeByte(uint32_t _address, uint8_t _value)
{
  EEPROM.write(_address, _value);
}

/**
 * persist every staged byte.
 */
void DatabaseInterface::commitConfigs()
{
  EEPROM.commit();
}

DatabaseInterface __i_db;
