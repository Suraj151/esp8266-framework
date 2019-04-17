#ifndef _SMARTLOO_CONFIG_H_
#define _SMARTLOO_CONFIG_H_

#include <Arduino.h>

#define SMART_LOO_BUF_SIZE        20
#define SMART_LOO_HOST_ADDR_SIZE  60
#define SMART_LOO_TOKEN_SIZE      400
#define SMARTLOO_CONFIG_TABLE_ADDRESS 2000
#define SENSOR_DATA_POST_FREQ     5

enum smartloo_site_genders {
  ALL,
  MALE,
  FEMALE
};

enum smartloo_site_sensors {
  UNDEFINED,
  PEOPLE_COUNTER,
  SMELL,
  LIGHT,
  WATER_LEVEL,
  SCREAM
};

struct smartloo_configs {
  char site_id[SMART_LOO_BUF_SIZE];
  smartloo_site_genders allowed_gender;
  smartloo_site_sensors sensor_type;
  char sensor_host[SMART_LOO_HOST_ADDR_SIZE];
  int sensor_port;
  int sensor_post_frequency;
  float latitude;
  float longitude;
  char city[SMART_LOO_BUF_SIZE];
  char _token[SMART_LOO_TOKEN_SIZE];
};

const smartloo_configs PROGMEM _smartloo_config_defaults = {
  "", MALE, PEOPLE_COUNTER, "", 80, SENSOR_DATA_POST_FREQ, 0, 0, "", ""
};

using smartloo_config_table = smartloo_configs;

#endif
