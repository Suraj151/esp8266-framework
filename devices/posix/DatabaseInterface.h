/***************************** Database Interface *****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st Jan 2024
******************************************************************************/

#ifndef _PDI_POSIX_DATABASE_INTERFACE_H_
#define _PDI_POSIX_DATABASE_INTERFACE_H_

#include "posix.h"
#include <interface/pdi/iDatabaseInterface.h>

/**
 * define eeprom max size available here
 */
#define DATABASE_MAX_SIZE 4096

/**
 * DatabaseInterface class
 *
 * Keeps the config store in a plain byte array. A backing file can be attached
 * so a store survives across runs the way real NVM does.
 */
class DatabaseInterface : public iDatabaseInterface
{

public:
  /**
   * DatabaseInterface constructor.
   */
  DatabaseInterface();

  /**
   * DatabaseInterface destructor.
   */
  virtual ~DatabaseInterface();

  void beginConfigs(uint32_t _size) override;
  void endConfigs() override;
  void cleanAllConfigs() override;
  uint32_t getMaxDBSize() override;

  /**
   * @brief Back the store with a file, loading it when it already exists.
   *        Every later write is persisted to it.
   */
  bool attachBackingFile(const char *path);

  /**
   * @brief Drop the backing file and keep the store in memory only.
   */
  void detachBackingFile();

  uint8_t readByte(uint32_t _address) override;
  void writeByte(uint32_t _address, uint8_t _value) override;
  void commitConfigs() override;

private:
  uint8_t m_store[DATABASE_MAX_SIZE];
  uint32_t m_size;
  pdiutil::string m_backingfile;
};

#endif // _PDI_POSIX_DATABASE_INTERFACE_H_
