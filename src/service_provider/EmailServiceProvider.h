/******************************* Email service *********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EMAIL_SERVICE_PROVIDER_H_
#define _EMAIL_SERVICE_PROVIDER_H_


#include <service_provider/ServiceProvider.h>
#ifdef ENABLE_GPIO_SERVICE
#include <service_provider/GpioServiceProvider.h>
#endif
#include <helpers/SMTPDriver.h>


/**
 * EmailServiceProvider class
 */
class EmailServiceProvider : public ServiceProvider {

  public:

    /**
     * EmailServiceProvider constructor.
     */
    EmailServiceProvider(){

      this->wifi_client = NULL;
      this->wifi = NULL;
    }

    /**
		 * EmailServiceProvider destructor
		 */
    ~EmailServiceProvider(){

      this->wifi_client = NULL;
      this->wifi = NULL;
      this->smtp.end();
    }

    /**
		 * @var	int|0 _mail_handler_cb_id
		 */
    int _mail_handler_cb_id=0;

    void begin( ESP8266WiFiClass* _wifi, WiFiClient* _wifi_client );
    bool sendMail( String mail_body );
    bool sendMail( char* mail_body );
    bool sendMail( PGM_P mail_body );
    // template <typename T> bool sendMail( T mail_body );
    void handleEmail( void );

    #ifdef EW_SERIAL_LOG
    void printEmailConfigLogs( void );
    #endif

  protected:

    WiFiClient* wifi_client;
    ESP8266WiFiClass* wifi;
    SMTPdriver smtp;
};

extern EmailServiceProvider __email_service;

#endif
