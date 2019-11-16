/************** Over The Air firmware update service **************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _HTTP_OTA_SERVICE_PROVIDER_H_
#define _HTTP_OTA_SERVICE_PROVIDER_H_

#include <service_provider/ServiceProvider.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

/**
 * ota status enum
 */
enum http_ota_status{

  BEGIN_FAILED,
  CONFIG_ERROR,
  VERSION_CHECK_FAILED,
  VERSION_JSON_ERROR,
  ALREADY_LATEST_VERSION,
  UPDATE_FAILD,
  NO_UPDATES,
  UPDATE_OK,
  UNKNOWN
};

/**
 * OtaServiceProvider class
 */
class OtaServiceProvider : public ServiceProvider{

  public:

    /**
     * @var	WiFiClient*|NULL	wifi_client
     */
    WiFiClient* wifi_client=NULL;

    /**
     * @var	HTTPClient*|NULL	http_client
     */
    HTTPClient* http_client=NULL;

    /**
     * OtaServiceProvider constructor.
     */
    OtaServiceProvider(){
    }

    /**
     * OtaServiceProvider destructor.
     */
    ~OtaServiceProvider(){
    }

    void begin_ota( WiFiClient* _wifi_client, HTTPClient* _http_client );
    void handleOta( void );
    http_ota_status handle( void );
    #ifdef EW_SERIAL_LOG
    void printOtaConfigLogs( void );
    #endif
};

extern OtaServiceProvider __ota_service;

#endif
