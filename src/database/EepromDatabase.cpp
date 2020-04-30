/*************************** EEPROM Database **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "EepromDatabase.h"

/**
 * begin eeprom configs.
 *
 * @param uint16_t  _size
 */
void beginConfigs(  uint16_t _size ){
  EEPROM.begin(_size);
}

/**
 * clear eeprom by writing zero ot its all locations.
 */
void cleanAllConfigs( void ){
  for (int i = 0; i < DATABASE_MAX_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.end();
}
