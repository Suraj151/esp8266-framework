/***************************** Session Handler ********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_SESSION_HANDLER_
#define _EW_SERVER_SESSION_HANDLER_

/**
 * @define	max buf size for client cookies.
 */
EwWebResourceProvider* web_resource;
#define EW_SERIAL_LOG
#define EW_COOKIE_BUFF_MAX_SIZE 60

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <utility/Utility.h>
#include <utility/Log.h>
#include <database/EwingsDefaultDB.h>

/**
 * EwSessionHandler class
 */
class EwSessionHandler{

  public:

    /**
		 * EwSessionHandler constructor
		 */
    EwSessionHandler( void ){
    }

    /**
		 * EwSessionHandler constructor
     *
     * @param ESP8266WebServer* server
		 */
    EwSessionHandler( ESP8266WebServer* server ){
      this->begin(server );
    }

    /**
		 * EwSessionHandler destructor
		 */
    ~EwSessionHandler(){
      this->EwServer = NULL;
    }

    /**
		 * begin with web server
     *
     * @param ESP8266WebServer* server
		 */
    void begin( ESP8266WebServer* server ){
      this->EwServer = server;
    }

    /**
		 * assign login credential table to session handler
     *
     * @param login_credential_table _login_credentials
		 */
    void use_login_credential( login_credential_table _login_credentials ){
      this->login_credentials = _login_credentials;
    }

    /**
		 * send inactive session to client. this will remove any login cache from client side.
		 */
    void send_inactive_session_headers(){

      char _session_cookie[EW_COOKIE_BUFF_MAX_SIZE];
      this->build_session_cookie( _session_cookie, false, EW_COOKIE_BUFF_MAX_SIZE, true, 0 );
      this->EwServer->sendHeader("Cache-Control", "no-cache");
      this->EwServer->sendHeader("Set-Cookie", _session_cookie);
    }

    /**
		 * build session cookies for login user with defined max age.
		 *
		 * @param	char*	_str
		 * @param	bool  _stat
     * @param	int   _max_size
		 * @param	bool|false	_enable_max_age
		 * @param	uint32_t|EW_COOKIE_MAX_AGE  _max_age
		 */
    void build_session_cookie( char* _str, bool _stat, int _max_size, bool _enable_max_age=false, uint32_t _max_age=EW_COOKIE_MAX_AGE ){

      // login_credential_table _login_credentials = this->get_login_credential_table();
      memset( _str, 0, _max_size );
      strcat( _str, this->login_credentials.session_name );
      strcat( _str, _stat ? "=1": "=0" );
      if( _enable_max_age ){
        strcat( _str, ";Max-Age=" );
        __appendUintToBuff( _str,  "%08ld", _max_age, 8 );
      }
    }

    /**
		 * check whether client has active valid session
     *
     * @return  bool
		 */
    bool has_active_session( void ){

      #ifdef EW_SERIAL_LOG
      Logln(F("checking active session"));
      #endif

      if ( this->EwServer->hasHeader("Cookie") ) {

        String cookie = this->EwServer->header("Cookie");
        char _session_cookie[EW_COOKIE_BUFF_MAX_SIZE];
        this->build_session_cookie( _session_cookie, true, EW_COOKIE_BUFF_MAX_SIZE );

        #ifdef EW_SERIAL_LOG
        Log(F("Found cookie: "));
        Logln(cookie);
        #endif

        if ( cookie.indexOf(_session_cookie) != -1 ) {
          #ifdef EW_SERIAL_LOG
          Logln(F("active session found"));
          #endif
          return true;
        }
      }
      #ifdef EW_SERIAL_LOG
      Logln(F("active session not found"));
      #endif
      return false;
    }

    /**
		 * check whether client has inactive session
     *
     * @return  bool
		 */
    bool has_inactive_session( void ){

      #ifdef EW_SERIAL_LOG
      Logln(F("checking inactive session"));
      #endif

      if ( this->EwServer->hasHeader("Cookie") ) {
        String cookie = this->EwServer->header("Cookie");
        char _session_cookie[EW_COOKIE_BUFF_MAX_SIZE];
        this->build_session_cookie( _session_cookie, false, EW_COOKIE_BUFF_MAX_SIZE );

        #ifdef EW_SERIAL_LOG
        Log(F("Found cookie: "));
        Logln(cookie);
        #endif

        if ( cookie.indexOf(_session_cookie) != -1 ) {
          #ifdef EW_SERIAL_LOG
          Logln(F("inactive session found"));
          #endif
          return true;
        }
      }
      #ifdef EW_SERIAL_LOG
      Logln(F("inactive session not found"));
      #endif
      return false;
    }

  protected:

    /**
		 * @var	ESP8266WebServer*	EwServer
		 */
    ESP8266WebServer* EwServer;

    /**
		 * @var	login_credential_table	login_credentials
		 */
    login_credential_table login_credentials;

};

#endif
