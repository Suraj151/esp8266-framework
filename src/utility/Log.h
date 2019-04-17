#ifndef __EWINGS_LOG_UTILITY_H__
#define __EWINGS_LOG_UTILITY_H__

#include <Arduino.h>

#define EW_SERIAL_LOG
#define EW_DEFAULT_LOG_DURATION	5000

#ifdef EW_SERIAL_LOG

#define LogBegin(x)	Serial.begin(x)
#define Log(x)			Serial.print(x)
#define Log_format(x,t)		Serial.print(x,t)
#define Logln(x)		Serial.println(x)
#define Logln_format(x,t)	Serial.println(x,t)

#else

#define LogBegin(x)
#define Log(x)
#define Logln(x)

#endif

#endif
