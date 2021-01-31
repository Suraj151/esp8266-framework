/******************************* OTA Table ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _DEVICE_IOT_TABLE_H_
#define _DEVICE_IOT_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * DeviceIotTable class extends public DatabaseTable as its base/parent class
 */
class DeviceIotTable : public DatabaseTable<device_iot_config_table> {

  public:

    /**
     * DeviceIotTable constructor.
     */
    DeviceIotTable(){
    }

    /**
     * DeviceIotTable destructor.
     */
    ~DeviceIotTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( DEVICE_IOT_CONFIG_TABLE_ADDRESS, &_device_iot_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return device_iot_config_table
     */
    device_iot_config_table get(){
      return this->get_table(DEVICE_IOT_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param device_iot_config_table* _table
     */
    void set( device_iot_config_table* _table ){
      this->set_table( _table, DEVICE_IOT_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( DEVICE_IOT_CONFIG_TABLE_ADDRESS );
    }
};

extern DeviceIotTable	__device_iot_table;
#endif
