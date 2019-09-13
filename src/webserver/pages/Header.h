/***************************** header html page ********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _EW_SERVER_HEADER_HTML_H_
#define _EW_SERVER_HEADER_HTML_H_

#include <Arduino.h>

static const char EW_SERVER_HEADER_HTML[] PROGMEM = "\
<html>\
<head>\
<title>Smart Loo</title>\
<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=yes'>\
<style>\
.cntnr{\
border:1px solid #d0d0d0;\
border-radius:10px;\
max-width:350px;\
margin:auto;\
text-align:center;\
padding:15px;\
}\
select{\
padding-left:0px !important;\
}\
a{\
text-decoration:none;\
}\
input, select{\
padding:4px;\
border:1px solid #ccc;\
border-radius:4px;\
background:white;\
}\
select{\
width:100%\
}\
.msg{\
padding:3px;\
border-radius:4px;\
}\
h1{\
color:#0062af;\
}\
table{\
margin:auto;\
}\
.mnwdth125{\
min-width:125px;\
}\
.btn{\
padding:6px;\
border-radius:5px;\
border:1px solid #ccc;\
color:white;\
background:#337ab7;\
margin:0px 2px;\
cursor: pointer;\
}\
</style>\
</head>\
<body>\
<div class='cntnr'>\
<a href='/'><h1>Device Manager</h1></a>";

#endif
