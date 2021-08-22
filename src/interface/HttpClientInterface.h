/************************* HTTP Client Interface *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _HTTP_CLIENT_INTERFACE_H_
#define _HTTP_CLIENT_INTERFACE_H_

#include "iHttpClientInterface.h"
#include <ESP8266HTTPClient.h>

/**
 * HttpClientInterface class
 */
class HttpClientInterface : public iHttpClientInterface {

  public:

    /**
     * HttpClientInterface constructor.
     */
    HttpClientInterface();
    /**
     * HttpClientInterface destructor.
     */
    ~HttpClientInterface();

    bool begin(iWiFiClientInterface &client, const String &url);
    bool begin(iWiFiClientInterface &client, const String &host, uint16_t port, const String &uri = "/", bool https = false);
    void end(void);
    bool connected(void);

    void setReuse(bool reuse);
    void setUserAgent(const String &userAgent);
    void setAuthorization(const char *user, const char *password);
    void setAuthorization(const char *auth);
    void setTimeout(uint16_t timeout);
    void setFollowRedirects(follow_redirects follow);
    void setRedirectLimit(uint16_t limit);
    bool setURL(const String &url);

    int GET();
    int POST(const uint8_t *payload, size_t size);
    int POST(const String &payload);
    int PUT(const uint8_t *payload, size_t size);
    int PUT(const String &payload);
    int PATCH(const uint8_t *payload, size_t size);
    int PATCH(const String &payload);

    void addHeader(const String &name, const String &value, bool first = false, bool replace = true);
    void collectHeaders(const char *headerKeys[], const size_t headerKeysCount);
    String header(const char *name);
    String header(size_t i);
    String headerName(size_t i);
    int headers();
    bool hasHeader(const char *name);

    int getSize(void);
    const String& getLocation(void);
    WiFiClient& getStream(void);
    const String& getString(void);

  private:

    /**
		 * @var	HTTPClient
		 */
    HTTPClient  m_http_client;
};

extern HttpClientInterface __http_client_interface;
#endif
