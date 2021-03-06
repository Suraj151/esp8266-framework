/******************************* Email Table ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EMAIL_TABLE_H_
#define _EMAIL_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * EmailTable class extends public DatabaseTable as its base/parent class
 */
class EmailTable : public DatabaseTable<email_config_table> {

  public:

    /**
     * EmailTable constructor.
     */
    EmailTable(){
    }

    /**
     * EmailTable destructor.
     */
    ~EmailTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( EMAIL_CONFIG_TABLE_ADDRESS, &_email_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return email_config_table
     */
    email_config_table get(){
      return this->get_table(EMAIL_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param email_config_table* _table
     */
    void set( email_config_table* _table ){
      this->set_table( _table, EMAIL_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( EMAIL_CONFIG_TABLE_ADDRESS );
    }
};

extern EmailTable	__email_table;
#endif
