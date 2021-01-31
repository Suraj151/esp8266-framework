/************************ i WiFi Client Interface ******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _I_WIFI_CLIENT_INTERFACE_H_
#define _I_WIFI_CLIENT_INTERFACE_H_

#include <Arduino.h>
#include <Print.h>
#include <IPAddress.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

/**
 * iWiFiClientInterface class
 */
class iWiFiClientInterface: public Print{

  public:

    /**
     * iWiFiClientInterface constructor.
     */
    iWiFiClientInterface(){}
    /**
     * iWiFiClientInterface destructor.
     */
    virtual ~iWiFiClientInterface(){}


    virtual uint8_t status() = 0;
    virtual int connect(IPAddress ip, uint16_t port) = 0;
    virtual int connect(const char *host, uint16_t port) = 0;
    virtual int connect(const String &host, uint16_t port) = 0;
    virtual size_t write(uint8_t byte) = 0;
    virtual size_t write(const uint8_t *buf, size_t size) = 0;
    virtual size_t write_P(PGM_P buf, size_t size) = 0;
    virtual size_t write(Stream& stream) = 0;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int read(uint8_t *buf, size_t size) = 0;
    virtual int peek() = 0;
    virtual size_t peekBytes(uint8_t *buffer, size_t length) = 0;
    virtual bool flush(uint32_t maxWaitMs = 0) = 0;
    virtual bool stop(uint32_t maxWaitMs = 0) = 0;
    virtual uint8_t connected() = 0;
    virtual void setTimeout(uint32_t timeout) = 0;

    virtual IPAddress remoteIP() = 0;
    virtual uint16_t  remotePort() = 0;
    virtual IPAddress localIP() = 0;
    virtual uint16_t  localPort() = 0;
    virtual void stopAll() = 0;
    virtual size_t availableForWrite() = 0;

    using Print::write;

    virtual WiFiClient* getWiFiClient() = 0;
};

#endif
