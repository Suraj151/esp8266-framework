/***************************** PDI STD File ***********************************
This file is third party source, taken from the open source uClibc++ library and
used here with modifications. Thanking to author for providing this .

for the license terms of this file refer the source link below.

referred from   : https://github.com/mike-matera/ArduinoSTL
added Date      : 1st Dec 2024
added by        : Suraj I.
******************************************************************************/

#include "exception"
#include "stdexcept"

#ifdef __UCLIBCXX_EXCEPTION_SUPPORT__

namespace pdistd{

	_UCXXEXPORT logic_error::logic_error() _UCXX_USE_NOEXCEPT : mstring(){

	}

	_UCXXEXPORT logic_error::logic_error(const string& what_arg) : mstring(what_arg){

	}

	_UCXXEXPORT const char * logic_error::what() const _UCXX_USE_NOEXCEPT{
		return mstring.c_str();
	}


	_UCXXEXPORT out_of_range::out_of_range() : logic_error(){

	}

	_UCXXEXPORT out_of_range::out_of_range(const string & what_arg) : logic_error(what_arg) {
	
	}

	_UCXXEXPORT runtime_error::runtime_error() : mstring(){

	}

	_UCXXEXPORT runtime_error::runtime_error(const string& what_arg) : mstring(what_arg){

	}

	_UCXXEXPORT const char * runtime_error::what() const _UCXX_USE_NOEXCEPT{
		return mstring.c_str();
	}

}

#endif

