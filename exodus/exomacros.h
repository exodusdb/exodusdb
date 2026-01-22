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

#ifndef LIBEXODUS_EXOMACROS_H_
#define LIBEXODUS_EXOMACROS_H_

// This file provides various macros to make exodus application programming
// look simpler. Sadly, C++ macros cannot be declared within namespaces
// and pollute the global namespace. However it is considered so useful to
// hide C++ syntax from the application programmer that we put them in this
// header file.
// You can program without exodus macros by including var.h directly

#if EXO_MODULE
	import var;
#else
#	include <var/var.h>
#	include <exodus/exocallable.h>
#endif

// Flag to force exodus/format.h to be included BEFORE other exodus headers and their intrusive macros (e.g. ev)
#define EXODUS_MACROS

// Order of C++ standard includes
// ==============================
//
// Please include <exodus> AFTER <iostream> and other c/c++ headers For example:
//
//  #include <iostream>
//  #include <exodus>
//
// OR insert "#undef eq" after #include <exodus> etc. For example:
//
//  #include <exodus>
//  #undef eq
//
// OR insert #undef eq before #include <iostream> etc. For example:
//
//  #undef eq
//  #include <iostream>
//
// You can regain the use of the keyword "eq" by inserting the following after any other includes
//
//  #define eq ==
//
// Regrettably "eq" is defined in global namespace in some libraries and is used in Exodus too


// "Function" and "Subroutine"
// ===========================
// Allow simplified syntax eg "function xyz(in arg1, out arg2) { ..."
//
//#undef function
//#undef subroutine
//#undef func
//#undef subr
//#undef subroutine_noreturn

//// TODO move to exoprog
//using function = var;
//using subroutine = void;
//using func = var;
//using subr = void;
// Replaced by "using" in exoprog
//#define function public: var
//#define subroutine public: void
//#define func public: var
//#define subr public: void
//#define subroutine_noreturn public: [[noreturn]] void

// "Call" and "Gosub"
// ==================
//
// Throw away words indicate calling internal or external functions
//
// Internal function = member functions of current Exodus Program
// External function = member function "main" of Exodus Programs in dynamically loaded shared libraries
//
// call xyz() ... is just xyz()  (external function)
// gosub xyz() ... is just xyz() (local function)
//
#define call
#define gosub


// Named Common
// ===============
// Utility macro to test if a "Named common" is available.
//
#define iscommon(COMMONNAME) ((&COMMONNAME)!=nullptr)


// Parameter types in, io, out
// ===========================
//
// Simplify declaration of function/subroutine arguments
//
//   function xyz(in arg1, out arg2)
//
// Instead of:
//
//  function xyz(in arg1, io arg2)"
//
// Note: io and out parameters must be variables not constants.
//
// The above function *cannot* be called with a string or number for the 2nd argument
//
//  abc=xyz(100,200); // This will not compile.
//
// Declared in exodus namespace which is useful since "in" and "out" could easily
// occur in other libraries.
//
namespace exo {
	using in = const var&;  // CVR;
	using io = var&;        // VARREF;
	using out = var&;       // VARREF;
}


// Alphabetic operators eq/ne lt/gt le/ge
// ======================================
// Forcibly redefine "eq" even if already previously defined in some other library like iostream
// to generate a compilation error so that the issue can be corrected (see heading) and the "eq"
// keyword remain available
//
// Note: https://gcc.gnu.org/onlinedocs/cpp/System-Headers.html
// Macros defined in a system header are immune to a few warnings wherever they are expanded.
// (g++9 seems to treat /usr/local/include (where exodus includes are installed) as system headers
//
#define eq ==
// alternative
#define EQ ==
#define ne !=
#define lt <
#define gt >
#define le <=
#define ge >=


// Meaningful commas
// =================
// Experimental. Included last so least likely to interfere with other heads
// Allow syntax like "read(rec from file)" and "convert(a to b)"
// Sadly "in" cannot be used since it is already used for "cnst var&"
#define on ,
#define from ,
#define with ,
#define to ,

// Various
// =======
#define LISTACTIVE hasnext()
#define CALLMEMBERFUNCTION(object, ptrToMember) ((object).*(ptrToMember))

#endif  // LIBEXODUS_EXOMACROS_H_
