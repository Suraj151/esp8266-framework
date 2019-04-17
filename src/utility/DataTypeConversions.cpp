#include "DataTypeConversions.h"

uint8_t BcdToUint8(uint8_t val){
    return val - 6 * (val >> 4);
}

uint8_t Uint8ToBcd(uint8_t val){
    return val + 6 * (val / 10);
}

uint16_t StringToUint16(char* pString, uint8_t _len){
    uint16_t value = 0;
	uint8_t n = 0;

    while ('0' == *pString || *pString == ' ' || *pString == '"' && n < _len)
    {
		pString++;
		n++;
    }

    while ('0' <= *pString && *pString <= '9' && n < _len)
    {
        value *= 10;
        value += *pString - '0';
		pString++;
		n++;
    }
    return value;
}

uint8_t StringToUint8(char* pString, uint8_t _len){
	uint8_t value = 0, n = 0;

	while ('0' == *pString || *pString == ' ' || *pString == '"' && n < _len)
	{
		pString++;
		n++;
	}

	while ('0' <= *pString && *pString <= '9' && n < _len)
	{
		value *= 10;
		value += *pString - '0';
		pString++;
    	n++;
	}
	return value;
}

uint16_t StringToHex16(char* pString, uint8_t _strlen){
	uint16_t value = 0;
	uint16_t hexValue = 0;
	uint16_t hexWeight = 1;

	for(uint8_t i=0; i < _strlen-1; i++) hexWeight *= 16;
	while (*pString == ' ' || *pString == '"') pString++;
	while (('0' <= *pString && *pString <= '9') ||
		 ('A' <= *pString && *pString <= 'F') ||
		 ('a' <= *pString && *pString <= 'f')){
	  if('A' <= *pString && *pString <= 'F') value = 10 + (*pString - 'A');
	  else if('a' <= *pString && *pString <= 'f') value = 10 + (*pString - 'a');
	  else value = (*pString - '0');
	  hexValue += hexWeight * value;
	  hexWeight /= 16;
	  pString++;
	}
	return hexValue;
}
