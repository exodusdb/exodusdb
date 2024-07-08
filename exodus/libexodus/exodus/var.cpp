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

template RETVAR VARBASE1::dump() const;
template<typename var> RETVAR VARBASE2::dump() const {
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

// DEFINE implementation of all friends previously friended in var.h
////////////////////////////////////////////////////////////////////
#undef VAR_FRIEND
#define VAR_FRIEND
#include "varfriends_impl.h"

}  // namespace exo
