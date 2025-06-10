/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef EXOIMPL_H
#define EXOIMPL_H

#define BACKTRACE_MAXADDRESSES 100

//#undef eq
//module #include <iostream>

// Using map for dllib cache instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// Perhaps switch to this https://youtu.be/M2fKMP47slQ?t=476
//#include <unordered_map>
//module #include <map>

#include <version> // Only for __cpp_lib_constexpr_string >= 201907L TODO remove

#if EXO_MODULE
	import std;
	import var;
#else
#	include <exodus/var.h>
//#	include <exodus/dim.h>
//#	include <exodus/rex.h>
#endif

#include <exodus/vardefs.h> // for PUBLIC

    using VAR    =       exo::var;
    using VARREF =       exo::var&;
    using CVR    = const exo::var&;
    using TVR    =       exo::var&&;

    using in     = const exo::var&;
    using out    =       exo::var&;
    using io     =       exo::var&;

//// Visibility
////
//// If using g++ -fvisibility=hidden to make all hidden except those marked PUBLIC ie "default"
//// "Weak" template functions seem to get excluded if visiblity is hidden, despite being marked as PUBLIC
//// so we explictly instantiate them as non-template functions with "template<> ..." syntax.
//// nm -C *so |&grep -F "exo::var_base<exo::var_mid<exo::var> >::"
//// nm -D libexodus.so --demangle |grep T -w
//#define PUBLIC __attribute__((visibility("default")))
//
//using SV = std::string_view;
//
//// constinit/consteval where possible otherwise constexpt
////
//// constinit https://en.cppreference.com/w/cpp/language/constinit
////
//// constinit - asserts that a variable has static initialization,
//// i.e. zero initialization and constant initialization, otherwise the program is ill-formed.
////
//// The constinit specifier declares a variable with static or thread storage duration.
//// If a variable is declared with constinit, its initializing declaration must be applied with constinit.
//// If a variable declared with constinit has dynamic initialization
//// (even if it is performed as static initialization), the program is ill-formed.
//// If no constinit declaration is reachable at the point of the initializing declaration,
//// the program is ill-formed, no diagnostic required.
////
//// constinit cannot be used together with constexpr.
//// When the declared variable is a reference, constinit is equivalent to constexpr.
//// When the declared variable is an object, constexpr mandates that the object must
//// have static initialization and constant destruction and makes the object const-qualified,
//// however, constinit does not mandate constant destruction and const-qualification.
//// As a result, an object of a type which has constexpr constructors and no constexpr destructor
//// (e.g. std::shared_ptr<T>) might be declared with constinit but not constexpr.
//
//// Make var constinit/constexpr if std::string is constexpr (c++20 but g++-12 has some limitation)
////
//#if __cpp_lib_constexpr_string >= 201907L
//#	define CONSTEXPR constexpr
//#	define CONSTINIT_OR_CONSTEXPR constinit const // const because constexpr implies const
//
//#if ( __GNUC__  >= 13 ) || ( __clang_major__ > 1)
//#		define CONSTINIT_VAR constinit
//#	else
//		// Ubuntu 22.04 g++12 doesnt support constinit var
//#		define CONSTINIT_VAR
//#	endif
//
//#else
//#	define CONSTEXPR
//#	define CONSTINIT_OR_CONSTEXPR constexpr
//#	define CONSTINIT_VAR
//#endif
//
//#if __cpp_consteval >= 201811L
//#	define CONSTEVAL_OR_CONSTEXPR consteval
//#else
//#	define CONSTEVAL_OR_CONSTEXPR constexpr
//#endif
//
//// [[likely]] [[unlikely]]
////
//#if __has_cpp_attribute(likely)
//#	define LIKELY [[likely]]
//#	define UNLIKELY [[unlikely]]
//#else
//#	define LIKELY
//#	define UNLIKELY
//#endif
//
//// [[nodiscard]]
////
//#define ND [[nodiscard]]

namespace exo {

	PUBLIC extern const var RM;
	PUBLIC extern const var FM;
	PUBLIC extern const var VM;
	PUBLIC extern const var SM;
	PUBLIC extern const var TM;
	PUBLIC extern const var STM;

	PUBLIC extern const var BS;
	PUBLIC extern const var DQ;
	PUBLIC extern const var SQ;
	PUBLIC extern const var NL;

	PUBLIC extern const char* const _OS_NAME;
	PUBLIC extern const char* const _OS_VERSION;

	PUBLIC void debug(in = "");
	PUBLIC auto exo_savestack(void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t* stack_size) -> bool;
 ND PUBLIC auto exo_backtrace( void* stack_addresses[BACKTRACE_MAXADDRESSES], std::size_t stack_size, std::size_t limit = 0) -> std::string;

	// Set by signals for threads to poll
	PUBLIC inline bool TERMINATE_req = false;
	PUBLIC inline bool RELOAD_req = false;

	// clang-format off

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wreserved-identifier"

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__

#	define                     _EOL        "\r\n"
	PUBLIC inline const var     EOL      = _EOL;

#	define              _OSSLASH    "\\"
	PUBLIC inline const var     OSSLASH  = _OSSLASH;

#	define               OSSLASH_   '\\'

#	define               OSSLASH_IS_BACKSLASH true

#else

#	define              _EOL        "\n"
	PUBLIC inline const var     EOL      = _EOL;

#	define              _OSSLASH    "/"
	PUBLIC inline const var     OSSLASH  = _OSSLASH;
//inline CONSTINIT_OR_CONSTEXPR var     OSSLASH  = _OSSLASH;

#	define               OSSLASH_   '/'
#	define               OSSLASH_IS_BACKSLASH false

#endif

	// _cplusplus is in format YYYYMM e.g. 202002, 202102, 202302 etc.
	// We will extract the two digit year only - using integer division and integer remainder ops.
	// Years e.g. 21 which are in between the actual standards like c++20, c++23, c++26 etc.
	// indicate partial informal support for some features of the next standard
	// 20 c++20
	// 21 some c++23
	// 23 c++23
	// 24 some c++26


//#pragma message "CONSTINIT_OR_CONSTEXPR == " DUMPDEFINE(CONSTINIT_OR_CONSTEXPR)

	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _CPP_STANDARD=__cplusplus / 100 % 1000;

#if defined(_WIN64) or defined(_LP64)
#	define                    _PLATFORM   "x64"
#else
#	define                    _PLATFORM   "x86"
#endif
	PUBLIC inline const var    PLATFORM = _PLATFORM;

#ifdef __clang__
	//__clang_major__
	//__clang_minor__
	//__clang_patchlevel__
	//__clang_version__
	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _COMPILER         =  "clang";
	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _COMPILER_VERSION =  __clang_major__;
#elif defined(__GNUC__)
	//__GNUC__
	//__GNUC_MINOR__
	//__GNUC_PATCHLEVEL__
	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _COMPILER         =  "gcc";
	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _COMPILER_VERSION =  __GNUC__;
#else
	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _COMPILER         =  "unknown";
	PUBLIC inline CONSTINIT_OR_CONSTEXPR auto _COMPILER_VERSION = 0;
#endif

#pragma clang diagnostic pop

// clang-format on

}

#endif //EXOIMPL_H
