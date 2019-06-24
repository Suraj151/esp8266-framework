/******************************* GPIO Controller ******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_GPIO_CONTROLLER_
#define _EW_SERVER_GPIO_CONTROLLER_

#include <webserver/resources/WebResource.h>
#include <webserver/pages/GpioConfigPage.h>

/**
 * GpioController class
 */
class GpioController {

	protected:

		/**
		 * @var	EwWebResourceProvider*	web_resource
		 */
		EwWebResourceProvider* web_resource;

		/**
		 * @var	gpio_config_table	gpio_configs
		 */
    gpio_config_table gpio_configs;

		/**
		 * @var	gpio_config_table*	_ewstack_virtual_gpio_configs
		 */
    gpio_config_table* _ewstack_virtual_gpio_configs;

		/**
		 * @var	CallBackIntArgFn|NULL	_ewstack_gpio_config_callback_fn
		 */
    CallBackIntArgFn _ewstack_gpio_config_callback_fn=NULL;

		/**
		 * @var	last_gpio_monitor_point	_last_monitor_point
		 */
    last_gpio_monitor_point _last_monitor_point;

	public:

		/**
		 * GpioController constructor
		 */
		GpioController(){
		}

		/**
		 * GpioController destructor
		 */
		~GpioController(){
		}

		/**
		 * register gpio route handler
		 *
		 * @param	EwWebResourceProvider*	_web_resource
		 */
		void handle( EwWebResourceProvider* _web_resource ){

			this->web_resource = _web_resource;
			this->use_gpio_configs( this->web_resource->ew_db->get_gpio_config_table() );
			this->web_resource->register_route( EW_SERVER_GPIO_MANAGE_CONFIG_ROUTE, [&]() { this->handleGpioManageRoute(); }, AUTH_MIDDLEWARE );
      this->web_resource->register_route( EW_SERVER_GPIO_SERVER_CONFIG_ROUTE, [&]() { this->handleGpioServerConfigRoute(); }, AUTH_MIDDLEWARE );
      this->web_resource->register_route( EW_SERVER_GPIO_MODE_CONFIG_ROUTE, [&]() { this->handleGpioModeConfigRoute(); }, AUTH_MIDDLEWARE );
      this->web_resource->register_route( EW_SERVER_GPIO_WRITE_CONFIG_ROUTE, [&]() { this->handleGpioWriteConfigRoute(); }, AUTH_MIDDLEWARE );
      this->web_resource->register_route( EW_SERVER_GPIO_MONITOR_ROUTE, [&]() { this->handleGpioMonitorRoute(); }, AUTH_MIDDLEWARE );
      this->web_resource->register_route( EW_SERVER_GPIO_ANALOG_MONITOR_ROUTE, [&]() { this->handleAnalogMonitor(); } );
		}

		/**
		 * set gpio config callback. called when gpio config modified by client.
		 *
		 * @param	CallBackIntArgFn	_fn
		 */
		void set_ewstack_gpio_config_callback( CallBackIntArgFn _fn ){
      this->_ewstack_gpio_config_callback_fn = _fn;
    }

		/**
		 * set virtual gpio config. this config created in heap to get sync with
		 * real time values of gpios.
		 *
		 * @param	gpio_config_table*	_config
		 */
    void set_ewstack_virtual_gpio_configs( gpio_config_table* _config ){
      this->_ewstack_virtual_gpio_configs = _config;
    }

		/**
		 * get and set gpio config object.
		 *
		 * @param	gpio_config_table	_gpio_configs
		 */
    void use_gpio_configs( gpio_config_table _gpio_configs ){
      this->gpio_configs = _gpio_configs;
    }

		/**
		 * handle adc pin monitor calls.
		 * it provides live analog readings mapped to graph height.
		 */
    void handleAnalogMonitor( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling analog monitor route"));
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
      _response += !this->web_resource->has_active_session();
      _response += "}";

      this->_last_monitor_point.x = x2;
      this->_last_monitor_point.y = y2;
      this->web_resource->EwServer->sendHeader("Cache-Control", "no-cache");
      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _response );
    }

		/**
		 * build html page with header, middle and footer part.
		 *
		 * @param	char*	_page
		 * @param	PGM_P	_pgm_page
		 * @param	bool|false	_enable_flash
		 * @param	char*|""	_message
		 * @param	FLASH_MSG_TYPE|ALERT_SUCCESS	_alert_type
		 * @param	bool|true	_enable_header_footer
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
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
    }

		/**
		 * build and send gpio manage page to client
		 */
    void handleGpioManageRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Manage route"));
      #endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_GPIO_MANAGE_PAGE );

      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

		/**
		 * build gpio monitor html page
		 *
		 * @param	char*	_page
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
    void build_gpio_monitor_html( char* _page, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_MONITOR_PAGE_TOP );
      strcat_P( _page, PSTR("<div style='display:inline-flex;'>") );
      concat_graph_axis_title_div( _page, (char*)"A0 ( 0 - 1024 )", (char*)"writing-mode:vertical-lr" );
      strcat_P( _page, EW_SERVER_GPIO_MONITOR_SVG_ELEMENT );
      strcat_P( _page, PSTR("</div>") );
      concat_graph_axis_title_div( _page, (char*)"Time" );
      strcat_P( _page, EW_SERVER_FOOTER_WITH_ANALOG_MONITOR_HTML );
    }

		/**
		 * build and send gpio monitor config page to client
		 */
    void handleGpioMonitorRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Monitor Config route"));
      #endif

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_monitor_html( _page );

      this->_last_monitor_point.x = 0;
      this->_last_monitor_point.y = GPIO_MAX_GRAPH_HEIGHT - GPIO_GRAPH_BOTTOM_MARGIN;
      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

		/**
		 * build gpio server config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
    void build_gpio_server_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_SERVER_PAGE_TOP );

      char _port[10],_freq[10];memset( _port, 0, 10 );memset( _freq, 0, 10 );
      __appendUintToBuff( _port, "%d", this->gpio_configs.gpio_port, 8 );
      __appendUintToBuff( _freq, "%d", this->gpio_configs.gpio_post_frequency, 8 );

      concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), this->gpio_configs.gpio_host, GPIO_HOST_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Host Port:"), PSTR("prt"), _port );
      concat_tr_input_html_tags( _page, PSTR("Post Frequency:"), PSTR("frq"), _freq );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
      // Log(F("free stack :"));Logln(ESP.getFreeContStack());
      // Log(F("free heap :"));Logln(ESP.getFreeHeap());
    }

		/**
		 * build and send gpio server config page.
		 * when posted, get gpio server configs from client and set them in database.
		 */
    void handleGpioServerConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Server Config route"));
      #endif
      bool _is_posted = false;

      if ( this->web_resource->EwServer->hasArg("hst") && this->web_resource->EwServer->hasArg("prt") ) {

        String _gpio_host = this->web_resource->EwServer->arg("hst");
        String _gpio_port = this->web_resource->EwServer->arg("prt");
        String _post_freq = this->web_resource->EwServer->arg("frq");

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
        this->web_resource->ew_db->set_gpio_config_table( &this->gpio_configs );

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_server_config_html( _page, _is_posted );

      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_gpio_config_callback_fn!=NULL ){
        this->_ewstack_gpio_config_callback_fn(GPIO_SERVER_CONFIG);
      }

    }

		/**
		 * build gpio mode config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
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
    }

		/**
		 * build and send gpio mode config page.
		 * when posted, get gpio mode configs from client and set them in database.
		 */
    void handleGpioModeConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Mode Config route"));
      #endif
      bool _is_posted = false;

      if ( this->web_resource->EwServer->hasArg("d0") && this->web_resource->EwServer->hasArg("a0") ) {

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
        #endif
        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin );
          this->gpio_configs.gpio_mode[_pin] = !is_exceptional_gpio_pin(_pin) ? (int)this->web_resource->EwServer->arg(_label).toInt():0;
          #ifdef EW_SERIAL_LOG
            Log(F("Pin ")); Log( _pin ); Log(F(" : ")); Logln( (int)this->web_resource->EwServer->arg(_label).toInt() );
          #endif
        }
        this->gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS] = (int)this->web_resource->EwServer->arg("a0").toInt();

        #ifdef EW_SERIAL_LOG
          Log(F("Pin A: ")); Logln( this->gpio_configs.gpio_mode[MAX_NO_OF_GPIO_PINS] );
          Logln();
        #endif

        this->web_resource->ew_db->set_gpio_config_table( &this->gpio_configs );

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_mode_config_html( _page, _is_posted );

      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_gpio_config_callback_fn!=NULL ){
        this->_ewstack_gpio_config_callback_fn(GPIO_MODE_CONFIG);
      }
    }

		/**
		 * build gpio write config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
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

		/**
		 * build and send gpio write config page.
		 * when posted, get gpio write configs from client and set them in database.
		 */
    void handleGpioWriteConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Write Config route"));
      #endif
      bool _is_posted = false;

      if ( true ) {

        // #ifdef EW_SERIAL_LOG
        //   Logln(F("\nSubmitted info :\n"));
        // #endif
        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin );
          if( this->web_resource->EwServer->hasArg(_label) ){
            this->gpio_configs.gpio_readings[_pin] = is_exceptional_gpio_pin(_pin) ? 0 : this->gpio_configs.gpio_mode[_pin] == DIGITAL_WRITE ?
            (int)this->web_resource->EwServer->arg(_label).toInt()-1 : (int)this->web_resource->EwServer->arg(_label).toInt();
            #ifdef EW_SERIAL_LOG
              Log(F("Pin ")); Log( _pin ); Log(F(" : ")); Logln( (int)this->web_resource->EwServer->arg(_label).toInt() );
            #endif
            _is_posted = true;
          }
        }

        if( _is_posted )
        this->web_resource->ew_db->set_gpio_config_table( &this->gpio_configs );
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_write_config_html( _page, _is_posted );

      this->web_resource->EwServer->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
      if( _is_posted && this->_ewstack_gpio_config_callback_fn!=NULL ){
        this->_ewstack_gpio_config_callback_fn(GPIO_WRITE_CONFIG);
      }

    }

};

#endif
