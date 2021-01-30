/****************************** Web Resource **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _WEB_RESOURCE_PROVIDER_
#define _WEB_RESOURCE_PROVIDER_

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <utility/Utility.h>
#include <database/DefaultDatabase.h>
#include <webserver/routes/Routes.h>
#include <webserver/pages/Header.h>
#include <webserver/pages/Footer.h>
#include <webserver/pages/NotFound.h>
#include <webserver/helpers/DynamicPageBuildHelper.h>

static const char EW_HTML_CONTENT[] PROGMEM = "text/html";

/**
 * @define html page max size
 */
#define EW_HTML_MAX_SIZE 5000
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
 * WebResourceProvider class
 */
class WebResourceProvider {

  public:

    /**
		 * WebResourceProvider constructor
		 */
    WebResourceProvider();
    /**
		 * WebResourceProvider destructor
		 */
    ~WebResourceProvider();

    void collect_resource( ESP8266WebServer* _server, ESP8266WiFiClass* _wifi );

  // protected:

    /**
		 * @var	ESP8266WiFiClass*  m_wifi
		 */
    ESP8266WiFiClass    *m_wifi;
    /**
		 * @var	ESP8266WebServer*	m_server
		 */
    ESP8266WebServer    *m_server;
    /**
		 * @var	DefaultDatabase*  m_db_conn
		 */
    DefaultDatabase     *m_db_conn;

};

extern WebResourceProvider __web_resource;

#endif
