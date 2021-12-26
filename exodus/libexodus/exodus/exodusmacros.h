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

#ifndef EXODUSMACROS_H
#define EXODUSMACROS_H 1

// this file provides a few macros to make exodus application programming
// look simpler. sadly, in c++, macros cannot be declared within namespaces
// and pollute the global namespace. However it is considered so useful to
// hide c++ syntax from the application programmer that we put them in this
// header file.
// you can program without exodus macros cause conflict then you can
// include m.h directly

#include <exodus/mv.h>
#include <exodus/mvfunctor.h>

// please include <exodus> AFTER <iostream> etc. For example:
// #include <iostream>
// #include <exodus>
// OR insert "#undef eq" after #include <exodus> etc. For example:
// #include <exodus>
// #undef eq
// OR insert #undef eq before #include <iostream> etc. For example:
// #undef eq
// #include <iostream>
// you can regain the use of the keyword "eq" by inserting the following after any other includes
// #define eq ==
//(regrettably eq is defined in global namespace in some libraries)

// allow pseudo pick syntax
#define sentence() SENTENCE

// for dll/so determine how functions are to be exported without name mangling
#ifndef EXODUS_LINK_MAPORDEF
#define EXODUS_LINK_MAPORDEF 0
#endif

#if EXODUS_LINK_MAPORDEF == 0
#define EXODUS_EXTERN_C extern "C"
// disable the following warning because seems it can be ignored at least in MSVC 2005 32bit
// warning C4190: 'xyz' has C-linkage specified, but returns UDT 'exodus::var' which is incompatible
// with C
#endif

// work out if any functions are being exported or imported
// used in function and subroutine macros
// also perhaps need to avoid "gnu export all" performance issue
// http://gcc.gnu.org/wiki/Visibility
#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#if defined _DLL || defined _SO
#ifdef __GNUC__
#define EXODUSMACRO_IMPORTEXPORT EXODUS_EXTERN_C __attribute__((dllexport))
#else
#define EXODUSMACRO_IMPORTEXPORT \
	EXODUS_EXTERN_C __declspec(  \
		dllexport)	// Note: actually gcc seems to also support this syntax.
#endif
#else
#ifdef __GNUC__
#define EXODUSMACRO_IMPORTEXPORT __attribute__((dllimport))
#else
#define EXODUSMACRO_IMPORTEXPORT \
	__declspec(dllimport)  // Note: actually gcc seems to also support this syntax.
#endif
#endif
#else
#if __GNUC__ >= 4
// use g++ -fvisibility=hidden to make all hidden except those marked DLL_PUBLIC ie "default"
#define EXODUSMACRO_IMPORTEXPORT EXODUS_EXTERN_C __attribute__((visibility("default")))
#define DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define EXODUSMACRO_IMPORTEXPORT EXODUS_EXTERN_C
#define DLL_LOCAL
#endif
#endif

// allow simplified syntax eg "function xyz(in arg1, out arg2) { ..."
#define subroutine EXODUSMACRO_IMPORTEXPORT void
#define function EXODUSMACRO_IMPORTEXPORT var

// throw away words
// call xyz() ... is just xyz()  (external function)
// gosub xyz() ... is just xyz() (local function)
#define call
#define gosub

// class init
// allow multiple named "exodus classes" useful for dictionary item programs where each dict item
// becomes a separate class with its own "main" entry point
#define classinit(CLASSNAME) \
	class CLASSNAME##ExodusProgram : public ExodusProgramBase {

// class exit
// insert a constructor function accepting an mvenvironment just before the class exit
// and then insert the class termination "};"
#define classexit(CLASSNAME)                                               \
   public:                                                                 \
	CLASSNAME##ExodusProgram(MvEnvironment& mv) : ExodusProgramBase(mv) {} \
	}                                                                      \
	;

#define iscommon(COMMONNAME) ((&COMMONNAME) != NULL)

// simplify declaration of function/subroutine arguments
// eg. allow "function xyz(in arg1, out arg2)"
// instead of "function xyz(const var& arg1, var& arg2)"
// NB out parameters must be variables not constants
// so the above function *cannot* be called with a string or number for the 2nd argument
// abc=xyz(100,200);//wont compile
// Can be declared in exodus namespace which is useful since "in" and "out" could easily
// occur in other libraries.
namespace exodus {
using in = const var&;
using io = var&;
using out = var&;
}  // namespace exodus

// forcibly redefine "eq" even if already previously defined in some other library like iostream
// to generate a compilation error so that the issue can be corrected (see heading) and the "eq"
// keyword remain available
#define eq ==
// alternative
#define EQ ==
#define ne !=
#define gt >
#define lt <
#define le <=
#define ge >=

//perhaps a little crazy but included last so should not interfer with other heads
//allow syntax like "read(rec from file)" and "convert(a to b)"
#define on ,
#define from ,
#define with ,
#define to ,

// narrow char* versions of classic pick delimiters
//_RM_, _RM and RM_ versions (wchar*, char* and wchar respectively)
//#define _RM "\u01FF"
//#define _FM "\u01FE"
//#define _VM "\u01FD"
//#define _SM "\u01FC"
//#define _TM "\u01FB"
//#define _STM "\u01FA"

// aliases for different multivalue implementations
//#define _IM _RM
//#define _AM _FM
//#define _SVM _SM

//#define _DQ "\""
//#define _SQ "\'"

#define ID mv.ID
#define RECORD mv.RECORD
#define DICT mv.DICT
#define ANS mv.ANS
#define MV mv.MV
#define PSEUDO mv.PSEUDO
#define DATA mv.DATA
//#define LISTACTIVE mv.LISTACTIVE
#define LISTACTIVE hasnext()

#define USERNAME mv.USERNAME
#define APPLICATION mv.APPLICATION
#define SENTENCE mv.SENTENCE
#define CHAIN mv.CHAIN

#define USER0 mv.USER0
#define USER1 mv.USER1
#define USER2 mv.USER2
#define USER3 mv.USER3
#define USER4 mv.USER4

#define RECUR0 mv.RECUR0
#define RECUR1 mv.RECUR1
#define RECUR2 mv.RECUR2
#define RECUR3 mv.RECUR3
#define RECUR4 mv.RECUR4

#define EXECPATH mv.EXECPATH
#define COMMAND mv.COMMAND
#define OPTIONS mv.OPTIONS

#define DEFINITIONS mv.DEFINITIONS
#define SECURITY mv.SECURITY
#define SYSTEM mv.SYSTEM
#define SESSION mv.SESSION
#define ROLLOUTFILE mv.ROLLOUTFILE
#define THREADNO mv.THREADNO

#define INTERNALCHARS mv.INTERNALCHARS
#define EXTERNALCHARS mv.EXTERNALCHARS
#define LOWERCASE mv.LOWERCASE
#define UPPERCASE mv.UPPERCASE

#define STATION mv.STATION
#define DATEFMT mv.DATEFMT
#define BASEFMT mv.BASEFMT
#define PRIVILEGE mv.PRIVILEGE
#define FILES mv.FILES
#define TCLSTACK mv.TCLSTACK
#define INTCONST mv.INTCONST
#define STATUS mv.STATUS
#define COL1 mv.COL1
#define COL2 mv.COL2
#define PRIORITYINT mv.PRIORITYINT
#define FILEERRORMODE mv.FILEERRORMODE
#define FILEERROR mv.FILEERROR

#define RECCOUNT mv.RECCOUNT

#define AW mv.AW
#define EW mv.EW
#define HW mv.HW
#define MW mv.MW
#define PW mv.PW
#define SW mv.SW
#define VW mv.VW
#define XW mv.XW

#define CRTHIGH mv.CRTHIGH
#define CRTWIDE mv.CRTWIDE
#define LPTRHIGH mv.LPTRHIGH
#define LPTRWIDE mv.LPTRWIDE

// obsolete
#define ENVIRONKEYS mv.ENVIRONKEYS
#define ENVIRONSET mv.ENVIRONSET
#define DEFAULTSTOPS mv.DEFAULTSTOPS
#define MOVEKEYS mv.MOVEKEYS

#define INDEXTIME mv.INDEXTIME

#define TERMINAL mv.TERMINAL
#define LEVEL mv.LEVEL
#define VOLUMES mv.VOLUMES

#define PROCESSNO mv.PROCESSNO
#define CURSOR mv.CURSOR

#endif	// EXODUSMACROS_H
