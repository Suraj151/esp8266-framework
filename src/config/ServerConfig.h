#ifndef _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include <Arduino.h>
#include "WifiConfig.h"

/**
 * enable/disable http server feature here
 */
#define ENABLE_EWING_HTTP_SERVER

#define DEFAULT_LOGIN_USERNAME     "Ewings"
#define DEFAULT_LOGIN_PASSWORD     "Ewings@123"

#define EW_SESSION_NAME      "ew_session"
#define EW_COOKIE_MAX_AGE    300

#define LOGIN_CONFIGS_BUF_SIZE WIFI_CONFIGS_BUF_SIZE

struct login_credential {
  char username[LOGIN_CONFIGS_BUF_SIZE];
  char password[LOGIN_CONFIGS_BUF_SIZE];
  char session_name[LOGIN_CONFIGS_BUF_SIZE];
  uint16_t cookie_max_age;
};

const login_credential PROGMEM _login_credential_defaults = {
  DEFAULT_LOGIN_USERNAME,  DEFAULT_LOGIN_PASSWORD, EW_SESSION_NAME, EW_COOKIE_MAX_AGE
};

const int login_credential_size = sizeof(login_credential) + 5;

using login_credential_table = login_credential;

#endif
