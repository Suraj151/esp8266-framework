/******************************* GPIO Controller ******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_GPIO_CONTROLLER_
#define _EW_SERVER_GPIO_CONTROLLER_

#include "Controller.h"
#include <webserver/pages/GpioConfigPage.h>
#include <service_provider/GpioServiceProvider.h>

/**
 * GpioController class
 */
class GpioController : public Controller {

	protected:

		/**
		 * @var	gpio_config_table	gpio_configs
		 */
    // gpio_config_table gpio_configs;

		/**
		 * @var	last_gpio_monitor_point	_last_monitor_point
		 */
    last_gpio_monitor_point _last_monitor_point;

	public:

		/**
		 * GpioController constructor
		 */
		GpioController():Controller("gpio"){
		}

		/**
		 * GpioController destructor
		 */
		~GpioController(){
		}

		/**
		 * register gpio controller
		 *
		 */
		void boot( void ){

			if( nullptr != this->m_route_handler ){
				this->m_route_handler->register_route( EW_SERVER_GPIO_MANAGE_CONFIG_ROUTE, [&]() { this->handleGpioManageRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_GPIO_SERVER_CONFIG_ROUTE, [&]() { this->handleGpioServerConfigRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_GPIO_MODE_CONFIG_ROUTE, [&]() { this->handleGpioModeConfigRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_GPIO_WRITE_CONFIG_ROUTE, [&]() { this->handleGpioWriteConfigRoute(); }, AUTH_MIDDLEWARE );
				this->m_route_handler->register_route( EW_SERVER_GPIO_ALERT_CONFIG_ROUTE, [&]() { this->handleGpioAlertConfigRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_GPIO_MONITOR_ROUTE, [&]() { this->handleGpioMonitorRoute(); }, AUTH_MIDDLEWARE );
	      this->m_route_handler->register_route( EW_SERVER_GPIO_ANALOG_MONITOR_ROUTE, [&]() { this->handleAnalogMonitor(); } );
			}
			// this->gpio_configs = this->m_web_resource->m_db_conn->get_gpio_config_table();
		}

		/**
		 * handle adc pin monitor calls.
		 * it provides live analog readings mapped to graph height.
		 */
    void handleAnalogMonitor( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling analog monitor route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_server ||
					nullptr == this->m_route_handler ){
				return;
			}

      int y1 = this->_last_monitor_point.y,
      y2 = map(
        __gpio_service.m_gpio_config_copy.gpio_readings[MAX_NO_OF_GPIO_PINS], 0, ANALOG_GPIO_RESOLUTION,
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
      _response += !this->m_route_handler->has_active_session();
			_response += ",\"d\":";
			__gpio_service.appendGpioJsonPayload(_response);
			_response += ",\"md\":[\"OFF\", \"DOUT\", \"DIN\", \"BLINK\", \"AOUT\", \"AIN\"]";
      _response += "}";

      this->_last_monitor_point.x = x2;
      this->_last_monitor_point.y = y2;
      this->m_web_resource->m_server->sendHeader("Cache-Control", "no-cache");
      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _response );
    }

		/**
		 * build and send gpio manage page to client
		 */
    void handleGpioManageRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Manage route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      char* _page = new char[EW_HTML_MAX_SIZE];
			memset( _page, 0, EW_HTML_MAX_SIZE );

			strcat_P( _page, EW_SERVER_HEADER_HTML );
			strcat_P( _page, EW_SERVER_MENU_CARD_PAGE_WRAP_TOP );

			concat_svg_menu_card( _page, EW_SERVER_GPIO_MENU_TITLE_MODES, SVG_ICON48_PATH_TUNE, EW_SERVER_GPIO_MODE_CONFIG_ROUTE );
			concat_svg_menu_card( _page, EW_SERVER_GPIO_MENU_TITLE_CONTROL, SVG_ICON48_PATH_GAME_ASSET, EW_SERVER_GPIO_WRITE_CONFIG_ROUTE );
			concat_svg_menu_card( _page, EW_SERVER_GPIO_MENU_TITLE_SERVER, SVG_ICON48_PATH_COMPUTER, EW_SERVER_GPIO_SERVER_CONFIG_ROUTE );
			concat_svg_menu_card( _page, EW_SERVER_GPIO_MENU_TITLE_MONITOR, SVG_ICON48_PATH_EYE, EW_SERVER_GPIO_MONITOR_ROUTE );
			concat_svg_menu_card( _page, EW_SERVER_GPIO_MENU_TITLE_ALERT, SVG_ICON48_PATH_NOTIFICATION, EW_SERVER_GPIO_ALERT_CONFIG_ROUTE );

			strcat_P( _page, EW_SERVER_MENU_CARD_PAGE_WRAP_BOTTOM );
			strcat_P( _page, EW_SERVER_FOOTER_HTML );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
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

			char* _gpio_monitor_table_heading[] = {"Pin", "Mode", "value"};
			strcat_P( _page, HTML_TABLE_OPEN_TAG );
			concat_style_attribute( _page, PSTR("width:92%") );
			strcat_P( _page, HTML_TAG_CLOSE_BRACKET );
			concat_table_heading_row( _page, _gpio_monitor_table_heading, 3, nullptr, nullptr, PSTR("btn"), nullptr );
			char _name[3]; memset(_name, 0, 3); strcpy( _name, "D0" );
			char* _gpio_monitor_table_data[] = {_name, "", ""};
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
        if( !__gpio_service.is_exceptional_gpio_pin(_pin) ){

					_name[1] = (0x30 + _pin );
					concat_table_data_row( _page, _gpio_monitor_table_data, 3, nullptr, nullptr, PSTR("btnd"), nullptr );
				}
      }
			memset(_name, 0, 3); strcpy( _name, "A0" );
			concat_table_data_row( _page, _gpio_monitor_table_data, 3, nullptr, nullptr, PSTR("btnd"), nullptr );
			strcat_P( _page, HTML_TABLE_CLOSE_TAG );

			strcat_P( _page, HTML_DIV_OPEN_TAG );
			concat_style_attribute( _page, PSTR("display:inline-flex;margin-top:25px;") );
			strcat_P( _page, HTML_TAG_CLOSE_BRACKET );
      concat_graph_axis_title_div( _page, (char*)"A0 ( 0 - 1024 )", (char*)"writing-mode:vertical-lr" );
      strcat_P( _page, EW_SERVER_GPIO_MONITOR_SVG_ELEMENT );
			strcat_P( _page, HTML_DIV_CLOSE_TAG );
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

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_monitor_html( _page );

      this->_last_monitor_point.x = 0;
      this->_last_monitor_point.y = GPIO_MAX_GRAPH_HEIGHT - GPIO_GRAPH_BOTTOM_MARGIN;
      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
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
      __appendUintToBuff( _port, "%d", __gpio_service.m_gpio_config_copy.gpio_port, 8 );
      __appendUintToBuff( _freq, "%d", __gpio_service.m_gpio_config_copy.gpio_post_frequency, 8 );

      concat_tr_input_html_tags( _page, PSTR("Host Address:"), PSTR("hst"), __gpio_service.m_gpio_config_copy.gpio_host, GPIO_HOST_BUF_SIZE-1 );
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

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

      if ( this->m_web_resource->m_server->hasArg("hst") && this->m_web_resource->m_server->hasArg("prt") ) {

        String _gpio_host = this->m_web_resource->m_server->arg("hst");
        String _gpio_port = this->m_web_resource->m_server->arg("prt");
        String _post_freq = this->m_web_resource->m_server->arg("frq");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("gpio host : ")); Logln( _gpio_host );
          Log(F("gpio port : ")); Logln( _gpio_port );
          Log(F("post freq : ")); Logln( _post_freq );
          Logln();
        #endif

        _gpio_host.toCharArray( __gpio_service.m_gpio_config_copy.gpio_host, _gpio_host.length()+1 );
        __gpio_service.m_gpio_config_copy.gpio_port = (int)_gpio_port.toInt();
        __gpio_service.m_gpio_config_copy.gpio_post_frequency = (int)_post_freq.toInt() < 0 ? GPIO_DATA_POST_FREQ : (int)_post_freq.toInt();
        this->m_web_resource->m_db_conn->set_gpio_config_table( &__gpio_service.m_gpio_config_copy );

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_server_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
				__gpio_service.handleGpioModes(GPIO_SERVER_CONFIG);
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
      char* _gpio_mode_general_options[] = {"OFF", "DOUT", "DIN", "BLINK", "AOUT"};
      char* _gpio_mode_analog_options[] = {"OFF", "", "", "", "", "AIN"};

      char _name[4], _label[4];memset(_name, 0, 4);memset(_label, 0, 4);
      strcpy( _name, "D0:" );strcpy( _label, "d0" );
      int _exception = -1;
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
        _name[1] = (0x30 + _pin );_label[1] = (0x30 + _pin );
        _exception = _pin == 0 ? ANALOG_WRITE:-1;
        if( !__gpio_service.is_exceptional_gpio_pin(_pin) )
        concat_tr_select_html_tags( _page, _name, _label, _gpio_mode_general_options, 5, (int)__gpio_service.m_gpio_config_copy.gpio_mode[_pin], _exception );
      }
      concat_tr_select_html_tags( _page, (char*)"A0:", (char*)"a0", _gpio_mode_analog_options, 6, (int)__gpio_service.m_gpio_config_copy.gpio_mode[MAX_NO_OF_GPIO_PINS] );

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

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

      if ( this->m_web_resource->m_server->hasArg("d0") && this->m_web_resource->m_server->hasArg("a0") ) {

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
        #endif
        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin );
          __gpio_service.m_gpio_config_copy.gpio_mode[_pin] = !__gpio_service.is_exceptional_gpio_pin(_pin) ? (int)this->m_web_resource->m_server->arg(_label).toInt():0;
          #ifdef EW_SERIAL_LOG
            Log(F("Pin ")); Log( _pin ); Log(F(" : ")); Logln( (int)this->m_web_resource->m_server->arg(_label).toInt() );
          #endif
					if( __gpio_service.m_gpio_config_copy.gpio_mode[_pin] == OFF || __gpio_service.m_gpio_config_copy.gpio_mode[_pin] == DIGITAL_WRITE || __gpio_service.m_gpio_config_copy.gpio_mode[_pin] == ANALOG_WRITE ){
			      __gpio_service.m_gpio_config_copy.gpio_alert_comparator[_pin] = EQUAL;
						__gpio_service.m_gpio_config_copy.gpio_alert_channel[_pin] = NO_ALERT;
						__gpio_service.m_gpio_config_copy.gpio_alert_values[_pin] = OFF;
			    }
        }
        __gpio_service.m_gpio_config_copy.gpio_mode[MAX_NO_OF_GPIO_PINS] = (int)this->m_web_resource->m_server->arg("a0").toInt();
				if( __gpio_service.m_gpio_config_copy.gpio_mode[MAX_NO_OF_GPIO_PINS] == OFF ){
					__gpio_service.m_gpio_config_copy.gpio_alert_comparator[MAX_NO_OF_GPIO_PINS] = EQUAL;
					__gpio_service.m_gpio_config_copy.gpio_alert_channel[MAX_NO_OF_GPIO_PINS] = NO_ALERT;
					__gpio_service.m_gpio_config_copy.gpio_alert_values[MAX_NO_OF_GPIO_PINS] = OFF;
				}

        #ifdef EW_SERIAL_LOG
          Log(F("Pin A: ")); Logln( __gpio_service.m_gpio_config_copy.gpio_mode[MAX_NO_OF_GPIO_PINS] );
          Logln();
        #endif

        this->m_web_resource->m_db_conn->set_gpio_config_table( &__gpio_service.m_gpio_config_copy );

        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_mode_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
				__gpio_service.handleGpioModes(GPIO_MODE_CONFIG);
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

      char _input_value[10], _name[4], _label[4];memset(_name, 0, 4);memset(_label, 0, 4);
      strcpy( _name, "D0:" );strcpy( _label, "d0" );

      bool _added_options = false;
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
        _name[1] = (0x30 + _pin );_label[1] = (0x30 + _pin );

        if( !__gpio_service.is_exceptional_gpio_pin(_pin) ){

          if( __gpio_service.m_gpio_config_copy.gpio_mode[_pin] == DIGITAL_WRITE ){
            _added_options = true;
            concat_tr_select_html_tags( _page, _name, _label, _gpio_digital_write_options, 2, (int)__gpio_service.m_gpio_config_copy.gpio_readings[_pin] );
          }
          if( __gpio_service.m_gpio_config_copy.gpio_mode[_pin] == ANALOG_WRITE ||
						__gpio_service.m_gpio_config_copy.gpio_mode[_pin] == DIGITAL_BLINK
					){
            _added_options = true;
            memset( _input_value, 0, 10 );
            __appendUintToBuff( _input_value, "%d", __gpio_service.m_gpio_config_copy.gpio_readings[_pin], 8 );

						if( DIGITAL_BLINK == __gpio_service.m_gpio_config_copy.gpio_mode[_pin] ){

							concat_tr_input_html_tags( _page, _name, _label, _input_value, 10, HTML_INPUT_TEXT_TAG_TYPE, false, false );
						}else{

							concat_tr_input_html_tags( _page, _name, _label, _input_value, 0, HTML_INPUT_RANGE_TAG_TYPE, false, false );
						}
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

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

      if ( true ) {

        // #ifdef EW_SERIAL_LOG
        //   Logln(F("\nSubmitted info :\n"));
        // #endif
        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin );
          if( this->m_web_resource->m_server->hasArg(_label) ){
            __gpio_service.m_gpio_config_copy.gpio_readings[_pin] = __gpio_service.is_exceptional_gpio_pin(_pin) ? 0 : (int)this->m_web_resource->m_server->arg(_label).toInt();
            #ifdef EW_SERIAL_LOG
              Log(F("Pin ")); Log( _pin ); Log(F(" : ")); Logln( (int)this->m_web_resource->m_server->arg(_label).toInt() );
            #endif
            _is_posted = true;
          }
        }

        if( _is_posted )
        this->m_web_resource->m_db_conn->set_gpio_config_table( &__gpio_service.m_gpio_config_copy );
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_write_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
				__gpio_service.handleGpioModes(GPIO_WRITE_CONFIG);
      }
    }



		/**
		 * build gpio alert config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
    void build_gpio_alert_config_html( char* _page, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_GPIO_ALERT_PAGE_TOP );
      char* _gpio_digital_alert_options[] = {"LOW","HIGH"};
			#ifdef ENABLE_EMAIL_SERVICE
			char* _gpio_alert_channels[] = {"NOALERT","MAIL"};
			#else
			char* _gpio_alert_channels[] = {"NOALERT",""};
			#endif
			char* _gpio_analog_alert_comparators[] = {"=",">","<"};

      char _analog_value[10], _name[4], _label[4], _alert_label[4];
			memset(_name, 0, 4); memset(_label, 0, 4); memset(_alert_label, 0, 4);
      strcpy( _name, "D0" );strcpy( _label, "d0" );strcpy( _alert_label, "al0" );

      bool _added_options = false;
      for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
        _name[1] = (0x30 + _pin );_label[1] = (0x30 + _pin );_alert_label[2] = (0x30 + _pin );

        if( !__gpio_service.is_exceptional_gpio_pin(_pin) && __gpio_service.m_gpio_config_copy.gpio_mode[_pin] == DIGITAL_READ ){

          _added_options = true;
					strcat_P( _page, HTML_TR_OPEN_TAG );
					strcat_P( _page, HTML_TAG_CLOSE_BRACKET );
					concat_td_select_html_tags( _page, _name, _label, _gpio_digital_alert_options, 2, (int)__gpio_service.m_gpio_config_copy.gpio_alert_values[_pin] );
					concat_td_select_html_tags( _page, (char*)" ? ", _alert_label, _gpio_alert_channels, 2, (int)__gpio_service.m_gpio_config_copy.gpio_alert_channel[_pin] );
				  strcat_P( _page, HTML_TR_CLOSE_TAG );
        }
      }

			if( __gpio_service.m_gpio_config_copy.gpio_mode[MAX_NO_OF_GPIO_PINS] == ANALOG_READ ){
				_added_options = true;
				memset( _analog_value, 0, 10 );
				__appendUintToBuff( _analog_value, "%d", __gpio_service.m_gpio_config_copy.gpio_alert_values[MAX_NO_OF_GPIO_PINS], 8 );
				strcat_P( _page, HTML_TR_OPEN_TAG );
				strcat_P( _page, HTML_TAG_CLOSE_BRACKET );
				strcat_P( _page, HTML_TD_OPEN_TAG );
				strcat_P( _page, HTML_TAG_CLOSE_BRACKET );
			  strcat( _page, (char*)"A0" );
			  strcat_P( _page, HTML_TD_CLOSE_TAG );
				strcat_P( _page, HTML_TD_OPEN_TAG );
				strcat_P( _page, HTML_STYLE_ATTR );
				strcat( _page, (char*)"'display:flex;'" );
				strcat_P( _page, HTML_TAG_CLOSE_BRACKET );
				concat_select_html_tag( _page, PSTR("a0"), _gpio_analog_alert_comparators, 3, (int)__gpio_service.m_gpio_config_copy.gpio_alert_comparator[MAX_NO_OF_GPIO_PINS] );
			  concat_input_html_tag( _page, PSTR("aval"), _analog_value );
			  strcat_P( _page, HTML_TD_CLOSE_TAG );
				concat_td_select_html_tags( _page, (char*)" ? ", (char*)"anlt0", _gpio_alert_channels, 2, (int)__gpio_service.m_gpio_config_copy.gpio_alert_channel[MAX_NO_OF_GPIO_PINS] );
				strcat_P( _page, HTML_TR_CLOSE_TAG );
			}

      if( _added_options ){
        strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      }else{
        strcat_P( _page, EW_SERVER_GPIO_ALERT_EMPTY_MESSAGE );
      }
      if( _enable_flash )
      concat_flash_message_div( _page, HTML_SUCCESS_FLASH, ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send gpio alert config page.
		 * when posted, get gpio alert configs from client and set them in database.
		 */
    void handleGpioAlertConfigRoute( void ) {
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Gpio Alert Config route"));
      #endif

			if( nullptr == this->m_web_resource ||
					nullptr == this->m_web_resource->m_db_conn ||
					nullptr == this->m_web_resource->m_server ){
				return;
			}

      bool _is_posted = false;

      if ( true ) {

        char _label[6]; memset(_label, 0, 6); strcpy( _label, "d0" );
				char _alert_label[6]; memset(_alert_label, 0, 6); strcpy( _alert_label, "al0" );
        for (uint8_t _pin = 0; _pin < MAX_NO_OF_GPIO_PINS; _pin++) {
          _label[1] = (0x30 + _pin ); _alert_label[2] = (0x30 + _pin );
          if( this->m_web_resource->m_server->hasArg(_label) && this->m_web_resource->m_server->hasArg(_alert_label) ){

						__gpio_service.m_gpio_config_copy.gpio_alert_comparator[_pin] = EQUAL;
						__gpio_service.m_gpio_config_copy.gpio_alert_values[_pin] = (int)this->m_web_resource->m_server->arg(_label).toInt();
						__gpio_service.m_gpio_config_copy.gpio_alert_channel[_pin] = (int)this->m_web_resource->m_server->arg(_alert_label).toInt();
            #ifdef EW_SERIAL_LOG
              Log(F("Pin ")); Log( _pin );
							Log(F(" : ")); Log( __gpio_service.m_gpio_config_copy.gpio_alert_values[_pin] );
							Log(F(" : ")); Logln( __gpio_service.m_gpio_config_copy.gpio_alert_channel[_pin] );
            #endif
            _is_posted = true;
          }
        }

				if( this->m_web_resource->m_server->hasArg("a0") && this->m_web_resource->m_server->hasArg("aval") && this->m_web_resource->m_server->hasArg("anlt0") ){

					__gpio_service.m_gpio_config_copy.gpio_alert_comparator[MAX_NO_OF_GPIO_PINS] = (int)this->m_web_resource->m_server->arg("a0").toInt();
					__gpio_service.m_gpio_config_copy.gpio_alert_values[MAX_NO_OF_GPIO_PINS] = (int)this->m_web_resource->m_server->arg("aval").toInt();
					__gpio_service.m_gpio_config_copy.gpio_alert_channel[MAX_NO_OF_GPIO_PINS] = (int)this->m_web_resource->m_server->arg("anlt0").toInt();
					#ifdef EW_SERIAL_LOG
						Log(F("Pin A0"));
						Log(F(" : ")); Log( __gpio_service.m_gpio_config_copy.gpio_alert_values[MAX_NO_OF_GPIO_PINS] );
						Log(F(" : ")); Logln( __gpio_service.m_gpio_config_copy.gpio_alert_channel[MAX_NO_OF_GPIO_PINS] );
					#endif
					_is_posted = true;
				}

        if( _is_posted )
        this->m_web_resource->m_db_conn->set_gpio_config_table( &__gpio_service.m_gpio_config_copy );
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_gpio_alert_config_html( _page, _is_posted );

      this->m_web_resource->m_server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
			if( _is_posted ){
				__gpio_service.handleGpioModes(GPIO_ALERT_CONFIG);
      }
    }

};

#endif
