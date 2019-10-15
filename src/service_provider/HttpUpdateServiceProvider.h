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
  VERSION_JSON_ERROR,
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
       global_config_table _global_configs = this->ew_db->get_global_config_table();

       String _macstr = WiFi.macAddress();
       String _firmware_version_url = String(_ota_configs.ota_host);
       if( !_firmware_version_url.endsWith("/") ) _firmware_version_url += "/";
       _firmware_version_url += "ota?mac_id=";
       _firmware_version_url += _macstr;
       _firmware_version_url += "&version=";
       _firmware_version_url += _global_configs.firmware_version;

       if( _firmware_version_url.length() > 12 && _ota_configs.ota_port > 0 &&
         this->http_client->begin( *this->wifi_client, _firmware_version_url )
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

           String _response = this->http_client->getString();
           this->http_client->end();

           int _rsponse_len = (_response.length()+1) > OTA_VERSION_API_RESP_LENGTH ? OTA_VERSION_API_RESP_LENGTH : (_response.length()+1) ;
           char *_buf = new char[_rsponse_len]; memset( _buf, 0, _rsponse_len );
           char *_version_buf = new char[OTA_VERSION_LENGTH]; memset( _version_buf, 0, OTA_VERSION_LENGTH );
           _response.toCharArray( _buf, _rsponse_len );

           if( __get_from_json( _buf, (char*)OTA_VERSION_KEY, _version_buf, OTA_VERSION_LENGTH ) ){

             uint32_t _firm_version = StringToUint32(_version_buf);
             delete[] _buf; delete[] _version_buf;

             #ifdef EW_SERIAL_LOG
             Log( F("Http OTA current version : ") );
             Logln( _global_configs.firmware_version );
             Log( F("Http OTA got version : ") );
             Logln( _firm_version );
             #endif

             if( _firm_version > _global_configs.firmware_version ){

               String _firmware_bin_url = String(_ota_configs.ota_host);
               if( !_firmware_bin_url.endsWith("/") ) _firmware_bin_url += "/";
               _firmware_bin_url += "bin/";
               _firmware_bin_url += _macstr;
               _firmware_bin_url += "/";
               _firmware_bin_url += _firm_version;
               _firmware_bin_url += ".bin";

               ESPhttpUpdate.rebootOnUpdate(false);
               ESPhttpUpdate.followRedirects(true);
               t_httpUpdate_return ret = ESPhttpUpdate.update( *this->wifi_client, _firmware_bin_url );

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
                 _global_configs.firmware_version = _firm_version;
                 this->ew_db->set_global_config_table( &_global_configs);
                 return UPDATE_OK;
               }else{

                 #ifdef EW_SERIAL_LOG
                 Logln( F("HTTP_UPDATE_UNKNOWN_RETURN") );
                 #endif
                 return UNKNOWN;
               }

             }else{
               return ALREADY_LATEST_VERSION;
             }
           }else{
             delete[] _buf; delete[] _version_buf;
             return VERSION_JSON_ERROR;
           }
         }else{
           this->http_client->end();
           return VERSION_CHECK_FAILED;
         }
       }else{
         #ifdef EW_SERIAL_LOG
         Logln( F("Http OTA Update not initializing or failed or Not Configured Correctly") );
         #endif
         return CONFIG_ERROR;
       }
     }

};

#endif
