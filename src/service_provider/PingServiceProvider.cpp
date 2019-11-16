/****************************** ping service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "PingServiceProvider.h"

volatile bool _host_resp = false;
const uint32_t _pinghostip = 0x08080808;
static const char _pinghostname[] = "google.com";
// IPAddress PING_TARGET(8,8,8,8);

// This function is called when a ping is received or the request times out:
static void ICACHE_FLASH_ATTR ping_recv_cb (void* arg, void *pdata){

  struct ping_resp *pingrsp = (struct ping_resp *)pdata;

  if (pingrsp->bytes > 0) {
    #ifdef EW_SERIAL_LOG
    Plain_Log(F("\nPing: Reply "));
    // Plain_Log(PING_TARGET);
    Plain_Log(F(": "));
    Plain_Log(F("bytes="));
    Plain_Log(pingrsp->bytes);
    Plain_Log(F(" time="));
    Plain_Log(pingrsp->resp_time);
    Plain_Logln(F("ms"));
    #endif
    _host_resp = true;
  } else {
    #ifdef EW_SERIAL_LOG
    Plain_Logln(F("\nPing: Request timed out"));
    #endif
    _host_resp = false;
  }
}

void PingServiceProvider::init_ping( ESP8266WiFiClass* _wifi ){

  this->wifi = _wifi;
  memset(&this->_opt, 0, sizeof(struct ping_option));
  this->_opt.count = 1;
  this->_opt.ip = _pinghostip;
  this->_opt.coarse_time = 0;
  // _opt.sent_function = NULL;
  // _opt.recv_function = NULL;
  // _opt.reverse = NULL;
  ping_regist_sent(&this->_opt, NULL);
  // ping_regist_recv(&this->_opt, reinterpret_cast<ping_recv_function>(&PingServiceProvider::ping_recv));
  ping_regist_recv(&this->_opt, ping_recv_cb);
}

bool PingServiceProvider::ping(){

  IPAddress _ip;
  this->wifi->hostByName(_pinghostname, _ip, 1500);
  this->_opt.ip = (uint32_t)_ip;
  #ifdef EW_SERIAL_LOG
  Log(F("\nPing ip: "));
  Logln(_ip);
  #endif
  return _ip.isSet() ? ping_start(&this->_opt) : false;
}

bool PingServiceProvider::isHostRespondingToPing(){

  return _host_resp;
}

PingServiceProvider __ping_service;
