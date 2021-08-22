/******************************** web server **********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_EWING_HTTP_SERVER)

#include "EwingsWebServer.h"

/**
 * EwWebServer constructor.
 */
EwWebServer::EwWebServer():
  m_wifi(nullptr),
  m_server(&__wifi_server_interface)
{
}

/**
 * EwWebServer destructor.
 */
EwWebServer::~EwWebServer(){
  this->m_wifi = nullptr;
}

/**
 * start http server functionality. this requires wifi should work as access point
 */
void EwWebServer::start_server( iWiFiInterface* _wifi ){

  this->m_wifi = _wifi;
  __web_resource.collect_resource( this->m_wifi, this->m_server );

  for (int i = 0; i < Controller::m_controllers.size(); i++) {
    #ifdef EW_SERIAL_LOG
    Log(F("booting :"));Log(Controller::m_controllers[i].controller->m_controller_name);Logln(F(" controller"));
    #endif
    Controller::m_controllers[i].controller->boot();
  }

  // here the list of headers to be recorded
  // const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  const char *headerkeys[] = {"Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  // ask server to track these headers
  this->m_server->collectHeaders(headerkeys, headerkeyssize);

  // start the server
  this->m_server->begin();
  #ifdef EW_SERIAL_LOG
  Logln(F("HTTP server started !"));
  #endif
}

/**
 * handle server functionality.
 */
void EwWebServer::handle_clients(){

  this->m_server->handleClient();
}

EwWebServer __web_server;

#endif
