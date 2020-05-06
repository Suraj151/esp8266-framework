/****************************** Mqtt service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _MQTT_SERVICE_PROVIDER_H_
#define _MQTT_SERVICE_PROVIDER_H_


#include <service_provider/ServiceProvider.h>
#ifdef ENABLE_GPIO_SERVICE
#include <service_provider/GpioServiceProvider.h>
#endif
#include <mqtt_client/Mqtt.h>

#define MQTT_PAYLOAD_BUF_SIZE 400

typedef std::function<void(char*, uint16_t)> MqttPublishDataCallback;
typedef MqttDataCallback MqttSubscribeDataCallback;

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
      this->stop();
      delete[] this->_mqtt_payload;
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
    /**
		 * @array	char  _mqtt_payload
		 */
    char* _mqtt_payload;
    /**
		 * @var	MqttPublishDataCallback  _mqtt_publish_data_cb
		 */
    MqttPublishDataCallback _mqtt_publish_data_cb = nullptr;
    /**
		 * @var	MqttSubscribeDataCallback  _mqtt_subscribe_data_cb
		 */
    MqttSubscribeDataCallback _mqtt_subscribe_data_cb = nullptr;


    /**
		 * @var	MQTTClient  mqtt_client
		 */
    MQTTClient mqtt_client;

    void begin( ESP8266WiFiClass* _wifi );
    void handleMqttPublish( void );
    void handleMqttSubScribe( void );
    void handleMqttConfigChange( int _mqtt_config_type=MQTT_GENERAL_CONFIG );
    static void handleMqttDataCb( uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len);
    void setMqttPublishDataCallback( MqttPublishDataCallback _cb );
    void setMqttSubscribeDataCallback( MqttSubscribeDataCallback _cb );
    void stop( void );

    #ifdef EW_SERIAL_LOG
    void printMqttConfigLogs( void );
    #endif

  protected:

    /**
		 * @var	ESP8266WiFiClass*|&WiFi wifi
		 */
    ESP8266WiFiClass* wifi;
};

extern MqttServiceProvider __mqtt_service;

#endif
