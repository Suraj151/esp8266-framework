/***************************** GPIO Config page *******************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#ifndef _GPIO_CONFIG_H_
#define _GPIO_CONFIG_H_

#include "Common.h"

#define MAX_NO_OF_GPIO_PINS           9
#define GPIO_HOST_BUF_SIZE            60
#define ANALOG_GPIO_RESOLUTION        1024
#define GPIO_GRAPH_ADJ_POINT_DISTANCE 10
#define GPIO_MAX_GRAPH_WIDTH          300
#define GPIO_MAX_GRAPH_HEIGHT         150
#define GPIO_GRAPH_TOP_MARGIN         25
#define GPIO_GRAPH_BOTTOM_MARGIN      GPIO_GRAPH_TOP_MARGIN
#define GPIO_DATA_POST_FREQ           5

#define GPIO_ALERT_DURATION_FOR_SUCCEED 3600000
#define GPIO_ALERT_DURATION_FOR_FAILED  300000

/**
 * @define gpio parameters
 */
#define GPIO_OPERATION_DURATION MILLISECOND_DURATION_1000
#define GPIO_TABLE_UPDATE_DURATION 300000

/**
 * global gpio alert status
 */
typedef struct {
  bool is_last_alert_succeed;
  uint32_t last_alert_millis;
} __gpio_alert_track_t;

extern __gpio_alert_track_t __gpio_alert_track;

struct last_gpio_monitor_point{
  int x;
  int y;
};
const uint8_t EXCEPTIONAL_GPIO_PINS[] = {3};

enum GPIO_CONFIG_TYPE {
  GPIO_MODE_CONFIG,
  GPIO_WRITE_CONFIG,
  GPIO_SERVER_CONFIG,
  GPIO_ALERT_CONFIG,
};

enum GPIO_MODE {
  OFF=0,
  DIGITAL_WRITE=1,
  DIGITAL_READ=2,
  ANALOG_WRITE=3,
  ANALOG_READ=4
};

enum GPIO_ALERT_COMPARATOR {
  EQUAL=0,
  GREATER_THAN=1,
  LESS_THAN=2,
};

enum GPIO_ALERT_CHANNEL {
  NO_ALERT=0,
  EMAIL=1,
};

struct gpio_configs {
  uint8_t gpio_mode[MAX_NO_OF_GPIO_PINS+1];
  uint16_t gpio_readings[MAX_NO_OF_GPIO_PINS+1];
  uint8_t gpio_alert_comparator[MAX_NO_OF_GPIO_PINS+1];
  uint8_t gpio_alert_channel[MAX_NO_OF_GPIO_PINS+1];
  uint16_t gpio_alert_values[MAX_NO_OF_GPIO_PINS+1];
  char gpio_host[GPIO_HOST_BUF_SIZE];
  int gpio_port;
  int gpio_post_frequency;
};

const gpio_configs PROGMEM _gpio_config_defaults = {
  {OFF}, {0}, {EQUAL}, {NO_ALERT}, {0}, "", 80, GPIO_DATA_POST_FREQ
};

const int gpio_config_size = sizeof(gpio_configs) + 5;

using gpio_config_table = gpio_configs;

#endif
