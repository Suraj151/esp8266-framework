/****************************** ping service **********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "PingServiceProvider.h"

volatile bool _host_resp = false;
const uint32_t _host = 0x08080808;
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

void PingServiceProvider::init_ping(){

  memset(&this->_opt, 0, sizeof(struct ping_option));
  this->_opt.count = 1;
  this->_opt.ip = _host;
  this->_opt.coarse_time = 0;
  // _opt.sent_function = NULL;
  // _opt.recv_function = NULL;
  // _opt.reverse = NULL;
  ping_regist_sent(&this->_opt, NULL);
  // ping_regist_recv(&this->_opt, reinterpret_cast<ping_recv_function>(&PingServiceProvider::ping_recv));
  ping_regist_recv(&this->_opt, ping_recv_cb);
}

bool PingServiceProvider::ping(){

  return ping_start(&this->_opt);
}

bool PingServiceProvider::isHostRespondingToPing(){

  return _host_resp;
}

PingServiceProvider __ping_service;
