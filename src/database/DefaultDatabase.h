/*************************** Default Database *********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _DEFAULT_DATABASE_H_
#define _DEFAULT_DATABASE_H_

#include <config/Config.h>
#include "DatabaseTable.h"
#include "GlobalTable.h"
#include "LoginTable.h"
#include "WiFiTable.h"
#include "OtaTable.h"
#ifdef ENABLE_GPIO_SERVICE
#include "GpioTable.h"
#endif
#ifdef ENABLE_MQTT_SERVICE
#include "MqttTables.h"
#endif
#ifdef ENABLE_EMAIL_SERVICE
#include "EmailTable.h"
#endif
#ifdef ENABLE_DEVICE_IOT
#include "DeviceIotTable.h"
#endif


/**
 * DefaultDatabase class
 */
class DefaultDatabase {

  public:

    /**
     * DefaultDatabase constructor.
     */
    DefaultDatabase(){
    }

    /**
     * DefaultDatabase destructor.
     */
    ~DefaultDatabase(){
    }

    void init_default_database();
    void clear_default_tables();

    global_config_table get_global_config_table();
    login_credential_table get_login_credential_table();
    wifi_config_table get_wifi_config_table();
    ota_config_table get_ota_config_table();

    #ifdef ENABLE_GPIO_SERVICE
    gpio_config_table get_gpio_config_table();
    #endif

    #ifdef ENABLE_MQTT_SERVICE
    mqtt_general_config_table get_mqtt_general_config_table();
    mqtt_lwt_config_table get_mqtt_lwt_config_table();
    mqtt_pubsub_config_table get_mqtt_pubsub_config_table();
    #endif

    #ifdef ENABLE_EMAIL_SERVICE
    email_config_table get_email_config_table();
    #endif

    #ifdef ENABLE_DEVICE_IOT
    device_iot_config_table get_device_iot_config_table();
    #endif

    void set_global_config_table( global_config_table* _table );
    void set_login_credential_table( login_credential_table* _table );
    void set_wifi_config_table( wifi_config_table* _table );
    void set_ota_config_table( ota_config_table* _table );

    #ifdef ENABLE_GPIO_SERVICE
    void set_gpio_config_table( gpio_config_table* _table );
    #endif

    #ifdef ENABLE_MQTT_SERVICE
    void set_mqtt_general_config_table( mqtt_general_config_table* _table );
    void set_mqtt_lwt_config_table( mqtt_lwt_config_table* _table );
    void set_mqtt_pubsub_config_table( mqtt_pubsub_config_table* _table );
    #endif

    #ifdef ENABLE_EMAIL_SERVICE
    void set_email_config_table( email_config_table* _table );
    #endif

    #ifdef ENABLE_DEVICE_IOT
    void set_device_iot_config_table( device_iot_config_table* _table );
    #endif

};

extern DefaultDatabase __database_service;

#endif
