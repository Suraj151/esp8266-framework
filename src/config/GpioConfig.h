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

const int gpio_config_size = sizeof(gpio_configs) + 5;

using gpio_config_table = gpio_configs;

#endif
