/************************** Ewings Esp8266 Stack ******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EWINGS_ESP8266_STACK_H_
#define _EWINGS_ESP8266_STACK_H_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#include <database/EwingsDefaultDB.h>
#include <webserver/EwingsWebServer.h>
#include <service_provider/NtpServiceProvider.h>
#include <service_provider/PingServiceProvider.h>
#include <service_provider/HttpUpdateServiceProvider.h>

#ifdef ENABLE_ESP_NOW
#include <service_provider/ESPNOWServiceProvider.h>
#endif

#include <utility/Log.h>

#ifdef ENABLE_MQTT_CONFIG
#include <mqtt_client/Mqtt.h>
#endif

#if defined( ENABLE_NAPT_FEATURE )
#include "lwip/lwip_napt.h"
#include "lwip/app/dhcpserver.h"

#elif defined( ENABLE_NAPT_FEATURE_LWIP_V2 )

#include <lwip/napt.h>
#include <lwip/dns.h>
#include <dhcpserver.h>
#endif

/**
 * EwingsEsp8266Stack class
 * @parent  TaskScheduler|public
 * @parent  DeviceFactoryReset|public
 * @parent  EwingsDefaultDB|public
 * @parent  NTPServiceProvider|public
 * @parent  PingServiceProvider|public
 * @parent  HTTPUpdateServiceProvider|public
 * @parent  ESPNOWServiceProvider|public
 */
class EwingsEsp8266Stack :
public TaskScheduler,
public DeviceFactoryReset,
public EwingsDefaultDB,
public NTPServiceProvider,
public PingServiceProvider,
#ifdef ENABLE_ESP_NOW
public ESPNOWServiceProvider,
#endif
public HTTPUpdateServiceProvider
{

  public:

    void initialize(void);
    void serve( void );

  protected:

    /**
		 * @var	uint8_t|18  wifi_connection_timeout
		 */
    uint8_t wifi_connection_timeout=15;
    /**
		 * @var	uint8_t|0 flash_key_pressed
		 */
    uint8_t flash_key_pressed=0;

    #ifdef ENABLE_EWING_HTTP_SERVER
    /**
		 * @var	ESP8266WebServer  EwServer
		 */
    ESP8266WebServer EwServer;
    /**
		 * @var	EwRouteHandler  RouteHandler
		 */
    EwRouteHandler RouteHandler;
    void start_http_server( void );
    #endif

    /**
		 * @var	ESP8266WiFiClass*|&WiFi wifi
		 */
    ESP8266WiFiClass* wifi = &WiFi;
    /**
		 * @var	HTTPClient  http_request
		 */
    HTTPClient http_request;
    /**
		 * @var	WiFiClient  wifi_client
		 */
    WiFiClient wifi_client;
    #ifdef ENABLE_MQTT_CONFIG
    /**
		 * @var	MQTTClient  mqtt_client
		 */
    MQTTClient mqtt_client;
    #endif

    /**
		 * @var	uint8_t array temperory mac buffer
		 */
    uint8_t temp_mac[6];
    /**
		 * @var	char array http_host
		 */
    char http_host[HTTP_HOST_ADDR_MAX_SIZE];
    /**
		 * @var	int|80  http_port
		 */
    int http_port=80;
    /**
		 * @var	int|HTTP_REQUEST_RETRY  http_retry
		 */
    int http_retry=HTTP_REQUEST_RETRY;

    bool configure_wifi_access_point( wifi_config_table* _wifi_credentials );
    bool configure_wifi_station( wifi_config_table* _wifi_credentials, uint8_t* mac = NULL );

    bool scan_within_station( char* ssid, uint8_t* bssid );
    bool scan_within_station_async( char* ssid, uint8_t* bssid, int _scanCount );
    void scan_aps_and_configure_wifi_station( void );
    void scan_aps_and_configure_wifi_station_async( int _scanCount );

    void reconfigure_wifi_access_point( void );
    void start_wifi(void);
    uint32_t getStationSubnetIP(void);
    uint32_t getStationBroadcastIP(void);

    void handleFlashKeyPress( void );

    void handleLogPrints( void );
    void printWiFiConfigLogs( void );
    void printOtaConfigLogs( void );

    bool followHttpRequest( int _httpCode );

    void handleInternetConnectivity( void );
    void handleWiFiConnectivity( void );
    void handleOta( void );
    void connected_softap_client_info( void );

    #if defined( ENABLE_NAPT_FEATURE ) || defined( ENABLE_NAPT_FEATURE_LWIP_V2 )
    void enable_napt_service( void );
    #endif

    #ifdef ENABLE_GPIO_CONFIG
    /**
		 * @var gpio_config_table virtual_gpio_configs
		 */
    gpio_config_table virtual_gpio_configs;
    /**
		 * @var	int|0 _gpio_http_request_cb_id
		 */
    int _gpio_http_request_cb_id=0;
    /**
		 * @var	bool|true update_gpio_table_from_virtual
		 */
    bool update_gpio_table_from_virtual=true;
    void enable_update_gpio_table_from_virtual( void );
    void handleGpioOperations( void );
    void handleGpioModes( int _gpio_config_type=GPIO_MODE_CONFIG );
    uint8_t getGpioFromPinMap( uint8_t _pin );
    void start_gpio_service( void );
    void handleGpioHttpRequest( void );
    void printGpioConfigLogs( void );
    #endif

    #ifdef ENABLE_MQTT_CONFIG
    /**
		 * @var	int|0 _mqtt_timer_cb_id
		 */
    int _mqtt_timer_cb_id=0;
    /**
		 * @var	int|0 _mqtt_publish_cb_id
		 */
    int _mqtt_publish_cb_id=0;
    /**
		 * @var	int|0 _mqtt_subscribe_cb_id
		 */
    int _mqtt_subscribe_cb_id=0;
    void start_mqtt_service( void );
    void handleMqttPublish( void );
    void handleMqttSubScribe( void );
    void handleMqttConfigChange( int _mqtt_config_type=MQTT_GENERAL_CONFIG );
    void printMqttConfigLogs( void );
    #endif

};

extern EwingsEsp8266Stack EwStack;

#endif
