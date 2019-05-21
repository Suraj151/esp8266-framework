#ifndef EW_STORE_STRUCT_H
#define EW_STORE_STRUCT_H

#include <Arduino.h>

// #define ENABLE_GPIO_CONFIG
#define ENABLE_MQTT_CONFIG

#define CONFIG_START      5
#define CONFIG_VERSION    "1.0"
#define FIRMWARE_VERSION  2019041100
#define LAUNCH_YEAR       19
#define LAUNCH_UNIX_TIME  1546300800  // 2019 Unix time stamp

#define DEFAULT_USERNAME     "Ewings"
#define DEFAULT_PASSWORD     "Ewings@123"

#define DEFAULT_SSID         "COHIVE L-5"
#define DEFAULT_PASSPHRASE   "CoHiVE@56#&NeTworK"

#define EW_SESSION_NAME      "ew_session"
#define EW_COOKIE_MAX_AGE    300

#define DEFAULT_STA_LOCAL_IP  {0, 0, 0, 0}
#define DEFAULT_STA_GATEWAY   {0, 0, 0, 0}
#define DEFAULT_STA_SUBNET    {0, 0, 0, 0}

#define DEFAULT_AP_LOCAL_IP  {192, 168, 0, 1}
#define DEFAULT_AP_GATEWAY   {192, 168, 0, 1}
#define DEFAULT_AP_SUBNET    {255, 255, 255, 0}

#define WIFI_CONFIGS_BUF_SIZE 20
#define LOGIN_CONFIGS_BUF_SIZE WIFI_CONFIGS_BUF_SIZE
#define OTA_HOST_BUF_SIZE 50

enum eeprom_db_table_address {
  GLOBAL_CONFIG_TABLE_ADDRESS = CONFIG_START,
  LOGIN_CREDENTIAL_TABLE_ADDRESS = 50,
  WIFI_CONFIG_TABLE_ADDRESS = 150,
  OTA_CONFIG_TABLE_ADDRESS = 300,
  #ifdef ENABLE_GPIO_CONFIG
  GPIO_CONFIG_TABLE_ADDRESS = 500,
  #endif
  #ifdef ENABLE_MQTT_CONFIG
  MQTT_GENERAL_CONFIG_TABLE_ADDRESS = 700,
  MQTT_LWT_CONFIG_TABLE_ADDRESS = 1400,
  MQTT_PUBSUB_CONFIG_TABLE_ADDRESS = 1600,
  #endif
};

struct global_config {
  char version[4];
  uint8_t current_year;
};

struct login_credential {
  char username[LOGIN_CONFIGS_BUF_SIZE];
  char password[LOGIN_CONFIGS_BUF_SIZE];
  char session_name[LOGIN_CONFIGS_BUF_SIZE];
  uint16_t cookie_max_age;
};

struct wifi_configs {
  char sta_ssid[WIFI_CONFIGS_BUF_SIZE];
  char sta_password[WIFI_CONFIGS_BUF_SIZE];
  char ap_ssid[WIFI_CONFIGS_BUF_SIZE];
  char ap_password[WIFI_CONFIGS_BUF_SIZE];

  uint8_t sta_local_ip[4];
  uint8_t sta_gateway[4];
  uint8_t sta_subnet[4];

  uint8_t ap_local_ip[4];
  uint8_t ap_gateway[4];
  uint8_t ap_subnet[4];
};

struct ota_configs {
  uint32_t firmware_version;
  char ota_host[OTA_HOST_BUF_SIZE];
  int ota_port;
};

const global_config PROGMEM _global_config_defaults = {
  CONFIG_VERSION, LAUNCH_YEAR
};

const login_credential PROGMEM _login_credential_defaults = {
  DEFAULT_USERNAME,  DEFAULT_PASSWORD, EW_SESSION_NAME, EW_COOKIE_MAX_AGE
};

const wifi_configs PROGMEM _wifi_config_defaults = {
  DEFAULT_SSID, DEFAULT_PASSPHRASE, DEFAULT_USERNAME, DEFAULT_PASSWORD,
  DEFAULT_STA_LOCAL_IP, DEFAULT_STA_GATEWAY, DEFAULT_STA_SUBNET,
  DEFAULT_AP_LOCAL_IP, DEFAULT_AP_GATEWAY, DEFAULT_AP_SUBNET
};

const ota_configs PROGMEM _ota_config_defaults = {
  FIRMWARE_VERSION, {0}, 80
};

using global_config_table = global_config;
using login_credential_table = login_credential;
using wifi_config_table = wifi_configs;
using ota_config_table = ota_configs;

#ifdef ENABLE_GPIO_CONFIG

#define MAX_NO_OF_GPIO_PINS           9
#define GPIO_HOST_BUF_SIZE            60
#define ANALOG_GPIO_RESOLUTION        1024
#define GPIO_GRAPH_ADJ_POINT_DISTANCE 10
#define GPIO_MAX_GRAPH_WIDTH          300
#define GPIO_MAX_GRAPH_HEIGHT         150
#define GPIO_GRAPH_TOP_MARGIN         25
#define GPIO_GRAPH_BOTTOM_MARGIN      GPIO_GRAPH_TOP_MARGIN
#define GPIO_DATA_POST_FREQ           5


struct last_gpio_monitor_point{
  int x;
  int y;
};
// #define EXCEPTIONAL_GPIO_PINS {3}
const uint8_t EXCEPTIONAL_GPIO_PINS[] = {3};

enum GPIO_CONFIG_TYPE {
  GPIO_MODE_CONFIG,
  GPIO_WRITE_CONFIG,
  GPIO_SERVER_CONFIG,
};

enum GPIO_MODE {
  OFF=1,
  DIGITAL_WRITE=2,
  DIGITAL_READ=3,
  ANALOG_WRITE=4,
  ANALOG_READ=5
};

struct gpio_configs {
  uint8_t gpio_mode[MAX_NO_OF_GPIO_PINS+1];
  uint16_t gpio_readings[MAX_NO_OF_GPIO_PINS+1];
  char gpio_host[GPIO_HOST_BUF_SIZE];
  int gpio_port;
  int gpio_post_frequency;
};

const gpio_configs PROGMEM _gpio_config_defaults = {
  {OFF}, {0},"",80,GPIO_DATA_POST_FREQ
};

using gpio_config_table = gpio_configs;

#endif


#ifdef ENABLE_MQTT_CONFIG

#define MQTT_HOST_BUF_SIZE      50
#define MQTT_CLIENT_ID_BUF_SIZE 100
#define MQTT_DEFAULT_KEEPALIVE  120     /*second*/
#define MQTT_HOST_CONNECT_TIMEOUT  5    /*second*/
#define MQTT_DEFAULT_PORT       1883
#define MQTT_USERNAME_BUF_SIZE  25
#define MQTT_PASSWORD_BUF_SIZE  500
#define MQTT_TOPIC_BUF_SIZE     50
#define MQTT_WILL_MSG_BUF_SIZE  80

#define MQTT_MAX_QOS_LEVEL      2
#define MQTT_MAX_PUBLISH_TOPIC  2
#define MQTT_MAX_SUBSCRIBE_TOPIC  MQTT_MAX_PUBLISH_TOPIC

#define MQTT_INITIALIZE_DURATION   10000

enum MQTT_CONFIG_TYPE {
  MQTT_GENERAL_CONFIG,
  MQTT_LWT_CONFIG,
  MQTT_PUBSUB_CONFIG,
};

typedef struct {

  char topic[MQTT_TOPIC_BUF_SIZE];
  uint8_t qos;
} mqtt_sub_topics_t;

typedef struct {

  char topic[MQTT_TOPIC_BUF_SIZE];
  uint8_t qos;
  uint8_t retain;
} mqtt_pub_topics_t;

struct mqtt_general_configs {

  char host[MQTT_HOST_BUF_SIZE];
  int port;
  int security;
  char client_id[MQTT_CLIENT_ID_BUF_SIZE];
  char username[MQTT_USERNAME_BUF_SIZE];
  char password[MQTT_PASSWORD_BUF_SIZE];
  int keepalive;
  int clean_session;
};

struct mqtt_lwt_configs {

  char will_topic[MQTT_TOPIC_BUF_SIZE];
  char will_message[MQTT_WILL_MSG_BUF_SIZE];
  int will_qos;
  int will_retain;
};

struct mqtt_pubsub_configs {

  mqtt_pub_topics_t publish_topics[MQTT_MAX_PUBLISH_TOPIC];
  mqtt_sub_topics_t subscribe_topics[MQTT_MAX_SUBSCRIBE_TOPIC];
  int publish_frequency;
};

const mqtt_sub_topics_t PROGMEM _mqtt_sub_topic_defaults = {
  "", 0
};

const mqtt_pub_topics_t PROGMEM _mqtt_pub_topic_defaults = {
  "", 0, 0
};

const mqtt_general_configs PROGMEM _mqtt_general_config_defaults = {
  "", MQTT_DEFAULT_PORT, 0, "", "", "", MQTT_DEFAULT_KEEPALIVE, 1
};

const mqtt_lwt_configs PROGMEM _mqtt_lwt_config_defaults = {
  "", "", 0, 0
};

// const mqtt_pubsub_configs PROGMEM _mqtt_pubsub_config_defaults = {
//   {_mqtt_pub_topic_defaults}, {_mqtt_sub_topic_defaults}, 0
// };

const mqtt_pubsub_configs PROGMEM _mqtt_pubsub_config_defaults = {
  NULL, NULL, 0
};

using mqtt_general_config_table = mqtt_general_configs;
using mqtt_lwt_config_table = mqtt_lwt_configs;
using mqtt_pubsub_config_table = mqtt_pubsub_configs;

#endif

#endif
