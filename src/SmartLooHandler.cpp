#include <SmartLooHandler.h>

void SmartLooWorker::begin(){
  this->initialize();
  this->register_table( &_smartloo_config_defaults, SMARTLOO_CONFIG_TABLE_ADDRESS );
  this->smartloo_configs = this->get_smartloo_config_table();
  this->RouteHandler.register_route( EW_SERVER_SL_CONFIG_ROUTE, [&]() { this->handleSmartLooConfigRoute(); } );
  this->setInterval( [&]() { this->printSmartLooLogs(); }, EW_DEFAULT_LOG_DURATION );
  this->setSmartLooHttpRequestRoutine();
  // this->_smartloo_http_request_cb_id = this->setInterval( [&]() { this->handleSmartLooHttpRequest(); }, SENSOR_DATA_POST_FREQ*1000 );
  this->run_while_factory_reset( [&]() { this->clear_smarLoo_table_to_defaults(); } );
}

void SmartLooWorker::run(){
  this->serve();
}

void SmartLooWorker::setSmartLooHttpRequestRoutine(){

  if( strlen( this->smartloo_configs.sensor_host ) > 5 && this->smartloo_configs.sensor_port > 0 &&
    this->smartloo_configs.sensor_post_frequency > 0
  ){
    this->_smartloo_http_request_cb_id = this->updateInterval(
      this->_smartloo_http_request_cb_id,
      [&]() { this->handleSmartLooHttpRequest(); },
      this->smartloo_configs.sensor_post_frequency*1000
    );
  }else{
    this->clearInterval( this->_smartloo_http_request_cb_id );
    this->_smartloo_http_request_cb_id = 0;
  }
}


void SmartLooWorker::build_smartloo_config_html( char* _page, bool _enable_flash, int _max_size ){

  char* _gender_options[] = {"male", "female"};
  char* _sensor_options[] = {"people counter", "smell", "light", "water level", "scream"};
  char _lat[10], _long[10], _port[10],_freq[10];
  memset( _lat, 0, 10 );memset( _long, 0, 10 );memset( _port, 0, 10 );memset( _freq, 0, 10 );
  dtostrf( this->smartloo_configs.latitude, 8, 2, _lat ); char* __lat=__strtrim( _lat, 8 );
  dtostrf( this->smartloo_configs.longitude, 8, 2, _long ); char* __long=__strtrim( _long, 8 );
  __appendUintToBuff( _port, "%d", this->smartloo_configs.sensor_port, 8 );
  __appendUintToBuff( _freq, "%d", this->smartloo_configs.sensor_post_frequency, 8 );
  memset( _page, 0, _max_size );
  strcat_P( _page, EW_SERVER_HEADER_HTML );
  strcat_P( _page, EW_SERVER_SL_CONFIG_PAGE_TOP );

  concat_tr_input_html_tags( _page, PSTR("Site id:"), PSTR("st_id"), this->smartloo_configs.site_id );
  concat_tr_select_html_tags( _page, PSTR("Gender:"), PSTR("gndr"), _gender_options, 2, (int)this->smartloo_configs.allowed_gender );
  concat_tr_select_html_tags( _page, PSTR("Sensor type:"), PSTR("snsr"), _sensor_options, 5, (int)this->smartloo_configs.sensor_type );
  concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), this->smartloo_configs.sensor_host, (char*)"40" );
  concat_tr_input_html_tags( _page, PSTR("Host Port:"), PSTR("prt"), _port );
  concat_tr_input_html_tags( _page, PSTR("Post Frequency:"), PSTR("frq"), _freq );
  concat_tr_input_html_tags( _page, PSTR("Latitude:"), PSTR("lat"), __lat );
  concat_tr_input_html_tags( _page, PSTR("Longitude:"), PSTR("long"), __long );
  concat_tr_input_html_tags( _page, PSTR("City:"), PSTR("ct"), this->smartloo_configs.city );

  strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
  if( _enable_flash )
  concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
  strcat_P( _page, EW_SERVER_FOOTER_HTML );
}

void SmartLooWorker::handleSmartLooConfigRoute( void ) {

  #ifdef EW_SERIAL_LOG
  Logln(F("Handling Smart Loo Config route"));
  #endif
  bool _is_posted = false;
  if ( !this->RouteHandler.has_active_session() ) {

    this->EwServer.sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
    this->EwServer.sendHeader("Cache-Control", "no-cache");
    this->EwServer.send(HTTP_REDIRECT);
    return;
  }

  if ( this->EwServer.hasArg("st_id") && this->EwServer.hasArg("gndr") ) {

    String _site_id = this->EwServer.arg("st_id");
    String _gender = this->EwServer.arg("gndr");
    String _sensor = this->EwServer.arg("snsr");
    String _sensor_host = this->EwServer.arg("hst");
    String _sensor_port = this->EwServer.arg("prt");
    String _post_freq = this->EwServer.arg("frq");
    String _lat = this->EwServer.arg("lat");
    String _long = this->EwServer.arg("long");
    String _city = this->EwServer.arg("ct");
    #ifdef EW_SERIAL_LOG
      Logln(F("\nSubmitted info :\n"));
      Log(F("Site id : ")); Logln( _site_id );
      Log(F("Gender : ")); Logln( _gender );
      Log(F("Sensor type : ")); Logln( _sensor );
      Log(F("Sensor Host : ")); Logln( _sensor_host );
      Log(F("Sensor Port : ")); Logln( _sensor_port );
      Log(F("Sensor Post Frequency : ")); Logln( _post_freq );
      Log(F("Latitude : ")); Logln( _lat );
      Log(F("Longitude : ")); Logln( _long );
      Log(F("City : ")); Logln( _city );
      Logln();
    #endif

    if( _site_id.length() <= SMART_LOO_BUF_SIZE ){
      memset( this->smartloo_configs.site_id, 0, SMART_LOO_BUF_SIZE );
      _site_id.toCharArray( this->smartloo_configs.site_id, _site_id.length()+1 );
    }
    this->smartloo_configs.allowed_gender = (smartloo_site_genders)_gender.toInt();
    this->smartloo_configs.sensor_type = (smartloo_site_sensors)_sensor.toInt();

    if( _sensor_host.length() <= SMART_LOO_HOST_ADDR_SIZE ){
      memset( this->smartloo_configs.sensor_host, 0, SMART_LOO_HOST_ADDR_SIZE );
      _sensor_host.toCharArray( this->smartloo_configs.sensor_host, _sensor_host.length()+1 );
    }

    this->smartloo_configs.sensor_port = _sensor_port.toInt();
    this->smartloo_configs.sensor_post_frequency = _post_freq.toInt();
    this->smartloo_configs.latitude = _lat.toFloat();
    this->smartloo_configs.longitude = _long.toFloat();

    if( _city.length() <= SMART_LOO_BUF_SIZE ){
      memset( this->smartloo_configs.city, 0, SMART_LOO_BUF_SIZE );
      _city.toCharArray( this->smartloo_configs.city, _city.length()+1 );
    }

    this->set_smartloo_config_table( &this->smartloo_configs );
    this->setSmartLooHttpRequestRoutine();

    _is_posted = true;
  }

  char* _page = new char[EW_HTML_MAX_SIZE];
  // char _page[EW_HTML_MAX_SIZE];
  this->build_smartloo_config_html( _page, _is_posted );

  this->EwServer.send( HTTP_OK, EW_HTML_CONTENT, _page );
  delete[] _page;
}

void SmartLooWorker::printSmartLooLogs(){

  #ifdef EW_SERIAL_LOG
  this->smartloo_configs = this->get_smartloo_config_table();

  Logln(F("\nSmart Loo Configs :"));
  Logln(F("Site\tGender\tSensor\tHost\tPort\tPost-Freq\tLatitude\tLongitude\tCity"));

  Log(this->smartloo_configs.site_id); Log("\t");
  Log(this->smartloo_configs.allowed_gender); Log("\t");
  Log(this->smartloo_configs.sensor_type); Log("\t");
  Log(this->smartloo_configs.sensor_host); Log("\t");
  Log(this->smartloo_configs.sensor_port); Log("\t");
  Log(this->smartloo_configs.sensor_post_frequency); Log("\t");
  Log(this->smartloo_configs.latitude); Log("\t\t");
  Log(this->smartloo_configs.longitude); Log("\t\t");
  Log(this->smartloo_configs.city); Log("\t\n\n");
  #endif
}


void SmartLooWorker::handleSmartLooHttpRequest( ){

  memset( this->http_host, 0, HTTP_HOST_ADDR_MAX_SIZE);

  this->smartloo_configs = this->get_smartloo_config_table();
  strcpy( this->http_host, this->smartloo_configs.sensor_host );
  this->http_port = this->smartloo_configs.sensor_port;

  #ifdef EW_SERIAL_LOG
  Logln( F("Handling Smart Loo Http Request") );
  #endif

  if( strlen( this->http_host ) > 5 && this->http_port > 0 &&
    this->smartloo_configs.sensor_post_frequency > 0 &&
    this->http_request.begin( this->wifi_client, this->http_host )
  ){

    int sensorValue = random(0, 1024);
    String _payload = "{\"data\":[{\"fields\":{\"value\":";
    _payload += sensorValue;
    _payload += "}}]}";

    this->http_request.setUserAgent("SmartLoo");
    this->http_request.addHeader("Content-Type", "application/json");
    this->http_request.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJzc3RfaWQiOjMsInNlbnNvcl90eXBlIjoid2F0ZXJfbGV2ZWwiLCJsYXRpdHVkZSI6MTguNDY4NzgxNSwibG9uZ2l0dWRlIjo3My44NzQzMDM4LCJzaXRlX2lkIjo1LCJzaXRlX25hbWUiOiJrb25kaHdhIHVuaXR5IHBhcmsgMyIsInNpdGVfdHlwZSI6InRvaWxldCIsImNpdHkiOiJQdW5lIiwib3JnX2RhdGFiYXNlIjoidGVzdF9vcmcifQ.xHMzWUM7NWHz1qYVLgZWQUWSEs8uKQIp4QcAvJ550vI");
    this->http_request.setTimeout(2000);

    int _httpCode = this->http_request.POST( _payload );

    if( this->followHttpRequest( _httpCode ) ) this->handleSmartLooHttpRequest();
  }else{
    #ifdef EW_SERIAL_LOG
    Logln( F("Smart Loo Http Request not initializing or failed or Not Configured Correctly") );
    #endif
  }
}

void SmartLooWorker::clear_smarLoo_table_to_defaults( void ){
  this->clear_table_to_defaults( &_smartloo_config_defaults, SMARTLOO_CONFIG_TABLE_ADDRESS );
}

smartloo_config_table SmartLooWorker::get_smartloo_config_table( void ){
  this->get_table( &this->smartloo_configs, SMARTLOO_CONFIG_TABLE_ADDRESS );
  return this->smartloo_configs;
}

void SmartLooWorker::set_smartloo_config_table( smartloo_config_table* _table ){
  this->set_table( _table, SMARTLOO_CONFIG_TABLE_ADDRESS );
}
