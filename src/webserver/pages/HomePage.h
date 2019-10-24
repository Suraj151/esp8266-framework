/****************************** home html page ********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_HOME_PAGE_H_
#define _EW_SERVER_HOME_PAGE_H_

#include <Arduino.h>
#include <config/Config.h>

static const char EW_SERVER_HOME_PAGE[] PROGMEM = "\
<h4>Please login to change settings</h4>\
<div>\
<a href='/login'>\
<button class='btn'>\
Click Here\
</button>\
</a>\
to login\
</div>";


#if defined( ENABLE_GPIO_SERVICE ) && defined( ENABLE_MQTT_SERVICE )

static const char EW_SERVER_HOME_AUTHORIZED_PAGE[] PROGMEM = "\
<div>\
\
<div>\
<a href='/login-config'>\
<button class='btn mnwdth125'>\
Login Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/wifi-config'>\
<button class='btn mnwdth125'>\
WiFi Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/ota-config'>\
<button class='btn mnwdth125'>\
OTA Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/mqtt-manage'>\
<button class='btn mnwdth125'>\
MQTT Manage\
</button>\
</a>\
</div>\
\
<div>\
<a href='/gpio-manage'>\
<button class='btn mnwdth125'>\
GPIO Manage\
</button>\
</a>\
</div>\
\
<div>\
<a href='/dashboard'>\
<button class='btn mnwdth125'>\
Dashboard\
</button>\
</a>\
</div>\
\
<div>\
<a href='/logout'>\
<button class='btn mnwdth125'>\
Logout\
</button>\
</a>\
</div>\
\
</div>";

#elif defined( ENABLE_GPIO_SERVICE )

static const char EW_SERVER_HOME_AUTHORIZED_PAGE[] PROGMEM = "\
<div>\
\
<div>\
<a href='/login-config'>\
<button class='btn mnwdth125'>\
Login Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/wifi-config'>\
<button class='btn mnwdth125'>\
WiFi Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/ota-config'>\
<button class='btn mnwdth125'>\
OTA Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/gpio-manage'>\
<button class='btn mnwdth125'>\
GPIO Manage\
</button>\
</a>\
</div>\
\
<div>\
<a href='/dashboard'>\
<button class='btn mnwdth125'>\
Dashboard\
</button>\
</a>\
</div>\
\
<div>\
<a href='/logout'>\
<button class='btn mnwdth125'>\
Logout\
</button>\
</a>\
</div>\
\
</div>";

#elif  defined( ENABLE_MQTT_SERVICE )

static const char EW_SERVER_HOME_AUTHORIZED_PAGE[] PROGMEM = "\
<div>\
\
<div>\
<a href='/login-config'>\
<button class='btn mnwdth125'>\
Login Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/wifi-config'>\
<button class='btn mnwdth125'>\
WiFi Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/ota-config'>\
<button class='btn mnwdth125'>\
OTA Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/mqtt-manage'>\
<button class='btn mnwdth125'>\
MQTT Manage\
</button>\
</a>\
</div>\
\
<div>\
<a href='/dashboard'>\
<button class='btn mnwdth125'>\
Dashboard\
</button>\
</a>\
</div>\
\
<div>\
<a href='/logout'>\
<button class='btn mnwdth125'>\
Logout\
</button>\
</a>\
</div>\
\
</div>";

#else

static const char EW_SERVER_HOME_AUTHORIZED_PAGE[] PROGMEM = "\
<div>\
\
<div>\
<a href='/login-config'>\
<button class='btn mnwdth125'>\
Login Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/wifi-config'>\
<button class='btn mnwdth125'>\
WiFi Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/ota-config'>\
<button class='btn mnwdth125'>\
OTA Settings\
</button>\
</a>\
</div>\
\
<div>\
<a href='/dashboard'>\
<button class='btn mnwdth125'>\
Dashboard\
</button>\
</a>\
</div>\
\
<div>\
<a href='/logout'>\
<button class='btn mnwdth125'>\
Logout\
</button>\
</a>\
</div>\
\
</div>";
#endif

#endif
