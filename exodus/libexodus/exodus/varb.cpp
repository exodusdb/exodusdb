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

// vartype
//
//#define xEXO_IMPORT import
//#ifdef EXO_IMPORT
////	EXO_IMPORT vartyp;
//	import vartyp;
//#else
//	#include <exodus/vartyp.h>
//#endif
//#include <exodus/varb.h>

#include <exodus/var.h>
#include <exodus/exoimpl.h>

namespace exo {

//// clone to var
////
//CONSTEXPR
//operator VARBASE1::var() &{
//	EXO_SNITCH("var_base cp>v")
//	return this->clone();
//}
//
//// move to var
////
//CONSTEXPR
//operator VARBASE1::var() && {
//	EXO_SNITCH("var_base mv>v")
//	return this->move();
//}

// ::clone to var
//
template<> PUBLIC RETVAR VARBASE1::clone() const {

	RETVAR rvo;
	rvo.var_typ = var_typ;
	rvo.var_str = var_str;

	// Avoid copying int and dbl in case cloning an unassigned var
	// since default ctor var() = default and int/dbl are not initialised to zero.
	//
	// Strategy is as follows:
	//
	// Cloning uninitialised data should leave the target uninitialised as well
	//
	// If var implementation ever changes so that int/dbl must be cloned
	// even if vartype is unassigned then int/dbl will have to be default initialised
	// to something, probably zero.
	//
	// Avoid triggering a compiler warning
	// warning: ‘<anonymous>.exo::var::var_dbl’ may be used uninitialized in this function [-Wmaybe-uninitialized]
	if (var_typ) {
		rvo.var_int = var_int;
		rvo.var_dbl = var_dbl;
	}

	return rvo;
}

// ::move to var
//
template<> PUBLIC RETVAR VARBASE1::move() {

	// Like clone but moves the std::string and sets the old var to UNA empty string.

	RETVAR rvo;
	rvo.var_typ = var_typ;
	rvo.var_str = std::move(var_str);

	// Avoid copying int and dbl in case cloning an unassigned var
	// since default ctor var() = default and int/dbl are not initialised to zero.
	//
	// Strategy is as follows:
	//
	// Cloning uninitialised data should leave the target uninitialised as well
	//
	// If var implementation ever changes so that int/dbl must be cloned
	// even if vartype is unassigned then int/dbl will have to be default initialised
	// to something, probably zero.
	//
	// Avoid triggering a compiler warning
	// warning: ‘<anonymous>.exo::var::var_dbl’ may be used uninitialized in this function [-Wmaybe-uninitialized]
	if (var_typ) {
		rvo.var_int = var_int;
		rvo.var_dbl = var_dbl;
	}

	// Clean up this source
	var_str.clear();
	var_typ = VARTYP_UNA;

	// TODO
	// Clear int and dbl?

	return rvo;
}

// ::first_
//
// utility string function on var_base for throwing errors
// TODO refactor out
//
template<> PUBLIC RETVAR VARBASE1::first_(int nchars) const {
	assertString(__PRETTY_FUNCTION__);
	if (nchars < static_cast<int>(var_str.size()))
		return var_str.substr(0, nchars);
	return var_str;
}

// ::dump
//
template<> PUBLIC RETVAR VARBASE1::dump() const {
	var nrvo = "var: ";

	if (var_typ & VARTYP_STR)
		nrvo ^= "str: " _DQ ^ var_base(var_str).first_(1024).convert(_ALL_FMS, _VISIBLE_FMS) ^ _DQ " ";

	if (var_typ & VARTYP_OSFILE) {
		nrvo ^= "osfile int:" ^ var_base(var_int) ^ " ";
		nrvo ^= "dbl:" ^ var_base(var_dbl) ^ " ";

	} else {

		if (var_typ & VARTYP_INT)
			nrvo ^= "int:" ^ var_base(var_int) ^ " ";

		if (var_typ & VARTYP_DBL)
			nrvo ^= "dbl:" ^ var_base(var_dbl) ^ " ";
	}

//	nrvo ^= "typ:" ^ (var_base(int(var_typ)).oconv("MB").trimfirst("0") + 0);
	nrvo ^= "typ:" ^ (var_base(int(var_typ))/*.oconv("MB").trimfirst("0") + 0*/);

	return nrvo;
}

// Exception constructors
//
// clang-format off
VarUnconstructed   ::VarUnconstructed   (std::string errmsg) : VarError("VarUnconstructed:"    + errmsg) {}
VarUnassigned      ::VarUnassigned      (std::string errmsg) : VarError("VarUnassigned:"       + errmsg) {}
VarDivideByZero    ::VarDivideByZero    (std::string errmsg) : VarError("VarDivideByZero:"     + errmsg) {}
VarNonNumeric      ::VarNonNumeric      (std::string errmsg) : VarError("VarNonNumeric:"       + errmsg) {}
VarNonPositive     ::VarNonPositive     (std::string errmsg) : VarError("VarNonPositive:"      + errmsg) {}
VarNumOverflow     ::VarNumOverflow     (std::string errmsg) : VarError("VarNumOverflow:"      + errmsg) {}
VarNumUnderflow    ::VarNumUnderflow    (std::string errmsg) : VarError("VarNumUnderflow:"     + errmsg) {}
VarOutOfMemory     ::VarOutOfMemory     (std::string errmsg) : VarError("VarOutOfMemory:"      + errmsg) {}
VarInvalidPointer  ::VarInvalidPointer  (std::string errmsg) : VarError("VarInvalidPointer:"   + errmsg) {}
VarDBException     ::VarDBException     (std::string errmsg) : VarError("VarDBException:"      + errmsg) {}
VarNotImplemented  ::VarNotImplemented  (std::string errmsg) : VarError("VarNotImplemented:"   + errmsg) {}
VarDebug           ::VarDebug           (std::string errmsg) : VarError("VarDebug"             + errmsg) {}

DimUndimensioned   ::DimUndimensioned   (std::string errmsg) : VarError("DimUndimensioned"     + errmsg) {}
DimIndexOutOfBounds::DimIndexOutOfBounds(std::string errmsg) : VarError("DimIndexOutOfBounds:" + errmsg) {}

// clang-format on

// std::string utility
inline void varb_replace_string (std::string& subject, const std::string& search, const std::string& replace) {
	std::size_t pos = 0;
//	while
	if ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
//		pos += replace.length();
		}
	}

// VarError constructor
//
VarError::VarError(std::string description_)
	: description(description_) {
	// *** WARNING ***
	// any errors in this constructor
	// will cause recursion and hang/segfault

//	if (description.assigned())
//		description.put(std::cerr);
//	var("\n").put(std::cerr);

	// Capture the stack at point of creation i.e. when thrown
	// TODO capture in caller using default argument to VarError?
	exo_savestack(stack_addresses_, &stack_size_);

	// Flush any stdout error messages out
	std::cout << std::flush;

//	this->stack = exo_backtrace();
//	((description.assigned() ? description : "") ^ "\n" ^ stack.convert(FM, "\n") ^ "\n").put(std::cerr);
//	this->stack = "";

//	stack.convert(FM, "\n").put(std::cerr);
//	var("\n").put(std::cerr);

	// Hide complexity
//	if (description.contains("var_")) {
	if (description.find("var_") != std::string::npos) {
	    varb_replace_string(description, "exo::var_base<exo::var_mid<exo::var>>", "var");
    	varb_replace_string(description, " [var = exo::var_mid<exo::var>]", "");
    	varb_replace_string(description, " [var = exo::var_mid<exo::var>, ", "[");
	}

	// Break into debugger if EXO_DEBUG is set to 1
	// otherwise allow catch at a higher level or terminate
	var exo_debug;
	if (exo_debug.osgetenv("EXO_DEBUG") and exo_debug == 1) {
		var(description).errputl("\n");
		var(stack()).convert(FM, "\n").errputl();
		debug();
	}
}

// stack - source line acquisition
//
std::string VarError::stack(const std::size_t limit) const {

	// Convert the stack addresses into source code lines
	return exo_backtrace(stack_addresses_, stack_size_, limit);

}

// var friend binary ops implementations
//
// Must be after definition of clone to avoid
// error: explicit specialization of 'clone' after instantiation
#undef VAR_FRIEND
#define VAR_FRIEND
#include "varfriends_impl.h"

}  // namespace exo
