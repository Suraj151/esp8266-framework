/****************************** Web Resource **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "WebResource.h"

/**
 * collect resources for web services
 *
 * @param ESP8266WebServer* _server
 * @param ESP8266WiFiClass* _wifi
 */
void WebResourceProvider::collect_resource( ESP8266WebServer* _server, ESP8266WiFiClass* _wifi ){

  this->server = _server;
  this->db_conn = &__database_service;
  this->wifi = _wifi;
}

WebResourceProvider __web_resource;
