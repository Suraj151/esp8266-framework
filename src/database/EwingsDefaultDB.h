#ifndef _EWINGS_DEFAULT_DATABASE_H_
#define _EWINGS_DEFAULT_DATABASE_H_

#define EEPROM_MAX      4096

#include "StoreStruct.h"
#include "EwingsEepromDB.h"

class EwingsDefaultDB : public EwingsEepromDB {

  public:

    EwingsDefaultDB(){
    }

    ~EwingsDefaultDB(){
    }

    void init_default_database(  uint16_t _size = EEPROM_MAX ){

      this->init_database(_size);
      this->register_table( &_global_config_defaults, GLOBAL_CONFIG_TABLE_ADDRESS );
      this->register_table( &_login_credential_defaults, LOGIN_CREDENTIAL_TABLE_ADDRESS );
      this->register_table( &_wifi_config_defaults, WIFI_CONFIG_TABLE_ADDRESS );
      this->register_table( &_ota_config_defaults, OTA_CONFIG_TABLE_ADDRESS );
      #ifdef ENABLE_GPIO_CONFIG
      this->register_table( &_gpio_config_defaults, GPIO_CONFIG_TABLE_ADDRESS );
      #endif
      #ifdef ENABLE_MQTT_CONFIG
      this->register_table( &_mqtt_general_config_defaults, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
      this->register_table( &_mqtt_lwt_config_defaults, MQTT_LWT_CONFIG_TABLE_ADDRESS );
      this->register_table( &_mqtt_pubsub_config_defaults, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
      #endif
    }

    EwingsDefaultDB* EwingsDefaultDatabase( void ){
      return this;
    }

    void clear_default_tables(){

      this->clear_table_to_defaults( &_global_config_defaults, GLOBAL_CONFIG_TABLE_ADDRESS );
      this->clear_table_to_defaults( &_login_credential_defaults, LOGIN_CREDENTIAL_TABLE_ADDRESS );
      this->clear_table_to_defaults( &_wifi_config_defaults, WIFI_CONFIG_TABLE_ADDRESS );
      this->clear_table_to_defaults( &_ota_config_defaults, OTA_CONFIG_TABLE_ADDRESS );
      #ifdef ENABLE_GPIO_CONFIG
      this->clear_table_to_defaults( &_gpio_config_defaults, GPIO_CONFIG_TABLE_ADDRESS );
      #endif
      #ifdef ENABLE_MQTT_CONFIG
      this->clear_table_to_defaults( &_mqtt_general_config_defaults, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
      this->clear_table_to_defaults( &_mqtt_lwt_config_defaults, MQTT_LWT_CONFIG_TABLE_ADDRESS );
      this->clear_table_to_defaults( &_mqtt_pubsub_config_defaults, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
      #endif
    }

    global_config_table get_global_config_table(){
      return this->get_table_by_address<global_config_table>(GLOBAL_CONFIG_TABLE_ADDRESS);
    }

    login_credential_table get_login_credential_table(){
      return this->get_table_by_address<login_credential_table>(LOGIN_CREDENTIAL_TABLE_ADDRESS);
    }

    wifi_config_table get_wifi_config_table(){
      return this->get_table_by_address<wifi_config_table>(WIFI_CONFIG_TABLE_ADDRESS);
    }

    ota_config_table get_ota_config_table(){
      return this->get_table_by_address<ota_config_table>(OTA_CONFIG_TABLE_ADDRESS);
    }

    #ifdef ENABLE_GPIO_CONFIG
    gpio_config_table get_gpio_config_table(){
      return this->get_table_by_address<gpio_config_table>(GPIO_CONFIG_TABLE_ADDRESS);
    }
    #endif

    #ifdef ENABLE_MQTT_CONFIG
    mqtt_general_config_table get_mqtt_general_config_table(){
      return this->get_table_by_address<mqtt_general_config_table>(MQTT_GENERAL_CONFIG_TABLE_ADDRESS);
    }
    mqtt_lwt_config_table get_mqtt_lwt_config_table(){
      return this->get_table_by_address<mqtt_lwt_config_table>(MQTT_LWT_CONFIG_TABLE_ADDRESS);
    }
    mqtt_pubsub_config_table get_mqtt_pubsub_config_table(){
      return this->get_table_by_address<mqtt_pubsub_config_table>(MQTT_PUBSUB_CONFIG_TABLE_ADDRESS);
    }
    #endif

    void set_global_config_table( global_config_table* _table ){
      this->set_table( _table, GLOBAL_CONFIG_TABLE_ADDRESS );
    }

    void set_login_credential_table( login_credential_table* _table ){
      this->set_table( _table, LOGIN_CREDENTIAL_TABLE_ADDRESS );
    }

    void set_wifi_config_table( wifi_config_table* _table ){
      this->set_table( _table, WIFI_CONFIG_TABLE_ADDRESS );
    }

    void set_ota_config_table( ota_config_table* _table ){
      this->set_table( _table, OTA_CONFIG_TABLE_ADDRESS );
    }

    #ifdef ENABLE_GPIO_CONFIG
    void set_gpio_config_table( gpio_config_table* _table ){
      this->set_table( _table, GPIO_CONFIG_TABLE_ADDRESS );
    }
    #endif

    #ifdef ENABLE_MQTT_CONFIG
    void set_mqtt_general_config_table( mqtt_general_config_table* _table ){
      this->set_table( _table, MQTT_GENERAL_CONFIG_TABLE_ADDRESS );
    }
    void set_mqtt_lwt_config_table( mqtt_lwt_config_table* _table ){
      this->set_table( _table, MQTT_LWT_CONFIG_TABLE_ADDRESS );
    }
    void set_mqtt_pubsub_config_table( mqtt_pubsub_config_table* _table ){
      this->set_table( _table, MQTT_PUBSUB_CONFIG_TABLE_ADDRESS );
    }
    #endif

    template <typename Struct> Struct get_table_by_address( uint16_t _address ) {

      Struct _t;

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
        #ifdef ENABLE_GPIO_CONFIG
        case GPIO_CONFIG_TABLE_ADDRESS:{

          this->get_table( &_t, GPIO_CONFIG_TABLE_ADDRESS );
          break;
        }
        #endif
        #ifdef ENABLE_MQTT_CONFIG
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
        default: {
          // _t = NULL;
          break;
        }
      }
      return _t;
    }

};


#endif
