#if EXO_MODULE > 1
	import std;
#else
#endif
////#include <wctype.h>
//
//#if defined(_MSC_VER)
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#else
//#include <string.h>
//#endif
//
//#include <locale.h>
//#ifdef __APPLE__
//#include <xlocale.h>
////#include <CoreFoundation/CFBase.h>
//#include <CoreFoundation/CFString.h>
////#include <CoreFoundation/CFStringEncodingExt.h>
//#endif

#include <exodus/var.h>
#include <clocale> // for LC_ALL

namespace exo {

bool var_os::setxlocale(const char* newlocalecode ) {

	THISIS("bool var::setxlocale(const char* newlocalecode) static")
//	assertString(function_sig);

	// make a thread local locale if not done already
	// TODO do this in thread creation
	// TODO destroy threalocale in thread destruction *OTHERWISE MEMORY LEAK
	// to avoid checking on every setxlocale
//	if (uselocale(nullptr) == uselocale(LC_GLOBAL_LOCALE))
//		uselocale(duplocale(uselocale(LC_GLOBAL_LOCALE)));

	// TODO make thread safe
	if (std::setlocale(LC_ALL, newlocalecode) == nullptr) {
		var::setlasterror(var(function_sig) ^ "Error: '" ^ newlocalecode ^ "' is invalid");
	}
	return true;

//#if defined(_MSC_VER) && defined(UNICODE)
//
//	//http://msdn.microsoft.com/en-us/library/dd374051%28v=VS.85%29.aspx
//	////locale list
//	////XP 2003 http://msdn.microsoft.com/en-us/goglobal/bb895996.aspx
//	////http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
//	//BOOL SetThreadLocale(
//	//	__in  LCID Locale
//	//);
//	//
//	// GetSystemDefaultLCID()
//	// GetUserDefaultLCID()
//	// LCID locale_lcid=1031;//German Standard
//	// LCID locale_lcid=1032;//Greek
//	// LCID locale_lcid=1055;//Turkish
//
//	return SetThreadLocale((*this).toInt()) != nullptr;
//
//#else

//#endif
}

var  var_os::getxlocale() {

	THISIS("out  var::getxlocale() static")

#if defined(_MSC_VER) && defined(UNICODE)
	return var(static_cast<int>GetThreadLocale());
#else
	// cppref: Make a "deep copy" of current locale name.
	return std::setlocale(LC_ALL, nullptr);
#endif
}

}  // namespace exo
