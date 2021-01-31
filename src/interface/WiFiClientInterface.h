/************************** WiFi Client Interface ******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _WIFI_CLIENT_INTERFACE_H_
#define _WIFI_CLIENT_INTERFACE_H_

#include "iWiFiClientInterface.h"

/**
 * WiFiClientInterface class
 */
class WiFiClientInterface : public iWiFiClientInterface {

  public:

    /**
     * WiFiClientInterface constructor.
     */
    WiFiClientInterface();
    /**
     * WiFiClientInterface destructor.
     */
    ~WiFiClientInterface();


    uint8_t status();
    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);
    int connect(const String &host, uint16_t port);
    size_t write(uint8_t byte);
    size_t write(const uint8_t *buf, size_t size);
    size_t write_P(PGM_P buf, size_t size);
    size_t write(Stream& stream);
    int available();
    int read();
    int read(uint8_t *buf, size_t size);
    int peek();
    size_t peekBytes(uint8_t *buffer, size_t length);
    bool flush(uint32_t maxWaitMs = 0);
    bool stop(uint32_t maxWaitMs = 0);
    uint8_t connected();
    void setTimeout(uint32_t timeout);

    IPAddress remoteIP();
    uint16_t  remotePort();
    IPAddress localIP();
    uint16_t  localPort();
    size_t availableForWrite();
    void stopAll();

    WiFiClient* getWiFiClient(){ return &this->m_wifi_client; }

  private:

    /**
     * @var	WiFiClient  m_wifi_client
     */
    WiFiClient        m_wifi_client;
};

extern WiFiClientInterface __wifi_client_interface;
#endif
