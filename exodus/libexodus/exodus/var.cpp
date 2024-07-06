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

namespace exodus {

// Note: VAR_TEMPLATE both defines and instantiates an instance of the member function for var_base<var>::xxxxxxxxxxx

#if __clang_major__
#	define NORETURN
#else
#	define NORETURN [[noreturn]]
#endif
VAR_TEMPLATE(NORETURN void VARBASE::throwUndefined(CBR message) const) {
	throw VarUndefined(message);
}

VAR_TEMPLATE(NORETURN void VARBASE::throwUnassigned(CBR message) const) {
	throw VarUnassigned(message);
}

VAR_TEMPLATE(NORETURN void VARBASE::throwNonNumeric(CBR message) const) {
	throw VarNonNumeric(message);
}

VAR_TEMPLATE(NORETURN void VARBASE::throwNonPositive(CBR message) const) {
	throw VarNonPositive(message);
}

VAR_TEMPLATE(NORETURN void VARBASE::throwNumOverflow(CBR message) const) {
	throw VarNumOverflow(message);
}

VAR_TEMPLATE(NORETURN void VARBASE::throwNumUnderflow(CBR message) const) {
	throw VarNumUnderflow(message);
}

VAR_TEMPLATE(RETVAR VARBASE::dump() const) {
	var nrvo = "var: ";

	if (var_typ & VARTYP_STR)
		nrvo ^= "str: " _DQ ^ var(var_str).first(1024).convert(_ALL_FMS, _VISIBLE_FMS) ^ _DQ " ";
	if (var_typ & VARTYP_INT)
		nrvo ^= "int:" ^ var(var_int) ^ " ";
	if (var_typ & VARTYP_DBL)
		nrvo ^= "dbl:" ^ var(var_dbl) ^ " ";
	nrvo ^= "typ:" ^ (var(int(var_typ)).oconv("MB").trimfirst("0") + 0);
	return nrvo;
}

// Forward declaration of free functions defined in mvdebug.cpp
void save_stack_addresses();

// Exception constructors
/////////////////////////

// clang-format off

VarUnassigned      ::VarUnassigned      (CBR errmsg) : VarError("VarUnassigned:"       ^ errmsg) {}
VarDivideByZero    ::VarDivideByZero    (CBR errmsg) : VarError("VarDivideByZero:"     ^ errmsg) {}
VarNonNumeric      ::VarNonNumeric      (CBR errmsg) : VarError("VarNonNumeric:"       ^ errmsg) {}
VarNonPositive     ::VarNonPositive     (CBR errmsg) : VarError("VarNonPositive:"      ^ errmsg) {}
VarNumOverflow     ::VarNumOverflow     (CBR errmsg) : VarError("VarNumOverflow:"      ^ errmsg) {}
VarNumUnderflow    ::VarNumUnderflow    (CBR errmsg) : VarError("VarNumUnderflow:"     ^ errmsg) {}
VarUndefined       ::VarUndefined       (CBR errmsg) : VarError("VarUndefined:"        ^ errmsg) {}
VarOutOfMemory     ::VarOutOfMemory     (CBR errmsg) : VarError("VarOutOfMemory:"      ^ errmsg) {}
VarInvalidPointer  ::VarInvalidPointer  (CBR errmsg) : VarError("VarInvalidPointer:"   ^ errmsg) {}
VarDBException     ::VarDBException     (CBR errmsg) : VarError("VarDBException:"      ^ errmsg) {}
VarNotImplemented  ::VarNotImplemented  (CBR errmsg) : VarError("VarNotImplemented:"   ^ errmsg) {}
VarDebug           ::VarDebug           (CBR errmsg) : VarError("VarDebug"             ^ errmsg) {}

DimNotDimensioned  ::DimNotDimensioned  (CBR errmsg) : VarError("DimNotDimensioned"    ^ errmsg) {}
DimDimensionedZero ::DimDimensionedZero (CBR errmsg) : VarError("DimDimensionedZero:"  ^ errmsg) {}
DimIndexOutOfBounds::DimIndexOutOfBounds(CBR errmsg) : VarError("DimIndexOutOfBounds:" ^ errmsg) {}

// clang-format on

VarError::VarError(CVR description_)
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
		description.errputl("\n");
		stack().convert(FM, "\n").errputl();
		debug();
	}
}

var VarError::stack(const size_t limit) const {

	// Convert the stack addresses into source code lines
	return exo_backtrace(stack_addresses_, stack_size_, limit);

}

// DEFINE implementation of all friends previously friended in var.h
////////////////////////////////////////////////////////////////////
#undef VAR_FRIEND
#define VAR_FRIEND
#include "varfriends_impl.h"

}  // namespace exodus
