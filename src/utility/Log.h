/**************************** Log Utility *************************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef __EWINGS_LOG_UTILITY_H__
#define __EWINGS_LOG_UTILITY_H__

#include <Arduino.h>

#define EW_SERIAL_LOG
#define EW_DEFAULT_LOG_DURATION	5000

#ifdef EW_SERIAL_LOG

#define LogBegin(x)           Serial.begin(x)
#define Log(x)                {yield();Serial.print(x);}
#define Logln(x)		          {yield();Serial.println(x);}
#define Log_format(x,t)       {yield();Serial.print(x,t);}
#define Logln_format(x,t)     {yield();Serial.println(x,t);}

#else

#define LogBegin(x)
#define Log(x)
#define Logln(x)
#define Log_format(x,t)
#define Logln_format(x,t)

#endif

#endif
