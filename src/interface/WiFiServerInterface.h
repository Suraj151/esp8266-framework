/************************* WiFi Server Interface *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _WIFI_SERVER_INTERFACE_H_
#define _WIFI_SERVER_INTERFACE_H_

#include "iWiFiServerInterface.h"
#include <ESP8266WebServer.h>

/**
 * WiFiServerInterface class
 */
class WiFiServerInterface : public iWiFiServerInterface {

  public:

    /**
     * WiFiServerInterface constructor.
     */
    WiFiServerInterface();
    /**
     * WiFiServerInterface destructor.
     */
    ~WiFiServerInterface();


    void begin();
    void begin(uint16_t port);
    void handleClient();
    void close();
    void stop();

    void on(const String &uri, THandlerFunction handler);
    void on(const String &uri, HTTP_Method method, THandlerFunction fn);
    //void on(const String &uri, HTTP_Method method, THandlerFunction fn, THandlerFunction ufn);
    void onNotFound(THandlerFunction fn);  //called when handler is not assigned
    void onFileUpload(THandlerFunction fn); //handle file uploads

    String arg(const String& name);    // get request argument value by name
    String arg(int i);          // get request argument value by number
    String argName(int i);      // get request argument name by number
    int args() const;                        // get arguments count
    bool hasArg(const String& name) const;   // check if argument exists
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount); // set the request headers to collect
    String header(const String& name); // get request header value by name
    String header(int i);       // get request header value by number
    String headerName(int i);   // get request header name by number
    int headers() const;                     // get header count
    bool hasHeader(const String& name) const;       // check if header exists
    String hostHeader();        // get request host header if available or empty String if not

    void send(int code, const char* content_type = nullptr, const String& content = String(""));
    void send(int code, char* content_type, const String& content);
    void send(int code, const String& content_type, const String& content);
    void send(int code, const char *content_type, const char *content);
    void send(int code, const char *content_type, const char *content, size_t content_length);
    void send(int code, const char *content_type, const uint8_t *content, size_t content_length);
    void send_P(int code, PGM_P content_type, PGM_P content);
    void send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength);
    void sendHeader(const String& name, const String& value, bool first = false);

  private:

    /**
		 * @var	ESP8266WebServer  m_server
		 */
    ESP8266WebServer  m_server;
};

extern WiFiServerInterface __wifi_server_interface;
#endif
