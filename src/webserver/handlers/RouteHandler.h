/****************************** Route Handler *********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_ROUTE_HANDLER_
#define _EW_SERVER_ROUTE_HANDLER_

#include <Esp.h>

#include <webserver/resources/WebResource.h>

#include <webserver/controllers/HomeController.h>
#include <webserver/controllers/DashboardController.h>
#include <webserver/controllers/OtaController.h>
#include <webserver/controllers/WiFiConfigController.h>
#include <webserver/controllers/LoginController.h>
#ifdef ENABLE_GPIO_CONFIG
#include <webserver/controllers/GPIOController.h>
#endif
#ifdef ENABLE_MQTT_CONFIG
#include <webserver/controllers/MQTTController.h>
#endif


/**
 * EwRouteHandler class
 */
class EwRouteHandler {

  protected:

    /**
		 * @var	EwWebResourceProvider*  _web_resource
		 */
    EwWebResourceProvider* _web_resource = new EwWebResourceProvider;

  private:

    /**
		 * @var	HomeController  _home_controller
		 */
    HomeController _home_controller;

    /**
		 * @var	DashboardController  _dashboard_controller
		 */
    DashboardController _dashboard_controller;

    /**
		 * @var	OtaController  _ota_controller
		 */
    OtaController _ota_controller;

    /**
		 * @var	WiFiConfigController  _wificonfig_controller
		 */
    WiFiConfigController _wificonfig_controller;

    /**
		 * @var	LoginController  _login_controller
		 */
    LoginController _login_controller;

    #ifdef ENABLE_GPIO_CONFIG
    /**
		 * @var	GpioController  _gpio_controller
		 */
    GpioController _gpio_controller;
    #endif

    #ifdef ENABLE_MQTT_CONFIG
    /**
		 * @var	MqttController  _mqtt_controller
		 */
    MqttController _mqtt_controller;
    #endif

  public:

    /**
		 * @friend class EwingsEsp8266Stack
		 */
    friend class EwingsEsp8266Stack;

    /**
		 * EwRouteHandler constructor
		 */
    EwRouteHandler(){
    }

    /**
		 * EwRouteHandler constructor
     *
     * @param ESP8266WebServer* server
     * @param EwingsDefaultDB*  db_handler
     * @param ESP8266WiFiClass* _wifi
		 */
    EwRouteHandler( ESP8266WebServer* server, EwingsDefaultDB* db_handler, ESP8266WiFiClass* _wifi ){
      this->handle(server,db_handler,_wifi);
    }

    /**
		 * EwRouteHandler destructor
		 */
    ~EwRouteHandler(){
      delete _web_resource;
    }

    /**
		 * collect resources for web services
     *
     * @param ESP8266WebServer* server
     * @param EwingsDefaultDB*  db_handler
     * @param ESP8266WiFiClass* _wifi
		 */
    void handle( ESP8266WebServer* server, EwingsDefaultDB* db_handler, ESP8266WiFiClass* _wifi ){

      this->_web_resource->collect_resource( server, db_handler, _wifi );
      this->handle(server);
    }

    /**
		 * register controllers to handle individual routes.
     * collect required headers from client.
     *
     * @param ESP8266WebServer* server
		 */
    void handle( ESP8266WebServer* server ){

      this->_home_controller.handle( _web_resource );
      this->_dashboard_controller.handle( _web_resource );
      this->_ota_controller.handle( _web_resource );
      this->_wificonfig_controller.handle( _web_resource );
      this->_login_controller.handle( _web_resource );
      #ifdef ENABLE_GPIO_CONFIG
      this->_gpio_controller.handle( _web_resource );
      #endif
      #ifdef ENABLE_MQTT_CONFIG
      this->_mqtt_controller.handle( _web_resource );
      #endif
      //here the list of headers to be recorded
      // const char * headerkeys[] = {"User-Agent", "Cookie"} ;
      const char * headerkeys[] = {"Cookie"} ;
      size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
      //ask Server to track these headers
      this->_web_resource->EwServer->collectHeaders(headerkeys, headerkeyssize);
    }

};

#endif
