/*************************** Default Database *********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _DEFAULT_DATABASE_H_
#define _DEFAULT_DATABASE_H_

#include <config/Config.h>
#include "EepromDatabase.h"

/**
 * DefaultDatabase class extends public EepromDatabase as its base/parent class
 */
class DefaultDatabase : public EepromDatabase {

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

    void init_default_database(  uint16_t _size = EEPROM_MAX );
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

    template <typename T> T get_table_by_address( uint16_t _address );

};

extern DefaultDatabase __database_service;

#endif
