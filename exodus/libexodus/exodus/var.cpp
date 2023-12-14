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

[[noreturn]] void var::throwUndefined(CVR message) const {
	throw VarUndefined(message);
}

[[noreturn]] void var::throwUnassigned(CVR message) const {
	throw VarUnassigned(message);
}

[[noreturn]] void var::throwNonNumeric(CVR message) const {
	throw VarNonNumeric(message);
}

[[noreturn]] void var::throwNonPositive(CVR message) const {
	throw VarNonPositive(message);
}

[[noreturn]] void var::throwNumOverflow(CVR message) const {
	throw VarNumOverflow(message);
}

[[noreturn]] void var::throwNumUnderflow(CVR message) const {
	throw VarNumUnderflow(message);
}

var var::dump() const {
//	std::clog << "DUMP: " << text << " ";
//	if (var_typ & VARTYP_STR)
//		std::clog << "str: " _DQ << var(var_str).first(1024).convert(_ALL_FMS, _VISIBLE_FMS) << _DQ " ";
//	if (var_typ & VARTYP_INT)
//		std::clog << "int:" << var(var_int) << " ";
//	if (var_typ & VARTYP_DBL)
//		std::clog << "dbl:" << var(var_dbl) << " ";
//	std::clog << "typ:" << var_typ << std::endl;
//	return *this;
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
//var backtrace();

// Exception constructors
/////////////////////////

// clang-format off

VarUnassigned     ::VarUnassigned    (CVR errmsg) : VarError("VarUnassigned:"     ^ errmsg) {}
VarDivideByZero   ::VarDivideByZero  (CVR errmsg) : VarError("VarDivideByZero:"   ^ errmsg) {}
VarNonNumeric     ::VarNonNumeric    (CVR errmsg) : VarError("VarNonNumeric:"     ^ errmsg) {}
VarNonPositive    ::VarNonPositive   (CVR errmsg) : VarError("VarNonPositive:"    ^ errmsg) {}
VarNumOverflow    ::VarNumOverflow   (CVR errmsg) : VarError("VarNumOverflow:"    ^ errmsg) {}
VarNumUnderflow   ::VarNumUnderflow  (CVR errmsg) : VarError("VarNumUnderflow:"   ^ errmsg) {}
VarUndefined      ::VarUndefined     (CVR errmsg) : VarError("VarUndefined:"      ^ errmsg) {}
VarOutOfMemory    ::VarOutOfMemory   (CVR errmsg) : VarError("VarOutOfMemory:"    ^ errmsg) {}
VarInvalidPointer ::VarInvalidPointer(CVR errmsg) : VarError("VarInvalidPointer:" ^ errmsg) {}
VarDBException    ::VarDBException   (CVR errmsg) : VarError("VarDBException:"    ^ errmsg) {}
VarNotImplemented ::VarNotImplemented(CVR errmsg) : VarError("VarNotImplemented:" ^ errmsg) {}
VarDebug          ::VarDebug         (CVR errmsg) : VarError("VarDebug"           ^ errmsg) {}

DimNotDimensioned   ::DimNotDimensioned  (CVR errmsg) : VarError("DimNotDimensioned"    ^ errmsg) {}
DimDimensionedZero  ::DimDimensionedZero (CVR errmsg) : VarError("DimDimensionedZero:"  ^ errmsg) {}
DimIndexOutOfBounds ::DimIndexOutOfBounds(CVR errmsg) : VarError("DimIndexOutOfBounds:" ^ errmsg) {}

// clang-format on

VarError::VarError(CVR description_)
	: description(description_) {

	// *** WARNING ***
	// any errors in this constructor
	// will cause recursion and hang/segfault

//	if (description.assigned())
//		description.put(std::cerr);
//	var("\n").put(std::cerr);

	// capture the stack at point of creation i.e. when thrown
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

}  // namespace exodus
