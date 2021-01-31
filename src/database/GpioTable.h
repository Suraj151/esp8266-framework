/******************************* GPIO Table ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _GPIO_TABLE_H_
#define _GPIO_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * GpioTable class extends public DatabaseTable as its base/parent class
 */
class GpioTable : public DatabaseTable<gpio_config_table> {

  public:

    /**
     * GpioTable constructor.
     */
    GpioTable(){
    }

    /**
     * GpioTable destructor.
     */
    ~GpioTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( GPIO_CONFIG_TABLE_ADDRESS, &_gpio_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return gpio_config_table
     */
    gpio_config_table get(){
      return this->get_table(GPIO_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param gpio_config_table* _table
     */
    void set( gpio_config_table* _table ){
      this->set_table( _table, GPIO_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( GPIO_CONFIG_TABLE_ADDRESS );
    }
};

extern GpioTable	__gpio_table;
#endif
