#include "EwingsEsp8266Stack.h"

void EwingsEsp8266Stack::initialize(){

  #ifdef EW_SERIAL_LOG
  LogBegin(9600);
  Logln(F("Initializing..."));
  #endif
  this->init_default_database();
  this->start_wifi();
  this->start_http_server();
  this->begin_ota( &this->wifi_client, &this->http_request, this->EwingsDefaultDatabase() );
  #ifdef ENABLE_GPIO_CONFIG
  this->start_gpio_service();
  #endif
  #ifdef ENABLE_MQTT_CONFIG
  this->start_mqtt_service();
  #endif

  this->setInterval( [&]() { this->handleLogPrints(); }, EW_DEFAULT_LOG_DURATION );
  this->setInterval( [&]() { this->handleFlashKeyPress(); }, FLASH_KEY_PRESS_DURATION );
  this->setInterval( [&]() { this->handleOta(); }, HTTP_REQUEST_DURATION );
  this->run_while_factory_reset( [&]() { this->clear_default_tables(); } );

  #ifdef ENABLE_NAPT_FEATURE
  this->enable_napt_service();
  #endif
}

void EwingsEsp8266Stack::start_wifi(){

  wifi_config_table _wifi_credentials = this->get_wifi_config_table();

  this->wifi->mode(WIFI_AP_STA);
  this->wifi->persistent(false);
  this->configure_wifi_station( &_wifi_credentials );
  this->configure_wifi_access_point( &_wifi_credentials );
  this->setInterval( [&]() { this->handleWiFiConnectivity(); }, WIFI_CONNECTIVITY_CHECK_DURATION );

  _ClearObject(&_wifi_credentials);
}

bool EwingsEsp8266Stack::configure_wifi_access_point( wifi_config_table* _wifi_credentials ){

  #ifdef EW_SERIAL_LOG
  Log(F("Configuring access point "));
  Log( _wifi_credentials->ap_ssid );
  Logln(F(" .."));
  #endif

  IPAddress local_IP(
    _wifi_credentials->ap_local_ip[0],_wifi_credentials->ap_local_ip[1],_wifi_credentials->ap_local_ip[2],_wifi_credentials->ap_local_ip[3]
  );
  IPAddress gateway(
    _wifi_credentials->ap_gateway[0],_wifi_credentials->ap_gateway[1],_wifi_credentials->ap_gateway[2],_wifi_credentials->ap_gateway[3]
  );
  IPAddress subnet(
    _wifi_credentials->ap_subnet[0],_wifi_credentials->ap_subnet[1],_wifi_credentials->ap_subnet[2],_wifi_credentials->ap_subnet[3]
  );

  if( this->wifi->softAPConfig( local_IP, gateway, subnet ) &&
    this->wifi->softAP( _wifi_credentials->ap_ssid, _wifi_credentials->ap_password )
  ){
    #ifdef EW_SERIAL_LOG
    Log(F("AP IP address: "));
    Logln(this->wifi->softAPIP());
    #endif
    return true;
  }else{
    #ifdef EW_SERIAL_LOG
    Logln(F("Configuring access point failed!"));
    #endif
    return false;
  }
}

bool EwingsEsp8266Stack::configure_wifi_station( wifi_config_table* _wifi_credentials ){

  #ifdef EW_SERIAL_LOG
    Log(F("\nConnecing To "));
    Log(_wifi_credentials->sta_ssid);
  #endif

  IPAddress local_IP(
    _wifi_credentials->sta_local_ip[0],_wifi_credentials->sta_local_ip[1],_wifi_credentials->sta_local_ip[2],_wifi_credentials->sta_local_ip[3]
  );
  IPAddress gateway(
    _wifi_credentials->sta_gateway[0],_wifi_credentials->sta_gateway[1],_wifi_credentials->sta_gateway[2],_wifi_credentials->sta_gateway[3]
  );
  IPAddress subnet(
    _wifi_credentials->sta_subnet[0],_wifi_credentials->sta_subnet[1],_wifi_credentials->sta_subnet[2],_wifi_credentials->sta_subnet[3]
  );

  this->wifi->config( local_IP, gateway, subnet );
  this->wifi->begin(_wifi_credentials->sta_ssid, _wifi_credentials->sta_password);

  uint8_t _wait = 1;
  while ( ! this->wifi->isConnected() ) {

    delay(1000);
    if( _wait%10 == 0 ){
      #ifdef EW_SERIAL_LOG
        Log(F("\ntrying reconnect"));
      #endif
      this->wifi->reconnect();
    }
    if( _wait++ > this->wifi_connection_timeout ){
      break;
    }
    #ifdef EW_SERIAL_LOG
      Log(".");
    #endif
  }
  #ifdef EW_SERIAL_LOG
  Logln();
  #endif
  if( this->wifi->status() == WL_CONNECTED ){
    #ifdef EW_SERIAL_LOG
    Log(F("Connected to "));
    Logln(_wifi_credentials->sta_ssid);
    Log(F("IP address: "));
    Logln(this->wifi->localIP());
    #endif
    this->wifi->setAutoConnect(true);
    this->wifi->setAutoReconnect(true);
    return true;
  }else if( this->wifi->status() == WL_NO_SSID_AVAIL ){
    #ifdef EW_SERIAL_LOG
    Log(_wifi_credentials->sta_ssid);
    Logln(F(" Not Found/reachable. Make sure it's availability in range."));
    #endif
  }else if( this->wifi->status() == WL_CONNECT_FAILED ){
    #ifdef EW_SERIAL_LOG
    Log(_wifi_credentials->sta_ssid);
    Logln(F(" is available but not connecting. Please check password."));
    #endif
  }else{
    #ifdef EW_SERIAL_LOG
      Logln(F("WiFi Not Connecting. Will try later soon.."));
    #endif
  }
  return false;
}

#ifdef ENABLE_NAPT_FEATURE
void EwingsEsp8266Stack::enable_napt_service(){
  // Initialize the NAT feature
  ip_napt_init(IP_NAPT_MAX, IP_PORTMAP_MAX);
  // Enable NAT on the AP interface
  ip_napt_enable_no(1, 1);
  // Set the DNS server for clients of the AP to the one we also use for the STA interface
  dhcps_set_DNS(this->wifi->dnsIP());
}
#endif

void EwingsEsp8266Stack::start_http_server(){

  this->RouteHandler.handle( &this->EwServer, this->EwingsDefaultDatabase() );
  // this->RouteHandler.handle( &this->EwServer );
  this->RouteHandler.set_ewstack_callback( [&](int _t) { this->restart_device(_t); } );
  #ifdef ENABLE_GPIO_CONFIG
  this->RouteHandler.set_ewstack_gpio_config_callback( [&](int _t) { this->handleGpioModes(_t); } );
  this->RouteHandler.set_ewstack_virtual_gpio_configs( &this->virtual_gpio_configs );
  #endif
  #ifdef ENABLE_MQTT_CONFIG
  this->RouteHandler.set_ewstack_mqtt_config_callback( [&](int _t) { this->handleMqttConfigChange(_t); } );
  #endif
  this->EwServer.begin();
  #ifdef EW_SERIAL_LOG
    Logln(F("HTTP server started"));
  #endif
}

void EwingsEsp8266Stack::serve(){

  this->EwServer.handleClient();
  this->handle_periodic_callbacks(millis());
}

bool EwingsEsp8266Stack::followHttpRequest( int _httpCode ){

  #ifdef EW_SERIAL_LOG
  Logln( F("Following Http Request") );
  Log( F("Http Request Status Code : ") ); Logln( _httpCode );
  if ( _httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
    Log( F("Http Response : ") );
    Logln( this->http_request.getString() );
  }
  #endif
  this->http_request.end();

  if( _httpCode < 0 && this->http_retry > 0 ){
    this->http_retry--;
    #ifdef EW_SERIAL_LOG
    Logln( F("Http Request retrying...") );
    #endif
    return true;
  }
  this->http_retry = HTTP_REQUEST_RETRY;

  return false;
}

void EwingsEsp8266Stack::handleWiFiConnectivity(){

  #ifdef EW_SERIAL_LOG
  Logln( F("\nHandeling WiFi Connectivity") );
  #endif
  if( !this->wifi->isConnected() ){
    #ifdef EW_SERIAL_LOG
    Logln( F("Handeling WiFi Reconnect Manually.") );
    #endif
    this->wifi->reconnect();
  }else{
    #ifdef EW_SERIAL_LOG
    Log(F("IP address: "));
    Logln(this->wifi->localIP());
    #endif
  }
}

void EwingsEsp8266Stack::handleOta(){

  #ifdef EW_SERIAL_LOG
  Logln( F("\nHandeling OTA") );
  #endif
  http_ota_status _stat = this->handle_ota();
  #ifdef EW_SERIAL_LOG
  Log( F("OTA status : ") );Logln( _stat );
  #endif
  if( _stat == UPDATE_OK ){
    #ifdef EW_SERIAL_LOG
    Logln( F("\nOTA Done ....Rebooting ") );
    #endif
    ESP.restart();
  }
}

void EwingsEsp8266Stack::handleLogPrints(){

  #ifdef EW_SERIAL_LOG
  this->printWiFiConfigLogs();
  this->printOtaConfigLogs();
  #ifdef ENABLE_GPIO_CONFIG
  this->printGpioConfigLogs();
  #endif
  #ifdef ENABLE_MQTT_CONFIG
  this->printMqttConfigLogs();
  #endif
  Log( F("\nNTP Validity : ") );
  Logln( this->is_ntp_in_sync() );
  Log( F("NTP Time : ") );
  Logln( this->get_ntp_time() );
  #endif
}

void EwingsEsp8266Stack::handleFlashKeyPress(){

  if( digitalRead(FLASH_KEY_PIN) == LOW ){
    this->flash_key_pressed++;
    #ifdef EW_SERIAL_LOG
    Log( F("Flash Key pressed : ") );
    Logln( this->flash_key_pressed );
    #endif
  }else{
    this->flash_key_pressed=0;
  }

  if( this->flash_key_pressed > FLASH_KEY_PRESS_COUNT_THR ){
    #ifdef EW_SERIAL_LOG
    Logln( F("requested factory reset !") );
    #endif
    // this->clear_default_tables();
    this->factory_reset();
  }
}

void EwingsEsp8266Stack::printWiFiConfigLogs(){

  #ifdef EW_SERIAL_LOG
  wifi_config_table _table = this->get_wifi_config_table();
  char _ip_address[20];

  Logln(F("\nWiFi Configs :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));

  Log(_table.sta_ssid); Log("\t");
  Log(_table.sta_password); Log("\t");
  __int_ip_to_str( _ip_address, _table.sta_local_ip, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.sta_gateway, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.sta_subnet, 20 ); Log(_ip_address); Log("\t\n");

  Logln(F("\nAccess Configs :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));
  Log(_table.ap_ssid); Log("\t");
  Log(_table.ap_password); Log("\t");
  __int_ip_to_str( _ip_address, _table.ap_local_ip, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.ap_gateway, 20 ); Log(_ip_address); Log("\t");
  __int_ip_to_str( _ip_address, _table.ap_subnet, 20 ); Log(_ip_address); Log("\t\n\n");

  #endif
}

void EwingsEsp8266Stack::printOtaConfigLogs(){

  #ifdef EW_SERIAL_LOG
  // gpio_config_table _table = this->get_gpio_config_table();
  ota_config_table _ota_configs = this->get_ota_config_table();

  Logln(F("\nOTA Configs :"));
  Log(_ota_configs.firmware_version); Log("\t");
  Log(_ota_configs.ota_host); Log("\t");
  Log(_ota_configs.ota_port); Logln("\n");

  #endif
}

void EwingsEsp8266Stack::connected_softap_client_info(){

  unsigned char number_client;
  struct station_info *stat_info;

  struct ip_addr *IPaddress;
  IPAddress address;
  int i=1;

  number_client= wifi_softap_get_station_num(); // Count of stations which are connected to ESP8266 soft-AP
  stat_info = wifi_softap_get_station_info();

  #ifdef EW_SERIAL_LOG
  Log(F(" Total connected_clients : "));
  Logln(number_client);
  #endif

  while (stat_info != NULL) {

    IPaddress = &stat_info->ip;
    address = IPaddress->addr;

    #ifdef EW_SERIAL_LOG
    Log(F("client: ")); Log(i);
    Log(F(" ip address: ")); Log((address));
    Log(F(" mac address: "));
    Log_format(stat_info->bssid[0],HEX);
    Log_format(stat_info->bssid[1],HEX);
    Log_format(stat_info->bssid[2],HEX);
    Log_format(stat_info->bssid[3],HEX);
    Log_format(stat_info->bssid[4],HEX);
    Logln_format(stat_info->bssid[5],HEX);
    #endif

    stat_info = STAILQ_NEXT(stat_info, next);
    i++;
  }
}

#ifdef ENABLE_MQTT_CONFIG

void EwingsEsp8266Stack::start_mqtt_service(){

  this->handleMqttConfigChange();
}

void EwingsEsp8266Stack::handleMqttPublish(){

  #ifdef EW_SERIAL_LOG
  Logln( F("MQTT: handling mqtt publish interval") );
  #endif
  if( !this->mqtt_client.is_mqtt_connected() ) return;

  mqtt_pubsub_config_table _mqtt_pubsub_configs = this->get_mqtt_pubsub_config_table();
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

      #ifdef ENABLE_GPIO_CONFIG

      _payload += "{\"data\":{";
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {

        if( !is_exceptional_gpio_pin(_pin) ){

          _payload += "\"D";
          _payload += _pin;
          _payload += "\":{\"mode\":";
          _payload += this->virtual_gpio_configs.gpio_mode[_pin];
          _payload += ",\"val\":";
          _payload += this->virtual_gpio_configs.gpio_readings[_pin];
          _payload += "},";
        }
      }
      _payload += "\"A0\":{\"mode\":";
      _payload += this->virtual_gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS];
      _payload += ",\"val\":";
      _payload += this->virtual_gpio_configs.gpio_readings[MAX_NO_OF_GPIO_PINS];
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

void EwingsEsp8266Stack::handleMqttSubScribe(){

  #ifdef EW_SERIAL_LOG
  Logln( F("MQTT: handling mqtt subscribe interval") );
  #endif
  if( !this->mqtt_client.is_mqtt_connected() ) return;

  mqtt_pubsub_config_table _mqtt_pubsub_configs = this->get_mqtt_pubsub_config_table();

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

void EwingsEsp8266Stack::handleMqttConfigChange( int _mqtt_config_type ){


  mqtt_general_config_table _mqtt_general_configs = this->get_mqtt_general_config_table();
  mqtt_pubsub_config_table _mqtt_pubsub_configs = this->get_mqtt_pubsub_config_table();

  if( _mqtt_config_type == MQTT_GENERAL_CONFIG || _mqtt_config_type == MQTT_LWT_CONFIG ){

    this->mqtt_client.DeleteClient();
    int _stat = this->setTimeout( [&]() {

      mqtt_general_config_table _mqtt_general_configs = this->get_mqtt_general_config_table();
      mqtt_lwt_config_table _mqtt_lwt_configs = this->get_mqtt_lwt_config_table();

      uint8_t mac[6];
      char macStr[18] = { 0 };
      wifi_get_macaddr(STATION_IF, mac);
      sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

      __find_and_replace( _mqtt_general_configs.username, "[mac]", macStr, 2 );
      __find_and_replace( _mqtt_general_configs.client_id, "[mac]", macStr, 2 );
      __find_and_replace( _mqtt_lwt_configs.will_message, "[mac]", macStr, 2 );

      if( this->mqtt_client.begin( this->wifi, &_mqtt_general_configs, &_mqtt_lwt_configs ) ){
        this->_mqtt_timer_cb_id = this->updateInterval(
          this->_mqtt_timer_cb_id,
          [&]() { this->mqtt_client.mqtt_timer(); },
          1000
        );
      }else{
        this->clearInterval( this->_mqtt_timer_cb_id );
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
    this->_mqtt_publish_cb_id = this->updateInterval(
      this->_mqtt_publish_cb_id,
      [&]() { this->handleMqttPublish(); },
      _mqtt_pubsub_configs.publish_frequency*1000
    );
  }else{
    this->clearInterval( this->_mqtt_publish_cb_id );
    this->_mqtt_publish_cb_id = 0;
  }

  if( _mqtt_general_configs.keepalive > 0 ){
    this->_mqtt_subscribe_cb_id = this->updateInterval(
      this->_mqtt_subscribe_cb_id,
      [&]() { this->handleMqttSubScribe(); },
      _mqtt_general_configs.keepalive*1000
    );
  }else{
    this->clearInterval( this->_mqtt_subscribe_cb_id );
    this->_mqtt_subscribe_cb_id = 0;
  }

}

void EwingsEsp8266Stack::printMqttConfigLogs(){

  #ifdef EW_SERIAL_LOG
  mqtt_general_config_table _mqtt_general_configs = this->get_mqtt_general_config_table();
  mqtt_lwt_config_table _mqtt_lwt_configs = this->get_mqtt_lwt_config_table();
  mqtt_pubsub_config_table _mqtt_pubsub_configs = this->get_mqtt_pubsub_config_table();

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

  #endif
}

#endif


#ifdef ENABLE_GPIO_CONFIG

void EwingsEsp8266Stack::start_gpio_service(){

  this->handleGpioModes();
  this->virtual_gpio_configs = this->get_gpio_config_table();

  this->setInterval( [&]() { this->handleGpioOperations(); }, GPIO_OPERATION_DURATION );
  this->setInterval( [&]() { this->enable_update_gpio_table_from_virtual(); }, GPIO_TABLE_UPDATE_DURATION );
}

void EwingsEsp8266Stack::handleGpioHttpRequest( ){

  memset( this->http_host, 0, HTTP_HOST_ADDR_MAX_SIZE);

  strcpy( this->http_host, this->virtual_gpio_configs.gpio_host );
  this->http_port = this->virtual_gpio_configs.gpio_port;

  #ifdef EW_SERIAL_LOG
  Logln( F("Handling GPIO Http Request") );
  #endif

  if( strlen( this->http_host ) > 5 && this->http_port > 0 &&
    this->virtual_gpio_configs.gpio_post_frequency > 0 &&
    this->http_request.begin( this->wifi_client, this->http_host )
  ){

    String _payload = "{\"data\":{";

    for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {

      if( !is_exceptional_gpio_pin(_pin) ){

        _payload += "\"D";
        _payload += _pin;
        _payload += "\":{\"mode\":";
        _payload += this->virtual_gpio_configs.gpio_mode[_pin];
        _payload += ",\"val\":";
        _payload += this->virtual_gpio_configs.gpio_readings[_pin];
        _payload += "},";
      }
    }
    _payload += "\"A0\":{\"mode\":";
    _payload += this->virtual_gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS];
    _payload += ",\"val\":";
    _payload += this->virtual_gpio_configs.gpio_readings[MAX_NO_OF_GPIO_PINS];
    _payload += "}}}";

    this->http_request.setUserAgent("Ewings");
    this->http_request.addHeader("Content-Type", "application/json");
    this->http_request.setAuthorization("user", "password");
    this->http_request.setTimeout(2000);

    int _httpCode = this->http_request.POST( _payload );

    if( this->followHttpRequest( _httpCode ) ) this->handleGpioHttpRequest();
  }else{
    #ifdef EW_SERIAL_LOG
    Logln( F("GPIO Http Request not initializing or failed or Not Configured Correctly") );
    #endif
  }
}

void EwingsEsp8266Stack::handleGpioOperations(){

  // gpio_config_table _gpio_configs = this->get_gpio_config_table();

  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {

    switch ( is_exceptional_gpio_pin(_pin) ? OFF : this->virtual_gpio_configs.gpio_mode[_pin] ) {

      default:
      case OFF:{
        this->virtual_gpio_configs.gpio_readings[_pin] = LOW;
        break;
      }
      case DIGITAL_WRITE:{
        // Log( F("writing "));Log( this->getGpioFromPinMap( _pin ) );Log( F(" pin to "));Logln( this->virtual_gpio_configs.gpio_readings[_pin]);
        digitalWrite( this->getGpioFromPinMap( _pin ), this->virtual_gpio_configs.gpio_readings[_pin] );
        break;
      }
      case DIGITAL_READ:{
        this->virtual_gpio_configs.gpio_readings[_pin] = digitalRead( this->getGpioFromPinMap( _pin ) );
        break;
      }
      case ANALOG_WRITE:{
        analogWrite( this->getGpioFromPinMap( _pin ), this->virtual_gpio_configs.gpio_readings[_pin] );
        break;
      }
      case ANALOG_READ:{
        if( _pin == MAX_NO_OF_GPIO_PINS )
        this->virtual_gpio_configs.gpio_readings[_pin] = analogRead( A0 );
        break;
      }
    }

  }

  if( this->update_gpio_table_from_virtual ){
    this->set_gpio_config_table(&this->virtual_gpio_configs);
    this->update_gpio_table_from_virtual = false;
  }

}

void EwingsEsp8266Stack::enable_update_gpio_table_from_virtual(){
  this->update_gpio_table_from_virtual = true;
}

void EwingsEsp8266Stack::handleGpioModes( int _gpio_config_type ){

  gpio_config_table _gpio_configs = this->get_gpio_config_table();

  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {

    switch ( _gpio_configs.gpio_mode[_pin] ) {

      case OFF:
      case DIGITAL_READ:
      default:
        pinMode( this->getGpioFromPinMap( _pin ), INPUT );
        break;
      case DIGITAL_WRITE:
      case ANALOG_WRITE:
        pinMode( this->getGpioFromPinMap( _pin ), OUTPUT );
        break;
      case ANALOG_READ:
      break;
    }
  }

  this->virtual_gpio_configs = _gpio_configs;

  if( strlen( this->virtual_gpio_configs.gpio_host ) > 5 && this->virtual_gpio_configs.gpio_port > 0 &&
    this->virtual_gpio_configs.gpio_post_frequency > 0
  ){
    this->_gpio_http_request_cb_id = this->updateInterval(
      this->_gpio_http_request_cb_id,
      [&]() { this->handleGpioHttpRequest(); },
      this->virtual_gpio_configs.gpio_post_frequency*1000
    );
  }else{
    this->clearInterval( this->_gpio_http_request_cb_id );
    this->_gpio_http_request_cb_id = 0;
  }

}

void EwingsEsp8266Stack::printGpioConfigLogs(){

  #ifdef EW_SERIAL_LOG
  // gpio_config_table _table = this->get_gpio_config_table();

  Logln(F("\nGPIO Configs (mode) :"));
  // Logln(F("ssid\tpassword\tlocal\tgateway\tsubnet"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_mode[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (readings) :"));
  for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS+1; _pin++) {
    Log(this->virtual_gpio_configs.gpio_readings[_pin]); Log("\t");
  }
  Logln(F("\nGPIO Configs (server) :"));
  Log(this->virtual_gpio_configs.gpio_host); Log("\t");
  Log(this->virtual_gpio_configs.gpio_port); Log("\t");
  Log(this->virtual_gpio_configs.gpio_post_frequency); Log("\n\n");

  #endif
}

uint8_t EwingsEsp8266Stack::getGpioFromPinMap( uint8_t _pin ){

  uint8_t mapped_pin;

  // Map
  switch ( _pin ) {

    case 0:
      mapped_pin = 16;
      break;
    case 1:
      mapped_pin = 5;
      break;
    case 2:
      mapped_pin = 4;
      break;
    case 3:
      mapped_pin = 0;
      break;
    case 4:
      mapped_pin = 2;
      break;
    case 5:
      mapped_pin = 14;
      break;
    case 6:
      mapped_pin = 12;
      break;
    case 7:
      mapped_pin = 13;
      break;
    case 8:
      mapped_pin = 15;
      break;
    case 9:
      mapped_pin = 3;
      break;
    case 10:
      mapped_pin = 1;
      break;
    default:
      mapped_pin = 0;
  }

  return mapped_pin;
}
#endif

EwingsEsp8266Stack EwStack;
