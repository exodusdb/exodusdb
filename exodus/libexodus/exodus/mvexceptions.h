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

// mvexceptions are actually in mv.h and this file should be called
// something like mvcheckingmacros.h

#ifndef MVEXCEPTIONS_H
#define MVEXCEPTIONS_H 1

/* not using this c++20 feature until established if it requires a runtime overhead or not
//https://en.cppreference.com/w/cpp/utility/source_location/function_name
#if defined __has_include
#  if __has_include (<source_location>)
#    include <sourcelocation>
#  endif
#endif
*/

// C4530: C++ exception handler used, but unwind semantics are not enabled.
//#pragma warning (disable: 4530)

// to insert code at front of every var member function
#define THISIS(OBJECT) static const char* functionname = OBJECT;

// TRY TO PREVENT var abc=abc+1; at runtime since compilers DONT BLOCK IT!!
// prevent var x=f(x); of any kind since compilers dont block it but runtime behaviour is undefined
// prevent undefined c++ behaviour from syntax like "var x=f(x);"
// risky to allow undefined behaviour
// it produces the expected result on MSVC2005 in debugging mode at least IF there is no string to
// be copied var undefinedassign=undefinedassign=11; risk it? to enable speed since copy constuction
// is so frequent var xyz=xyz="xxx"; Unhandled exception at 0x0065892c in service.exe: 0xC0000005:
// Access violation writing location 0xcccccccc. other words ccould be ISCONSTRUCTED or ISALIVE
#ifndef EXO_NOCHECKDEFINED
#define ISDEFINED(VARNAME)                                                                     \
	if (VARNAME.var_typ & VARTYP_MASK)                                                         \
		throw MVUndefined(var(#VARNAME) ^ " in " ^ var(functionname));
#else
#define ISDEFINED(VARNAME) {}
#endif

// includes isdefined
#define ISASSIGNED(VARNAME)                                                                        \
	ISDEFINED(VARNAME)                                                                         \
	if (!VARNAME.var_typ)                                                                      \
		throw MVUnassigned(var(#VARNAME) ^ " in " ^ var(functionname));

// includes isdefined directly and checks assigned if not string
#define ISSTRING(VARNAME)                                                                          \
	ISDEFINED(VARNAME)                                                                         \
	if (!(VARNAME.var_typ & VARTYP_STR))                                                       \
	{                                                                                          \
		if (!VARNAME.var_typ)                                                              \
			throw MVUnassigned(var(#VARNAME) ^ " in " ^ var(functionname));            \
		VARNAME.createString();                                                            \
	};

// includes isassigned which includes ISDEFINED
#define ISNUMERIC(VARNAME)                                                                         \
	ISASSIGNED(VARNAME)                                                                        \
	if (!VARNAME.isnum())                                                                      \
		throw MVNonNumeric(var(functionname) ^ " : " ^ var(#VARNAME) ^ " is " ^            \
				   VARNAME.substr(1, 20).quote());

// in some bizarre case we cant show contents of variable so put the following line instead of the
// last one above 	throw MVNonNumeric(var(functionname) ^ " : " ^ var(#VARNAME));

// see long comment on ISDEFINED
#define THISISDEFINED()                                                                            \
	/*std::cout<< functionname << " " <<var_typ<<std::endl;*/                                  \
	if (/*(!this) ||*/ this->var_typ & VARTYP_MASK)                                            \
		throw MVUndefined("var in " ^ var(functionname));

// includes isdefined
#define THISISASSIGNED()                                                                           \
	THISISDEFINED()                                                                            \
	if (!this->var_typ)                                                                        \
		throw MVUnassigned("var in " ^ var(functionname));

// includes isdefined directly and checks assigned if not string
#define THISISSTRINGMUTATOR()                                                                      \
	THISISSTRING()                                                                             \
	this->var_typ=VARTYP_STR;//reset all flags

// includes isdefined directly and checks assigned if not string
#define THISISSTRING()                                                                             \
	THISISDEFINED()                                                                            \
	if (!(this->var_typ & VARTYP_STR))                                                         \
	{                                                                                          \
		if (!this->var_typ)                                                                \
			throw MVUnassigned("var in " ^ var(functionname));                         \
		this->createString();                                                              \
	};

// includes isassigned which includes ISDEFINED
#define THISISNUMERIC()                                                                            \
	THISISASSIGNED()                                                                           \
	if (!this->isnum())                                                                        \
		throw MVNonNumeric(var(functionname) ^ " : var is " ^ this->substr(1, 20).quote());

// includes isassigned which includes ISDEFINED
#define THISISDECIMAL()                                                                            \
	THISISASSIGNED()                                                                           \
	if (!this->isnum())                                                                        \
		throw MVNonNumeric(var(functionname) ^ " : var is " ^                              \
				   this->substr(1, 20).quote());                                   \
	if (!(var_typ & VARTYP_DBL))                                                               \
	{                                                                                          \
		var_dbl = double(var_int);                                                         \
		var_typ |= VARTYP_DBL;                                                             \
	}

// includes isassigned which includes ISDEFINED
#define THISISINTEGER()                                                                            \
	THISISASSIGNED()                                                                           \
	if (!this->isnum())                                                                        \
		throw MVNonNumeric(var(functionname) ^ " : var is " ^                              \
				   this->substr(1, 20).quote());                                   \
	if (!(var_typ & VARTYP_INT))                                                               \
	{                                                                                          \
		var_int = mvint_t(var_dbl);                                                        \
		var_typ |= VARTYP_INT;                                                             \
	}

#endif /*MVEXCEPTIONS_H*/
