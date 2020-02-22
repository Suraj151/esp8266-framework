/****************************** Mqtt service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_MQTT_SERVICE)

#include "MqttServiceProvider.h"

/**
 * start mqtt service. initialize it with mqtt configs at database
 */
void MqttServiceProvider::begin( ESP8266WiFiClass* _wifi ){

  this->wifi = _wifi;
  this->handleMqttConfigChange();
}

/**
 * check and do mqtt publish on given configs.
 */
void MqttServiceProvider::handleMqttPublish(){

  #ifdef EW_SERIAL_LOG
  Logln( F("MQTT: handling mqtt publish interval") );
  #endif
  if( !this->mqtt_client.is_mqtt_connected() ) return;

  mqtt_pubsub_config_table _mqtt_pubsub_configs = __database_service.get_mqtt_pubsub_config_table();
  uint8_t mac[6];
  char macStr[18] = { 0 };
  wifi_get_macaddr(STATION_IF, mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  for (uint8_t i = 0; i < MQTT_MAX_PUBLISH_TOPIC; i++) {

    __find_and_replace( _mqtt_pubsub_configs.publish_topics[i].topic, "[mac]", macStr, 2 );
    #ifdef EW_SERIAL_LOG
    Log( F("MQTT: publishing on topic : ") );Logln( _mqtt_pubsub_configs.publish_topics[i].topic );
    #endif
    if( strlen(_mqtt_pubsub_configs.publish_topics[i].topic) > 0 ){

      String _payload = "";

      #ifdef ENABLE_GPIO_SERVICE

      _payload += "{\"data\":{";
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {

        if( !__gpio_service.is_exceptional_gpio_pin(_pin) ){

          _payload += "\"D";
          _payload += _pin;
          _payload += "\":{\"mode\":";
          _payload += __gpio_service.virtual_gpio_configs.gpio_mode[_pin];
          _payload += ",\"val\":";
          _payload += __gpio_service.virtual_gpio_configs.gpio_readings[_pin];
          _payload += "},";
        }
      }
      _payload += "\"A0\":{\"mode\":";
      _payload += __gpio_service.virtual_gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS];
      _payload += ",\"val\":";
      _payload += __gpio_service.virtual_gpio_configs.gpio_readings[MAX_NO_OF_GPIO_PINS];
      _payload += "}}}";

      #else

      _payload += "Hello from Esp Client : ";
      _payload += ESP.getChipId();
      #endif

      int _size = _payload.length() + 20;
      char* _pload = new char[ _size ];
      memset( _pload, 0, _size );
      _payload.toCharArray( _pload, _size );
      __find_and_replace( _pload, "[mac]", macStr, 2 );
      _size = strlen( _pload );

      this->mqtt_client.Publish(

        _mqtt_pubsub_configs.publish_topics[i].topic,
        _pload,
        _size,
        _mqtt_pubsub_configs.publish_topics[i].qos < MQTT_MAX_QOS_LEVEL ?
        _mqtt_pubsub_configs.publish_topics[i].qos : MQTT_MAX_QOS_LEVEL,
        _mqtt_pubsub_configs.publish_topics[i].retain
      );
      delete[] _pload;
    }
  }
}

/**
 * handle mqtt subscribe cycle. suscribe to topics in pubsub configs
 */
void MqttServiceProvider::handleMqttSubScribe(){

  #ifdef EW_SERIAL_LOG
  Logln( F("MQTT: handling mqtt subscribe interval") );
  #endif
  if( !this->mqtt_client.is_mqtt_connected() ) return;

  mqtt_pubsub_config_table _mqtt_pubsub_configs = __database_service.get_mqtt_pubsub_config_table();

  for (uint8_t i = 0; i < MQTT_MAX_SUBSCRIBE_TOPIC; i++) {

    if( strlen(_mqtt_pubsub_configs.subscribe_topics[i].topic) > 0 && !this->mqtt_client.is_topic_subscribed(_mqtt_pubsub_configs.subscribe_topics[i].topic) ){

      this->mqtt_client.Subscribe(

        _mqtt_pubsub_configs.subscribe_topics[i].topic,
        _mqtt_pubsub_configs.subscribe_topics[i].qos < MQTT_MAX_QOS_LEVEL ?
        _mqtt_pubsub_configs.subscribe_topics[i].qos : MQTT_MAX_QOS_LEVEL
      );
    }
  }
}

/**
 * stop mqtt client
 *
 */
void MqttServiceProvider::stop(){

  this->mqtt_client.DeleteClient();
  __task_scheduler.clearInterval( this->_mqtt_timer_cb_id );
  this->_mqtt_timer_cb_id = 0;
}

/**
 * handle restart of mqtt services on config change from autherised client
 *
 * @param   int _mqtt_config_type
 */
void MqttServiceProvider::handleMqttConfigChange( int _mqtt_config_type ){


  mqtt_general_config_table _mqtt_general_configs = __database_service.get_mqtt_general_config_table();
  mqtt_pubsub_config_table _mqtt_pubsub_configs = __database_service.get_mqtt_pubsub_config_table();

  if( _mqtt_config_type == MQTT_GENERAL_CONFIG || _mqtt_config_type == MQTT_LWT_CONFIG ){

    this->mqtt_client.DeleteClient();
    int _stat = __task_scheduler.setTimeout( [&]() {

      mqtt_general_config_table _mqtt_general_configs = __database_service.get_mqtt_general_config_table();
      mqtt_lwt_config_table _mqtt_lwt_configs = __database_service.get_mqtt_lwt_config_table();

      uint8_t mac[6];
      char macStr[18] = { 0 };
      wifi_get_macaddr(STATION_IF, mac);
      sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

      __find_and_replace( _mqtt_general_configs.username, "[mac]", macStr, 2 );
      __find_and_replace( _mqtt_general_configs.client_id, "[mac]", macStr, 2 );
      __find_and_replace( _mqtt_lwt_configs.will_message, "[mac]", macStr, 2 );

      if( this->mqtt_client.begin( this->wifi, &_mqtt_general_configs, &_mqtt_lwt_configs ) ){
        this->_mqtt_timer_cb_id = __task_scheduler.updateInterval(
          this->_mqtt_timer_cb_id,
          [&]() { this->mqtt_client.mqtt_timer(); },
          MILLISECOND_DURATION_1000
        );
      }else{
        __task_scheduler.clearInterval( this->_mqtt_timer_cb_id );
        this->_mqtt_timer_cb_id = 0;
      }
    }, MQTT_INITIALIZE_DURATION );
  }else if( _mqtt_config_type == MQTT_PUBSUB_CONFIG ){

    for ( uint16_t i = 0; i < this->mqtt_client.mqttClient.subscribed_topics.size(); i++) {

      bool _found = false;
      for (uint8_t j = 0; j < MQTT_MAX_SUBSCRIBE_TOPIC; j++) {

        if( __are_str_equals(
          _mqtt_pubsub_configs.subscribe_topics[j].topic,
          this->mqtt_client.mqttClient.subscribed_topics[i].topic,
          strlen( _mqtt_pubsub_configs.subscribe_topics[j].topic )
        ) ) _found = true;

      }
      if( !_found )
        this->mqtt_client.UnSubscribe( this->mqtt_client.mqttClient.subscribed_topics[i].topic );
    }

  }else{

  }

  if( _mqtt_pubsub_configs.publish_frequency > 0 ){
    this->_mqtt_publish_cb_id = __task_scheduler.updateInterval(
      this->_mqtt_publish_cb_id,
      [&]() { this->handleMqttPublish(); },
      _mqtt_pubsub_configs.publish_frequency*MILLISECOND_DURATION_1000
    );
  }else{
    __task_scheduler.clearInterval( this->_mqtt_publish_cb_id );
    this->_mqtt_publish_cb_id = 0;
  }

  if( _mqtt_general_configs.keepalive > 0 ){
    this->_mqtt_subscribe_cb_id = __task_scheduler.updateInterval(
      this->_mqtt_subscribe_cb_id,
      [&]() { this->handleMqttSubScribe(); },
      _mqtt_general_configs.keepalive*MILLISECOND_DURATION_1000
    );
  }else{
    __task_scheduler.clearInterval( this->_mqtt_subscribe_cb_id );
    this->_mqtt_subscribe_cb_id = 0;
  }

}

#ifdef EW_SERIAL_LOG
/**
 * print mqtt configs
 */
void MqttServiceProvider::printMqttConfigLogs(){

  mqtt_general_config_table _mqtt_general_configs = __database_service.get_mqtt_general_config_table();
  mqtt_lwt_config_table _mqtt_lwt_configs = __database_service.get_mqtt_lwt_config_table();
  mqtt_pubsub_config_table _mqtt_pubsub_configs = __database_service.get_mqtt_pubsub_config_table();


  Logln(F("\nMqtt General Configs :"));
  Log( _mqtt_general_configs.host ); Log("\t");
  Log( _mqtt_general_configs.port ); Log("\t");
  Log( _mqtt_general_configs.security ); Log("\t");
  Log( _mqtt_general_configs.client_id ); Log("\t");
  Log( _mqtt_general_configs.username ); Log("\t");
  Log( _mqtt_general_configs.password ); Log("\t");
  Log( _mqtt_general_configs.keepalive ); Log("\t");
  Log( _mqtt_general_configs.clean_session ); Logln();

  Logln(F("\nMqtt Lwt Configs :"));
  Log( _mqtt_lwt_configs.will_topic ); Log("\t");
  Log( _mqtt_lwt_configs.will_message ); Log("\t");
  Log( _mqtt_lwt_configs.will_qos ); Log("\t");
  Log( _mqtt_lwt_configs.will_retain ); Logln();

  Logln(F("\nMqtt Pub Configs :"));
  for (uint8_t i = 0; i < MQTT_MAX_PUBLISH_TOPIC; i++) {

    if( strlen(_mqtt_pubsub_configs.publish_topics[i].topic) > 0 ){

      Log( _mqtt_pubsub_configs.publish_topics[i].topic ); Log("\t");
      Log( _mqtt_pubsub_configs.publish_topics[i].qos ); Log("\t");
      Log( _mqtt_pubsub_configs.publish_topics[i].retain ); Logln();
    }
  }

  Logln(F("\nMqtt Sub Configs :"));
  for (uint8_t i = 0; i < MQTT_MAX_SUBSCRIBE_TOPIC; i++) {

    if( strlen(_mqtt_pubsub_configs.subscribe_topics[i].topic) > 0 ){

      Log( _mqtt_pubsub_configs.subscribe_topics[i].topic ); Log("\t");
      Log( _mqtt_pubsub_configs.subscribe_topics[i].qos ); Logln();
    }
  }
}
#endif


MqttServiceProvider __mqtt_service;

#endif
