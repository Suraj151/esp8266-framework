#ifndef _EW_SERVER_ROUTE_HANDLER_
#define _EW_SERVER_ROUTE_HANDLER_

#include <Esp.h>

#include <webserver/controllers/SessionHandler.h>
#include <webserver/pages/Header.h>
#include <webserver/pages/Footer.h>
#include <webserver/pages/HomePage.h>
#include <webserver/pages/LoginPage.h>
#include <webserver/pages/LogoutPage.h>
#include <webserver/pages/WiFiConfigPage.h>
#include <webserver/pages/LoginConfigPage.h>
#include <webserver/pages/OtaConfigPage.h>
#include <webserver/pages/NotFound.h>
#include <webserver/controllers/DynamicPageBuildHelper.h>
#include <webserver/routes/Routes.h>
#ifdef ENABLE_GPIO_CONFIG
#include <webserver/pages/GpioConfigPage.h>
#endif
#ifdef ENABLE_MQTT_CONFIG
#include <webserver/pages/MqttConfigPage.h>
#endif

// #include <database/EwingsEepromDB.h>

static const char EW_HTML_CONTENT[] PROGMEM = "text/html";

#define EW_HTML_MAX_SIZE 4000

#define MIN_ACCEPTED_ARG_SIZE 3

enum HTTP_RETURN_CODE {
  HTTP_OK = 200,
  HTTP_NOT_FOUND = 404,
  HTTP_REDIRECT = 301
};

class EwRouteHandler : public EwSessionHandler{

  public:

    EwRouteHandler( void ){
    }

    EwRouteHandler( ESP8266WebServer* server, EwingsDefaultDB* db_handler ){
      this->handle(server,db_handler);
    }

    EwRouteHandler( ESP8266WebServer* server ){
      this->handle(server);
    }

    ~EwRouteHandler(){
      this->EwServer = NULL;
      this->ew_db = NULL;
      _ClearObject( &this->login_credentials );
      _ClearObject( &this->wifi_configs );
    }

    void handle( ESP8266WebServer* server, EwingsDefaultDB* db_handler ){

      this->ew_db = db_handler;
      this->handle(server);
      this->use_login_credential( db_handler->get_login_credential_table() );
      this->use_wifi_configs( db_handler->get_wifi_config_table() );
      #ifdef ENABLE_GPIO_CONFIG
      this->use_gpio_configs( db_handler->get_gpio_config_table() );
      #endif
    }

    void handle( ESP8266WebServer* server ){

      this->EwServer = server;
      this->register_route( EW_SERVER_HOME_ROUTE, [&]() { this->handleHomeRoute(); } );
      this->register_route( EW_SERVER_LOGIN_ROUTE, [&]() { this->handleLoginRoute(); } );
      this->register_route( EW_SERVER_LOGOUT_ROUTE, [&]() { this->handleLogoutRoute(); } );
      this->register_route( EW_SERVER_WIFI_CONFIG_ROUTE, [&]() { this->handleWiFiConfigRoute(); } );
      this->register_route( EW_SERVER_LOGIN_CONFIG_ROUTE, [&]() { this->handleLoginConfigRoute(); } );
      this->register_route( EW_SERVER_OTA_CONFIG_ROUTE, [&]() { this->handleOtaServerConfigRoute(); } );
      this->register_route( EW_SERVER_LISTEN_TO_CLIENT_ROUTE, [&]() { this->handleClientVoice(); } );
      #ifdef ENABLE_GPIO_CONFIG
      this->register_route( EW_SERVER_GPIO_MANAGE_CONFIG_ROUTE, [&]() { this->handleGpioManageRoute(); } );
      this->register_route( EW_SERVER_GPIO_SERVER_CONFIG_ROUTE, [&]() { this->handleGpioServerConfigRoute(); } );
      this->register_route( EW_SERVER_GPIO_MODE_CONFIG_ROUTE, [&]() { this->handleGpioModeConfigRoute(); } );
      this->register_route( EW_SERVER_GPIO_WRITE_CONFIG_ROUTE, [&]() { this->handleGpioWriteConfigRoute(); } );
      this->register_route( EW_SERVER_GPIO_MONITOR_ROUTE, [&]() { this->handleGpioMonitorRoute(); } );
      this->register_route( EW_SERVER_LISTEN_TO_MONITOR_ROUTE, [&]() { this->handleMonitorListen(); } );
      #endif
      #ifdef ENABLE_MQTT_CONFIG
      this->register_route( EW_SERVER_MQTT_MANAGE_CONFIG_ROUTE, [&]() { this->handleMqttManageRoute(); } );
      this->register_route( EW_SERVER_MQTT_GENERAL_CONFIG_ROUTE, [&]() { this->handleMqttGeneralConfigRoute(); } );
      this->register_route( EW_SERVER_MQTT_LWT_CONFIG_ROUTE, [&]() { this->handleMqttLWTConfigRoute(); } );
      this->register_route( EW_SERVER_MQTT_PUBSUB_CONFIG_ROUTE, [&]() { this->handleMqttPubSubConfigRoute(); } );
      #endif

      this->register_not_found_fn( [&]() { this->handleNotFound(); } );

      //here the list of headers to be recorded
      const char * headerkeys[] = {"User-Agent", "Cookie"} ;
      size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
      //ask Server to track these headers
      this->EwServer->collectHeaders(headerkeys, headerkeyssize);
    }

    void set_ewstack_callback( CallBackIntArgFn _fn ){
      this->_ewstack_callback_fn = _fn;
    }

    #ifdef ENABLE_GPIO_CONFIG
    void set_ewstack_gpio_config_callback( CallBackIntArgFn _fn ){
      this->_ewstack_gpio_config_callback_fn = _fn;
    }
    void set_ewstack_virtual_gpio_configs( gpio_config_table* _config ){
      this->_ewstack_virtual_gpio_configs = _config;
    }
    void use_gpio_configs( gpio_config_table _gpio_configs ){
      this->gpio_configs = _gpio_configs;
    }
    #endif

    #ifdef ENABLE_MQTT_CONFIG
    void set_ewstack_mqtt_config_callback( CallBackIntArgFn _fn ){
      this->_ewstack_mqtt_config_callback_fn = _fn;
    }
    #endif

    void use_wifi_configs( wifi_config_table _wifi_configs ){
      this->wifi_configs = _wifi_configs;
    }

    void register_route( const char* _uri, CallBackVoidArgFn _fn ){
      this->EwServer->on( _uri, _fn );
    }

    void register_not_found_fn( CallBackVoidArgFn _fn ){

      this->EwServer->onNotFound( _fn );
    }

    void build_html(
      char* _page,
      PGM_P _pgm_page,
      bool _enable_flash=false,
      char* _message="",
      FLASH_MSG_TYPE _alert_type=ALERT_SUCCESS ,
      bool _enable_header_footer=true,
      int _max_size=EW_HTML_MAX_SIZE
    ){

      memset( _page, 0, _max_size );

      if( _enable_header_footer ) strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, _pgm_page );
      if( _enable_flash )
      concat_flash_message_div( _page, _message, _alert_type );
      if( _enable_header_footer ) strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleClientVoice( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling client voice route"));
      #endif

      String _response = "{\"rpc\":";
      _response += 0;
      _response += ",\"bdy\":\"";
      _response += !this->has_active_session() ? "<h4>sorry, you are expired</h4>":"";
      _response += "\"}";

      this->EwServer->sendHeader("Cache-Control", "no-cache");
      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _response );
    }

    void handleNotFound( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling 404 route"));
      #endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_404_PAGE );

      this->EwServer->send( HTTP_NOT_FOUND, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    void handleHomeRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Home route"));
      #endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_html( _page, this->has_active_session() ? EW_SERVER_HOME_AUTHORIZED_PAGE : EW_SERVER_HOME_PAGE );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    void build_wifi_config_html( char* _page, bool _is_error=false, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      char _ip_address[20];
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_TOP );

      concat_tr_input_html_tags( _page, PSTR("WiFi Name:"), PSTR("sta_ssid"), this->wifi_configs.sta_ssid );
      concat_tr_input_html_tags( _page, PSTR("WiFi Password:"), PSTR("sta_pswd"), this->wifi_configs.sta_password );

      __int_ip_to_str( _ip_address, this->wifi_configs.sta_local_ip, 20 );
      concat_tr_input_html_tags( _page, PSTR("WiFi Local Ip:"), PSTR("sta_lip"), _ip_address );
      __int_ip_to_str( _ip_address, this->wifi_configs.sta_gateway, 20 );
      concat_tr_input_html_tags( _page, PSTR("WiFi Gateway:"), PSTR("sta_gip"), _ip_address );
      __int_ip_to_str( _ip_address, this->wifi_configs.sta_subnet, 20 );
      concat_tr_input_html_tags( _page, PSTR("WiFi Subnet:"), PSTR("sta_sip"), _ip_address );

      concat_tr_input_html_tags( _page, PSTR("Access Name:"), PSTR("ap_ssid"), this->wifi_configs.ap_ssid );
      concat_tr_input_html_tags( _page, PSTR("Access Password:"), PSTR("ap_pswd"), this->wifi_configs.ap_password );

      __int_ip_to_str( _ip_address, this->wifi_configs.ap_local_ip, 20 );
      concat_tr_input_html_tags( _page, PSTR("Access Local Ip:"), PSTR("ap_lip"), _ip_address );
      __int_ip_to_str( _ip_address, this->wifi_configs.ap_gateway, 20 );
      concat_tr_input_html_tags( _page, PSTR("Access Gateway:"), PSTR("ap_gip"), _ip_address );
      __int_ip_to_str( _ip_address, this->wifi_configs.ap_subnet, 20 );
      concat_tr_input_html_tags( _page, PSTR("Access Subnet:"), PSTR("ap_sip"), _ip_address );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, _is_error ? PSTR("Invalid length error(3-20)"): PSTR("Config saved Successfully..applying new configs."), _is_error ? ALERT_DANGER:ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleWiFiConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling WiFi Config route"));
      #endif
      bool _is_posted = false;
      bool _is_error = true;
      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("sta_ssid") && this->EwServer->hasArg("sta_pswd") ) {

        String _sta_ssid = this->EwServer->arg("sta_ssid");
        String _sta_pswd = this->EwServer->arg("sta_pswd");
        String _sta_lip = this->EwServer->arg("sta_lip");
        String _sta_gip = this->EwServer->arg("sta_gip");
        String _sta_sip = this->EwServer->arg("sta_sip");

        String _ap_ssid = this->EwServer->arg("ap_ssid");
        String _ap_pswd = this->EwServer->arg("ap_pswd");
        String _ap_lip = this->EwServer->arg("ap_lip");
        String _ap_gip = this->EwServer->arg("ap_gip");
        String _ap_sip = this->EwServer->arg("ap_sip");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("sta ssid : ")); Logln( _sta_ssid );
          Log(F("sta password : ")); Logln( _sta_pswd );
          Log(F("sta local ip : ")); Logln( _sta_lip );
          Log(F("sta gateway : ")); Logln( _sta_gip );
          Log(F("sta subnet : ")); Logln( _sta_sip );
          Log(F("ap ssid : ")); Logln( _ap_ssid );
          Log(F("ap password : ")); Logln( _ap_pswd );
          Log(F("ap local ip : ")); Logln( _ap_lip );
          Log(F("ap gateway : ")); Logln( _ap_gip );
          Log(F("ap subnet : ")); Logln( _ap_sip );
          Logln();
        #endif

        if( _sta_ssid.length() <= WIFI_CONFIGS_BUF_SIZE && _sta_pswd.length() <= WIFI_CONFIGS_BUF_SIZE &&
          _ap_ssid.length() <= WIFI_CONFIGS_BUF_SIZE && _ap_pswd.length() <= WIFI_CONFIGS_BUF_SIZE &&
          _sta_ssid.length() > MIN_ACCEPTED_ARG_SIZE && _sta_pswd.length() > MIN_ACCEPTED_ARG_SIZE &&
          _ap_ssid.length() > MIN_ACCEPTED_ARG_SIZE && _ap_pswd.length() > MIN_ACCEPTED_ARG_SIZE
        ){

          char _ip_address[20]; memset( _ip_address, 0, 20 );
          // wifi_config_table this->wifi_configs = this->get_wifi_config_table();
          _ClearObject( &this->wifi_configs );

          _sta_ssid.toCharArray( this->wifi_configs.sta_ssid, _sta_ssid.length()+1 );
          _sta_pswd.toCharArray( this->wifi_configs.sta_password, _sta_pswd.length()+1 );
          _sta_lip.toCharArray( _ip_address, _sta_lip.length()+1 ); __str_ip_to_int(_ip_address, this->wifi_configs.sta_local_ip, 20);
          _sta_gip.toCharArray( _ip_address, _sta_gip.length()+1 ); __str_ip_to_int(_ip_address, this->wifi_configs.sta_gateway, 20);
          _sta_sip.toCharArray( _ip_address, _sta_sip.length()+1 ); __str_ip_to_int(_ip_address, this->wifi_configs.sta_subnet, 20);

          _ap_ssid.toCharArray( this->wifi_configs.ap_ssid, _ap_ssid.length()+1 );
          _ap_pswd.toCharArray( this->wifi_configs.ap_password, _ap_pswd.length()+1 );
          _ap_lip.toCharArray( _ip_address, _ap_lip.length()+1 ); __str_ip_to_int(_ip_address, this->wifi_configs.ap_local_ip, 20);
          _ap_gip.toCharArray( _ip_address, _ap_gip.length()+1 ); __str_ip_to_int(_ip_address, this->wifi_configs.ap_gateway, 20);
          _ap_sip.toCharArray( _ip_address, _ap_sip.length()+1 ); __str_ip_to_int(_ip_address, this->wifi_configs.ap_subnet, 20);

          this->ew_db->set_wifi_config_table( &this->wifi_configs );
          // this->set_wifi_config_table( &this->wifi_configs );
          _is_error = false;
        }
        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_wifi_config_html( _page, _is_error, _is_posted );

      if( _is_posted && !_is_error ){
        this->send_inactive_session_headers();
      }
      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && !_is_error && this->_ewstack_callback_fn!=NULL){
        this->_ewstack_callback_fn( 1000 );
      }
    }

    void build_ota_server_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_OTA_CONFIG_PAGE_TOP );

      ota_config_table _ota_configs = this->ew_db->get_ota_config_table();

      char _port[10];memset( _port, 0, 10 );
      __appendUintToBuff( _port, "%d", _ota_configs.ota_port, 8 );

      concat_tr_input_html_tags( _page, PSTR("OTA Host:"), PSTR("hst"), _ota_configs.ota_host, "49" );
      concat_tr_input_html_tags( _page, PSTR("OTA Port:"), PSTR("prt"), _port );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleOtaServerConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling OTA Server Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("hst") && this->EwServer->hasArg("prt") ) {

        String _ota_host = this->EwServer->arg("hst");
        String _ota_port = this->EwServer->arg("prt");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("ota host : ")); Logln( _ota_host );
          Log(F("ota port : ")); Logln( _ota_port );
          Logln();
        #endif

        ota_config_table _ota_configs = this->ew_db->get_ota_config_table();

        _ota_host.toCharArray( _ota_configs.ota_host, _ota_host.length()+1 );
        _ota_configs.ota_port = (int)_ota_port.toInt();

        this->ew_db->set_ota_config_table( &_ota_configs);

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_ota_server_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }


    void build_login_config_html( char* _page, bool _is_error=false, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_LOGIN_CONFIG_PAGE_TOP );

      concat_tr_input_html_tags( _page, PSTR("Username:"), PSTR("usrnm"), this->login_credentials.username );
      concat_tr_input_html_tags( _page, PSTR("Password:"), PSTR("pswd"), this->login_credentials.password );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, _is_error ? PSTR("Invalid length error(3-20)"): HTML_SUCCESS_FLASH, _is_error ? ALERT_DANGER:ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleLoginConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Login Config route"));
      #endif
      bool _is_posted = false;
      bool _is_error = true;
      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("usrnm") && this->EwServer->hasArg("pswd") ) {

        String _username = this->EwServer->arg("usrnm");
        String _password = this->EwServer->arg("pswd");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("Username : ")); Logln( _username );
          Log(F("Password : ")); Logln( _password );
          Logln();
        #endif

        if( _username.length() <= LOGIN_CONFIGS_BUF_SIZE && _password.length() <= LOGIN_CONFIGS_BUF_SIZE &&
          _username.length() > MIN_ACCEPTED_ARG_SIZE && _password.length() > MIN_ACCEPTED_ARG_SIZE
        ){

          // login_credential_table this->login_credentials = this->get_login_credential_table();
          memset( this->login_credentials.username, 0, LOGIN_CONFIGS_BUF_SIZE );
          memset( this->login_credentials.password, 0, LOGIN_CONFIGS_BUF_SIZE );
          _username.toCharArray( this->login_credentials.username, _username.length()+1 );
          _password.toCharArray( this->login_credentials.password, _password.length()+1 );
          this->ew_db->set_login_credential_table( &this->login_credentials );
          // this->set_login_credential_table( &this->login_credentials );

          _is_error = false;
        }
        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_login_config_html( _page, _is_error, _is_posted );

      if( _is_posted && !_is_error ){
        this->send_inactive_session_headers();
      }
      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    #ifdef ENABLE_MQTT_CONFIG

    void handleMqttManageRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt Manage route"));
      #endif

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_MQTT_MANAGE_PAGE );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    void build_mqtt_general_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_MQTT_GENERAL_PAGE_TOP );

      mqtt_general_config_table _mqtt_general_configs = this->ew_db->get_mqtt_general_config_table();

      char _port[10],_keepalive[10];memset( _port, 0, 10 );memset( _keepalive, 0, 10 );
      __appendUintToBuff( _port, "%d", _mqtt_general_configs.port, 8 );
      __appendUintToBuff( _keepalive, "%d", _mqtt_general_configs.keepalive, 8 );

      concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), _mqtt_general_configs.host, "50" );
      concat_tr_input_html_tags( _page, PSTR("Host Port:"), PSTR("prt"), _port );
      concat_tr_input_html_tags( _page, PSTR("Client Id:"), PSTR("clid"), _mqtt_general_configs.client_id, "100" );
      concat_tr_input_html_tags( _page, PSTR("Username:"), PSTR("usrn"), _mqtt_general_configs.username, "25" );
      concat_tr_input_html_tags( _page, PSTR("Password:"), PSTR("pswd"), _mqtt_general_configs.password, "350" );
      concat_tr_input_html_tags( _page, PSTR("Keep Alive:"), PSTR("kpalv"), _keepalive );
      concat_tr_input_html_tags( _page, PSTR("Clean Session:"), PSTR("cln"), "clean", "", HTML_INPUT_CHECKBOX_TAG_TYPE,_mqtt_general_configs.clean_session != 0 );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleMqttGeneralConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt General Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("hst") && this->EwServer->hasArg("prt") ) {

        String _mqtt_host = this->EwServer->arg("hst");
        String _mqtt_port = this->EwServer->arg("prt");
        String _client_id = this->EwServer->arg("clid");
        String _username = this->EwServer->arg("usrn");
        String _password = this->EwServer->arg("pswd");
        String _keep_alive = this->EwServer->arg("kpalv");
        String _clean_session = this->EwServer->arg("cln");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("mqtt host : ")); Logln( _mqtt_host );
          Log(F("mqtt port : ")); Logln( _mqtt_port );
          Log(F("client id : ")); Logln( _client_id );
          Log(F("username : ")); Logln( _username );
          Log(F("password : ")); Logln( _password );
          Log(F("keep alive : ")); Logln( _keep_alive );
          Log(F("clean session : ")); Logln( _clean_session );
          Logln();
        #endif

        mqtt_general_config_table* _mqtt_general_configs = new mqtt_general_config_table;
        memset( (void*)_mqtt_general_configs, 0, sizeof(mqtt_general_config_table) );
        // mqtt_general_config_table _mqtt_general_configs = this->ew_db->get_mqtt_general_config_table();

        _mqtt_host.toCharArray( _mqtt_general_configs->host, _mqtt_host.length()+1 );
        _mqtt_general_configs->port = (int)_mqtt_port.toInt();
        _client_id.toCharArray( _mqtt_general_configs->client_id, _client_id.length()+1 );
        _username.toCharArray( _mqtt_general_configs->username, _username.length()+1 );
        _password.toCharArray( _mqtt_general_configs->password, _password.length()+1 );
        _mqtt_general_configs->keepalive = (int)_keep_alive.toInt();
        _mqtt_general_configs->clean_session = (int)( _clean_session == "clean" );

        this->ew_db->set_mqtt_general_config_table( _mqtt_general_configs);
        delete _mqtt_general_configs;

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_mqtt_general_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_mqtt_config_callback_fn!=NULL ){
        this->_ewstack_mqtt_config_callback_fn(MQTT_GENERAL_CONFIG);
      }
    }

    void build_mqtt_lwt_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      char _ip_address[20];
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_MQTT_LWT_PAGE_TOP );

      mqtt_lwt_config_table _mqtt_lwt_configs = this->ew_db->get_mqtt_lwt_config_table();

      char* _qos_options[] = {"0", "1", "2"};

      concat_tr_input_html_tags( _page, PSTR("Will Topic:"), PSTR("wtpc"), _mqtt_lwt_configs.will_topic, "50" );
      concat_tr_input_html_tags( _page, PSTR("Will Message:"), PSTR("wmsg"), _mqtt_lwt_configs.will_message, "50"  );
      concat_tr_select_html_tags( _page, PSTR("Will QoS:"), PSTR("wqos"), _qos_options, 3, _mqtt_lwt_configs.will_qos+1 );
      concat_tr_input_html_tags( _page, PSTR("Will Retain:"), PSTR("wrtn"), "retain", "", HTML_INPUT_CHECKBOX_TAG_TYPE,_mqtt_lwt_configs.will_retain != 0 );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleMqttLWTConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt LWT Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("wtpc") && this->EwServer->hasArg("wmsg") ) {

        String _will_topic = this->EwServer->arg("wtpc");
        String _will_message = this->EwServer->arg("wmsg");
        String _will_qos = this->EwServer->arg("wqos");
        String _will_retain = this->EwServer->arg("wrtn");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("will topic : ")); Logln( _will_topic );
          Log(F("will message : ")); Logln( _will_message );
          Log(F("will qos : ")); Logln( _will_qos );
          Log(F("will retain : ")); Logln( _will_retain );
          Logln();
        #endif

        mqtt_lwt_config_table* _mqtt_lwt_configs = new mqtt_lwt_config_table;
        memset( (void*)_mqtt_lwt_configs, 0, sizeof(mqtt_lwt_config_table) );

        _will_topic.toCharArray( _mqtt_lwt_configs->will_topic, _will_topic.length()+1 );
        _will_message.toCharArray( _mqtt_lwt_configs->will_message, _will_message.length()+1 );
        _mqtt_lwt_configs->will_qos = (int)_will_qos.toInt() - 1;
        _mqtt_lwt_configs->will_retain = (int)( _will_retain == "retain" );

        this->ew_db->set_mqtt_lwt_config_table( _mqtt_lwt_configs );

        delete _mqtt_lwt_configs;
        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_mqtt_lwt_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_mqtt_config_callback_fn!=NULL ){
        this->_ewstack_mqtt_config_callback_fn(MQTT_LWT_CONFIG);
      }
    }

    void build_mqtt_pubsub_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      char _ip_address[20];
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_MQTT_PUBSUB_PAGE_TOP );

      mqtt_pubsub_config_table _mqtt_pubsub_configs = this->ew_db->get_mqtt_pubsub_config_table();

      char* _qos_options[] = {"0", "1", "2"};
      char _topic_name[10], _topic_label[10];memset(_topic_name, 0, 10);memset(_topic_label, 0, 10);
      char _qos_name[10], _qos_label[10];memset(_qos_name, 0, 10);memset(_qos_label, 0, 10);
      char _retain_name[10], _retain_label[10];memset(_retain_name, 0, 10);memset(_retain_label, 0, 10);
      char _publish_freq[10]; memset(_publish_freq, 0, 10);__appendUintToBuff( _publish_freq, "%d", _mqtt_pubsub_configs.publish_frequency, 8 );
      strcpy( _topic_label, "Topic0:" );strcpy( _topic_name, "ptpc0" );
      strcpy( _qos_label, "QoS0:" );strcpy( _qos_name, "pqos0" );
      strcpy( _retain_label, "Retain0:" );strcpy( _retain_name, "prtn0" );

      concat_tr_header_html_tags( _page, PSTR("Publish Topics") );
      for (uint8_t i = 0; i < MQTT_MAX_PUBLISH_TOPIC; i++) {

        _topic_label[5] = (0x30 + i );_topic_name[4] = (0x30 + i );
        _qos_label[3] = (0x30 + i );_qos_name[4] = (0x30 + i );
        _retain_label[6] = (0x30 + i );_retain_name[4] = (0x30 + i );

        concat_tr_input_html_tags( _page, _topic_label, _topic_name, _mqtt_pubsub_configs.publish_topics[i].topic, "50"  );
        concat_tr_select_html_tags( _page, _qos_label, _qos_name, _qos_options, 3, _mqtt_pubsub_configs.publish_topics[i].qos+1 );
        concat_tr_input_html_tags( _page, _retain_label, _retain_name, "retain", "", HTML_INPUT_CHECKBOX_TAG_TYPE, _mqtt_pubsub_configs.publish_topics[i].retain != 0 );
      }
      concat_tr_input_html_tags( _page, PSTR("Publish Frequency:"), PSTR("pfrq"), _publish_freq );

      _topic_name[0] = 's'; _qos_name[0] = 's';
      concat_tr_header_html_tags( _page, PSTR("Subscribe Topics") );
      for (uint8_t i = 0; i < MQTT_MAX_SUBSCRIBE_TOPIC; i++) {

        _topic_label[5] = (0x30 + i );_topic_name[4] = (0x30 + i );
        _qos_label[3] = (0x30 + i );_qos_name[4] = (0x30 + i );

        concat_tr_input_html_tags( _page, _topic_label, _topic_name, _mqtt_pubsub_configs.subscribe_topics[i].topic, "50"  );
        concat_tr_select_html_tags( _page, _qos_label, _qos_name, _qos_options, 3, _mqtt_pubsub_configs.subscribe_topics[i].qos+1 );
      }

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleMqttPubSubConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Mqtt LWT Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("ptpc0") && this->EwServer->hasArg("pqos0") ) {

        mqtt_pubsub_config_table* _mqtt_pubsub_configs = new mqtt_pubsub_config_table;
        memset( (void*)_mqtt_pubsub_configs, 0, sizeof(mqtt_pubsub_config_table) );

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n\nPublish Topics"));
        #endif
        char _topic_name[10];memset(_topic_name, 0, 10);strcpy( _topic_name, "ptpc0" );
        char _qos_name[10];memset(_qos_name, 0, 10);strcpy( _qos_name, "pqos0" );
        char _retain_name[10];memset(_retain_name, 0, 10);strcpy( _retain_name, "prtn0" );

        for (uint8_t i = 0; i < MQTT_MAX_PUBLISH_TOPIC; i++) {

          _topic_name[4] = (0x30 + i );
          _qos_name[4] = (0x30 + i );
          _retain_name[4] = (0x30 + i );

          String _topic = this->EwServer->arg(_topic_name);
          String _qos = this->EwServer->arg(_qos_name);
          String _retain = this->EwServer->arg(_retain_name);

          _topic.toCharArray( _mqtt_pubsub_configs->publish_topics[i].topic, _topic.length()+1 );
          _mqtt_pubsub_configs->publish_topics[i].qos = (int)_qos.toInt() - 1;
          _mqtt_pubsub_configs->publish_topics[i].retain = (int)( _retain == "retain" );

          #ifdef EW_SERIAL_LOG
            Log(F("Topic")); Log(i); Log(F(" : ")); Logln(_topic);
            Log(F("QoS")); Log(i); Log(F(" : ")); Logln(_qos);
            Log(F("Retain")); Log(i); Log(F(" : ")); Logln(_retain);
          #endif
        }

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubscribe Topics"));
        #endif
        _topic_name[0] = 's'; _qos_name[0] = 's';
        for (uint8_t i = 0; i < MQTT_MAX_SUBSCRIBE_TOPIC; i++) {

          _topic_name[4] = (0x30 + i );
          _qos_name[4] = (0x30 + i );

          String _topic = this->EwServer->arg(_topic_name);
          String _qos = this->EwServer->arg(_qos_name);

          _topic.toCharArray( _mqtt_pubsub_configs->subscribe_topics[i].topic, _topic.length()+1 );
          _mqtt_pubsub_configs->subscribe_topics[i].qos = (int)_qos.toInt() - 1;

          #ifdef EW_SERIAL_LOG
            Log(F("Topic")); Log(i); Log(F(" : ")); Logln(_topic);
            Log(F("QoS")); Log(i); Log(F(" : ")); Logln(_qos);
          #endif
        }
        _mqtt_pubsub_configs->publish_frequency = (int)this->EwServer->arg("pfrq").toInt();

        this->ew_db->set_mqtt_pubsub_config_table( _mqtt_pubsub_configs );

        delete _mqtt_pubsub_configs;
        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_mqtt_pubsub_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_mqtt_config_callback_fn!=NULL ){
        this->_ewstack_mqtt_config_callback_fn(MQTT_PUBSUB_CONFIG);
      }
    }

    #endif

    #ifdef ENABLE_GPIO_CONFIG

    void handleMonitorListen( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling monitor voice route"));
      #endif

      int y1 = this->_last_monitor_point.y,
      y2 = map(
        this->_ewstack_virtual_gpio_configs->gpio_readings[MAX_NO_OF_GPIO_PINS], 0, ANALOG_GPIO_RESOLUTION,
        GPIO_GRAPH_TOP_MARGIN, GPIO_MAX_GRAPH_HEIGHT - GPIO_GRAPH_BOTTOM_MARGIN
      ),
      x1 = this->_last_monitor_point.x < GPIO_MAX_GRAPH_WIDTH ?
      this->_last_monitor_point.x:GPIO_MAX_GRAPH_WIDTH-GPIO_GRAPH_ADJ_POINT_DISTANCE,
      x2 = x1 + GPIO_GRAPH_ADJ_POINT_DISTANCE;

      y2 = GPIO_MAX_GRAPH_HEIGHT - y2;

      String _response = "{\"x1\":";
      _response += x1;
      _response += ",\"y1\":";
      _response += y1;
      _response += ",\"x2\":";
      _response += x2;
      _response += ",\"y2\":";
      _response += y2;
      _response += ",\"r\":";
      _response += !this->has_active_session();
      _response += "}";

      this->_last_monitor_point.x = x2;
      this->_last_monitor_point.y = y2;
      this->EwServer->sendHeader("Cache-Control", "no-cache");
      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _response );
    }

    void handleGpioManageRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Manage route"));
      #endif

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_GPIO_MANAGE_PAGE );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    void build_gpio_monitor_html( char* _page, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_MONITOR_PAGE_TOP );
      strcat_P( _page, PSTR("<div style='display:inline-flex;'>") );
      concat_graph_axis_title_div( _page, (char*)"A0 ( 0 - 1024 )", (char*)"writing-mode:vertical-lr" );
      strcat_P( _page, EW_SERVER_GPIO_MONITOR_SVG_ELEMENT );
      strcat_P( _page, PSTR("</div>") );
      concat_graph_axis_title_div( _page, (char*)"Time" );
      strcat_P( _page, EW_SERVER_FOOTER_WITH_MONITOR_LISTEN_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleGpioMonitorRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Monitor Config route"));
      #endif

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_gpio_monitor_html( _page );

      this->_last_monitor_point.x = 0;
      this->_last_monitor_point.y = GPIO_MAX_GRAPH_HEIGHT - GPIO_GRAPH_BOTTOM_MARGIN;
      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    void build_gpio_server_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_SERVER_PAGE_TOP );

      char _port[10],_freq[10];memset( _port, 0, 10 );memset( _freq, 0, 10 );
      __appendUintToBuff( _port, "%d", this->gpio_configs.gpio_port, 8 );
      __appendUintToBuff( _freq, "%d", this->gpio_configs.gpio_post_frequency, 8 );

      concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), this->gpio_configs.gpio_host, (char*)"40" );
      concat_tr_input_html_tags( _page, PSTR("Host Port:"), PSTR("prt"), _port );
      concat_tr_input_html_tags( _page, PSTR("Post Frequency:"), PSTR("frq"), _freq );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleGpioServerConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Server Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("hst") && this->EwServer->hasArg("prt") ) {

        String _gpio_host = this->EwServer->arg("hst");
        String _gpio_port = this->EwServer->arg("prt");
        String _post_freq = this->EwServer->arg("frq");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("gpio host : ")); Logln( _gpio_host );
          Log(F("gpio port : ")); Logln( _gpio_port );
          Log(F("post freq : ")); Logln( _post_freq );
          Logln();
        #endif

        _gpio_host.toCharArray( this->gpio_configs.gpio_host, _gpio_host.length()+1 );
        this->gpio_configs.gpio_port = (int)_gpio_port.toInt();
        this->gpio_configs.gpio_post_frequency = (int)_post_freq.toInt() < 0 ? GPIO_DATA_POST_FREQ : (int)_post_freq.toInt();
        this->ew_db->set_gpio_config_table( &this->gpio_configs );

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_gpio_server_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_gpio_config_callback_fn!=NULL ){
        this->_ewstack_gpio_config_callback_fn(GPIO_SERVER_CONFIG);
      }

    }

    void build_gpio_mode_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_CONFIG_PAGE_TOP );
      char* _gpio_mode_general_options[] = {"OFF", "DOUT", "DIN", "AOUT"};
      char* _gpio_mode_analog_options[] = {"OFF", "", "", "", "AIN"};

      char _name[4], _label[4];memset(_name, 0, 4);memset(_label, 0, 4);
      strcpy( _name, "D0:" );strcpy( _label, "d0" );
      int _exception = 0;
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
        _name[1] = (0x30 + _pin );_label[1] = (0x30 + _pin );
        _exception = _pin == 0 ? 4:0;
        if( !is_exceptional_gpio_pin(_pin) )
        concat_tr_select_html_tags( _page, _name, _label, _gpio_mode_general_options, 4, (int)this->gpio_configs.gpio_mode[_pin], _exception );
      }
      concat_tr_select_html_tags( _page, (char*)"A0:", (char*)"a0", _gpio_mode_analog_options, 5, (int)this->gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS] );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

    void handleGpioModeConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Mode Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( this->EwServer->hasArg("d0") && this->EwServer->hasArg("a0") ) {

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
        #endif
        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin );
          this->gpio_configs.gpio_mode[_pin] = !is_exceptional_gpio_pin(_pin) ? (int)this->EwServer->arg(_label).toInt():0;
          #ifdef EW_SERIAL_LOG
            Log(F("Pin ")); Log( _pin ); Log(F(" : ")); Logln( (int)this->EwServer->arg(_label).toInt() );
          #endif
        }
        this->gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS] = (int)this->EwServer->arg("a0").toInt();

        #ifdef EW_SERIAL_LOG
          Log(F("Pin A: ")); Logln( this->gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS] );
          Logln();
        #endif

        this->ew_db->set_gpio_config_table( &this->gpio_configs );

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_gpio_mode_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_gpio_config_callback_fn!=NULL ){
        this->_ewstack_gpio_config_callback_fn(GPIO_MODE_CONFIG);
      }

    }

    void build_gpio_write_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_WRITE_PAGE_TOP );
      char* _gpio_digital_write_options[] = {"LOW","HIGH"};

      char _analog_value[10], _name[4], _label[4];memset(_name, 0, 4);memset(_label, 0, 4);
      strcpy( _name, "D0:" );strcpy( _label, "d0" );

      bool _added_options = false;
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
        _name[1] = (0x30 + _pin );_label[1] = (0x30 + _pin );

        if( !is_exceptional_gpio_pin(_pin) ){

          if( this->gpio_configs.gpio_mode[_pin] == DIGITAL_WRITE ){
            _added_options = true;
            concat_tr_select_html_tags( _page, _name, _label, _gpio_digital_write_options, 2, (int)this->gpio_configs.gpio_readings[_pin]+1 );
          }
          if( this->gpio_configs.gpio_mode[_pin] == ANALOG_WRITE ){
            _added_options = true;
            memset( _analog_value, 0, 10 );
            __appendUintToBuff( _analog_value, "%d", this->gpio_configs.gpio_readings[_pin], 8 );
            concat_tr_input_html_tags( _page, _name, _label, _analog_value );
          }
        }
      }

      if( _added_options ){
        strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      }else{
        strcat_P( _page, EW_SERVER_GPIO_WRITE_EMPTY_MESSAGE );
      }
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

    void handleGpioWriteConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Write Config route"));
      #endif
      bool _is_posted = false;

      if ( !this->has_active_session() ) {

        this->EwServer->sendHeader("Location", EW_SERVER_LOGIN_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->send(HTTP_REDIRECT);
        return;
      }

      if ( true ) {

        // #ifdef EW_SERIAL_LOG
        //   Logln(F("\nSubmitted info :\n"));
        // #endif
        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin );
          if( this->EwServer->hasArg(_label) ){
            this->gpio_configs.gpio_readings[_pin] = is_exceptional_gpio_pin(_pin) ? 0 : this->gpio_configs.gpio_mode[_pin] == DIGITAL_WRITE ?
            (int)this->EwServer->arg(_label).toInt()-1 : (int)this->EwServer->arg(_label).toInt();
            #ifdef EW_SERIAL_LOG
              Log(F("Pin ")); Log( _pin ); Log(F(" : ")); Logln( (int)this->EwServer->arg(_label).toInt() );
            #endif
            _is_posted = true;
          }
        }

        if( _is_posted )
        this->ew_db->set_gpio_config_table( &this->gpio_configs );
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_gpio_write_config_html( _page, _is_posted );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_gpio_config_callback_fn!=NULL ){
        this->_ewstack_gpio_config_callback_fn(GPIO_WRITE_CONFIG);
      }

    }

    #endif

    void handleLogoutRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling logout route"));
      #endif
      this->send_inactive_session_headers();

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_LOGOUT_PAGE );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

    void handleLoginRoute( void ) {

      bool _is_posted = false;

      if( this->EwServer->hasArg("username") && this->EwServer->hasArg("password") ){
        _is_posted = true;
      }

      if ( this->has_active_session() || ( _is_posted && this->EwServer->arg("username") == this->login_credentials.username &&
        this->EwServer->arg("password") == this->login_credentials.password
      ) ) {

        char _session_cookie[EW_COOKIE_BUFF_MAX_SIZE];
        this->build_session_cookie( _session_cookie, true, EW_COOKIE_BUFF_MAX_SIZE, true, this->login_credentials.cookie_max_age );

        this->EwServer->sendHeader("Location", EW_SERVER_HOME_ROUTE);
        this->EwServer->sendHeader("Cache-Control", "no-cache");
        this->EwServer->sendHeader("Set-Cookie", _session_cookie);
        this->EwServer->send(HTTP_REDIRECT);
        #ifdef EW_SERIAL_LOG
        Logln(F("Log in Successful"));
        #endif
        return;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      // char _page[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_LOGIN_PAGE, _is_posted, (char*)"Wrong Credentials.", ALERT_DANGER );

      this->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

  protected:

    EwingsDefaultDB* ew_db;
    wifi_config_table wifi_configs;

    #ifdef ENABLE_GPIO_CONFIG
    gpio_config_table gpio_configs;
    gpio_config_table* _ewstack_virtual_gpio_configs;
    CallBackIntArgFn _ewstack_gpio_config_callback_fn=NULL;
    last_gpio_monitor_point _last_monitor_point;
    #endif

    #ifdef ENABLE_MQTT_CONFIG
    CallBackIntArgFn _ewstack_mqtt_config_callback_fn=NULL;
    #endif

    CallBackIntArgFn _ewstack_callback_fn=NULL;
};

#endif
