/***************************** PDI STD File ***********************************
This file is third party source, taken from the open source uClibc++ library and
used here with modifications. Thanking to author for providing this .

for the license terms of this file refer the source link below.

referred from   : https://github.com/mike-matera/ArduinoSTL
added Date      : 1st Dec 2024
added by        : Suraj I.
******************************************************************************/

#include "cstdlib"
#include "cstring"
#include "func_exception"

//This is a system-specific header which does all of the error-handling management
#include "unwind-cxx.h"

//The following functionality is derived from reading of the GNU libstdc++ code and making it...simple


namespace __cxxabiv1{

static __UCLIBCXX_TLS __cxa_eh_globals eh_globals;

extern "C" __cxa_eh_globals* __cxa_get_globals() _UCXX_USE_NOEXCEPT{
	return &eh_globals;
}

extern "C" __cxa_eh_globals* __cxa_get_globals_fast() _UCXX_USE_NOEXCEPT{
	return &eh_globals;
}

}
