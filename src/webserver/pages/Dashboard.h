/************************** Dashboard html page *******************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_DASHBOARD_PAGE_H_
#define _EW_SERVER_DASHBOARD_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_DASHBOARD_PAGE[] PROGMEM = "\
<h2>Dashboard</h2>\
\
<style>\
#stnm,#strs,#stst,#stmc{\
text-align:right;\
}\
</style>\
\
<table>\
<tr>\
<td>sta ssid</td>\
<td id='stnm'></td>\
</tr>\
<tr>\
<td>rssi</td>\
<td id='strs'></td>\
</tr>\
<tr>\
<td>status</td>\
<td id='stst'></td>\
</tr>\
<tr>\
<td>mac</td>\
<td id='stmc'></td>\
</tr>\
</table>";

#endif
