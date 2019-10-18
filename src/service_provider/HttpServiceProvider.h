/*************************** Http Protocol service ****************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _HTTP_SERVICE_PROVIDER_H_
#define _HTTP_SERVICE_PROVIDER_H_

#include <service_provider/ServiceProvider.h>
#include <ESP8266HTTPClient.h>

/**
 * HttpServiceProvider class
 */
class HttpServiceProvider : public ServiceProvider {

  public:

    /**
     * HttpServiceProvider constructor.
     */
    HttpServiceProvider(){
    }

    /**
		 * HttpServiceProvider destructor
		 */
    ~HttpServiceProvider(){
    }

    /**
		 * @var	HTTPClient  http_service
		 */
    HTTPClient client;
    /**
		 * @var	char array host
		 */
    char host[HTTP_HOST_ADDR_MAX_SIZE];
    /**
		 * @var	int|80  port
		 */
    int port=80;
    /**
		 * @var	int|HTTP_REQUEST_RETRY  retry
		 */
    int retry=HTTP_REQUEST_RETRY;


    bool followHttpRequest( int _httpCode );
};

extern HttpServiceProvider __http_service;

#endif
