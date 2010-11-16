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


//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)
#include <iostream>

//TODO: check that "-" and "-." is NOT numeric

//TODO make mvtype a bit field indicating multiple types present
//or overlap the string/integer/float variables to save space

#include <sstream>

#define MV_NO_NARROW

#define EXO_MV_CPP //indicates globals are to be defined (omit extern keyword)
#include <exodus/mv.h>
#include <exodus/mvimpl.h>
#include <exodus/mvutf.h>
#include <exodus/mvexceptions.h>

namespace exodus {

//most help from Thinking in C++ Volume 1 Chapter 12
//http://www.camtp.uni-mb.si/books/Thinking-in-C++/TIC2Vone-distribution/html/Chapter12.html

//Also - very clearly written
//Addison Wesley Longman, Inc.
//C++ Primer, Third Edition 1998 Stanley B Lippman
//Chapter 15 Overloaded operators and User-Defined Conversions

//could also use http://www.informit.com/articles/article.asp?p=25264&seqNum=1
//which is effectively about makeing objects behave like ordinary variable syntactically
//implementing smartpointers

//NOTE
//if priv variables are moved directly into the var object then
// then mvtype etc should be initialised in proper initialisers to gain performance in those compilers
//that initialise basic types. ie prevent initialisation AND assignment
//currently the default priv object initialisation of mvint to 0 is inefficient since is ALSO assigned in most var constructions

//DESTRUCTOR
/////////////
var::~var()
{
	//not a pimpl style pointer anymore for speed
	//delete priv;

	//try to ensure any memory is not later recognises as initialised memory
	//(exodus tries to detect undefined use of uninitialised objects at runtime - that dumb compilers allow without warning)
	//this could be removed in production code perhaps
	//set all unused bits to 1 to ease detection of usage of uninitialised variables (bad c++ syntax like var x=x+1;
	//set all used bits to 0 to increase chance of detecting unassigned variables
	var_mvtyp=(wchar_t)0xFFFFFFF0;
}

//CONSTRUCTORS
//////////////

//default ctor to allow definition unassigned "var mv";
var::var()
: var_mvtyp(pimpl::MVTYPE_UNA)
{
	//int xyz=3;
	//WARNING neither initialisers nor constructors are called in the following case !!!
	//var xxx=xxx.somefunction()
	//known as "undefined usage of uninitialised variable";
	//and not even a compiler warning in msvc8 or g++4.1.2

	//so the following test is put everywhere to protect against this type of accidental programming
	//if (var_mvtyp&mvtypemask)
	//	throw MVUndefined(L"funcname()");
	//should really ensure a magic number and not just HOPE for some binary digits above bottom four 0-15 decimal 1111binary
	//this could be removed in production code perhaps

	//debuggCONSTRUCT&&wcout<<L"CONSTRUCT: var()\n";

	//not a pointer anymore for speed
	//priv=new pimpl;

	//moved here from pimpl ctor
	//moved up to initializer
	//var_mvtyp=pimpl::MVTYPE_UNA;

}

//copy ctor
//dont use initializers - check if copiedvar is assigned first
//(could initialise and check after but this feels bad due to loss of target)
var::var(const var& copiedvar)
{
	THISIS(L"var::var(const var& copiedvar)")

	//do first since initializer is copiedvar
	ISASSIGNED(copiedvar)

	//not a pointer anymore for speed
	//priv=new pimpl;

	//identical in copy ctor and load and call
	var_mvtyp=copiedvar.var_mvtyp;
	var_mvstr=copiedvar.var_mvstr;
	var_mvint=copiedvar.var_mvint;
	var_mvdbl=copiedvar.var_mvdbl;
}

//ctor for wchar_t
//would just use initializers since cannot fail
//(except cannot seem to init wstring from wchar_t!)
var::var(const wchar_t char1)
{
	var_mvstr=char1;
	var_mvtyp=pimpl::MVTYPE_STR;
}

//ctor for wide c_str
//use initializers since cannot fail
var::var(const wchar_t* cstr1)
{

	//not a pointer anymore for speed
	//priv=new pimpl;

	//protect against null pointer
	if (cstr1==0)
	{
		//THISIS(L"var::var(const wchar_t* cstr1)")
		throw MVInvalidPointer(L"Null pointer in var(const char*)");
	}

	var_mvstr=cstr1;
	var_mvtyp=pimpl::MVTYPE_STR;
}

//ctor for std::wstring
//just use initializers since cannot fail
var::var(const std::wstring& str1)
	: var_mvstr(str1)
	, var_mvtyp(pimpl::MVTYPE_STR)
{}

//ctor for std::string
//just use initializers since cannot fail
var::var(const std::string& str1)
	: var_mvstr(wstringfromUTF8((UTF8*)str1.data(),(int)str1.length()))
	, var_mvtyp(pimpl::MVTYPE_STR)
{}

//ctor for bool
//just use initializers since cannot fail
var::var(const bool bool1)
	: var_mvint(bool1)
	, var_mvtyp(pimpl::MVTYPE_INT)
{}

//ctor for int
//just use initializers since cannot fail
var::var(const int int1)
	: var_mvint(int1)
	, var_mvtyp(pimpl::MVTYPE_INT)
{}

//ctor for long long
//just use initializers since cannot fail
var::var(const long long longlong1)
	: var_mvint(longlong1)
	, var_mvtyp(pimpl::MVTYPE_INT)
{}

//ctor for double
//just use initializers since cannot fail
var::var(const double double1)
	: var_mvdbl(double1)
	, var_mvtyp(pimpl::MVTYPE_DBL)
{}

//EXPLICIT AND AUTOMATIC CONVERSIONS
////////////////////////////////////

//someone recommends not to create more than one automatic converter
//to avoid the compiler error "ambiguous conversion"
//(explicit means it is not automatic)

//allow conversion to string (IS THIS USED FOR ANYTHING AT THE MOMENT?
//allows the usage of any string function
//operator const std::wstring()
   	//{
//	//debuggCONVERT&& wcout<<L"CONVERT: operator const std::wstring() returns '"<<var_mvstr<<L"'\n";
//	return var_mvstr;
//}

var::operator void*() const
{
	THISIS(L"var::operator void*() const")
	//could be skipped for speed if can be proved there is no way in c++ syntax that would result in
	//an attempt to convert an uninitialised object to void*
	//since there is a bool conversion when does c++ use automatic conversion to void*
	//note that exodus operator ! uses (void*)
	//trial elimination of operator void* seems to cause no problems but without full regression testing
	THISISDEFINED()

	return (void*) toBool();
}

//supposed to be replaced with automatic void() and made explicit but just seems to force int conversion during "if (var)"
//necessary to allow var to be used standalone in "if (xxx)" but see mv.h for discussion of using void* instead of bool
//#ifndef _MSC_VER
var::operator bool() const
{
	return toBool();
}
//#endif

/*
var::operator const char*() const
{
	return tostring().c_str();
}
*/

var::operator int() const
{
	THISIS(L"var::operator int() const")
	THISISDEFINED()

	do
	{
		//prioritise int since conversion to int perhaps more likely to be an int already
		if (var_mvtyp&pimpl::MVTYPE_INT)
			return (int) var_mvint;
		if (var_mvtyp&pimpl::MVTYPE_DBL)
			return int(var_mvdbl);
		if (var_mvtyp&pimpl::MVTYPE_NAN)
			throw MVNonNumeric(L"int(" ^ substr(1,20) ^ L")");
		if (!(var_mvtyp))
		{
			THISISASSIGNED()
			throw MVUnassigned(L"int(var)");
		}
	}
	//must be string - try to convert to numeric
	while (isnum());

	THISISNUMERIC()
	throw MVNonNumeric(L"int(L" ^ substr(1,20) ^ L")");

}

//remove because causes "ambiguous" with -short_wchar on linux
/*
var::operator unsigned int() const
{
	THISIS(L"var::operator int() const")
	THISISDEFINED()

	do
	{
		//prioritise int since conversion to int perhaps more likely to be an int already
		if (var_mvtyp&pimpl::MVTYPE_INT)
			return var_mvint;
		if (var_mvtyp&pimpl::MVTYPE_DBL)
			return int(var_mvdbl);
		if (var_mvtyp&pimpl::MVTYPE_NAN)
			throw MVNonNumeric(L"int(" ^ substr(1,20) ^ L")");
		if (!(var_mvtyp))
		{
			THISISASSIGNED()
			throw MVUnassigned(L"int(var)");
		}
	}
	//must be string - try to convert to numeric
	while (isnum());

	THISISNUMERIC()
	throw MVNonNumeric(L"int(L" ^ substr(1,20) ^ L")");

}
*/

/*
//necessary to allow use of var inside STL containers
var::operator size_t() const
{
	return (size_t) operator int();
}
*/

/*
var::operator const wchar_t*()
{
	if (var_mvtyp&mvtypemask)
		throw MVUndefined(L"const wchar_t*()");
	wcout<<L"CONVERT: operator const wchar_t*() returns '"<<var_mvstr.c_str()<<L"'\n";
	return var_mvstr.c_str();
}
*/

//UNARY OPERATORS
/////////////////

//=var
//The assignment operator should always return a reference to *this.
//cant be (const var& rhs) because seems to cause a problem with var1=var2 in function parameters
//unfortunately causes problem of passing var by value and thereby unnecessary contruction
//see also ^= etc
var& var::operator = (const var& rhs)
{
	THISIS(L"var& var::operator = (const var& rhs)")
	THISISDEFINED()
	ISASSIGNED(rhs)

	//very important not to self assign!!!
	if (this==&rhs)
		return *this;

	//copy everything across
	var_mvstr=rhs.var_mvstr;
	var_mvdbl=rhs.var_mvdbl;
	var_mvint=rhs.var_mvint;
	var_mvtyp=rhs.var_mvtyp;

	return *this;

}

//=int
//The assignment operator should always return a reference to *this.
var& var::operator = (const int int1)
{
	//THISIS(L"var& var::operator = (const int int1)")
	//protect against unlikely syntax as follows:
	//var undefinedassign=undefinedassign=123';
	// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
	//THISISDEFINED()

	var_mvint=int1;
	var_mvtyp=pimpl::MVTYPE_INT;

	return *this;
}

//=double
//The assignment operator should always return a reference to *this.
var& var::operator = (const double double1)
{
	//THISIS(L"var& var::operator = (const double double1)")
	//protect against unlikely syntax as follows:
	//var undefinedassign=undefinedassign=9.9';
	// !!RISK NOT CHECKING TO SPEED THINGS UP SINCE SEEMS TO WORK IN MSVC AND GCC
	//THISISDEFINED()

	var_mvdbl=double1;
	var_mvtyp=pimpl::MVTYPE_DBL;

	return *this;
}

//=wchar_t
//The assignment operator should always return a reference to *this.
var& var::operator = (const wchar_t char2)
{
	
	THISIS(L"var& var::operator = (const wchar_t char2)")
	//protect against unlikely syntax as follows:
	//var undefinedassign=undefinedassign=L'X';
	//this causes crash due to bad memory access due to setting string that doesnt exist
	//slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()		//ALN:TODO: this definition kind of misleading, try to find
						//ALN:TODO: or change name to something like: THISISNOTDEAD :)
						//ALN:TODO: argumentation: var with mvtyp=0 is NOT defined

	var_mvstr=char2;
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//=wchar_t*
//The assignment operator should always return a reference to *this.
var& var::operator = (const wchar_t* char2)
{
	THISIS(L"var& var::operator = (const wchar_t* char2)")
	//protect against unlikely syntax as follows:
	//var undefinedassign=undefinedassign=L"xxx";
	//this causes crash due to bad memory access due to setting string that doesnt exist
	//slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()
	
	var_mvstr=char2;
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//=std::wstring
//The assignment operator should always return a reference to *this.
var& var::operator = (const std::wstring string2)
{
	
	THISIS(L"var& var::operator = (const wchar_t* char2)")
	//protect against unlikely syntax as follows:
	//var undefinedassign=undefinedassign=std::wstring(L"xxx"";
	//this causes crash due to bad memory access due to setting string that doesnt exist
	//slows down all string settings so consider NOT CHECKING in production code
	THISISDEFINED()
	var_mvstr=string2;
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}
//^= is not templated since slightly slower to go through the creation of an var()

//^=var
//The assignment operator should always return a reference to *this.
var& var::operator ^=(const var& rhs)
{
	THISIS(L"var& var::operator ^=(const var& rhs)")
	THISISSTRING()
	ISSTRING(rhs)

	//tack it onto our string
	var_mvstr+=rhs.towstring();

	//reset to unknown string (clear int/dbl/nan flags)
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//^=int
//The assignment operator should always return a reference to *this.
var& var::operator ^= (const int int1)
{
	THISIS(L"var& var::operator ^= (const int int1)")
	THISISSTRING()

	//var_mvstr+=var(int1).var_mvstr;
	var_mvstr+=intToString(int1);

	//reset to unknown string (clear int/dbl/nan flags)
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//^=double
//The assignment operator should always return a reference to *this.
var& var::operator ^= (const double double1)
{
	THISIS(L"var& var::operator ^= (const double double1)")
	THISISSTRING()

	//var_mvstr+=var(int1).var_mvstr;
	var_mvstr+=dblToString(double1);

	//reset to unknown string (clear int/dbl/nan flags)
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//^=wchar_t
//The assignment operator should always return a reference to *this.
var& var::operator ^= (const wchar_t char1)
{
	THISIS(L"var& var::operator ^= (const wchar_t char1)")
	THISISSTRING()

	//var_mvstr+=var(int1).var_mvstr;
	var_mvstr+=char1;

	//reset to unknown string (clear int/dbl/nan flags)
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//^=wchar_t*
//The assignment operator should always return a reference to *this.
var& var::operator ^= (const wchar_t* char1)
{
	THISIS(L"var& var::operator ^= (const wchar_t* char1)")
	THISISSTRING()

	//var_mvstr+=var(int1).var_mvstr;
	//var_mvstr+=std::wstring(char1);
	var_mvstr+=char1;

	//reset to unknown string (clear int/dbl/nan flags)
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}

//^=std::wstring
//The assignment operator should always return a reference to *this.
var& var::operator ^= (const std::wstring string1)
{
	THISIS(L"var& var::operator ^= (const std::wstring string1)")
	THISISSTRING()

	//var_mvstr+=var(int1).var_mvstr;
	var_mvstr+=string1;

	//reset to unknown string (clear int/dbl/nan flags)
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;
}


//You must *not* make the postfix version return the 'this' object by reference; you have been warned.

//not returning void so is usable in expressions
//int argument indicates that this is POSTFIX override v++
var var::operator ++ (int)
{
	THISIS(L"var var::operator ++ (int)")
	//full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	if (var_mvtyp&pimpl::MVTYPE_INT)
		var_mvint++;
	else if (var_mvtyp&pimpl::MVTYPE_DBL)
		var_mvdbl++;
	else if (var_mvtyp&pimpl::MVTYPE_STR)
	{
		//try to convert to numeric
		if (isnum())
		{
			//turn off string flag because it is about to be obsoleted;
			var_mvtyp=var_mvtyp^pimpl::MVTYPE_STR;
			goto tryagain;
		}

		//throw MVNonNumeric(L"(L" ^ substr(1,20) ^ L")++");
		THISISNUMERIC()
	}
	else
	{
		//throw MVUnassigned(L"var++");
		THISISNUMERIC()
	}

	//NO DO NOT! return *this ... postfix return a temporary!!! eg var(*this)
	return var(*this);

}

//not returning void so is usable in expressions
//int argument indicates that this is POSTFIX override v--
var var::operator -- (int)
{

	THISIS(L"var var::operator -- (int)")
	//full check done below to avoid double checking number type
	THISISDEFINED()

	if (var_mvtyp&mvtypemask)
		throw MVUndefined(L"var--");
tryagain:
	if (var_mvtyp&pimpl::MVTYPE_INT)
		var_mvint--;
	else if (var_mvtyp&pimpl::MVTYPE_DBL)
		var_mvdbl--;
	else if (var_mvtyp&pimpl::MVTYPE_STR)
	{
		//try to convert to numeric
		if (isnum())
		{
			//NB turn off string flag because it is about to be obsoleted;
			var_mvtyp=var_mvtyp^pimpl::MVTYPE_STR;
			goto tryagain;
		}

		//throw MVNonNumeric(L"(L" ^ substr(1,20) ^ L")--");
		THISISNUMERIC()
	}
	else
	{
		//throw MVUnassigned(L"()--");
		THISISNUMERIC()
	}

	//NO DO NOT! return *this ... postfix must return a temporary!!! eg var(*this)
	return var(*this);

}

//not returning void so is usable in expressions
//no argument indicates that this is prefix override ++var
var& var::operator ++ ()
{

	THISIS(L"var var::operator ++ ()")
	//full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	if (var_mvtyp&pimpl::MVTYPE_INT)
		var_mvint++;
	else if (var_mvtyp&pimpl::MVTYPE_DBL)
		var_mvdbl++;
	else if (var_mvtyp&pimpl::MVTYPE_STR)
	{
		//try to convert to numeric
		if (isnum())
		{
			//NB turn off string flag because it is about to be obsoleted;
			var_mvtyp=var_mvtyp^pimpl::MVTYPE_STR;
			goto tryagain;
		}

		//throw MVNonNumeric(L"++(L" ^ substr(1,20) ^ L")");
		THISISDEFINED()
	}
	else
	{
		throw MVUnassigned(L"++()");
		THISISDEFINED()
	}

	//OK to return *this in prefix ++
	return *this;


}

//not returning void so is usable in expressions
//no argument indicates that this is prefix override --var
var& var::operator -- ()
{
	THISIS(L"var& var::operator -- ()")
	//full check done below to avoid double checking number type
	THISISDEFINED()

tryagain:
	if (var_mvtyp&pimpl::MVTYPE_INT)
		var_mvint--;
	else if (var_mvtyp&pimpl::MVTYPE_DBL)
		var_mvdbl--;
	else if (var_mvtyp&pimpl::MVTYPE_STR)
	{
		//try to convert to numeric
		if (isnum())
		{
			//NB turn off string flag because it is about to be obsoleted;
			var_mvtyp=var_mvtyp^pimpl::MVTYPE_STR;
			goto tryagain;
		}

		throw MVNonNumeric(L"--(L" ^ substr(1,20) ^ L")");
	}
	else
		throw MVUnassigned(L"--()");

	//OK to return *this in prefix --
	return *this;

}

//+=var
var& var::operator += (const var& rhs)
{
	THISIS(L"var& var::operator += (const var& rhs)")
	THISISDEFINED()
	ISNUMERIC(rhs)

tryagain:

	//int target
	if (var_mvtyp&pimpl::MVTYPE_INT)
	{
		//int source
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
		{
			var_mvint+=rhs.var_mvint;
			return *this;
		}
		//dbl source, convert target to dbl
		var_mvdbl=var_mvint+rhs.var_mvdbl;
		var_mvtyp=pimpl::MVTYPE_DBL;
		return *this;
	}

	//dbl target
	else if (var_mvtyp&pimpl::MVTYPE_DBL)
	{
		//+= int or dbl from source
		var_mvdbl+=(rhs.var_mvtyp&pimpl::MVTYPE_INT)?rhs.var_mvint:rhs.var_mvdbl;
		return *this;
	}

	//last case(s) should be much less frequent since result of attempt to
	//convert strings to number is cached and only needs to be done once

	//nan (dont bother with this here because it is exceptional and will be caught below anyway
	//else if (var_mvtyp&pimpl::MVTYPE_NAN)
	//	throw MVNonNumeric(L"var::+= " ^ *this);

	//unassigned
	else if (!(var_mvtyp))
	{
		//throw MVUnassigned(L"+=");
		THISISNUMERIC()
	}

	//try to convert to numeric
	if (isnum())
	{
		// NB TODO turn off string flag because it is about to be obsoleted;
		//faster but less safe to do it here instead of at the point of updating
		//since increment and decrement probably mostly not on strings
		//xor
		var_mvtyp=var_mvtyp^pimpl::MVTYPE_STR;
		goto tryagain;
	}

 	THISISNUMERIC()
	throw MVNonNumeric(substr(1,20) ^ L"+= ");
}

//-=var
var& var::operator -= (const var& rhs)
{
	THISIS(L"var& var::operator -= (const var& rhs)")
	THISISDEFINED()
	ISNUMERIC(rhs)

tryagain:

	//int target
	if (var_mvtyp&pimpl::MVTYPE_INT)
	{
		//int source
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
		{
			var_mvint-=rhs.var_mvint;
			return *this;
		}
		//dbl source, convert target to dbl
		var_mvdbl=var_mvint-rhs.var_mvdbl;
		var_mvtyp=pimpl::MVTYPE_DBL;
		return *this;
	}

	//dbl target
	else if (var_mvtyp&pimpl::MVTYPE_DBL)
	{
		//-= int or dbl from source
		var_mvdbl-=(rhs.var_mvtyp&pimpl::MVTYPE_INT)?rhs.var_mvint:rhs.var_mvdbl;
		return *this;
	}

	//last case(s) should be much less frequent since result of attempt to
	//convert strings to number is cached and only needs to be done once

	//nan (dont bother with this here because it is exceptional and will be caught below anyway
	//else if (var_mvtyp&pimpl::MVTYPE_NAN)
	//	throw MVNonNumeric(L"var::-= " ^ *this);

	//unassigned
	else if (!(var_mvtyp))
	{
		//throw MVUnassigned(L"-=");
		THISISNUMERIC()

	}
	//try to convert to numeric
	if (isnum())
	{
		// NB TODO turn off string flag because it is about to be obsoleted;
		//faster but less safe to do it here instead of at the point of updating
		//since increment and decrement probably mostly not on strings
		var_mvtyp=var_mvtyp^pimpl::MVTYPE_STR;
		goto tryagain;
	}

	THISISNUMERIC()
	throw MVNonNumeric(substr(1,20) ^ L"-= ");

}

//almost identical between MVeq and MVlt except where noted
DLL_PUBLIC bool MVeq(const var& lhs,const var& rhs)
{
	THISIS(L"bool MVeq(const var& lhs,const var& rhs)")
	ISDEFINED(lhs)
	ISDEFINED(rhs)

	//NB empty string is always less than anything except another empty string

	//1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_mvtyp&pimpl::MVTYPE_STR)
	{
		if (rhs.var_mvtyp&pimpl::MVTYPE_STR)
		{
			//we have two strings
			//if they are both the same (including both empty) then eq is true
			if (lhs.var_mvstr==rhs.var_mvstr)
				//different from MVlt
				return true;
			//otherwise if either is empty then return eq false
			//(since empty string is ONLY eq to another empty string)
			if (lhs.var_mvstr.length()==0)
				//different from MVlt
				return false;
			if (rhs.var_mvstr.length()==0)
				//SAME as MVlt
				return false;
			//otherwise go on to test numerically then literally
		}
		else
		{
			//if rhs isnt a string and lhs is empty then eq is false
			//(after checking that rhs is actually assigned)
			if (lhs.var_mvstr.length()==0)
			{
				if (!rhs.var_mvtyp)
				{
					//throw MVUnassigned(L"eq(rhs)");
					ISASSIGNED(rhs)
				}
				//different from MVlt
				return false;
			}
		}

	}
	else
	{
		//if lhs isnt a string and rhs is an empty string then return eq false
		//(after checking that lhs is actually assigned)
		if ((rhs.var_mvtyp&pimpl::MVTYPE_STR) && (rhs.var_mvstr.length()==0))
		{
			if (!lhs.var_mvtyp)
			{
				//throw MVUnassigned(L"eq(lhs)");
				ISASSIGNED(lhs)
			}
			//SAME as MVlt
			return false;
		}

	}

	//2. both numerical strings
	if (lhs.isnum()&&rhs.isnum())
	{
		if (lhs.var_mvtyp&pimpl::MVTYPE_INT)
		{
			if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
				//different from MVlt
				return (lhs.var_mvint==rhs.var_mvint);
			else
				//different from MVlt
				return (lhs.var_mvint==rhs.var_mvdbl);
		}
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
			//different from MVlt
			return (lhs.var_mvdbl==rhs.var_mvint);
		else
			//different from MVlt
			return (lhs.var_mvdbl==rhs.var_mvdbl);
	}

	//3. either non-numerical strings
	if (!(lhs.var_mvtyp&pimpl::MVTYPE_STR))
		lhs.createString();
	if (!(rhs.var_mvtyp&pimpl::MVTYPE_STR))
		rhs.createString();
	//different from MVlt
	return lhs.var_mvstr==rhs.var_mvstr;

}

//almost identical between MVeq and MVlt except where noted
DLL_PUBLIC bool MVlt(const var& lhs,const var& rhs)
{
	THISIS(L"bool MVlt(const var& lhs,const var& rhs)")
	ISDEFINED(lhs)
	ISDEFINED(rhs)

	//NB empty string is always less than anything except another empty string

	//1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_mvtyp&pimpl::MVTYPE_STR)
	{
		if (rhs.var_mvtyp&pimpl::MVTYPE_STR)
		{
			//we have two strings
			//if they are both the same (including both empty) then eq is true
			if (lhs.var_mvstr==rhs.var_mvstr)
				//different from MVeq
				return false;
			//otherwise if either is empty then return eq false
			//(since empty string is ONLY eq to another empty string)
			if (lhs.var_mvstr.length()==0)
				//different from MVeq
				return true;
			if (rhs.var_mvstr.length()==0)
				//SAME as MVeq
				return false;
			//otherwise go on to test numerically then literally
		}
		else
		{
			//if rhs isnt a string and lhs is empty then eq is false
			//after checking that rhs is actually assigned
			if (lhs.var_mvstr.length()==0)
			{
				if (!rhs.var_mvtyp)
				{
					//throw MVUnassigned(L"eq(rhs)");
					ISASSIGNED(rhs)
				}
				//different from MVeq
				return true;
			}
		}
	
	}
	else
	{
		//if lhs isnt a string and rhs is an empty string then return eq false
		//after checking that lhs is actually assigned
		if ((rhs.var_mvtyp&pimpl::MVTYPE_STR) && (rhs.var_mvstr.length()==0))
		{
			if (!lhs.var_mvtyp)
			{
				//throw MVUnassigned(L"eq(lhs)");
				ISASSIGNED(lhs)
			}
			//SAME as MVeq
			return false;
		}

	}

	//2. both numerical strings
	if (lhs.isnum()&&rhs.isnum())
	{
		if (lhs.var_mvtyp&pimpl::MVTYPE_INT)
		{
			if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
				//different from MVeq
				return (lhs.var_mvint<rhs.var_mvint);
			else
				//different from MVeq
				return (double(lhs.var_mvint)<rhs.var_mvdbl);
		}
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
			//different from MVeq
			return (lhs.var_mvdbl<rhs.var_mvint);
		else
			//different from MVeq
			return (lhs.var_mvdbl<rhs.var_mvdbl);
	}

	//3. either or both non-numerical strings
	if (!(lhs.var_mvtyp&pimpl::MVTYPE_STR))
		lhs.createString();
	if (!(rhs.var_mvtyp&pimpl::MVTYPE_STR))
		rhs.createString();
	//different from MVeq
	//return lhs.var_mvstr<rhs.var_mvstr;
	return lhs.localeAwareCompare(lhs.var_mvstr,rhs.var_mvstr)<0;

}

//almost identical between MVeq and MVlt except where noted
 //this is the var<int version for speed
DLL_PUBLIC bool MVlt(const var& lhs,const int int2)
{
	THISIS(L"bool MVlt(const var& lhs,const int int2)")
	ISDEFINED(lhs)

	//NB empty string is always less than anything except another empty string

	//1. both empty or identical strings returns eq. one empty results false
	if (lhs.var_mvtyp&pimpl::MVTYPE_STR)
	{
		//if rhs isnt a string and lhs is empty then eq is false
		//after checking that rhs is actually assigned
		if (lhs.var_mvstr.length()==0)
		{
			//different from MVeq
			return true;
		}
	}

	//2. both numerical strings
	do
	{
		if (lhs.var_mvtyp&pimpl::MVTYPE_INT)
				//different from MVeq
				return (lhs.var_mvint<int2);

		if (lhs.var_mvtyp&pimpl::MVTYPE_DBL)
			//different from MVeq
			return (lhs.var_mvdbl<int2);
	}
	//go back and try again if can be converted to number
	while (lhs.isnum());

	//3. either or both non-numerical strings
	if (!(lhs.var_mvtyp&pimpl::MVTYPE_STR))
	{
		//lhs.createString();
		ISSTRING(lhs)
	}
	//different from MVeq
	return lhs.var_mvstr<intToString(int2);

}

//almost identical between MVeq and MVlt except where noted
 //this is the int<var version for speed
DLL_PUBLIC bool MVlt(const int int1,const var& rhs)
{
	THISIS(L"bool MVlt(const int int1,const var& rhs)")
	ISDEFINED(rhs)

	//NB empty string is always less than anything except another empty string

	//1. both empty or identical strings returns eq. one empty results false
	if (rhs.var_mvtyp&pimpl::MVTYPE_STR)
	{
		if (rhs.var_mvstr.length()==0)
			//SAME as MVeq
			return false;
		//otherwise go on to test numerically then literally
	}

	//2. both numerical strings
	do
	{
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
			//different from MVeq
			return (int1<rhs.var_mvint);
		if (rhs.var_mvtyp&pimpl::MVTYPE_DBL)
			//different from MVeq
			return (int1<rhs.var_mvdbl);
	}
	//go back and try again if can be converted to number
	while (rhs.isnum());

	//3. either or both non-numerical strings
	if (!(rhs.var_mvtyp&pimpl::MVTYPE_STR))
	{
		//lhs.createString();
		ISSTRING(rhs)
	}
	//different from MVeq
	return intToString(int1)<rhs.var_mvstr;

}
//SEE ALSO MV2.CPP

//== and !=
DLL_PUBLIC bool operator== (const var&     lhs    ,const var&     rhs     ){return  MVeq(lhs		,rhs          );}
DLL_PUBLIC bool operator== (const var&     lhs    ,const wchar_t* char2   ){return  MVeq(lhs		,var(char2)   );}
DLL_PUBLIC bool operator== (const var&     lhs    ,const int      int2    ){return  MVeq(lhs		,var(int2)    );}
DLL_PUBLIC bool operator== (const var&     lhs    ,const double   double2 ){return  MVeq(lhs		,var(double2) );}
DLL_PUBLIC bool operator== (const var&     lhs    ,const bool     bool2   ){return  MVeq(lhs		,var(bool2)   );}
DLL_PUBLIC bool operator== (const wchar_t* char1  ,const var&     rhs     ){return  MVeq(rhs		,var(char1)   );}
DLL_PUBLIC bool operator== (const int      int1   ,const var&     rhs     ){return  MVeq(rhs		,var(int1)    );}
DLL_PUBLIC bool operator== (const double   double1,const var&     rhs     ){return  MVeq(rhs		,var(double1) );}
DLL_PUBLIC bool operator== (const bool     bool1  ,const var&     rhs     ){return  MVeq(rhs		,var(bool1)   );}

DLL_PUBLIC bool operator!= (const var&     lhs    ,const var&     rhs     ){return !MVeq(lhs		,rhs          );}
DLL_PUBLIC bool operator!= (const var&     lhs    ,const wchar_t* char2   ){return !MVeq(lhs		,var(char2)	  );}
DLL_PUBLIC bool operator!= (const var&     lhs    ,const int      int2    ){return !MVeq(lhs		,var(int2)    );}
DLL_PUBLIC bool operator!= (const var&     lhs    ,const double   double2 ){return !MVeq(lhs		,var(double2) );}
DLL_PUBLIC bool operator!= (const var&     lhs    ,const bool     bool2   ){return !MVeq(lhs		,var(bool2)   );}
DLL_PUBLIC bool operator!= (const wchar_t* char1  ,const var&     rhs     ){return !MVeq(rhs		,var(char1)   );}
DLL_PUBLIC bool operator!= (const int      int1   ,const var&     rhs     ){return !MVeq(rhs		,var(int1)    );}
DLL_PUBLIC bool operator!= (const double   double1,const var&     rhs     ){return !MVeq(rhs		,var(double1) );}
DLL_PUBLIC bool operator!= (const bool     bool1  ,const var&     rhs     ){return !MVeq(rhs		,var(bool1)   );}

//< V<= > >=
DLL_PUBLIC bool operator<  (const var&     lhs    ,const var&     rhs     ){return  MVlt(lhs         ,rhs         );}
DLL_PUBLIC bool operator<  (const var&     lhs    ,const wchar_t* char2   ){return  MVlt(lhs         ,var(char2)  );}
DLL_PUBLIC bool operator<  (const var&     lhs    ,const int      int2    ){return  MVlt(lhs         ,int2        );}
DLL_PUBLIC bool operator<  (const var&     lhs    ,const double   double2 ){return  MVlt(lhs         ,var(double2));}
DLL_PUBLIC bool operator<  (const wchar_t* char1  ,const var&     rhs     ){return  MVlt(var(char1)  ,rhs         );}
DLL_PUBLIC bool operator<  (const int      int1   ,const var&     rhs     ){return  MVlt(int1        ,rhs         );}
DLL_PUBLIC bool operator<  (const double   double1,const var&     rhs     ){return  MVlt(var(double1),rhs         );}

DLL_PUBLIC bool operator>= (const var&     lhs    ,const var&     rhs     ){return !MVlt(lhs         ,rhs         );}
DLL_PUBLIC bool operator>= (const var&     lhs    ,const wchar_t* char2   ){return !MVlt(lhs         ,var(char2)  );}
DLL_PUBLIC bool operator>= (const var&     lhs    ,const int      int2    ){return !MVlt(lhs         ,int2        );}
DLL_PUBLIC bool operator>= (const var&     lhs    ,const double   double2 ){return !MVlt(lhs         ,var(double2));}
DLL_PUBLIC bool operator>= (const wchar_t* char1  ,const var&     rhs     ){return !MVlt(var(char1)  ,rhs         );}
DLL_PUBLIC bool operator>= (const int      int1   ,const var&     rhs     ){return !MVlt(int1        ,rhs         );}
DLL_PUBLIC bool operator>= (const double   double1,const var&     rhs     ){return !MVlt(var(double1),rhs         );}

DLL_PUBLIC bool operator>  (const var&     lhs    ,const var&     rhs     ){return  MVlt(rhs         ,lhs         );}
DLL_PUBLIC bool operator>  (const var&     lhs    ,const wchar_t* char2   ){return  MVlt(var(char2)  ,lhs         );}
DLL_PUBLIC bool operator>  (const var&     lhs    ,const int      int2    ){return  MVlt(int2        ,lhs         );}
DLL_PUBLIC bool operator>  (const var&     lhs    ,const double   double2 ){return  MVlt(var(double2),lhs         );}
DLL_PUBLIC bool operator>  (const wchar_t* char1  ,const var&     rhs     ){return  MVlt(rhs         ,var(char1)  );}
DLL_PUBLIC bool operator>  (const int      int1   ,const var&     rhs     ){return  MVlt(rhs         ,int1        );}
DLL_PUBLIC bool operator>  (const double   double1,const var&     rhs     ){return  MVlt(rhs         ,var(double1));}

DLL_PUBLIC bool operator<= (const var&     lhs    ,const var&     rhs     ){return !MVlt(rhs         ,lhs         );}
DLL_PUBLIC bool operator<= (const var&     lhs    ,const wchar_t* char2   ){return !MVlt(var(char2)  ,lhs         );}
DLL_PUBLIC bool operator<= (const var&     lhs    ,const int      int2    ){return !MVlt(int2        ,lhs         );}
DLL_PUBLIC bool operator<= (const var&     lhs    ,const double   double2 ){return !MVlt(var(double2),lhs         );}
DLL_PUBLIC bool operator<= (const wchar_t* char1  ,const var&     rhs     ){return !MVlt(rhs         ,var(char1)  );}
DLL_PUBLIC bool operator<= (const int      int1   ,const var&     rhs     ){return !MVlt(rhs         ,int1        );}
DLL_PUBLIC bool operator<= (const double   double1,const var&     rhs     ){return !MVlt(rhs         ,var(double1));}

//+var
DLL_PUBLIC var operator+(const var& var1)
{
	THISIS(L"var operator+(const var& var1)")
	ISDEFINED(var1)

	do
	{
		//int
		if (var1.var_mvtyp&pimpl::MVTYPE_INT)
			return var1.var_mvint;

		//dbl
		if (var1.var_mvtyp&pimpl::MVTYPE_DBL)
			return var1.var_mvdbl;

		//unassigned
		if (!var1.var_mvtyp)
		{
			ISASSIGNED(var1)
			throw MVUnassigned(L"+()");
		}
	}
	//must be string - try to convert to numeric
	while (var1.isnum());

	//non-numeric
	ISNUMERIC(var1)
	//will never get here
	throw MVNonNumeric(L"+(L" ^ var1.substr(1,20) ^ L")");

}

//-var (identical to +var above except for two additional - signs)
DLL_PUBLIC var operator -(const var& var1)
{
	THISIS(L"var operator -(const var& var1)")
	ISDEFINED(var1)

	do
	{
		//int
		if (var1.var_mvtyp&pimpl::MVTYPE_INT)
			return -var1.var_mvint;

		//dbl
		if (var1.var_mvtyp&pimpl::MVTYPE_DBL)
			return -var1.var_mvdbl;

		//unassigned
		if (!var1.var_mvtyp)
		{
			ISASSIGNED(var1)
			throw MVUnassigned(L"+()");
		}
	}
	//must be string - try to convert to numeric
	while (var1.isnum());

	//non-numeric
	ISNUMERIC(var1)
	//will never get here
	throw MVNonNumeric(L"+(L" ^ var1.substr(1,20) ^ L")");
}

//!var
bool operator !(const var& var1)
{
	THISIS(L"bool operator !(const var& var1)")
	ISASSIGNED(var1)

	//might need converting to work on void pointer
	//if bool replaced with void* (or made explicit instead of implict)
	//is there really any difference since the bool and void operators are defined identically?
	//return !(bool)(var1);
	return !(void*)(var1);
}

var MVadd(const var& lhs,const var& rhs)
{
	THISIS(L"var MVadd(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	if (lhs.var_mvtyp&pimpl::MVTYPE_INT)
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
			return lhs.var_mvint + rhs.var_mvint;//only this returns an int, the following both return doubles
		else
			return	lhs.var_mvint + ((rhs.var_mvtyp&pimpl::MVTYPE_INT) ? rhs.var_mvint : rhs.var_mvdbl);
	else
		return	lhs.var_mvdbl + ((rhs.var_mvtyp&pimpl::MVTYPE_INT) ? rhs.var_mvint : rhs.var_mvdbl);
}

var MVsub(const var& lhs,const var& rhs)
{
	THISIS(L"var MVsub(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	if (lhs.var_mvtyp&pimpl::MVTYPE_INT)
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
			return lhs.var_mvint - rhs.var_mvint;//only this returns an int, the following both return doubles
		else
			return	lhs.var_mvint - ((rhs.var_mvtyp&pimpl::MVTYPE_INT) ? rhs.var_mvint : rhs.var_mvdbl);
	else
		return	lhs.var_mvdbl - ((rhs.var_mvtyp&pimpl::MVTYPE_INT) ? rhs.var_mvint : rhs.var_mvdbl);
}

var MVmul(const var& lhs,const var& rhs)
{
	THISIS(L"var MVmul(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	if (lhs.var_mvtyp&pimpl::MVTYPE_INT)
		if (rhs.var_mvtyp&pimpl::MVTYPE_INT)
			return lhs.var_mvint * rhs.var_mvint;//only this returns an int, the following both return doubles
		else
			return	lhs.var_mvint * ((rhs.var_mvtyp&pimpl::MVTYPE_INT) ? rhs.var_mvint : rhs.var_mvdbl);
	else
		return	lhs.var_mvdbl * ((rhs.var_mvtyp&pimpl::MVTYPE_INT) ? rhs.var_mvint : rhs.var_mvdbl);
}

var MVdiv(const var& lhs,const var& rhs)
{
	THISIS(L"var MVdiv(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	//always returns a double

	double bottom=(rhs.var_mvtyp&pimpl::MVTYPE_INT) ? double(rhs.var_mvint) : rhs.var_mvdbl;
	if (!bottom)
		throw MVDivideByZero(L"div('" ^ lhs.substr(1,20) ^ L"', '" ^ rhs.substr(1,20) ^ L"')");

	double top=(lhs.var_mvtyp&pimpl::MVTYPE_INT) ? double(lhs.var_mvint) : lhs.var_mvdbl;
	return top/bottom;
}

var MVmod(const var& lhs,const var& rhs)
{
	THISIS(L"var MVmod(const var& lhs,const var& rhs)")
	ISNUMERIC(lhs)
	ISNUMERIC(rhs)

	//integer version;
	if (lhs.var_mvtyp&pimpl::MVTYPE_INT && rhs.var_mvtyp&pimpl::MVTYPE_INT)
	{
		if (!rhs.var_mvint)
			throw MVDivideByZero(L"div('" ^ lhs.substr(1,20) ^ L"', '" ^ rhs.substr(1,20) ^ L"')");
		return lhs.var_mvint%rhs.var_mvint;
	}

	double bottom=(rhs.var_mvtyp&pimpl::MVTYPE_INT) ? double(rhs.var_mvint) : rhs.var_mvdbl;
	if (!bottom)
		throw MVDivideByZero(L"div('" ^ lhs.substr(1,20) ^ L"', '" ^ rhs.substr(1,20) ^ L"')");

	double top=(lhs.var_mvtyp&pimpl::MVTYPE_INT) ? double(lhs.var_mvint) : lhs.var_mvdbl;
	return neosysmodulus(top,bottom);
}

//var^var we reassign the logical xor operator ^ to be string concatenate!!!
//slightly wrong precedence but at least we have a reliable concat operator to replace the + which is now reserved for ADDITION
var MVcat(const var& lhs,const var& rhs)
{
	THISIS(L"var MVcat(const var& lhs,const var& rhs)")
	ISSTRING(lhs)
	ISSTRING(rhs)

	return lhs.var_mvstr+rhs.var_mvstr;
}

DLL_PUBLIC var operator+ (const var&     lhs    ,const var&     rhs     ){return MVadd(lhs         ,rhs         );}
DLL_PUBLIC var operator+ (const var&     lhs    ,const wchar_t* char2   ){return MVadd(lhs         ,var(char2)  );}
DLL_PUBLIC var operator+ (const var&     lhs    ,const int      int2    ){return MVadd(lhs         ,var(int2)   );}
DLL_PUBLIC var operator+ (const var&     lhs    ,const double   double2 ){return MVadd(lhs         ,var(double2));}
DLL_PUBLIC var operator+ (const var&     lhs    ,const bool     bool2   ){return MVadd(lhs         ,var(bool2)  );}
DLL_PUBLIC var operator+ (const wchar_t* char1  ,const var&     rhs     ){return MVadd(var(char1)  ,rhs         );}
DLL_PUBLIC var operator+ (const int      int1   ,const var&     rhs     ){return MVadd(var(int1)   ,rhs         );}
DLL_PUBLIC var operator+ (const double   double1,const var&     rhs     ){return MVadd(var(double1),rhs         );}
DLL_PUBLIC var operator+ (const bool     bool1  ,const var&     rhs     ){return MVadd(var(bool1)  ,rhs         );}

DLL_PUBLIC var operator- (const var&     lhs    ,const var&     rhs     ){return MVsub(lhs         ,rhs         );}
DLL_PUBLIC var operator- (const var&     lhs    ,const wchar_t* char2   ){return MVsub(lhs         ,var(char2)  );}
DLL_PUBLIC var operator- (const var&     lhs    ,const int      int2    ){return MVsub(lhs         ,var(int2)   );}
DLL_PUBLIC var operator- (const var&     lhs    ,const double   double2 ){return MVsub(lhs         ,var(double2));}
DLL_PUBLIC var operator- (const var&     lhs    ,const bool     bool2   ){return MVsub(lhs         ,var(bool2)  );}
DLL_PUBLIC var operator- (const wchar_t* char1  ,const var&     rhs     ){return MVsub(var(char1)  ,rhs         );}
DLL_PUBLIC var operator- (const int      int1   ,const var&     rhs     ){return MVsub(var(int1)   ,rhs         );}
DLL_PUBLIC var operator- (const double   double1,const var&     rhs     ){return MVsub(var(double1),rhs         );}
DLL_PUBLIC var operator- (const bool     bool1  ,const var&     rhs     ){return MVsub(var(bool1)  ,rhs         );}

DLL_PUBLIC var operator* (const var&     lhs    ,const var&     rhs     ){return MVmul(lhs         ,rhs         );}
DLL_PUBLIC var operator* (const var&     lhs    ,const wchar_t* char2   ){return MVmul(lhs         ,var(char2)  );}
DLL_PUBLIC var operator* (const var&     lhs    ,const int      int2    ){return MVmul(lhs         ,var(int2)   );}
DLL_PUBLIC var operator* (const var&     lhs    ,const double   double2 ){return MVmul(lhs         ,var(double2));}
DLL_PUBLIC var operator* (const var&     lhs    ,const bool     bool2   ){return MVmul(lhs         ,var(bool2)  );}
DLL_PUBLIC var operator* (const wchar_t* char1  ,const var&     rhs     ){return MVmul(var(char1)  ,rhs         );}
DLL_PUBLIC var operator* (const int      int1   ,const var&     rhs     ){return MVmul(var(int1)   ,rhs         );}
DLL_PUBLIC var operator* (const double   double1,const var&     rhs     ){return MVmul(var(double1),rhs         );}
DLL_PUBLIC var operator* (const bool     bool1  ,const var&     rhs     ){return MVmul(var(bool1)  ,rhs         );}

DLL_PUBLIC var operator/ (const var&     lhs    ,const var&     rhs     ){return MVdiv(lhs         ,rhs         );}
DLL_PUBLIC var operator/ (const var&     lhs    ,const wchar_t* char2   ){return MVdiv(lhs         ,var(char2)  );}
DLL_PUBLIC var operator/ (const var&     lhs    ,const int      int2    ){return MVdiv(lhs         ,var(int2)   );}
DLL_PUBLIC var operator/ (const var&     lhs    ,const double   double2 ){return MVdiv(lhs         ,var(double2));}
//disallow divide by boolean to prevent possible runtime divide by zero
//DLL_PUBLIC var operator/ (const var&     lhs    ,const bool     bool2   ){return MVdiv(lhs,var(bool2)  );}
DLL_PUBLIC var operator/ (const wchar_t* char1  ,const var&     rhs     ){return MVdiv(var(char1)  ,rhs         );}
DLL_PUBLIC var operator/ (const int      int1   ,const var&     rhs     ){return MVdiv(var(int1)   ,rhs         );}
DLL_PUBLIC var operator/ (const double   double1,const var&     rhs     ){return MVdiv(var(double1),rhs         );}
DLL_PUBLIC var operator/ (const bool     bool1  ,const var&     rhs     ){return MVdiv(var(bool1)  ,rhs         );}

DLL_PUBLIC var operator% (const var&     lhs    ,const var&     rhs     ){return MVmod(lhs         ,rhs         );}
DLL_PUBLIC var operator% (const var&     lhs    ,const wchar_t* char2   ){return MVmod(lhs         ,var(char2)  );}
DLL_PUBLIC var operator% (const var&     lhs    ,const int      int2    ){return MVmod(lhs         ,var(int2)   );}
DLL_PUBLIC var operator% (const var&     lhs    ,const double   double2 ){return MVmod(lhs         ,var(double2));}
//disallow divide by boolean to prevent possible runtime divide by zero
//DLL_PUBLIC var operator% (const var&     lhs    ,const bool    bool2   ){return MVmod(lhs,var(bool2)  );}
DLL_PUBLIC var operator% (const wchar_t* char1  ,const var&     rhs     ){return MVmod(var(char1)  ,rhs         );}
DLL_PUBLIC var operator% (const int      int1   ,const var&     rhs     ){return MVmod(var(int1)   ,rhs         );}
DLL_PUBLIC var operator% (const double   double1,const var&     rhs     ){return MVmod(var(double1),rhs         );}
DLL_PUBLIC var operator% (const bool     bool1  ,const var&     rhs     ){return MVmod(var(bool1)  ,rhs         );}

//NB do *NOT* support concatenate with bool or vice versa!!!
//to avoid compiler doing wrong precendence issue between ^ and logical operators
DLL_PUBLIC var operator^ (const var&     lhs    ,const var&     rhs     ){return MVcat(lhs         ,rhs         );}
DLL_PUBLIC var operator^ (const var&     lhs    ,const wchar_t* char2   ){return MVcat(lhs         ,var(char2)  );}
DLL_PUBLIC var operator^ (const var&     lhs    ,const int      int2    ){return MVcat(lhs         ,var(int2)   );}
DLL_PUBLIC var operator^ (const var&     lhs    ,const double   double2 ){return MVcat(lhs         ,var(double2));}
DLL_PUBLIC var operator^ (const wchar_t* char1  ,const var&     rhs     ){return MVcat(var(char1)  ,rhs         );}
DLL_PUBLIC var operator^ (const int      int1   ,const var&     rhs     ){return MVcat(var(int1)   ,rhs         );}
DLL_PUBLIC var operator^ (const double   double1,const var&     rhs     ){return MVcat(var(double1),rhs         );}


#if defined __MINGW32__
//allow use of cout<<var
DLL_PUBLIC
	std::ostream& operator << (std::ostream& ostream1, const var& var1)
{
	THISIS(L"std::ostream& operator << (std::ostream& ostream1, const var& var1)")
	ISSTRING(var1)

	//use tostring() to avoid creating a ctor which logs here recursively
	//should this use a ut16/32 -> UTF8 code facet? or convert to UTF8 and output to ostream?
	ostream1 << var1.tostring();
	return ostream1;
}

std::istream& operator >> (std::istream& istream1,var& var1)
{
	THISIS(L"std::istream& operator >> (std::istream& istream1,var& var1)")
	ISDEFINED(var1)

	std::string tempstr;
	istream1 >> std::noskipws >> tempstr;

	var1.var_mvtyp=pimpl::MVTYPE_STR;
	var1.var_mvstr=wstringfromUTF8((UTF8*)tempstr.data(),(int)tempstr.length());
	return istream1;
}

#else

//allow use of wcout<<var
DLL_PUBLIC
	std::wostream& operator << (std::wostream& wostream1, const var& var1)
{
	THISIS(L"std::wostream& operator << (std::wostream& wostream1, const var& var1)")
	ISSTRING(var1)

	//use towstring() to avoid creating a ctor which logs here recursively
	//should this use a ut16/32 -> UTF8 code facet? or convert to UTF8 and output to ostream?
	wostream1 << var1.towstring();
	return wostream1;
}

//#ifdef false //allow use of cin>>var
std::wistream& operator >> (std::wistream& wistream1,var& var1)
{
	THISIS(L"std::wistream& operator >> (std::wistream& wistream1,var& var1)")
	ISDEFINED(var1)

	var1.var_mvtyp=pimpl::MVTYPE_STR;
	wistream1 >> std::noskipws >> var1.var_mvstr;
	return wistream1;
}
//#endif //allow use of std::wcin>>var
#endif

inline double neosysmodulus(const double top,const double bottom)
{
	return top-double(int(top/bottom)*bottom);
}

//TODO ensure locale doesnt produce like 123.456,78
std::wstring intToString(int int1)
{
	
	//TODO test ostringstream type creation speed and of slow then
	//consider using a thread global to avoid continual creation

	//see http://www.gotw.ca/publications/mill19.htm
    //and http://www.boost.org/libs/conversion/lexical_cast.htm
    //for background to choice of ostringstream for implementation

    //1. sprintf rejected because snprintf is safer
    //2. snprintf rejected because cannot handle wide characters
    //3. strstream rejected because it is deprecated and might cease to be supported
    //but is the fastest if you use its ability to preallocate memory in one go
    //RECONSIDER IMPLEMENTING IN strstream for performance
    //4. stringstream CHOSEN even though very slow versus snprintf
    //5. boost:lexical_cast rejected because two step conversion is very slow
    //and unnecessary if you just want a string and doesnt allow precision control
    //useful for float2string

    //NB plain stringstream causes a memory leak in msvc8 before sp1
	std::wostringstream ss;
	ss << int1;
	//debuggFUNCTION&& wcout<<L"intToString(int "<<int1<<L") returns '"<<s<<L"'\n";
#ifdef NARROW_IO
	return std::wstring(ss.str().begin(),ss.str().end());
#else
    return ss.str();
#endif
}

//TODO ensure locale doesnt produce like 123.456,78
//see 1997 http://www.cantrip.org/locale.html
std::wstring dblToString(double double1)
{
    //see intToString for choice of ostringstream for implementation
    //NB plain stringstream causes a memory leak in msvc8 before sp1
	std::wostringstream ss;
    ss.precision(10);
	ss << double1;
	//debuggFUNCTION&& wcout<<L"dblToString(int "<<double1<<L") returns '"<<s<<L"'\n";
#ifdef NARROW_IO
	return std::wstring(ss.str().begin(),ss.str().end());
#else
    return ss.str();
#endif

}

MVException::MVException(const var& description_) : description(description_)
{
	std::wcerr << L"MVException:" << description <<std::endl;
	backtrace().convert(FM,L"\n").outputl();
}

MVUnassigned		::MVUnassigned		(const var& var1)	: MVException(L"MVUnassigned:"				^ var1	){}
MVDivideByZero		::MVDivideByZero	(const var& var1)	: MVException(L"MVDivideByZero:"			^ var1	){}
MVNonNumeric		::MVNonNumeric		(const var& var1)	: MVException(L"MVNonNumeric:"				^ var1	){}
MVUndefined			::MVUndefined		(const var& var1)	: MVException(L"MVUndefined:"				^ var1	){}
MVInvalidPointer	::MVInvalidPointer	(const var& var1)	: MVException(L"MVInvalidPointer:"			^ var1	){}
MVDBException		::MVDBException		(const var& var1)	: MVException(L"MVDBException:"				^ var1	){}
MVNotImplemented	::MVNotImplemented	(const var& var1)	: MVException(L"MVNotImplemented:"			^ var1	){}
MVDebug				::MVDebug			(const var& var1)	: MVException(L"MVDebug"					^ var1	){}
MVStop				::MVStop			(const var& var1)	: MVException(L"MVStop:"					^ var1 	){}
MVAbort				::MVAbort			(const var& var1)	: MVException(L"MVAbort"					^ var1	){}

MVArrayDimensionedZero	::MVArrayDimensionedZero	()					: MVException(L"MVArrayDimensionedZero:"			){}
MVArrayIndexOutOfBounds	::MVArrayIndexOutOfBounds	(const var& var1)	: MVException(L"MVArrayIndexOutOfBounds:"	^ var1	){}
MVArrayNotDimensioned	::MVArrayNotDimensioned	()	: MVException(L"MVArrayNotDimensioned"){}

} // namespace exodus
