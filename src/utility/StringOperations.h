#ifndef __STRINGOPERATIONS_H__
#define __STRINGOPERATIONS_H__

#include <Arduino.h>
#include <utility/Log.h>

  int __strstr(char *str, char *substr, int _len=300);
  char* __strtrim(char *str, uint16_t _overflow_limit=300);
  bool __are_str_equals(char *str1, char *str2, uint16_t _overflow_limit=300);
  void __appendUintToBuff( char* _str, const char* _format, uint32_t _value, int _len);
  void __int_ip_to_str( char* _str, uint8_t* _ip, int _len=15 );
  void __str_ip_to_int( char* _str, uint8_t* _ip, int _len=15, bool _clear_str_after_done=true );
  void __find_and_replace( char* _str, char* _find_str, char* _replace_with, int _occurence );

#endif
