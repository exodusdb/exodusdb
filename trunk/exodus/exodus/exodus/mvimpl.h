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

#ifndef MVIMPL_H
#define MVIMPL_H 1

#include <string>

namespace exodus
{

//mvstr mvtype mvint mvdbl made directly private in var
//still holds the static constants
class pimpl
{

public:
	//throw an exception if used an unassigned variable
	static const wchar_t MVTYPE_UNA=0x0;

	//assigned string unknown if numeric or not
	static const wchar_t MVTYPE_STR=0x1;

	//indicated known non-numeric string
	static const wchar_t MVTYPE_NAN=0x2;

	//above here is numeric
	static const wchar_t MVTYPE_INT=0x4;
	static const wchar_t MVTYPE_DBL=0x8;

	static const wchar_t MVTYPE_INTDBL=MVTYPE_INT|MVTYPE_DBL;
	static const wchar_t MVTYPE_INTSTR=MVTYPE_INT|MVTYPE_STR;
	static const wchar_t MVTYPE_DBLSTR=MVTYPE_DBL|MVTYPE_STR;
	static const wchar_t MVTYPE_NANSTR=MVTYPE_NAN|MVTYPE_STR;

/* moved to default contructor
	//default ctor initialise to "unassigned"
	pimpl() : mvtype(MVTYPE_UNA) {};
*/

/* moved to var private	//data members
	//all mutable because asking for a string can create it from an integer and vice versa
	mutable std::wstring mvstr;
	mutable wchar_t mvtype;
	mutable int mvint;
	mutable double mvdbl;
*/

};

const bool debugg=false;
const bool debuggCONSTRUCT=false;
const bool debuggOPERATOR=false;
const bool debuggCONVERT=false;
const bool debuggFUNCTION=false;

}//namespace exodus

#endif /*MVIMPL_H*/
