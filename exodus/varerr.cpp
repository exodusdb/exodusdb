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

//#define EXO_VARB_CPP // extern for EXO_SMALLEST_NUMBER
//#include <exodus/var.h>
//#include <exodus/exoimpl.h>

#if EXO_MODULE
	import std;
#else
#	include <string>
#endif

#include "vardefs.h"
#include "varerr.h"
#include "exodebug.h"

namespace exo {

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
VarError::VarError(std::string message_)
	: message(message_) {
	// *** WARNING ***
	// any errors in this constructor
	// will cause recursion and hang/segfault

//	if (message.assigned())
//		message.put(std::cerr);
//	var("\n").put(std::cerr);

	// Capture the stack at point of creation i.e. when thrown
	// TODO capture in caller using default argument to VarError?
	if (not exo_savestack(stack_addresses_, &stack_size_)) {
		// gdb exists but could not attach to the process so it must already be attached.
		// Flush any stdout error messages out
		std::cout << std::flush;
		std::cerr << "\n" << message_ << std::endl;
		// break out into gdb
		debug();
	}

	// Flush any stdout error messages out
	std::cout << std::flush;

//	this->stack = exo_backtrace();
//	((message.assigned() ? message : "") ^ "\n" ^ stack.convert(FM, "\n") ^ "\n").put(std::cerr);
//	this->stack = "";

//	stack.convert(FM, "\n").put(std::cerr);
//	var("\n").put(std::cerr);

	// Hide complexity
//	if (message.contains("var_")) {
	if (message.find("var_") != std::string::npos) {
	    varb_replace_string(message, "exo::var_base<exo::var_mid<exo::var>>", "var");
    	varb_replace_string(message, " [var = exo::var_mid<exo::var>]", "");
    	varb_replace_string(message, " [var = exo::var_mid<exo::var>, ", "[");
	}

	// Break into debugger if EXO_DEBUG is set to 1
	// otherwise allow catch at a higher level or terminate
	// Probably already broken into debugger in the exo_savestack call above.
//	var exo_debug;
//	if (exo_debug.osgetenv("EXO_DEBUG") and exo_debug == 1) {
//		var(message).errputl("\n");
//		var(stack()).convert(FM, "\n").errputl();
//		debug();
//	}
}

// stack - source line acquisition
//
std::string VarError::stack(const std::size_t limit) const {

	// Convert the stack addresses into source code lines
	return exo_backtrace(stack_addresses_, stack_size_, limit);

}

}  // namespace exo
