/****************************** Mqtt service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _MQTT_SERVICE_PROVIDER_H_
#define _MQTT_SERVICE_PROVIDER_H_


#include <database/DefaultDatabase.h>
#include <service_provider/ServiceProvider.h>
#ifdef ENABLE_GPIO_CONFIG
#include <service_provider/GpioServiceProvider.h>
#endif
#include <mqtt_client/Mqtt.h>


/**
 * MqttServiceProvider class
 */
class MqttServiceProvider : public ServiceProvider {

  public:

    /**
     * MqttServiceProvider constructor.
     */
    MqttServiceProvider(){
    }

    /**
		 * MqttServiceProvider destructor
		 */
    ~MqttServiceProvider(){
    }

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

    void begin( ESP8266WiFiClass* _wifi );
    void handleMqttPublish( void );
    void handleMqttSubScribe( void );
    void handleMqttConfigChange( int _mqtt_config_type=MQTT_GENERAL_CONFIG );

    #ifdef EW_SERIAL_LOG
    void printMqttConfigLogs( void );
    #endif

  protected:

    /**
		 * @var	ESP8266WiFiClass*|&WiFi wifi
		 */
    ESP8266WiFiClass* wifi;
    /**
		 * @var	MQTTClient  mqtt_client
		 */
    MQTTClient mqtt_client;
};

extern MqttServiceProvider __mqtt_service;

#endif
