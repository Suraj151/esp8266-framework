/***************************** PDI STD File ***********************************
This file is third party source, taken from the open source uClibc++ library and
used here with modifications. Thanking to author for providing this .

for the license terms of this file refer the source link below.

referred from   : https://github.com/mike-matera/ArduinoSTL
added Date      : 1st Dec 2024
added by        : Suraj I.
******************************************************************************/

#define __UCLIBCXX_COMPILE_CHAR_TRAITS__ 1


#include "basic_definitions"
#include "char_traits"

namespace pdistd{

_UCXXEXPORT const char_traits<char>::char_type* char_traits<char>::find(const char_type* s, int n, const char_type& a){
	for(int i=0; i < n; i++){
		if(eq(s[i], a)){
			return (s+i);
		}
	}
	return 0;
}

_UCXXEXPORT bool char_traits<char>::eq(const char_type& c1, const char_type& c2){
	if(strncmp(&c1, &c2, 1) == 0){
		return true;
	}
	return false;
}

_UCXXEXPORT char_traits<char>::char_type char_traits<char>::to_char_type(const int_type & i){
	if(i > 0 && i <= 255){
		return (char)(unsigned char)i;
	}

	//Out of range
	return 0;
}



#ifdef __UCLIBCXX_HAS_WCHAR__

_UCXXEXPORT const char_traits<wchar_t>::char_type* char_traits<wchar_t>::find(const char_type* s, int n, const char_type& a){
	for(int i=0; i < n; i++){
		if(eq(s[i], a)){
			return (s+i);
		}
	}
	return 0;
}

#endif

}
