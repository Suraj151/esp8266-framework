/*************************** Device IOT service *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#include <config/Config.h>

#if defined(ENABLE_DEVICE_IOT)

#include "DeviceIotServiceProvider.h"

/**
 * DeviceIotServiceProvider constructor.
 */
DeviceIotServiceProvider::DeviceIotServiceProvider():
  m_token_validity(false),
  m_smaple_index(0),
  m_smaple_per_publish(SENSOR_DATA_SAMPLING_PER_PUBLISH),
  m_sensor_data_publish_freq(SENSOR_DATA_PUBLISH_FREQ),
  m_mqtt_keep_alive(MQTT_DEFAULT_KEEPALIVE),
  m_handle_sensor_data_cb_id(0),
  m_mqtt_connection_check_cb_id(0),
  m_device_config_request_cb_id(0),
  #ifdef ENABLE_LED_INDICATION
  m_wifi_led(0),
  #endif
  m_wifi(nullptr),
  m_wifi_client(nullptr),
  m_device_iot(nullptr)
{
}

/**
 * DeviceIotServiceProvider destructor
 */
DeviceIotServiceProvider::~DeviceIotServiceProvider(){
  this->m_wifi = nullptr;
  this->m_wifi_client = nullptr;
  this->m_device_iot = nullptr;
}

/**
 * start device registration services if enabled
 */
void DeviceIotServiceProvider::init( iWiFiInterface *_wifi, iWiFiClientInterface *_wifi_client ){

  this->m_wifi = _wifi;
  this->m_wifi_client = _wifi_client;

  // __task_scheduler.setInterval( [&]() { this->handleDeviceIotConfigRequest(); }, HTTP_REQUEST_DURATION );
  this->m_device_config_request_cb_id = __task_scheduler.updateInterval(
    this->m_device_config_request_cb_id,
    [&]() {
      this->handleDeviceIotConfigRequest();
    },
    this->m_mqtt_keep_alive*MILLISECOND_DURATION_1000
  );

  #ifdef ENABLE_LED_INDICATION
  this->beginWifiStatusLed();
  __task_scheduler.setInterval( [&]() { this->handleWifiStatusLed(); }, 2.5*MILLISECOND_DURATION_1000 );
  #endif

  // clear all mqtt old configs
  __mqtt_general_table.clear();
  __mqtt_pubsub_table.clear();
  __mqtt_lwt_table.clear();
}

/**
 * handle registration otp request
 */
void DeviceIotServiceProvider::handleRegistrationOtpRequest( device_iot_config_table *_device_iot_configs, String &_response ){

  if( nullptr == _device_iot_configs || nullptr == this->m_wifi_client ){
    return;
  }

  memset( __http_service.m_host, 0, HTTP_HOST_ADDR_MAX_SIZE);
  strcpy( __http_service.m_host, _device_iot_configs->device_iot_host );
  strcat_P( __http_service.m_host, DEVICE_IOT_OTP_REQ_URL );

  uint8_t mac[6];  char macStr[18] = { 0 };
  wifi_get_macaddr(STATION_IF, mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  __find_and_replace( __http_service.m_host, "[mac]", macStr, 2 );

  #ifdef EW_SERIAL_LOG
  Log( F("Handling device otp Http Request : ") );
  Logln( __http_service.m_host );
  #endif

  if( strlen( __http_service.m_host ) > 5 &&
    __http_service.m_http_client->begin( *this->m_wifi_client, __http_service.m_host )
  ){

    __http_service.m_http_client->setUserAgent("esp");
    __http_service.m_http_client->setAuthorization("iot-otp", macStr);
    __http_service.m_http_client->setTimeout(2*MILLISECOND_DURATION_1000);

    int _httpCode = __http_service.m_http_client->GET();

    #ifdef EW_SERIAL_LOG
    Log( F("Http device otp Response code : ") );
    Logln( _httpCode );
    #endif
    if ( (_httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY) && __http_service.m_http_client->getSize() <= DEVICE_IOT_OTP_API_RESP_LENGTH ) {

      _response = __http_service.m_http_client->getString();
    }else{

      _response = "{\"status\":false,\"remark\":";
      _response += "\"Device request failed. ErrCode(";
      _response += _httpCode;
      _response += ") !\"}";
    }
    __http_service.m_http_client->end();

  }else{
    #ifdef EW_SERIAL_LOG
    Logln( F("Device otp Request not initializing or failed or Not Configured Correctly") );
    #endif
  }
}

/**
 * handle connect mqtt request request
 */
void DeviceIotServiceProvider::handleDeviceIotConfigRequest(){

  if( this->m_token_validity || nullptr == this->m_wifi_client ) {
    return;
  }

  this->m_device_iot_configs = __database_service.get_device_iot_config_table();
  // strcat_P( this->m_device_iot_configs.device_iot_host, DEVICE_IOT_CONFIG_REQ_URL );
  memset( __http_service.m_host, 0, HTTP_HOST_ADDR_MAX_SIZE);
  strcpy( __http_service.m_host, this->m_device_iot_configs.device_iot_host );
  strcat_P( __http_service.m_host, DEVICE_IOT_CONFIG_REQ_URL );

  uint8_t mac[6];
  char macStr[18] = { 0 };
  wifi_get_macaddr(STATION_IF, mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  __find_and_replace( __http_service.m_host, "[mac]", macStr, 2 );
  #ifdef EW_SERIAL_LOG
  Logln( F("Handling device iot config Request") );
  // Log( F("Mac : ") );Logln( macStr );
  #endif

  if( strlen( __http_service.m_host ) > 5 &&
    __http_service.m_http_client->begin( *this->m_wifi_client, __http_service.m_host )
  ){
    __http_service.m_http_client->setUserAgent("esp");
    __http_service.m_http_client->setAuthorization("iot-otp", macStr);
    __http_service.m_http_client->setTimeout(3*MILLISECOND_DURATION_1000);
    int _httpCode = __http_service.m_http_client->GET();

    #ifdef EW_SERIAL_LOG
    Log( F("Device iot config request Status Code : ") ); Logln( _httpCode );
    #endif
    if ( _httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY ) {

      #ifdef EW_SERIAL_LOG
      Log( F("Http Response : ") );
      #endif
      if( __http_service.m_http_client->getSize() < DEVICE_IOT_CONFIG_RESP_MAX_SIZE ){

        String _response = __http_service.m_http_client->getString();
        #ifdef EW_SERIAL_LOG
        Logln( _response );
        #endif

        char *_buf = new char[DEVICE_IOT_CONFIG_RESP_MAX_SIZE];
        memset( _buf, 0, DEVICE_IOT_CONFIG_RESP_MAX_SIZE );
        _response.toCharArray( _buf, _response.length()+1 );

        if( 0 <= __strstr( _buf, (char*)DEVICE_IOT_CONFIG_TOKEN_KEY, DEVICE_IOT_CONFIG_RESP_MAX_SIZE - strlen(DEVICE_IOT_CONFIG_TOKEN_KEY) ) ){

          bool _json_result = __get_from_json( _buf, (char*)DEVICE_IOT_CONFIG_TOKEN_KEY, this->m_device_iot_configs.device_iot_token, DEVICE_IOT_CONFIG_TOKEN_MAX_SIZE );
          _json_result = __get_from_json( _buf, (char*)DEVICE_IOT_CONFIG_TOPIC_KEY, this->m_device_iot_configs.device_iot_topic, DEVICE_IOT_CONFIG_TOPIC_MAX_SIZE );
          if(  _json_result && strlen( this->m_device_iot_configs.device_iot_token ) && strlen( this->m_device_iot_configs.device_iot_topic ) ){

            #ifdef EW_SERIAL_LOG
            Log( F("Got Token : ") );
            Logln( this->m_device_iot_configs.device_iot_token );
            Log( F("Got Topic : ") );
            Logln( this->m_device_iot_configs.device_iot_topic );
            #endif
            __database_service.set_device_iot_config_table( &this->m_device_iot_configs );

            this->handleServerConfigurableParameters( _buf );

            this->m_mqtt_connection_check_cb_id = __task_scheduler.updateInterval(
              this->m_mqtt_connection_check_cb_id,
              [&]() {
                this->handleConnectivityCheck();
              },
              this->m_mqtt_keep_alive*MILLISECOND_DURATION_1000, DEFAULT_TASK_PRIORITY,
              ( millis() + MQTT_INITIALIZE_DURATION )
            );
            __task_scheduler.setTimeout( [&]() { this->configureMQTT(); }, 1 );
            this->m_token_validity = true;

          }else{

            this->m_token_validity = false;
          }
        }else{

          this->m_token_validity = false;
        }
        delete[] _buf;
      }else{
        #ifdef EW_SERIAL_LOG
        Logln( F(" response size over !") );
        #endif
      }
    }
    __http_service.m_http_client->end();
  }else{

    #ifdef EW_SERIAL_LOG
    Logln( F("Device iot config request not initializing or failed or Not Configured Correctly") );
    #endif
  }

}

/**
 * handle mqtt connections checks cycle
 */
void DeviceIotServiceProvider::handleConnectivityCheck(){

  bool _is_mqtt_connected = __mqtt_service.m_mqtt_client.is_mqtt_connected();
  #ifdef EW_SERIAL_LOG
  Log( F("Device iot mqtt connection check cycle : ") );
  Logln( _is_mqtt_connected );
  #endif
  if( !_is_mqtt_connected ) {
    this->m_token_validity = false;
    __mqtt_service.stop();
  }
}


/**
 * configure mqtt
 */
void DeviceIotServiceProvider::configureMQTT(){

  mqtt_general_config_table _mqtt_general_configs = __database_service.get_mqtt_general_config_table();
  mqtt_pubsub_config_table _mqtt_pubsub_configs = __database_service.get_mqtt_pubsub_config_table();
  mqtt_lwt_config_table _mqtt_lwt_configs = __database_service.get_mqtt_lwt_config_table();

  memset( &_mqtt_general_configs, 0, sizeof(mqtt_general_config_table));
  memset( &_mqtt_pubsub_configs, 0, sizeof(mqtt_pubsub_config_table));
  memset( &_mqtt_lwt_configs, 0, sizeof(mqtt_lwt_config_table));

  memcpy( _mqtt_general_configs.host, DEVICE_IOT_MQTT_DATA_HOST, strlen( DEVICE_IOT_MQTT_DATA_HOST ) );
  _mqtt_general_configs.port = DEVICE_IOT_MQTT_DATA_PORT;
  strcpy_P( _mqtt_general_configs.client_id, PSTR("[mac]") );
  strcpy_P( _mqtt_general_configs.username, PSTR("[mac]") );
  memcpy( _mqtt_general_configs.password, this->m_device_iot_configs.device_iot_token, DEVICE_IOT_CONFIG_TOKEN_MAX_SIZE );
  _mqtt_general_configs.keepalive = this->m_mqtt_keep_alive;
  _mqtt_general_configs.clean_session = 1;

  strcpy( _mqtt_pubsub_configs.publish_topics[0].topic, this->m_device_iot_configs.device_iot_topic );
  // _mqtt_pubsub_configs.publish_frequency = this->m_sensor_data_publish_freq;

  strcpy( _mqtt_lwt_configs.will_topic, this->m_device_iot_configs.device_iot_topic );
  // strcpy_P( _mqtt_lwt_configs.will_topic, DEVICE_IOT_MQTT_WILL_TOPIC );
  strcpy_P( _mqtt_lwt_configs.will_message, PSTR("{\"head\":{\"uid\":\"[mac]\",\"packet_type\":\"disconnect\"},\"payload\":{},\"tail\":{}}") );
  _mqtt_lwt_configs.will_qos = 1;
  _mqtt_lwt_configs.will_retain = 0;

  __database_service.set_mqtt_general_config_table( &_mqtt_general_configs );
  __database_service.set_mqtt_lwt_config_table( &_mqtt_lwt_configs );
  __database_service.set_mqtt_pubsub_config_table( &_mqtt_pubsub_configs );

  __task_scheduler.setTimeout( [&]() { __mqtt_service.handleMqttConfigChange(); }, 1 );
}

/**
 * handle mqtt config parameters from server
 */
void DeviceIotServiceProvider::handleServerConfigurableParameters(char* json_resp){

  char *_value_buff = new char[10];

  memset( _value_buff, 0, 10 );
  bool _json_result = __get_from_json( json_resp, (char*)DEVICE_IOT_CONFIG_DATA_RATE_KEY, _value_buff, 6 );
  uint16_t data_rate = StringToUint16( _value_buff, 6 );
  if( _json_result && SENSOR_DATA_PUBLISH_FREQ_MIN_LIMIT <= data_rate && data_rate <= SENSOR_DATA_PUBLISH_FREQ_MAX_LIMIT ){

    this->m_sensor_data_publish_freq = data_rate;
    #ifdef EW_SERIAL_LOG
    Log( F("Got Data rate : ") );
    Logln( data_rate );
    #endif
  }

  uint16_t sample_rate = round ( this->m_sensor_data_publish_freq / ( ( SENSOR_DATA_SAMPLES_PER_PUBLISH_MAX_LIMIT * 0.125 ) * ( log(this->m_sensor_data_publish_freq) ) ) );
  if( 0 < sample_rate && sample_rate <= SENSOR_DATA_SAMPLES_PER_PUBLISH_MAX_LIMIT ){
    this->m_smaple_per_publish = sample_rate;
    #ifdef EW_SERIAL_LOG
    Log( F("Got Sample rate : ") );
    Logln( sample_rate );
    #endif
  }

  memset( _value_buff, 0, 10 );
  _json_result = __get_from_json( json_resp, (char*)DEVICE_IOT_CONFIG_MQTT_KEEP_ALIVE_KEY, _value_buff, 6 );
  uint16_t keep_alive = StringToUint16( _value_buff, 6 );
  if( _json_result && DEVICE_IOT_MQTT_KEEP_ALIVE_MIN <= keep_alive && keep_alive <= DEVICE_IOT_MQTT_KEEP_ALIVE_MAX ){

    this->m_mqtt_keep_alive = keep_alive;
    #ifdef EW_SERIAL_LOG
    Log( F("Got keep alive : ") );
    Logln( keep_alive );
    #endif
  }

  this->beginSensorData();

  this->m_device_config_request_cb_id = __task_scheduler.updateInterval(
    this->m_device_config_request_cb_id,
    [&]() {
      this->handleDeviceIotConfigRequest();
    },
    this->m_mqtt_keep_alive*MILLISECOND_DURATION_1000
  );

  delete[] _value_buff;
}



/**
 * begin sensor data processs
 */
void DeviceIotServiceProvider::beginSensorData(){

  this->m_handle_sensor_data_cb_id = __task_scheduler.updateInterval(
    this->m_handle_sensor_data_cb_id,
    [&]() { this->handleSensorData(); },
    (((float)this->m_sensor_data_publish_freq/(float)this->m_smaple_per_publish)*1000.0)
  );
  this->m_smaple_index = 0;
  if( nullptr != this->m_device_iot ){
    this->m_device_iot->resetSampleHook();
  }
}

/**
 * init sensor device processs
 */
void DeviceIotServiceProvider::initDeviceIotSensor( iDeviceIotInterface *_device ){
  this->m_device_iot = _device;
  if( nullptr != this->m_device_iot ){
    this->beginSensorData();
  }
}

/**
 * handle sensor data. takes defined samples and average them to send.
 */
void DeviceIotServiceProvider::handleSensorData(){

  #ifdef EW_SERIAL_LOG
  Log(F("Handling sensor data samples: "));Logln(this->m_smaple_index);
  #endif

  if( nullptr == this->m_device_iot ){
    return;
  }

  this->m_device_iot->sampleHook();

  if( this->m_smaple_index >= this->m_smaple_per_publish-1 ){

    this->m_smaple_index = 0;
    String _payload = "{\"head\":{\"uid\":\"[mac]\",\"packet_type\":\"data\",\"packet_version\":\"";
    _payload += DEVICE_IOT_PACKET_VERSION;
    _payload += "\"},\"payload\":";
    this->m_device_iot->dataHook(_payload);
    _payload += ",\"tail\":{}}";
    __task_scheduler.setTimeout( [&]() { __mqtt_service.handleMqttPublish(); }, 1 );

    memset( __mqtt_service.m_mqtt_payload, 0, MQTT_PAYLOAD_BUF_SIZE );
    if( _payload.length()+1 < MQTT_PAYLOAD_BUF_SIZE ){
      _payload.toCharArray( __mqtt_service.m_mqtt_payload, _payload.length()+1);
    }else{
      strcat_P( __mqtt_service.m_mqtt_payload, PSTR("mqtt data is too big to fit in buffer !"));
    }
  }else{
    this->m_smaple_index++;
  }
}

/**
 * enable wifi status led indication
 */
#ifdef ENABLE_LED_INDICATION

void DeviceIotServiceProvider::beginWifiStatusLed( int _wifi_led ){

  this->m_wifi_led = _wifi_led;
  pinMode( _wifi_led, OUTPUT );
  digitalWrite( this->m_wifi_led, HIGH );
}

void DeviceIotServiceProvider::handleWifiStatusLed(){

  if( nullptr == this->m_wifi ){
    return;
  }

  #ifdef EW_SERIAL_LOG
  Logln(F("Handling LED Status Indications"));
  Log(F("RSSI : "));Logln( this->m_wifi->RSSI() );
  #endif

  if( !this->m_wifi->localIP().isSet() || !this->m_wifi->isConnected() || ( this->m_wifi->RSSI() < (int)WIFI_RSSI_THRESHOLD ) ){

    #ifdef EW_SERIAL_LOG
    Logln( F("WiFi not connected.") );
    #endif
    digitalWrite( this->m_wifi_led, LOW );
  }else{

    if( this->m_token_validity ){

      digitalWrite( this->m_wifi_led, HIGH );
      delay(40);
      digitalWrite( this->m_wifi_led, LOW );
    }else{
      digitalWrite( this->m_wifi_led, HIGH );
    }
  }

}
#endif

#ifdef EW_SERIAL_LOG
/**
 * print device register configs
 */
void DeviceIotServiceProvider::printDeviceIotConfigLogs(){

  device_iot_config_table _device_iot_configs = __database_service.get_device_iot_config_table();

  Logln(F("\nDevice IOT Configs :"));
  Log(_device_iot_configs.device_iot_host); Logln("\n");
}
#endif

DeviceIotServiceProvider __device_iot_service;

#endif
