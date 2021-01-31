/****************************** ping service **********************************
This file is part of the Ewings Esp Stack.

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

    /**
     * PingServiceProvider constructor.
     */
    PingServiceProvider();
    /**
		 * PingServiceProvider destructor
		 */
    ~PingServiceProvider();

    /**
     * initialize ping
     */
    void init_ping( iWiFiInterface* _wifi );
    bool ping( void );
    bool isHostRespondingToPing( void );

    ping_option m_opt;
    // bool host_resp;

  protected:
    /**
		 * @var	iWiFiInterface*|&WiFi wifi
		 */
    iWiFiInterface  *m_wifi;
};

extern PingServiceProvider __ping_service;

#endif
