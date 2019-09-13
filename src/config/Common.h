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
 * @define flash key parameters
 */
#define FLASH_KEY_PIN             D3
#define FLASH_KEY_PRESS_DURATION  1000
#define FLASH_KEY_PRESS_COUNT_THR 5

/**
 * @define wifi cycle check parameters
 */
#define WIFI_CONNECTIVITY_CHECK_DURATION 30000

/**
 * @define network address & port translation feature
 */
//#define ENABLE_NAPT_FEATURE

#define USER            "Ewings"
#define PASSPHRASE      "Ewings@123"

#endif
