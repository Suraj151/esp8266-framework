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

#define FLASH_KEY_PIN             D3
#define FLASH_KEY_PRESS_DURATION  1000
#define FLASH_KEY_PRESS_COUNT_THR 10

#define HTTP_HOST_ADDR_MAX_SIZE 100
#define HTTP_REQUEST_DURATION   10000
#define HTTP_REQUEST_RETRY      1

#ifdef ENABLE_GPIO_CONFIG
#define GPIO_OPERATION_DURATION 1000
#define GPIO_TABLE_UPDATE_DURATION 300000
#endif

#define WIFI_CONNECTIVITY_CHECK_DURATION 30000

#define ENABLE_NAPT_FEATURE

#ifdef ENABLE_NAPT_FEATURE
#include "lwip/lwip_napt.h"
#include "lwip/app/dhcpserver.h"
#endif


class EwingsDefaultDB;

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

    uint8_t wifi_connection_timeout=18;
    uint8_t flash_key_pressed=0;

    ESP8266WiFiClass* wifi = &WiFi;
    ESP8266WebServer EwServer;
    EwRouteHandler RouteHandler;
    HTTPClient http_request;
    WiFiClient wifi_client;
    #ifdef ENABLE_MQTT_CONFIG
    MQTTClient mqtt_client;
    #endif

    char http_host[HTTP_HOST_ADDR_MAX_SIZE];
    int http_port=80;
    int http_retry=HTTP_REQUEST_RETRY;

    bool configure_wifi_access_point( wifi_config_table* _wifi_credentials );
    bool configure_wifi_station( wifi_config_table* _wifi_credentials );
    void start_wifi(void);
    void start_http_server( void );

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
    gpio_config_table virtual_gpio_configs;
    int _gpio_http_request_cb_id=0;
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
    // mqtt_general_config_table _mqtt_general_configs;
    // mqtt_lwt_config_table _mqtt_lwt_configs;
    // mqtt_pubsub_config_table _mqtt_pubsub_configs;
    int _mqtt_timer_cb_id=0;
    int _mqtt_publish_cb_id=0;
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
