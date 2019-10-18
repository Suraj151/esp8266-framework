/************************ N/W Time Protocol service ***************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _WIFI_SERVICE_PROVIDER_H_
#define _WIFI_SERVICE_PROVIDER_H_

#include <ESP8266WiFi.h>
#include <database/DefaultDatabase.h>
#include <service_provider/ServiceProvider.h>
#include <service_provider/PingServiceProvider.h>

/**
 * WiFiServiceProvider class
 */
class WiFiServiceProvider : public ServiceProvider {

  public:

    /**
     * WiFiServiceProvider constructor.
     */
    WiFiServiceProvider(){
    }

    /**
		 * WiFiServiceProvider destructor
		 */
		~WiFiServiceProvider(){
		}

    /**
		 * @var	uint8_t|18  wifi_connection_timeout
		 */
    uint8_t wifi_connection_timeout=15;

    /**
		 * @var	uint8_t array temperory mac buffer
		 */
    uint8_t temp_mac[6];

    void begin( ESP8266WiFiClass* _wifi );

    bool configure_wifi_access_point( wifi_config_table* _wifi_credentials );
    bool configure_wifi_station( wifi_config_table* _wifi_credentials, uint8_t* mac = NULL );

    bool scan_within_station( char* ssid, uint8_t* bssid );
    bool scan_within_station_async( char* ssid, uint8_t* bssid, int _scanCount );
    void scan_aps_and_configure_wifi_station( void );
    void scan_aps_and_configure_wifi_station_async( int _scanCount );

    void reconfigure_wifi_access_point( void );

    uint32_t getStationSubnetIP(void);
    uint32_t getStationBroadcastIP(void);

    void handleInternetConnectivity( void );
    void handleWiFiConnectivity( void );

    #ifdef EW_SERIAL_LOG
    void printWiFiConfigLogs( void );
    #endif


  protected:
    /**
		 * @var	ESP8266WiFiClass*|&WiFi wifi
		 */
    ESP8266WiFiClass* wifi;
};

extern WiFiServiceProvider __wifi_service;

#endif
