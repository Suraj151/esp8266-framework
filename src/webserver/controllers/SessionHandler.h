#ifndef _EW_SERVER_SESSION_HANDLER_
#define _EW_SERVER_SESSION_HANDLER_

#define EW_SERIAL_LOG

#define EW_COOKIE_BUFF_MAX_SIZE 60

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <utility/Utility.h>
#include <utility/Log.h>
#include <database/EwingsDefaultDB.h>

class EwSessionHandler{

  public:

    EwSessionHandler( void ){
    }

    EwSessionHandler( ESP8266WebServer* server ){
      this->begin(server );
    }

    ~EwSessionHandler(){
      this->EwServer = NULL;
    }

    void begin( ESP8266WebServer* server ){
      this->EwServer = server;
    }

    void use_login_credential( login_credential_table _login_credentials ){
      this->login_credentials = _login_credentials;
    }

    void send_inactive_session_headers(){

      char _session_cookie[EW_COOKIE_BUFF_MAX_SIZE];
      this->build_session_cookie( _session_cookie, false, EW_COOKIE_BUFF_MAX_SIZE, true, 0 );
      this->EwServer->sendHeader("Cache-Control", "no-cache");
      this->EwServer->sendHeader("Set-Cookie", _session_cookie);
    }

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

    ESP8266WebServer* EwServer;
    login_credential_table login_credentials;

};

#endif
