/**************************** ota config html page ****************************
This file is part of the Ewings Esp Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_OTA_CONFIG_PAGE_H_
#define _EW_SERVER_OTA_CONFIG_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_OTA_CONFIG_PAGE_TOP[] PROGMEM = "\
<h2>OTA Configuration</h2>\
<form action='/ota-config' method='POST'>\
<table>";

#endif
