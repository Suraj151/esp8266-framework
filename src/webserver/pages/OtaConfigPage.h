#ifndef _EW_SERVER_OTA_CONFIG_PAGE_H_
#define _EW_SERVER_OTA_CONFIG_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_OTA_CONFIG_PAGE_TOP[] PROGMEM = "\
<h2>OTA Configuration</h2>\
<form action='/ota-config' method='POST'>\
<table>";

#endif
