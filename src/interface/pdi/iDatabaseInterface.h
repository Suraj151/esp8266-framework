/***************************** Database Interface *****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st Jan 2024
******************************************************************************/

#ifndef _I_DATABASE_INTERFACE_H_
#define _I_DATABASE_INTERFACE_H_

#include <interface/interface_includes.h>


// forward declaration of derived class for this interface
class DatabaseInterface;

/**
 * iDatabaseInterface class
 */
class iDatabaseInterface
{

public:
  /**
   * iDatabaseInterface constructor.
   */
  iDatabaseInterface() {}
  /**
   * iDatabaseInterface destructor.
   */
  virtual ~iDatabaseInterface() {}

  virtual void beginConfigs(uint32_t _size) = 0;
  virtual void endConfigs() = 0;
  virtual void cleanAllConfigs() = 0;
  virtual uint32_t getMaxDBSize() = 0;

  /**
   * Byte level access to the config store. The database engine frames its own
   * records on top of these, writes stay buffered until commitConfigs().
   */
  virtual uint8_t readByte(uint32_t _address) = 0;
  virtual void writeByte(uint32_t _address, uint8_t _value) = 0;
  virtual void commitConfigs() = 0;
};

// derived class must define this
extern DatabaseInterface __i_db;

#endif
