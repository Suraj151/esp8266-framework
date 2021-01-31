/****************************** Gpio service **********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _GPIO_SERVICE_PROVIDER_H_
#define _GPIO_SERVICE_PROVIDER_H_


#include <service_provider/ServiceProvider.h>
#include <service_provider/HttpServiceProvider.h>
#include <service_provider/EmailServiceProvider.h>

#define GPIO_PAYLOAD_DATA_KEY   "data"
#define GPIO_PAYLOAD_MODE_KEY   "mode"
#define GPIO_PAYLOAD_VALUE_KEY  "val"

/**
 * GpioServiceProvider class
 */
class GpioServiceProvider : public ServiceProvider {

  public:

    /**
     * GpioServiceProvider constructor.
     */
    GpioServiceProvider();
    /**
		 * GpioServiceProvider destructor
		 */
    ~GpioServiceProvider();

    void begin( iWiFiInterface* _wifi, iWiFiClientInterface* _wifi_client );
    void enable_update_gpio_table_from_copy( void );
    void appendGpioJsonPayload( String& _payload );
    void applyGpioJsonPayload( char* _payload, uint16_t _payload_length );
    #ifdef ENABLE_EMAIL_SERVICE
    bool handleGpioEmailAlert( void );
    #endif
    void handleGpioOperations( void );
    void handleGpioModes( int _gpio_config_type=GPIO_MODE_CONFIG );
    uint8_t getGpioFromPinMap( uint8_t _pin );
    void handleGpioHttpRequest( void );
    #ifdef EW_SERIAL_LOG
    void printGpioConfigLogs( void );
    #endif
    bool is_exceptional_gpio_pin( uint8_t _pin );

    /**
		 * @var gpio_config_table m_gpio_config_copy
		 */
    gpio_config_table m_gpio_config_copy;
    /**
		 * @var	int|0 m_gpio_http_request_cb_id
		 */
    int               m_gpio_http_request_cb_id;
    /**
		 * @var	bool|true m_update_gpio_table_from_copy
		 */
    bool              m_update_gpio_table_from_copy;

  protected:

    /**
		 * @var	iWiFiClientInterface  *m_wifi_client
		 */
    iWiFiClientInterface  *m_wifi_client;
    iWiFiInterface        *m_wifi;
};

extern GpioServiceProvider __gpio_service;

#endif
