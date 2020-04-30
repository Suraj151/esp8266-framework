/***************************** Custom EEPROM **********************************
This file is part of the Ewings Esp8266 Stack. It is modified/edited copy of
arduino esp8266 eeprom library.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_EEPROM_H_
#define _EW_EEPROM_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define EEPROM_VALIDITY_BYTES "EWEE"

class EW_EEPROMClass {
public:
  EW_EEPROMClass(uint32_t sector);
  EW_EEPROMClass(void);

  void begin(size_t size);
  uint8_t read(int const address);
  void write(int const address, uint8_t const val);
  bool commit();
  void end();

  uint8_t * getDataPtr();
  uint8_t const * getConstDataPtr() const;

  // template<typename T>
  // T &get(int const address, T &t) {
  //   if (address < 0 || address + sizeof(T) > _size)
  //     return t;
  //
  //   memcpy((uint8_t*) &t, _data + address, sizeof(T));
  //   return t;
  // }
  //
  // template<typename T>
  // const T &put(int const address, const T &t) {
  //   if (address < 0 || address + sizeof(T) > _size)
  //     return t;
  //   if (memcmp(_data + address, (const uint8_t*)&t, sizeof(T)) != 0) {
  //     _dirty = true;
  //     memcpy(_data + address, (const uint8_t*)&t, sizeof(T));
  //   }
  //
  //   return t;
  // }

  size_t length() {return _size;}

  uint8_t& operator[](int const address) {return getDataPtr()[address];}
  uint8_t const & operator[](int const address) const {return getConstDataPtr()[address];}

protected:
  uint32_t _sector;
  uint32_t _copy_sector=0;
  uint8_t* _data;
  size_t _size;
  bool _dirty;
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
extern EW_EEPROMClass EEPROM;
#endif

#endif
