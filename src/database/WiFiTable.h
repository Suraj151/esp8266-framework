/******************************** WiFi Table **********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _WIFI_TABLE_H_
#define _WIFI_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * WiFiTable class extends public DatabaseTable as its base/parent class
 */
class WiFiTable : public DatabaseTable<wifi_config_table> {

  public:

    /**
     * WiFiTable constructor.
     */
    WiFiTable(){
    }

    /**
     * WiFiTable destructor.
     */
    ~WiFiTable(){
    }


    /**
     * register tables to database
     */
    void boot(){
      this->register_table( WIFI_CONFIG_TABLE_ADDRESS, &_wifi_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return wifi_config_table
     */
    wifi_config_table get(){
      return this->get_table(WIFI_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param wifi_config_table* _table
     */
    void set( wifi_config_table* _table ){
      this->set_table( _table, WIFI_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( WIFI_CONFIG_TABLE_ADDRESS );
    }
};

extern WiFiTable	__wifi_table;
#endif
