/***************************** PDI STD File ***********************************
This file is third party source, taken from the open source uClibc++ library and
used here with modifications. Thanking to author for providing this .

for the license terms of this file refer the source link below.

referred from   : https://github.com/mike-matera/ArduinoSTL
added Date      : 1st Dec 2024
added by        : Suraj I.
******************************************************************************/

#define __UCLIBCXX_COMPILE_OSTREAM__ 1

#include "ostream"

namespace pdistd{
	

#ifdef __UCLIBCXX_EXPAND_OSTREAM_CHAR__
	
#ifdef __UCLIBCXX_EXPAND_CONSTRUCTORS_DESTRUCTORS__
	template _UCXXEXPORT ostream::~basic_ostream();
#endif //__UCLIBCXX_EXPAND_CONSTRUCTORS_DESTRUCTORS__

	template _UCXXEXPORT ostream & ostream::flush();

	template _UCXXEXPORT ostream & ostream::operator<<(bool n);
	template _UCXXEXPORT ostream & ostream::operator<<(short int n);
	template _UCXXEXPORT ostream & ostream::operator<<(unsigned short int n);
	template _UCXXEXPORT ostream & ostream::operator<<(int n);
	template _UCXXEXPORT ostream & ostream::operator<<(unsigned int n);
	template _UCXXEXPORT ostream & ostream::operator<<(long n);
	template _UCXXEXPORT ostream & ostream::operator<<(unsigned long n);
	template _UCXXEXPORT ostream & ostream::operator<<(float f);
	template _UCXXEXPORT ostream & ostream::operator<<(double f);
	template _UCXXEXPORT ostream & ostream::operator<<(long double f);
	template _UCXXEXPORT ostream & ostream::operator<<(void* p);
	template _UCXXEXPORT ostream & ostream::operator<<(basic_streambuf<char, char_traits<char> >* sb);

#ifdef __UCLIBCXX_EXPAND_CONSTRUCTORS_DESTRUCTORS__

	template _UCXXEXPORT ostream::sentry::sentry(ostream & os);
	template _UCXXEXPORT ostream::sentry::~sentry();

#endif //__UCLIBCXX_EXPAND_CONSTRUCTORS_DESTRUCTORS__

	template _UCXXEXPORT ostream & endl(ostream & os);
	template _UCXXEXPORT ostream & flush(ostream & os);
	template _UCXXEXPORT ostream & operator<<(ostream & out, char c);
	template _UCXXEXPORT ostream & operator<<(ostream & out, const char* c);
	template _UCXXEXPORT ostream & operator<<(ostream & out, unsigned char c);
	template _UCXXEXPORT ostream & operator<<(ostream & out, const unsigned char* c);

#endif


}
