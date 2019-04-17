#ifndef _EW_SERVER_WIFI_CONFIG_PAGE_H_
#define _EW_SERVER_WIFI_CONFIG_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_WIFI_CONFIG_PAGE_TOP[] PROGMEM = "\
<h2>WiFi Configuration</h2>\
<form action='/wifi-config' method='POST'>\
<table>";

static const char EW_SERVER_WIFI_CONFIG_PAGE_BOTTOM[] PROGMEM = "\
<tr>\
<td></td>\
<td>\
<button class='btn' type='submit'>\
Submit\
</button>\
<a href='/logout'>\
<button class='btn' type='button'>\
Logout\
</button>\
</a>\
</td>\
</tr>\
</table>\
</form>\
";

#endif
