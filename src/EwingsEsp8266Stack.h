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
#include <service_provider/HttpUpdateServiceProvider.h>
#include <utility/Log.h>

#ifdef ENABLE_MQTT_CONFIG
#include <mqtt_client/Mqtt.h>
#endif

/**
 * @define general http parameters
 */
#define HTTP_HOST_ADDR_MAX_SIZE 100
#define HTTP_REQUEST_DURATION   10000
#define HTTP_REQUEST_RETRY      1

/**
 * EwingsEsp8266Stack class
 * @parent  PeriodicCallBack|public
 * @parent  DeviceFactoryReset|public
 * @parent  EwingsDefaultDB|public
 * @parent  NTPServiceProvider|public
 * @parent  HTTPUpdateServiceProvider|public
 */
class EwingsEsp8266Stack :
public PeriodicCallBack,
public DeviceFactoryReset,
public EwingsDefaultDB,
public NTPServiceProvider,
public HTTPUpdateServiceProvider{

  public:

    void initialize(void);
    void serve( void );

  protected:

    /**
		 * @var	uint8_t|18  wifi_connection_timeout
		 */
    uint8_t wifi_connection_timeout=18;
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
    bool configure_wifi_station( wifi_config_table* _wifi_credentials );
    void start_wifi(void);

    void handleFlashKeyPress( void );

    void handleLogPrints( void );
    void printWiFiConfigLogs( void );
    void printOtaConfigLogs( void );

    bool followHttpRequest( int _httpCode );

    void handleWiFiConnectivity( void );
    void handleOta( void );
    void connected_softap_client_info( void );

    #ifdef ENABLE_NAPT_FEATURE
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
