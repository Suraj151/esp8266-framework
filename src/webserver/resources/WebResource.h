/****************************** Web Resource **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_WEB_RESOURCE_PROVIDER_
#define _EW_WEB_RESOURCE_PROVIDER_

#include <Esp.h>

#include <webserver/routes/Routes.h>
#include <webserver/pages/Header.h>
#include <webserver/pages/Footer.h>
#include <webserver/pages/NotFound.h>
#include <webserver/pages/UnauthorizedPage.h>

#include <webserver/handlers/RouteHandler.h>
#include <webserver/handlers/SessionHandler.h>
#include <webserver/helpers/DynamicPageBuildHelper.h>

static const char EW_HTML_CONTENT[] PROGMEM = "text/html";

/**
 * @define html page max size
 */
#define EW_HTML_MAX_SIZE 4000
#define MIN_ACCEPTED_ARG_SIZE 3

/**
 * @enum http return codes
 */
enum HTTP_RETURN_CODE {
  HTTP_OK = 200,
  HTTP_UNAUTHORIZED = 401,
  HTTP_NOT_FOUND = 404,
  HTTP_REDIRECT = 301
};

/**
 * @enum middlewares
 */
enum middlwares {
  AUTH_MIDDLEWARE,
  API_MIDDLEWARE,
  NO_MIDDLEWARE
};

/**
 * EwWebResourceProvider class
 * @parent  EwSessionHandler|protected
 */
class EwWebResourceProvider : protected EwSessionHandler{

  public:

    /**
		 * @friend all route controller classes should be friend of web resources
		 */
    friend class EwRouteHandler;
    friend class HomeController;
    friend class LoginController;
    #ifdef ENABLE_GPIO_CONFIG
    friend class GpioController;
    #endif
    #ifdef ENABLE_MQTT_CONFIG
    friend class MqttController;
    #endif
    friend class OtaController;
    friend class DashboardController;
    friend class WiFiConfigController;

    /**
		 * EwWebResourceProvider constructor
		 */
    EwWebResourceProvider( void ){
    }

    /**
		 * EwWebResourceProvider destructor
		 */
    ~EwWebResourceProvider(){
      this->EwServer = NULL;
      this->ew_db = NULL;
      this->wifi = NULL;
      _ClearObject( &this->login_credentials );
      _ClearObject( &this->wifi_configs );
    }

    /**
		 * collect resources for web services
     *
     * @param ESP8266WebServer* server
     * @param EwingsDefaultDB*  db_handler
     * @param ESP8266WiFiClass* _wifi
		 */
    void collect_resource( ESP8266WebServer* server, EwingsDefaultDB* db_handler, ESP8266WiFiClass* _wifi ){

      this->EwServer = server;
      this->ew_db = db_handler;
      this->wifi = _wifi;
      this->use_login_credential( db_handler->get_login_credential_table() );
      this->use_wifi_configs( db_handler->get_wifi_config_table() );
    }

    /**
		 * fetch wifi config table here
     *
     * @param wifi_config_table _wifi_configs
		 */
    void use_wifi_configs( wifi_config_table _wifi_configs ){
      this->wifi_configs = _wifi_configs;
    }

    /**
		 * register route, callbacks, middlware level etc.
     *
     * @param const char* _uri
     * @param CallBackVoidArgFn _fn
     * @param middlwares|NO_MIDDLEWARE _middleware_level
     * @param const char*|EW_SERVER_LOGIN_ROUTE _redirect_uri
		 */
    void register_route( const char* _uri, CallBackVoidArgFn _fn, middlwares _middleware_level=NO_MIDDLEWARE, const char* _redirect_uri=EW_SERVER_LOGIN_ROUTE ){
      this->EwServer->on( _uri, [=]() {
        if( this->handle_middleware(_middleware_level,_redirect_uri) )
        _fn();
      } );
    }

    /**
		 * register not found callback
     *
     * @param CallBackVoidArgFn _fn
		 */
    void register_not_found_fn( CallBackVoidArgFn _fn ){
      this->EwServer->onNotFound( _fn );
    }

    /**
		 * each request go through this middelware to check requests by parameters.
     * this method allow to redirect or unautherise any request based on conditions.
     *
     * @param   middlwares _middleware_level
     * @param   const char* _redirect_uri
     * @return  bool
		 */
    bool handle_middleware( middlwares _middleware_level, const char* _redirect_uri ){

      #ifdef EW_SERIAL_LOG
      Logln(F("checking through middleware"));
      #endif

      if ( _middleware_level == AUTH_MIDDLEWARE ) {

        if ( !this->has_active_session() ) {

          this->EwServer->sendHeader("Location", _redirect_uri);
          this->EwServer->sendHeader("Cache-Control", "no-cache");
          this->EwServer->send(HTTP_REDIRECT);
          return false;
        }
        return true;
      }else if ( _middleware_level == API_MIDDLEWARE ) {

        return true;

      }else{

        return true;
      }
    }

  protected:

    /**
		 * @var	EwingsDefaultDB*  ew_db
		 */
    EwingsDefaultDB* ew_db;
    /**
		 * @var	wifi_config_table  wifi_configs
		 */
    wifi_config_table wifi_configs;
    /**
		 * @var	ESP8266WiFiClass*  wifi
		 */
    ESP8266WiFiClass* wifi;

};

#endif
