/*
Copyright (c) 2009 Stephen John Bush

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

//mvexceptions are actually in mv.h and this file should be called
//something like mvcheckingmacros.h

#ifndef MVEXCEPTIONS_H
#define MVEXCEPTIONS_H 1

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
//#pragma warning (disable: 4530)

//to insert code at front of every var member function
#define THISIS(OBJECT) \
	static const wchar_t* functionname=OBJECT;

//TRY TO PREVENT var abc=abc+1; at runtime since compilers DONT BLOCK IT!!
//prevent var x=f(x); of any kind since compilers dont block it but runtime behaviour is undefined
//prevent undefined c++ behaviour from syntax like "var x=f(x);"
//risky to allow undefined behaviour
//it produces the expected result on MSVC2005 in debugging mode at least IF there is no string to be copied
//var undefinedassign=undefinedassign=11;
//risk it? to enable speed since copy constuction is so frequent
//var xyz=xyz="xxx";
//Unhandled exception at 0x0065892c in service.exe: 0xC0000005: Access violation writing location 0xcccccccc.
//other words ccould be ISCONSTRUCTED or ISALIVE
#define ISDEFINED(VARNAME) \
	if (VARNAME.var_mvtyp&mvtypemask) \
	throw MVUndefined(var(#VARNAME) ^ L" in " ^ var(functionname)); \

//includes isdefined
#define ISASSIGNED(VARNAME) \
	ISDEFINED(VARNAME) \
	if (!VARNAME.var_mvtyp) \
	throw MVUnassigned(var(#VARNAME) ^ L" in " ^ var(functionname)); \

//includes isdefined directly and checks assigned if not string
#define ISSTRING(VARNAME) \
	ISDEFINED(VARNAME) \
	if (!(VARNAME.var_mvtyp&pimpl::MVTYPE_STR)) \
	{ \
		if (!VARNAME.var_mvtyp) \
		throw MVUnassigned(var(#VARNAME) ^ L" in " ^ var(functionname)); \
		VARNAME.createString(); \
	}; \

//includes isassigned
#define ISNUMERIC(VARNAME) \
	ISASSIGNED(VARNAME) \
	if (!VARNAME.isnum()) \
	throw MVNonNumeric(var(functionname) ^ L" : " ^ var(#VARNAME) ^ L" is " ^ VARNAME.substr(1,20).quote()); \

//see long comment on ISDEFINED
#define THISISDEFINED() \
	if (!this||(*this).var_mvtyp&mvtypemask) \
		throw MVUndefined(L"var in " ^ var(functionname)); \

//includes isdefined
#define THISISASSIGNED() \
	THISISDEFINED() \
	if (!(*this).var_mvtyp) \
		throw MVUnassigned(L"var in " ^ var(functionname)); \

//includes isdefined directly and checks assigned if not string
#define THISISSTRING() \
	THISISDEFINED() \
	if (!((*this).var_mvtyp&pimpl::MVTYPE_STR)) \
	{ \
		if (!(*this).var_mvtyp) \
			throw MVUnassigned(L"var in " ^ var(functionname)); \
		(*this).createString(); \
	}; \

//includes isassigned
#define THISISNUMERIC() \
	THISISASSIGNED() \
	if (!(*this).isnum()) \
	throw MVNonNumeric(var(functionname) ^ L" : var is " ^ (*this).substr(1,20).quote()); \

#endif /*MVEXCEPTIONS_H*/
