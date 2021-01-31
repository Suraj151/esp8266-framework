/*************************** Email Config page ********************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#ifndef _EMAIL_CONFIG_H_
#define _EMAIL_CONFIG_H_

#include "Common.h"

/**
 * email configurations
 */

#define DEFAULT_SENDING_DOMAIN_MAX_SIZE   50
#define DEFAULT_MAIL_HOST_MAX_SIZE        DEFAULT_SENDING_DOMAIN_MAX_SIZE
#define DEFAULT_MAIL_USERNAME_MAX_SIZE    100
#define DEFAULT_MAIL_PASSWORD_MAX_SIZE    DEFAULT_MAIL_HOST_MAX_SIZE
#define DEFAULT_MAIL_SUBJECT_MAX_SIZE     DEFAULT_MAIL_HOST_MAX_SIZE
#define DEFAULT_MAIL_FROM_MAX_SIZE        DEFAULT_MAIL_HOST_MAX_SIZE
#define DEFAULT_MAIL_FROM_NAME_MAX_SIZE   DEFAULT_MAIL_HOST_MAX_SIZE
#define DEFAULT_MAIL_TO_MAX_SIZE          DEFAULT_MAIL_FROM_MAX_SIZE


#define DEFAULT_SENDING_DOMAIN            "www.example.com"
#define DEFAULT_MAIL_HOST                 "smtp.mailtrap.io"
#define DEFAULT_MAIL_PORT                 2525
#define DEFAULT_MAIL_USERNAME             "sdc10dfgdf326d"
#define DEFAULT_MAIL_PASSWORD             "zdvfvfdvddfvfd"
#define DEFAULT_MAIL_FROM                 "surajinamdar151@gmail.com"
#define DEFAULT_MAIL_TO                   DEFAULT_MAIL_FROM
#define DEFAULT_MAIL_SUBJECT              "device status"
#define DEFAULT_MAIL_FREQUENCY            300
#define DEFAULT_MAIL_FROM_NAME            "esp8266"
// #define DEFAULT_MAIL_ENCRYPTION           "tls"


struct email_config {
  char sending_domain[DEFAULT_SENDING_DOMAIN_MAX_SIZE];
  char mail_host[DEFAULT_MAIL_HOST_MAX_SIZE];
  uint16_t mail_port;
  char mail_username[DEFAULT_MAIL_USERNAME_MAX_SIZE];
  char mail_password[DEFAULT_MAIL_PASSWORD_MAX_SIZE];
  char mail_from[DEFAULT_MAIL_FROM_MAX_SIZE];
  char mail_from_name[DEFAULT_MAIL_FROM_NAME_MAX_SIZE];
  char mail_to[DEFAULT_MAIL_TO_MAX_SIZE];
  char mail_subject[DEFAULT_MAIL_SUBJECT_MAX_SIZE];
  uint16_t mail_frequency;
};


const email_config PROGMEM _email_config_defaults = {
  DEFAULT_SENDING_DOMAIN, DEFAULT_MAIL_HOST, DEFAULT_MAIL_PORT, DEFAULT_MAIL_USERNAME, DEFAULT_MAIL_PASSWORD,
  DEFAULT_MAIL_FROM, DEFAULT_MAIL_FROM_NAME, DEFAULT_MAIL_TO, DEFAULT_MAIL_SUBJECT, DEFAULT_MAIL_FREQUENCY
};

const int email_config_size = sizeof(email_config) + 5;

using email_config_table = email_config;

#endif
