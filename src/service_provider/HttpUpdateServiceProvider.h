/************** Over The Air firmware update service **************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _HTTP_OTA_SERVICE_PROVIDER_H_
#define _HTTP_OTA_SERVICE_PROVIDER_H_

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <database/EwingsDefaultDB.h>

/**
 * ota status enum
 */
enum http_ota_status{

  BEGIN_FAILED,
  CONFIG_ERROR,
  VERSION_CHECK_FAILED,
  ALREADY_LATEST_VERSION,
  UPDATE_FAILD,
  NO_UPDATES,
  UPDATE_OK,
  UNKNOWN
};

/**
 * HTTPUpdateServiceProvider class
 */
class HTTPUpdateServiceProvider{

  public:

    /**
     * @var	WiFiClient*|NULL	wifi_client
     */
    WiFiClient* wifi_client=NULL;

    /**
     * @var	HTTPClient*|NULL	http_client
     */
    HTTPClient* http_client=NULL;

    /**
     * @var	EwingsDefaultDB*|NULL	ew_db
     */
    EwingsDefaultDB* ew_db=NULL;

    /**
     * HTTPUpdateServiceProvider constructor.
     */
    HTTPUpdateServiceProvider(){
    }

    /**
     * HTTPUpdateServiceProvider destructor.
     */
    ~HTTPUpdateServiceProvider(){
    }

    /**
     * begin ota with client and database configs
     *
     * @param WiFiClient*	      _wifi_client
     * @param HTTPClient*	      _http_client
     * @param EwingsDefaultDB*	_database
     */
    void begin_ota( WiFiClient* _wifi_client, HTTPClient* _http_client, EwingsDefaultDB* _database ){

      this->wifi_client = _wifi_client;
      this->http_client = _http_client;
      this->ew_db = _database;
    }

    /**
     * handle ota by cheching firmware version first and update firmware if
     * latest version is available
     *
     * @return  http_ota_status
     */
    http_ota_status handle_ota(){

      if( !this->wifi_client || !this->http_client || !this->ew_db ) return BEGIN_FAILED;

      ota_config_table _ota_configs = this->ew_db->get_ota_config_table();

      int url_len = OTA_HOST_BUF_SIZE+20;
      char* _firm_ver_url = new char[url_len]; memset( _firm_ver_url, 0, url_len );
      char* _firm_bin_url = new char[url_len]; memset( _firm_bin_url, 0, url_len );

      String _firmware_version_url = String(_ota_configs.ota_host);
      if( !_firmware_version_url.endsWith("/") ) _firmware_version_url += "/";
      _firmware_version_url += "version/";
      _firmware_version_url += _ota_configs.firmware_version;
      _firmware_version_url.toCharArray( _firm_ver_url, _firmware_version_url.length()+1 );

      if( _firmware_version_url.length() > 12 && _ota_configs.ota_port > 0 &&
        this->http_client->begin( *this->wifi_client, _firm_ver_url )
      ){

        this->http_client->setUserAgent("Ewings");
        this->http_client->setAuthorization("ota", "firmware");
        this->http_client->setTimeout(2000);

        int _httpCode = this->http_client->GET();

        #ifdef EW_SERIAL_LOG
        Log( F("Http OTA version url Response code : ") );
        Logln( _httpCode );
        #endif
        if ( _httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY ) {

          uint32_t _firm_version = this->http_client->getString().toInt();
          this->http_client->end();
          #ifdef EW_SERIAL_LOG
          Log( F("Http OTA current version : ") );
          Logln( _ota_configs.firmware_version );
          Log( F("Http OTA got version : ") );
          Logln( _firm_version );
          #endif
          if( _firm_version > _ota_configs.firmware_version ){

            String _firmware_bin_url = String(_ota_configs.ota_host);
            if( !_firmware_bin_url.endsWith("/") ) _firmware_bin_url += "/";
            _firmware_bin_url += "bin/";
            _firmware_bin_url += "firmware.bin";
            _firmware_bin_url.toCharArray( _firm_bin_url, _firmware_bin_url.length()+1 );

            ESPhttpUpdate.rebootOnUpdate(false);
            t_httpUpdate_return ret = ESPhttpUpdate.update( *this->wifi_client, _firm_bin_url );
            // t_httpUpdate_return ret = ESPhttpUpdate.update(client, "server", 80, "file.bin");

            delete[] _firm_ver_url; delete[] _firm_bin_url;

            if( ret == HTTP_UPDATE_FAILED ){

              #ifdef EW_SERIAL_LOG
              Logln( F("HTTP_UPDATE_FAILD") );
              #endif
              return UPDATE_FAILD;
            }else if( ret == HTTP_UPDATE_NO_UPDATES ){

              #ifdef EW_SERIAL_LOG
              Logln( F("HTTP_UPDATE_NO_UPDATES") );
              #endif
              return NO_UPDATES;
            }else if( ret == HTTP_UPDATE_OK ){

              #ifdef EW_SERIAL_LOG
              Logln( F("HTTP_UPDATE_OK") );
              #endif
              _ota_configs.firmware_version = _firm_version;
              this->ew_db->set_ota_config_table( &_ota_configs);
              return UPDATE_OK;
            }else{

              #ifdef EW_SERIAL_LOG
              Logln( F("HTTP_UPDATE_UNKNOWN_RETURN") );
              #endif
              return UNKNOWN;
            }

          }else{
            delete[] _firm_ver_url; delete[] _firm_bin_url;
            return ALREADY_LATEST_VERSION;
          }
        }else{
          delete[] _firm_ver_url; delete[] _firm_bin_url;
          this->http_client->end();
          return VERSION_CHECK_FAILED;
        }
      }else{
        #ifdef EW_SERIAL_LOG
        Logln( F("Http OTA Update not initializing or failed or Not Configured Correctly") );
        #endif
        delete[] _firm_ver_url; delete[] _firm_bin_url;
        return CONFIG_ERROR;
      }
    }

};

#endif
