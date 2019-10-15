#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_

#include <Arduino.h>

/**
 * enable/disable mqtt feature here
 */
#define ENABLE_MQTT_CONFIG
#define ENABLE_GPIO_CONFIG

/**
 * enable/disable http server feature here
 */
#define ENABLE_EWING_HTTP_SERVER

/**
 * enable/disable esp now feature here
 */
// #define ENABLE_ESP_NOW

/**
 * @define flash key parameters
 */
#define FLASH_KEY_PIN             D3
#define FLASH_KEY_PRESS_DURATION  1000
#define FLASH_KEY_PRESS_COUNT_THR 5

/**
 * @define wifi & internet connectivity check cycle parameters
 */
#define WIFI_CONNECTIVITY_CHECK_DURATION 10000
#define INTERNET_CONNECTIVITY_CHECK_DURATION WIFI_CONNECTIVITY_CHECK_DURATION

/**
 * @define network address & port translation feature
 */
// #define ENABLE_NAPT_FEATURE
#define ENABLE_NAPT_FEATURE_LWIP_V2

#define USER            "Ewings"
#define PASSPHRASE      "Ewings@123"

/**
 * @define general http parameters
 */
#define HTTP_HOST_ADDR_MAX_SIZE 100
#define HTTP_REQUEST_DURATION   10000
#define HTTP_REQUEST_RETRY      1


#endif
