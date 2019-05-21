/* Copyright (c) 2009 steve.bush@neosys.com */

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
	static const char VARTYP_UNA=0x0;

	//assigned string unknown if numeric or not
	static const char VARTYP_STR=0x1;

	//indicated known non-numeric string
	static const char VARTYP_NAN=0x2;

	//above here is numeric
	static const char VARTYP_INT=0x4;
	static const char VARTYP_DBL=0x8;
	static const char VARTYP_OSFILE=0x10;
	static const char VARTYP_DBCONN=0x20;

	static const char VARTYP_INTDBL=VARTYP_INT|VARTYP_DBL;
	static const char VARTYP_INTSTR=VARTYP_INT|VARTYP_STR;
	static const char VARTYP_DBLSTR=VARTYP_DBL|VARTYP_STR;
	static const char VARTYP_NANSTR=VARTYP_NAN|VARTYP_STR;
	static const char VARTYP_NOTNUMFLAGS=~(VARTYP_INT|VARTYP_DBL|VARTYP_NAN);

	static const char VARTYP_NANSTR_OSFILE=VARTYP_NANSTR | VARTYP_OSFILE;
	static const char VARTYP_NANSTR_DBCONN=VARTYP_NANSTR | VARTYP_DBCONN;
/* moved to default contructor
	//default ctor initialise to "unassigned"
	pimpl() : mvtype(VARTYP_UNA) {};
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
