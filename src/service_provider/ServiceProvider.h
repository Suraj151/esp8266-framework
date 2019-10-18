/***************************** service provider *******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _SERVICE_PROVIDER_H_
#define _SERVICE_PROVIDER_H_

#include <config/Config.h>
#include <utility/Utility.h>
extern "C" {
#include "user_interface.h"
}

/**
 * ServiceProvider class
 */
class ServiceProvider{

  public:

    /**
     * ServiceProvider constructor.
     */
    ServiceProvider(){
    }

    /**
		 * ServiceProvider destructor
		 */
    ~ServiceProvider(){
    }
};

#endif
