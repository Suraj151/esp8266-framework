/*************************** Http Protocol service ****************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "HttpServiceProvider.h"

/**
 * check http response and give it retry if it not ok
 *
 * @param   int _httpCode
 * @return  bool
 */
bool HttpServiceProvider::followHttpRequest( int _httpCode ){

  #ifdef EW_SERIAL_LOG
  Logln( F("Following Http Request") );
  Log( F("Http Request Status Code : ") ); Logln( _httpCode );
  if ( _httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
    Log( F("Http Response : ") );
    Logln( this->client.getString() );
  }
  #endif
  this->client.end();

  if( _httpCode < 0 && this->retry > 0 ){
    this->retry--;
    #ifdef EW_SERIAL_LOG
    Logln( F("Http Request retrying...") );
    #endif
    return true;
  }
  this->retry = HTTP_REQUEST_RETRY;

  return false;
}

HttpServiceProvider __http_service;
