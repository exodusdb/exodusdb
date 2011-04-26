/*
Copyright (c) 2009 Stephen John Bush
See MIT Licence
*/

//var used to hold a pointer to its private data
//but for ease of debugging programs using var 
//and because the classic reason for doing "pointer to implementation"
//doesnt apply here (var is a stable library)
//so pimpls data members have been moved to var private
//mvstr mvtype mvint mvdbl made directly private in var
//still holds the static constants
//
//Summary:
//this class could be removed once it is confirmed by c++ gurus
//that the above strategy is correct.

#ifndef MVIMPL_H
#define MVIMPL_H 1

#include <string>

namespace exodus
{

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
	static const wchar_t MVTYPE_OSFILE=0x10;
	static const wchar_t MVTYPE_DBCONN=0x20;

	static const wchar_t MVTYPE_INTDBL=MVTYPE_INT|MVTYPE_DBL;
	static const wchar_t MVTYPE_INTSTR=MVTYPE_INT|MVTYPE_STR;
	static const wchar_t MVTYPE_DBLSTR=MVTYPE_DBL|MVTYPE_STR;
	static const wchar_t MVTYPE_NANSTR=MVTYPE_NAN|MVTYPE_STR;

	static const wchar_t MVTYPE_NANSTR_OSFILE=MVTYPE_NANSTR | MVTYPE_OSFILE;
	static const wchar_t MVTYPE_NANSTR_DBCONN=MVTYPE_NANSTR | MVTYPE_DBCONN;
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
