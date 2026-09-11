/***************************** PDI STD File ***********************************
This file is third party source, taken from the open source uClibc++ library and
used here with modifications. Thanking to author for providing this .

for the license terms of this file refer the source link below.

referred from   : https://github.com/mike-matera/ArduinoSTL
added Date      : 1st Dec 2024
added by        : Suraj I.
******************************************************************************/

#define __UCLIBCXX_COMPILE_IOSTREAM__ 1

#include "iostream"

namespace pdistd{

#ifdef __UCLIBCXX_EXPAND_OSTREAM_CHAR__
#ifdef __UCLIBCXX_EXPAND_ISTREAM_CHAR__

	template _UCXXEXPORT basic_iostream<char, char_traits<char> >::
		basic_iostream(basic_streambuf<char, char_traits<char> >* sb);
	template _UCXXEXPORT basic_iostream<char, char_traits<char> >::~basic_iostream();

#endif
#endif

}


