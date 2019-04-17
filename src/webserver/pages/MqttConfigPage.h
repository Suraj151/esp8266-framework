#ifndef _EW_MQTT_CONFIG_PAGE_H_
#define _EW_MQTT_CONFIG_PAGE_H_

#include <Arduino.h>

static const char EW_SERVER_MQTT_MANAGE_PAGE[] PROGMEM = "\
<div>\
\
<div>\
<a href='/mqtt-general-config'>\
<button class='btn' style='min-width:145px;'>\
MQTT General Config\
</button>\
</a>\
</div>\
\
<div>\
<a href='/mqtt-lwt-config'>\
<button class='btn' style='min-width:145px;'>\
MQTT LWT Config\
</button>\
</a>\
</div>\
\
<div>\
<a href='/mqtt-pubsub-config'>\
<button class='btn' style='min-width:145px;'>\
MQTT PubSub Config\
</button>\
</a>\
</div>\
\
</div>\
";

static const char EW_SERVER_MQTT_GENERAL_PAGE_TOP[] PROGMEM = "\
<h2>MQTT General Configuration</h2>\
<form action='/mqtt-general-config' method='POST'>\
<table>";

static const char EW_SERVER_MQTT_LWT_PAGE_TOP[] PROGMEM = "\
<h2>MQTT LWT Configuration</h2>\
<form action='/mqtt-lwt-config' method='POST'>\
<table>";

static const char EW_SERVER_MQTT_PUBSUB_PAGE_TOP[] PROGMEM = "\
<h2>MQTT PUBSUB Configuration</h2>\
<form action='/mqtt-pubsub-config' method='POST'>\
<table>";

#endif
