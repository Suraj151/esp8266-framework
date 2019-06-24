/***************************** gpio html page *********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_GPIO_CONFIG_PAGE_H_
#define _EW_GPIO_CONFIG_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_GPIO_MANAGE_PAGE[] PROGMEM = "\
<div>\
\
<div>\
<a href='/gpio-config'>\
<button class='btn' style='min-width:125px;'>\
GPIO Modes\
</button>\
</a>\
</div>\
\
<div>\
<a href='/gpio-write'>\
<button class='btn' style='min-width:125px;'>\
GPIO Control\
</button>\
</a>\
</div>\
\
<div>\
<a href='/gpio-server'>\
<button class='btn' style='min-width:125px;'>\
GPIO Server\
</button>\
</a>\
</div>\
\
<div>\
<a href='/gpio-monitor'>\
<button class='btn' style='min-width:125px;'>\
GPIO Monitor\
</button>\
</a>\
</div>\
\
</div>\
";

static const char EW_SERVER_GPIO_MONITOR_PAGE_TOP[] PROGMEM = "\
<h2>GPIO Monitor</h2>\
";

static const char EW_SERVER_GPIO_MONITOR_SVG_ELEMENT[] PROGMEM = "\
<svg id='svga0' width='300' height='150'>\
<rect width='300' height='150'/>\
<line x1='0' y1='25' x2='300' y2='25' stroke='white'></line>\
<line x1='0' y1='125' x2='300' y2='125' stroke='white'></line>\
</svg>\
";

static const char EW_SERVER_GPIO_SERVER_PAGE_TOP[] PROGMEM = "\
<h2>GPIO Server Configuration</h2>\
<form action='/gpio-server' method='POST'>\
<table>";

static const char EW_SERVER_GPIO_CONFIG_PAGE_TOP[] PROGMEM = "\
<h2>GPIO Mode Configuration</h2>\
<form action='/gpio-config' method='POST'>\
<table>";

static const char EW_SERVER_GPIO_WRITE_PAGE_TOP[] PROGMEM = "\
<h2>GPIO Control</h2>\
<form action='/gpio-write' method='POST'>\
<table>";

static const char EW_SERVER_GPIO_WRITE_EMPTY_MESSAGE[] PROGMEM = "\
<h4>No GPIO enabled for write operation.</h4>\
<div>enable from \
<a href='/gpio-config'>\
<button class='btn' type='button'>\
GPIO Modes\
</button>\
</a>\
</div>\
</table>\
</form>\
";

#endif
