/****************************** ping service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _PING_SERVICE_PROVIDER_H_
#define _PING_SERVICE_PROVIDER_H_

#include <service_provider/ServiceProvider.h>

extern "C" {
  #include <ping.h>
}

// This function is called when a ping is received or the request times out:
static void ICACHE_FLASH_ATTR ping_recv_cb(void* arg, void* pdata);

/**
 * PingServiceProvider class
 */
class PingServiceProvider : public ServiceProvider {

  public:

    ping_option _opt;
    // bool host_resp;

    /**
     * PingServiceProvider constructor.
     */
    PingServiceProvider(){
    }

    /**
		 * PingServiceProvider destructor
		 */
    ~PingServiceProvider(){
    }

    /**
     * initialize ping
     */
    void init_ping( ESP8266WiFiClass* _wifi );
    bool ping( void );
    bool isHostRespondingToPing( void );

  protected:
    /**
		 * @var	ESP8266WiFiClass*|&WiFi wifi
		 */
    ESP8266WiFiClass* wifi;
};

extern PingServiceProvider __ping_service;

#endif
