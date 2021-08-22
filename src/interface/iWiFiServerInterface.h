/************************ i WiFi Server Interface ******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _I_WIFI_SERVER_INTERFACE_H_
#define _I_WIFI_SERVER_INTERFACE_H_

#include "typedef.h"
#include <ESP8266WebServer.h>

// enum HTTP_Method { HTTP_ANY, HTTP_GET, HTTP_HEAD, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };
// enum HTTP_Upload_Status { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END,
//                         UPLOAD_FILE_ABORTED };


typedef HTTPMethod HTTP_Method;
typedef std::function<void(void)> THandlerFunction;

/**
 * iWiFiServerInterface class
 */
class iWiFiServerInterface{

  public:

    /**
     * iWiFiServerInterface constructor.
     */
    iWiFiServerInterface(){}
    /**
     * iWiFiServerInterface destructor.
     */
    virtual ~iWiFiServerInterface(){}


    virtual void begin() = 0;
    virtual void begin(uint16_t port) = 0;
    virtual void handleClient() = 0;
    virtual void close() = 0;
    virtual void stop() = 0;

    virtual void on(const String &uri, THandlerFunction handler) = 0;
    virtual void on(const String &uri, HTTP_Method method, THandlerFunction fn) = 0;
    // virtual void on(const String &uri, HTTP_Method method, THandlerFunction fn, THandlerFunction ufn) = 0;
    virtual void onNotFound(THandlerFunction fn) = 0;  //called when handler is not assigned
    virtual void onFileUpload(THandlerFunction fn) = 0; //handle file uploads

    virtual String arg(const String& name) = 0;    // get request argument value by name
    virtual String arg(int i) = 0;          // get request argument value by number
    virtual String argName(int i) = 0;      // get request argument name by number
    virtual int args() const = 0;                        // get arguments count
    virtual bool hasArg(const String& name) const = 0;   // check if argument exists
    virtual void collectHeaders(const char* headerKeys[], const size_t headerKeysCount) = 0; // set the request headers to collect
    virtual String header(const String& name) = 0; // get request header value by name
    virtual String header(int i) = 0;       // get request header value by number
    virtual String headerName(int i) = 0;   // get request header name by number
    virtual int headers() const = 0;                     // get header count
    virtual bool hasHeader(const String& name) const = 0;       // check if header exists
    virtual String hostHeader() = 0;        // get request host header if available or empty String if not

    virtual void send(int code, const char* content_type = nullptr, const String& content = String("")) = 0;
    virtual void send(int code, char* content_type, const String& content) = 0;
    virtual void send(int code, const String& content_type, const String& content) = 0;
    virtual void send(int code, const char *content_type, const char *content) = 0;
    virtual void send(int code, const char *content_type, const char *content, size_t content_length) = 0;
    virtual void send(int code, const char *content_type, const uint8_t *content, size_t content_length) = 0;
    virtual void send_P(int code, PGM_P content_type, PGM_P content) = 0;
    virtual void send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength) = 0;
    virtual void sendHeader(const String& name, const String& value, bool first = false) = 0;
};

#endif
