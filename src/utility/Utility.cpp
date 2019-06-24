/******************************* Utility **************************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "Utility.h"

#ifdef ENABLE_GPIO_CONFIG
bool is_exceptional_gpio_pin (uint8_t _pin) {

  for (uint8_t j = 0; j < sizeof(EXCEPTIONAL_GPIO_PINS); j++) {

    if( EXCEPTIONAL_GPIO_PINS[j] == _pin )return true;
  }
  return false;
}
#endif
