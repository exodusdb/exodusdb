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

#define EXO_VAR_CPP	// indicates globals are to be defined (omit extern keyword)
#include <var.h>
#include <varerr.h>
//#include <exodus/varimpl.h>
#include <exodus/exoimpl.h>

namespace exo {

// Note: VAR_TEMPLATE both defines and instantiates an instance of the member function for var_base<var>::xxxxxxxxxxx

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

// basic string function on var_base for throwing errors
template<> PUBLIC RETVAR VARBASE1::first_(int nchars) const {
	assertString(__PRETTY_FUNCTION__);
	if (nchars < static_cast<int>(var_str.size()))
		return var_str.substr(0, nchars);
	return var_str;
}

template<> PUBLIC RETVAR VARBASE1::dump() const {
	var nrvo = "var: ";

	if (var_typ & VARTYP_STR)
		nrvo ^= "str: " _DQ ^ var_base(var_str).first_(1024).convert(_ALL_FMS, _VISIBLE_FMS) ^ _DQ " ";
	if (var_typ & VARTYP_INT)
		nrvo ^= "int:" ^ var_base(var_int) ^ " ";
	if (var_typ & VARTYP_DBL)
		nrvo ^= "dbl:" ^ var_base(var_dbl) ^ " ";
//	nrvo ^= "typ:" ^ (var_base(int(var_typ)).oconv("MB").trimfirst("0") + 0);
	nrvo ^= "typ:" ^ (var_base(int(var_typ))/*.oconv("MB").trimfirst("0") + 0*/);
	return nrvo;
}

// Forward declaration of free functions defined in mvdebug.cpp
void save_stack_addresses();

// Exception constructors
/////////////////////////

// clang-format off

VarUnassigned      ::VarUnassigned      (std::string errmsg) : VarError("VarUnassigned:"       + errmsg) {}
VarDivideByZero    ::VarDivideByZero    (std::string errmsg) : VarError("VarDivideByZero:"     + errmsg) {}
VarNonNumeric      ::VarNonNumeric      (std::string errmsg) : VarError("VarNonNumeric:"       + errmsg) {}
VarNonPositive     ::VarNonPositive     (std::string errmsg) : VarError("VarNonPositive:"      + errmsg) {}
VarNumOverflow     ::VarNumOverflow     (std::string errmsg) : VarError("VarNumOverflow:"      + errmsg) {}
VarNumUnderflow    ::VarNumUnderflow    (std::string errmsg) : VarError("VarNumUnderflow:"     + errmsg) {}
VarUndefined       ::VarUndefined       (std::string errmsg) : VarError("VarUndefined:"        + errmsg) {}
VarOutOfMemory     ::VarOutOfMemory     (std::string errmsg) : VarError("VarOutOfMemory:"      + errmsg) {}
VarInvalidPointer  ::VarInvalidPointer  (std::string errmsg) : VarError("VarInvalidPointer:"   + errmsg) {}
VarDBException     ::VarDBException     (std::string errmsg) : VarError("VarDBException:"      + errmsg) {}
VarNotImplemented  ::VarNotImplemented  (std::string errmsg) : VarError("VarNotImplemented:"   + errmsg) {}
VarDebug           ::VarDebug           (std::string errmsg) : VarError("VarDebug"             + errmsg) {}

DimNotDimensioned  ::DimNotDimensioned  (std::string errmsg) : VarError("DimNotDimensioned"    + errmsg) {}
DimDimensionedZero ::DimDimensionedZero (std::string errmsg) : VarError("DimDimensionedZero:"  + errmsg) {}
DimIndexOutOfBounds::DimIndexOutOfBounds(std::string errmsg) : VarError("DimIndexOutOfBounds:" + errmsg) {}

// clang-format on

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

//	this->stack = exo_backtrace();
//	((description.assigned() ? description : "") ^ "\n" ^ stack.convert(FM, "\n") ^ "\n").put(std::cerr);
//	this->stack = "";

//	stack.convert(FM, "\n").put(std::cerr);
//	var("\n").put(std::cerr);

	// Break into debugger if EXO_DEBUG is set to non-zero
	// otherwise allow catch at a higher level or terminate
	var exo_debug;
	if (exo_debug.osgetenv("EXO_DEBUG") and exo_debug) {
		var(description).errputl("\n");
		var(stack()).convert(FM, "\n").errputl();
		debug();
	}
}

std::string VarError::stack(const size_t limit) const {

	// Convert the stack addresses into source code lines
	return exo_backtrace(stack_addresses_, stack_size_, limit);

}

// TODO replace .f with a version of .f that returns a string_view
// instead of wasting time constructing a temporary var only to extract a single char from it
ND RETVAR var_proxy1::at(const int pos1) const {
	//TODO! allow at() to work directly on var_str without extracting the required field first
	var v1 = var_.f(fn_);
	return v1.at(pos1);
}

ND var var_proxy2::at(const int pos1) const {
	var v1 = var_.f(fn_, vn_);
	return v1.at(pos1);
}

ND var var_proxy3::at(const int pos1) const {
	var v1 = var_.f(fn_, vn_, sn_);
	return v1.at(pos1);
}

// DEFINE implementation of all friends previously friended in var.h
////////////////////////////////////////////////////////////////////
#undef VAR_FRIEND
#define VAR_FRIEND
#include "varfriends_impl.h"

}  // namespace exo
