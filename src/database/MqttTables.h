/****************************** MQTT Tables ***********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _MQTT_TABLES_H_
#define _MQTT_TABLES_H_

#include <config/Config.h>
#include "DatabaseTable.h"

/**
 * MqttGeneralTable class extends public DatabaseTable as its base/parent class
 */
class MqttGeneralTable : public DatabaseTable<mqtt_general_config_table> {

  public:

    /**
     * MqttGeneralTable constructor.
     */
    MqttGeneralTable(){
    }

    /**
     * MqttGeneralTable destructor.
     */
    ~MqttGeneralTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( MQTT_GENERAL_CONFIG_TABLE_ADDRESS, &_mqtt_general_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return mqtt_general_config_table
     */
    mqtt_general_config_table get(){
      return this->get_table(MQTT_GENERAL_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param mqtt_general_config_table* _table
     */
    void set( mqtt_general_config_table* _table ){
      this->set_table( _table, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
    }
};


/**
 * MqttLwtTable class extends public DatabaseTable as its base/parent class
 */
class MqttLwtTable : public DatabaseTable<mqtt_lwt_config_table> {

  public:

    /**
     * MqttLwtTable constructor.
     */
    MqttLwtTable(){
    }

    /**
     * MqttLwtTable destructor.
     */
    ~MqttLwtTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( MQTT_LWT_CONFIG_TABLE_ADDRESS, &_mqtt_lwt_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return mqtt_lwt_config_table
     */
    mqtt_lwt_config_table get(){
      return this->get_table(MQTT_LWT_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param mqtt_lwt_config_table* _table
     */
    void set( mqtt_lwt_config_table* _table ){
      this->set_table( _table, MQTT_LWT_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( MQTT_LWT_CONFIG_TABLE_ADDRESS );
    }
};


/**
 * MqttPubSubTable class extends public DatabaseTable as its base/parent class
 */
class MqttPubSubTable : public DatabaseTable<mqtt_pubsub_config_table> {

  public:

    /**
     * MqttPubSubTable constructor.
     */
    MqttPubSubTable(){
    }

    /**
     * MqttPubSubTable destructor.
     */
    ~MqttPubSubTable(){
    }

    /**
     * register tables to database
     */
    void boot(){
      this->register_table( MQTT_PUBSUB_CONFIG_TABLE_ADDRESS, &_mqtt_pubsub_config_defaults );
    }

    /**
     * get/fetch table from database.
     *
     * @return mqtt_pubsub_config_table
     */
    mqtt_pubsub_config_table get(){
      return this->get_table(MQTT_PUBSUB_CONFIG_TABLE_ADDRESS);
    }

    /**
     * set table in database.
     *
     * @param mqtt_pubsub_config_table* _table
     */
    void set( mqtt_pubsub_config_table* _table ){
      this->set_table( _table, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
    }

    /**
     * clear table in database.
     */
    void clear(){
      this->clear_table( MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
    }
};


extern MqttGeneralTable	__mqtt_general_table;
extern MqttLwtTable	__mqtt_lwt_table;
extern MqttPubSubTable	__mqtt_pubsub_table;

#endif
