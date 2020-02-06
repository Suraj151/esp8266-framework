/*************************** Common Config page *******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_

#include <Arduino.h>
extern "C" {
  #include "user_interface.h"
}

/**
 * enable/disable gpio and mqtt feature here
 */
#define ENABLE_MQTT_SERVICE
#define ENABLE_GPIO_SERVICE

/**
 * enable/disable email service here
 */
#define ENABLE_EMAIL_SERVICE

/**
 * enable/disable exception notifier
 */
// #define ENABLE_EXCEPTION_NOTIFIER

/**
 * ignore free relay connections created by same ssid
 */
#define IGNORE_FREE_RELAY_CONNECTIONS

/**
 * enable/disable http server feature here
 */
#define ENABLE_EWING_HTTP_SERVER

/**
 * enable/disable network subnetting ( dynamically set ap subnet,gateway etc. )
 */
// #define ENABLE_DYNAMIC_SUBNETTING

/**
 * enable/disable internet availability based station connections
 */
// #define ENABLE_INTERNET_BASED_CONNECTIONS

/**
 * enable/disable esp now feature here
 */
// #define ENABLE_ESP_NOW

/**
 * @define flash key parameters for reset factory
 */
#define FLASH_KEY_PIN             D3
#define FLASH_KEY_PRESS_DURATION  1000
#define FLASH_KEY_PRESS_COUNT_THR 5

/**
 * @define wifi & internet connectivity check cycle durations
 */
#define WIFI_STATION_CONNECT_ATTEMPT_TIMEOUT 5  // will try to connect within this seconds
#define WIFI_CONNECTIVITY_CHECK_DURATION 10000
#define INTERNET_CONNECTIVITY_CHECK_DURATION WIFI_CONNECTIVITY_CHECK_DURATION

/**
 * @define network address & port translation feature
 */
// #define ENABLE_NAPT_FEATURE
#define ENABLE_NAPT_FEATURE_LWIP_V2

/**
 * @define default username/ssid and password
 */
#define USER            "esp8266Stack"
#define PASSPHRASE      "espStack@8266"

/**
 * @define general http parameters
 */
#define HTTP_HOST_ADDR_MAX_SIZE 100
#define HTTP_REQUEST_DURATION   10000
#define HTTP_REQUEST_RETRY      1

/**
 * max tasks and callbacks
 */
#define MAX_SCHEDULABLE_TASKS	25
#define MAX_FACTORY_RESET_CALLBACKS	MAX_SCHEDULABLE_TASKS
#define MAX_EVENT_CALLBACK_TASKS	MAX_SCHEDULABLE_TASKS

/**
 * Define required callback type
 */
typedef std::function<void(int)> CallBackIntArgFn;
typedef std::function<void(void)> CallBackVoidArgFn;
typedef std::function<void(void*)> CallBackVoidPointerArgFn;

#endif
