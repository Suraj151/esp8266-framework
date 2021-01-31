/******************************* OTA Table ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _OTA_TABLE_H_
#define _OTA_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * OtaTable class extends public DatabaseTable as its base/parent class
 */
class OtaTable : public DatabaseTable<ota_config_table> {

  public:

    /**
     * OtaTable constructor.
     */
    OtaTable(){
    }

    /**
     * OtaTable destructor.
     */
    ~OtaTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( OTA_CONFIG_TABLE_ADDRESS, &_ota_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return ota_config_table
     */
    ota_config_table get(){
      return this->get_table(OTA_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param ota_config_table* _table
     */
    void set( ota_config_table* _table ){
      this->set_table( _table, OTA_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( OTA_CONFIG_TABLE_ADDRESS );
    }
};

extern OtaTable	__ota_table;
#endif
