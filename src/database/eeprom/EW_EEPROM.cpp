/***************************** Custom EEPROM **********************************
This file is part of the Ewings Esp8266 Stack. It is modified/edited copy of
arduino esp8266 eeprom library.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#include "Arduino.h"
#include "EW_EEPROM.h"

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "spi_flash.h"
}

extern "C" uint32_t _SPIFFS_end;

EW_EEPROMClass::EW_EEPROMClass(uint32_t sector)
: _sector(sector)
, _data(0)
, _size(0)
, _dirty(false)
{
}

EW_EEPROMClass::EW_EEPROMClass(void)
: _sector((((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE))
, _data(0)
, _size(0)
, _dirty(false)
{
}

void EW_EEPROMClass::begin(size_t size) {
  if (size <= 0)
    return;
  if (size > SPI_FLASH_SEC_SIZE)
    size = SPI_FLASH_SEC_SIZE;

  size = (size + 3) & (~3);

  //In case begin() is called a 2nd+ time, don't reallocate if size is the same
  if(_data && size != _size) {
    delete[] _data;
    _data = new uint8_t[size];
  } else if(!_data) {
    _data = new uint8_t[size];
  }

  _size = size;
  _copy_sector = 255;

  noInterrupts();

  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size);
  if( _data[0] == EEPROM_VALIDITY_BYTES[0] && _data[1] == EEPROM_VALIDITY_BYTES[1] &&
      _data[2] == EEPROM_VALIDITY_BYTES[2] && _data[3] == EEPROM_VALIDITY_BYTES[3] ){

  }else{
    spi_flash_read(_copy_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size);
    if( _data[0] == EEPROM_VALIDITY_BYTES[0] && _data[1] == EEPROM_VALIDITY_BYTES[1] &&
        _data[2] == EEPROM_VALIDITY_BYTES[2] && _data[3] == EEPROM_VALIDITY_BYTES[3] ){

        if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {
          if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size) == SPI_FLASH_RESULT_OK) {
          }
        }
    }else{

        _data[0] = EEPROM_VALIDITY_BYTES[0];
        _data[1] = EEPROM_VALIDITY_BYTES[1];
        _data[2] = EEPROM_VALIDITY_BYTES[2];
        _data[3] = EEPROM_VALIDITY_BYTES[3];
    }
  }
  interrupts();

  _dirty = false; //make sure dirty is cleared in case begin() is called 2nd+ time
}

void EW_EEPROMClass::end() {
  if (!_size)
    return;

  commit();
  if(_data) {
    delete[] _data;
  }
  _data = 0;
  _size = 0;
  _dirty = false;
}


uint8_t EW_EEPROMClass::read(int const address) {
  if (address < 0 || (size_t)address >= _size)
    return 0;
  if(!_data)
    return 0;

  return _data[address];
}

void EW_EEPROMClass::write(int const address, uint8_t const value) {
  if (address < 0 || (size_t)address >= _size)
    return;
  if(!_data)
    return;

  // Optimise _dirty. Only flagged if data written is different.
  uint8_t* pData = &_data[address];
  if (*pData != value)
  {
    *pData = value;
    _dirty = true;
  }
}

bool EW_EEPROMClass::commit() {
  bool ret = false;
  if (!_size)
    return false;
  if(!_dirty)
    return true;
  if(!_data)
    return false;

  noInterrupts();
  if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {
    if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size) == SPI_FLASH_RESULT_OK) {
      _dirty = false;
      ret = true;
    }
  }
  if(spi_flash_erase_sector(_copy_sector) == SPI_FLASH_RESULT_OK) {
    if(spi_flash_write(_copy_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(_data), _size) == SPI_FLASH_RESULT_OK) {
    }
  }

  interrupts();

  return ret;
}

uint8_t * EW_EEPROMClass::getDataPtr() {
  _dirty = true;
  return &_data[0];
}

uint8_t const * EW_EEPROMClass::getConstDataPtr() const {
  return &_data[0];
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EEPROM)
EW_EEPROMClass EEPROM;
#endif
