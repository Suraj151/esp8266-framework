
#ifndef __DATATYPECONVERSIONS_H__
#define __DATATYPECONVERSIONS_H__

#include <Arduino.h>
#include <utility/Log.h>

	uint8_t BcdToUint8(uint8_t val);
	uint8_t Uint8ToBcd(uint8_t val);
	uint16_t StringToUint16(char* pString, uint8_t _len = 32);
	uint8_t StringToUint8(char* pString, uint8_t _len = 32);
	uint16_t StringToHex16(char* pString, uint8_t _strlen);

#endif
