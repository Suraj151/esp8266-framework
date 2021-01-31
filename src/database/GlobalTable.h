/***************************** Global Table ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _GLOBAL_TABLE_H_
#define _GLOBAL_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * GlobalTable class extends public DatabaseTable as its base/parent class
 */
class GlobalTable : public DatabaseTable<global_config_table> {

  public:

    /**
     * GlobalTable constructor.
     */
    GlobalTable(){
    }

    /**
     * GlobalTable destructor.
     */
    ~GlobalTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( GLOBAL_CONFIG_TABLE_ADDRESS, &_global_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return global_config_table
     */
    global_config_table get(){
      return this->get_table(GLOBAL_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param global_config_table* _table
     */
    void set( global_config_table* _table ){
      this->set_table( _table, GLOBAL_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( GLOBAL_CONFIG_TABLE_ADDRESS );
    }
};

extern GlobalTable	__global_table;

#endif
