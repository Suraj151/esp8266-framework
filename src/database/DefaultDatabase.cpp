/*************************** Default Database *********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "DefaultDatabase.h"

/**
 * initialize default database with size as input parameter.
 * defaults to eeprom max size as defined.
 *
 * @param uint16_t|EEPROM_MAX  _size
 */
void DefaultDatabase::init_default_database(  uint16_t _size ){

  this->init_database(_size);
  this->register_table( &_global_config_defaults, GLOBAL_CONFIG_TABLE_ADDRESS );
  this->register_table( &_login_credential_defaults, LOGIN_CREDENTIAL_TABLE_ADDRESS );
  this->register_table( &_wifi_config_defaults, WIFI_CONFIG_TABLE_ADDRESS );
  this->register_table( &_ota_config_defaults, OTA_CONFIG_TABLE_ADDRESS );
  #ifdef ENABLE_GPIO_SERVICE
  this->register_table( &_gpio_config_defaults, GPIO_CONFIG_TABLE_ADDRESS );
  #endif
  #ifdef ENABLE_MQTT_SERVICE
  this->register_table( &_mqtt_general_config_defaults, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
  this->register_table( &_mqtt_lwt_config_defaults, MQTT_LWT_CONFIG_TABLE_ADDRESS );
  this->register_table( &_mqtt_pubsub_config_defaults, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
  #endif
  #ifdef ENABLE_EMAIL_SERVICE
  this->register_table( &_email_config_defaults, EMAIL_CONFIG_TABLE_ADDRESS );
  #endif
}

/**
 * clear all tables to their defaults value.
 */
void DefaultDatabase::clear_default_tables(){

  this->clear_table_to_defaults( &_global_config_defaults, GLOBAL_CONFIG_TABLE_ADDRESS );
  this->clear_table_to_defaults( &_login_credential_defaults, LOGIN_CREDENTIAL_TABLE_ADDRESS );
  this->clear_table_to_defaults( &_wifi_config_defaults, WIFI_CONFIG_TABLE_ADDRESS );
  this->clear_table_to_defaults( &_ota_config_defaults, OTA_CONFIG_TABLE_ADDRESS );
  #ifdef ENABLE_GPIO_SERVICE
  this->clear_table_to_defaults( &_gpio_config_defaults, GPIO_CONFIG_TABLE_ADDRESS );
  #endif
  #ifdef ENABLE_MQTT_SERVICE
  this->clear_table_to_defaults( &_mqtt_general_config_defaults, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
  this->clear_table_to_defaults( &_mqtt_lwt_config_defaults, MQTT_LWT_CONFIG_TABLE_ADDRESS );
  this->clear_table_to_defaults( &_mqtt_pubsub_config_defaults, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
  #endif
  #ifdef ENABLE_EMAIL_SERVICE
  this->clear_table_to_defaults( &_email_config_defaults, EMAIL_CONFIG_TABLE_ADDRESS );
  #endif
}

/**
 * get/fetch global config table from database.
 *
 * @return global_config_table
 */
global_config_table DefaultDatabase::get_global_config_table(){
  return this->get_table_by_address<global_config_table>(GLOBAL_CONFIG_TABLE_ADDRESS);
}

/**
 * get/fetch login credential table from database.
 *
 * @return login_credential_table
 */
login_credential_table DefaultDatabase::get_login_credential_table(){
  return this->get_table_by_address<login_credential_table>(LOGIN_CREDENTIAL_TABLE_ADDRESS);
}

/**
 * get/fetch wifi config table from database.
 *
 * @return wifi_config_table
 */
wifi_config_table DefaultDatabase::get_wifi_config_table(){
  return this->get_table_by_address<wifi_config_table>(WIFI_CONFIG_TABLE_ADDRESS);
}

/**
 * get/fetch ota(over the air update) config table from database.
 *
 * @return ota_config_table
 */
ota_config_table DefaultDatabase::get_ota_config_table(){
  return this->get_table_by_address<ota_config_table>(OTA_CONFIG_TABLE_ADDRESS);
}

#ifdef ENABLE_GPIO_SERVICE
/**
 * get/fetch gpio config table from database.
 *
 * @return gpio_config_table
 */
gpio_config_table DefaultDatabase::get_gpio_config_table(){
  return this->get_table_by_address<gpio_config_table>(GPIO_CONFIG_TABLE_ADDRESS);
}
#endif

#ifdef ENABLE_MQTT_SERVICE
/**
 * get/fetch mqtt general config table from database.
 *
 * @return mqtt_general_config_table
 */
mqtt_general_config_table DefaultDatabase::get_mqtt_general_config_table(){
  return this->get_table_by_address<mqtt_general_config_table>(MQTT_GENERAL_CONFIG_TABLE_ADDRESS);
}

/**
 * get/fetch mqtt lwt config table from database.
 *
 * @return mqtt_lwt_config_table
 */
mqtt_lwt_config_table DefaultDatabase::get_mqtt_lwt_config_table(){
  return this->get_table_by_address<mqtt_lwt_config_table>(MQTT_LWT_CONFIG_TABLE_ADDRESS);
}

/**
 * get/fetch mqtt pubsub config table from database.
 *
 * @return mqtt_pubsub_config_table
 */
mqtt_pubsub_config_table DefaultDatabase::get_mqtt_pubsub_config_table(){
  return this->get_table_by_address<mqtt_pubsub_config_table>(MQTT_PUBSUB_CONFIG_TABLE_ADDRESS);
}
#endif

#ifdef ENABLE_EMAIL_SERVICE
/**
 * get/fetch email config table from database.
 *
 * @return email_config_table
 */
email_config_table DefaultDatabase::get_email_config_table(){
  return this->get_table_by_address<email_config_table>(EMAIL_CONFIG_TABLE_ADDRESS);
}
#endif

/**
 * set global config table in database.
 *
 * @param global_config_table* _table
 */
void DefaultDatabase::set_global_config_table( global_config_table* _table ){
  this->set_table( _table, GLOBAL_CONFIG_TABLE_ADDRESS );
}

/**
 * set login credential config table in database.
 *
 * @param login_credential_table* _table
 */
void DefaultDatabase::set_login_credential_table( login_credential_table* _table ){
  this->set_table( _table, LOGIN_CREDENTIAL_TABLE_ADDRESS );
}

/**
 * set wifi config table in database.
 *
 * @param wifi_config_table* _table
 */
void DefaultDatabase::set_wifi_config_table( wifi_config_table* _table ){
  this->set_table( _table, WIFI_CONFIG_TABLE_ADDRESS );
}

/**
 * set ota(over the air update) config table in database.
 *
 * @param ota_config_table* _table
 */
void DefaultDatabase::set_ota_config_table( ota_config_table* _table ){
  this->set_table( _table, OTA_CONFIG_TABLE_ADDRESS );
}

#ifdef ENABLE_GPIO_SERVICE
/**
 * set gpio config table in database.
 *
 * @param gpio_config_table* _table
 */
void DefaultDatabase::set_gpio_config_table( gpio_config_table* _table ){
  this->set_table( _table, GPIO_CONFIG_TABLE_ADDRESS );
}
#endif

#ifdef ENABLE_MQTT_SERVICE
/**
 * set mqtt general config table in database.
 *
 * @param mqtt_general_config_table* _table
 */
void DefaultDatabase::set_mqtt_general_config_table( mqtt_general_config_table* _table ){
  this->set_table( _table, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
}

/**
 * set mqtt lwt config table in database.
 *
 * @param mqtt_lwt_config_table* _table
 */
void DefaultDatabase::set_mqtt_lwt_config_table( mqtt_lwt_config_table* _table ){
  this->set_table( _table, MQTT_LWT_CONFIG_TABLE_ADDRESS );
}

/**
 * set mqtt pubsub config table in database.
 *
 * @param mqtt_pubsub_config_table* _table
 */
void DefaultDatabase::set_mqtt_pubsub_config_table( mqtt_pubsub_config_table* _table ){
  this->set_table( _table, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
}
#endif

#ifdef ENABLE_EMAIL_SERVICE
/**
 * set email config table in database.
 *
 * @param email_config_table* _table
 */
void DefaultDatabase::set_email_config_table( email_config_table* _table ){
  this->set_table( _table, EMAIL_CONFIG_TABLE_ADDRESS );
}
#endif

/**
 * template to return different type of tables in database by their
 * address.
 *
 * @param   uint16_t  _address
 * @return  type of database table
 */
template <typename T> T DefaultDatabase::get_table_by_address( uint16_t _address ) {

  T _t;

  switch (_address) {
    case GLOBAL_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, GLOBAL_CONFIG_TABLE_ADDRESS );
      break;
    }
    case LOGIN_CREDENTIAL_TABLE_ADDRESS:{

      this->get_table( &_t, LOGIN_CREDENTIAL_TABLE_ADDRESS );
      break;
    }
    case WIFI_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, WIFI_CONFIG_TABLE_ADDRESS );
      break;
    }
    case OTA_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, OTA_CONFIG_TABLE_ADDRESS );
      break;
    }
    #ifdef ENABLE_GPIO_SERVICE
    case GPIO_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, GPIO_CONFIG_TABLE_ADDRESS );
      break;
    }
    #endif
    #ifdef ENABLE_MQTT_SERVICE
    case MQTT_GENERAL_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
      break;
    }
    case MQTT_LWT_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, MQTT_LWT_CONFIG_TABLE_ADDRESS );
      break;
    }
    case MQTT_PUBSUB_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
      break;
    }
    #endif
    #ifdef ENABLE_EMAIL_SERVICE
    case EMAIL_CONFIG_TABLE_ADDRESS:{

      this->get_table( &_t, EMAIL_CONFIG_TABLE_ADDRESS );
      break;
    }
    #endif
    default: {
      // _t = NULL;
      break;
    }
  }
  return _t;
}

DefaultDatabase __database_service;
