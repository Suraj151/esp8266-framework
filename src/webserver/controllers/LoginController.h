/****************************** Login Controller ******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_LOGIN_CONTROLLER_
#define _EW_SERVER_LOGIN_CONTROLLER_

#include "Controller.h"
#include <webserver/pages/LoginPage.h>
#include <webserver/pages/LogoutPage.h>
#include <webserver/pages/LoginConfigPage.h>

/**
 * LoginController class
 */
class LoginController : public Controller {

	protected:

		/**
		 * @var	login_credential_table	login_credentials
		 */
    login_credential_table login_credentials;

	public:

		/**
		 * LoginController constructor
		 */
		LoginController():Controller("login"){
		}

		/**
		 * LoginController destructor
		 */
		~LoginController(){
		}

		/**
		 * register logins controller
		 *
		 */
		void boot( void ){

			this->login_credentials = this->web_resource->db_conn->get_login_credential_table();
			this->route_handler->register_route( EW_SERVER_LOGIN_ROUTE, [&]() { this->handleLoginRoute(); } );
			this->route_handler->register_route( EW_SERVER_LOGOUT_ROUTE, [&]() { this->handleLogoutRoute(); } );
      this->route_handler->register_route( EW_SERVER_LOGIN_CONFIG_ROUTE, [&]() { this->handleLoginConfigRoute(); }, AUTH_MIDDLEWARE );
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
		 * build login config html.
		 *
		 * @param	char*	_page
		 * @param	bool|false	_is_error
		 * @param	bool|false	_enable_flash
		 * @param	int|EW_HTML_MAX_SIZE	_max_size
		 */
		void build_login_config_html( char* _page, bool _is_error=false, bool _enable_flash=false, int _max_size=EW_HTML_MAX_SIZE ){

      memset( _page, 0, _max_size );
      strcat_P( _page, EW_SERVER_HEADER_HTML );
      strcat_P( _page, EW_SERVER_LOGIN_CONFIG_PAGE_TOP );

      concat_tr_input_html_tags( _page, PSTR("Username:"), PSTR("usrnm"), this->login_credentials.username, LOGIN_CONFIGS_BUF_SIZE-1 );
      concat_tr_input_html_tags( _page, PSTR("Password:"), PSTR("pswd"), this->login_credentials.password, LOGIN_CONFIGS_BUF_SIZE-1, (char*)"password" );

      strcat_P( _page, EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM );
      if( _enable_flash )
      concat_flash_message_div( _page, _is_error ? PSTR("Invalid length error(3-20)"): HTML_SUCCESS_FLASH, _is_error ? ALERT_DANGER:ALERT_SUCCESS );
      strcat_P( _page, EW_SERVER_FOOTER_HTML );
    }

		/**
		 * build and send logind config page.
		 * when posted, get login configs from client and set them in database.
		 */
    void handleLoginConfigRoute( void ) {

			this->login_credentials = this->web_resource->db_conn->get_login_credential_table();
      #ifdef EW_SERIAL_LOG
      Logln(F("Handling Login Config route"));
      #endif
      bool _is_posted = false;
      bool _is_error = true;

      if ( this->web_resource->server->hasArg("usrnm") && this->web_resource->server->hasArg("pswd") ) {

        String _username = this->web_resource->server->arg("usrnm");
        String _password = this->web_resource->server->arg("pswd");

        #ifdef EW_SERIAL_LOG
          Logln(F("\nSubmitted info :\n"));
          Log(F("Username : ")); Logln( _username );
          Log(F("Password : ")); Logln( _password );
          Logln();
        #endif

        if( _username.length() <= LOGIN_CONFIGS_BUF_SIZE && _password.length() <= LOGIN_CONFIGS_BUF_SIZE &&
          _username.length() > MIN_ACCEPTED_ARG_SIZE && _password.length() > MIN_ACCEPTED_ARG_SIZE
        ){

          memset( this->login_credentials.username, 0, LOGIN_CONFIGS_BUF_SIZE );
          memset( this->login_credentials.password, 0, LOGIN_CONFIGS_BUF_SIZE );
          _username.toCharArray( this->login_credentials.username, _username.length()+1 );
          _password.toCharArray( this->login_credentials.password, _password.length()+1 );
          this->web_resource->db_conn->set_login_credential_table( &this->login_credentials );
          // this->set_login_credential_table( &this->login_credentials );

          _is_error = false;
        }
        _is_posted = true;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_login_config_html( _page, _is_error, _is_posted );

      if( _is_posted && !_is_error ){
        this->route_handler->send_inactive_session_headers();
      }
      this->web_resource->server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

		/**
		 * build and send logout html page. also inactive session for client
		 */
    void handleLogoutRoute( void ) {

      #ifdef EW_SERIAL_LOG
      Logln(F("Handling logout route"));
      #endif
      this->route_handler->send_inactive_session_headers();

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_LOGOUT_PAGE );

      this->web_resource->server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

		/**
		 * check login details and authenticate client to access all configs.
		 */
    void handleLoginRoute( void ) {

      bool _is_posted = false;

      if( this->web_resource->server->hasArg("username") && this->web_resource->server->hasArg("password") ){
        _is_posted = true;
      }

      if ( this->route_handler->has_active_session() || ( _is_posted && this->web_resource->server->arg("username") == this->login_credentials.username &&
        this->web_resource->server->arg("password") == this->login_credentials.password
      ) ) {

        char _session_cookie[EW_COOKIE_BUFF_MAX_SIZE];
        this->route_handler->build_session_cookie( _session_cookie, true, EW_COOKIE_BUFF_MAX_SIZE, true, this->login_credentials.cookie_max_age );

        this->web_resource->server->sendHeader("Location", EW_SERVER_HOME_ROUTE);
        this->web_resource->server->sendHeader("Cache-Control", "no-cache");
        this->web_resource->server->sendHeader("Set-Cookie", _session_cookie);
        this->web_resource->server->send(HTTP_REDIRECT);
        #ifdef EW_SERIAL_LOG
        Logln(F("Log in Successful"));
        #endif
        return;
      }

      char* _page = new char[EW_HTML_MAX_SIZE];
      this->build_html( _page, EW_SERVER_LOGIN_PAGE, _is_posted, (char*)"Wrong Credentials.", ALERT_DANGER );

      this->web_resource->server->send( HTTP_OK, EW_HTML_CONTENT, _page );
      delete[] _page;
    }

};

#endif
