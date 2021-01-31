/****************************** Login Table ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _LOGIN_CREDENTIAL_TABLE_H_
#define _LOGIN_CREDENTIAL_TABLE_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * LoginTable class extends public DatabaseTable as its base/parent class
 */
class LoginTable : public DatabaseTable<login_credential_table> {

  public:

    /**
     * LoginTable constructor.
     */
    LoginTable(){
    }

    /**
     * LoginTable destructor.
     */
    ~LoginTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( LOGIN_CREDENTIAL_TABLE_ADDRESS, &_login_credential_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return login_credential_table
     */
    login_credential_table get(){
      return this->get_table(LOGIN_CREDENTIAL_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param login_credential_table* _table
     */
    void set( login_credential_table* _table ){
      this->set_table( _table, LOGIN_CREDENTIAL_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( LOGIN_CREDENTIAL_TABLE_ADDRESS );
    }
};

extern LoginTable	__login_table;
#endif
