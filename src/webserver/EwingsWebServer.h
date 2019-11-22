/******************************** web server **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EWINGS_WEB_SERVER_H_
#define _EWINGS_WEB_SERVER_H_

#include <webserver/controllers/HomeController.h>
#include <webserver/controllers/DashboardController.h>
#include <webserver/controllers/OtaController.h>
#include <webserver/controllers/WiFiConfigController.h>
#include <webserver/controllers/LoginController.h>
#ifdef ENABLE_GPIO_SERVICE
#include <webserver/controllers/GPIOController.h>
#endif
#ifdef ENABLE_MQTT_SERVICE
#include <webserver/controllers/MQTTController.h>
#endif
#ifdef ENABLE_EMAIL_SERVICE
#include <webserver/controllers/EmailConfigController.h>
#endif

/**
 * WebServer class
 */
class WebServer {

  public:

    /**
     * WebServer constructor.
     */
    WebServer(){

    }

    /**
     * WebServer destructor.
     */
    ~WebServer(){

    }

    void start_server( ESP8266WiFiClass* _wifi );
    void handle_clients( void );

  protected:
    /**
		 * @var	ESP8266WebServer  server
		 */
    ESP8266WebServer server;
    /**
		 * @var	ESP8266WiFiClass*|&WiFi wifi
		 */
    ESP8266WiFiClass* wifi;



  private:
    /**
		 * @var	HomeController  home_controller
		 */
    HomeController home_controller;

    /**
		 * @var	DashboardController  dashboard_controller
		 */
    DashboardController dashboard_controller;

    /**
		 * @var	OtaController  ota_controller
		 */
    OtaController ota_controller;

    /**
		 * @var	WiFiConfigController  wificonfig_controller
		 */
    WiFiConfigController wificonfig_controller;

    /**
		 * @var	LoginController  login_controller
		 */
    LoginController login_controller;

    #ifdef ENABLE_GPIO_SERVICE
    /**
		 * @var	GpioController  gpio_controller
		 */
    GpioController gpio_controller;
    #endif

    #ifdef ENABLE_MQTT_SERVICE
    /**
		 * @var	MqttController  mqtt_controller
		 */
    MqttController mqtt_controller;
    #endif
    #ifdef ENABLE_EMAIL_SERVICE
    /**
		 * @var	EmailConfigController  emailconfig_controller
		 */
    EmailConfigController emailconfig_controller;
    #endif

};

extern WebServer __web_server;

#endif
