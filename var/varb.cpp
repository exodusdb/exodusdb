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

#define EXO_VARB_CPP // extern for EXO_SMALLEST_NUMBER
#include <var/var.h>
//#include <exodus/exoimpl.h>

namespace exo {

//	RETVAR VB1::clone() const {
//	CONSTEXPR
	void VB1::assertInteger(const char* message, const char* varname /*= ""*/) const {
		assertNumeric(message, varname);
		if (!(var_typ & VARTYP_INT)) {

			//var_int = std::floor(var_dbl);

			// Truncate double to int
			//var_int = std::trunc(var_dbl);
			if (var_dbl >= 0) {
				// 2.9 -> 2
				// 2.9999 -> 2
				// 2.99999 -> 3
				var_int = static_cast<varint_t>(var_dbl + EXO_SMALLEST_NUMBER / 10);
			} else {
				// -2.9 -> -2
				// -2.9999 -> -2.9
				// -2.99999 -> -3
				var_int = static_cast<varint_t>(var_dbl - EXO_SMALLEST_NUMBER / 10);
			}

			// Add int flag
			var_typ |= VARTYP_INT;
		}
	}

//// clone to var
////
//CONSTEXPR
//operator VB1::var() &{
//	EXO_SNITCH("var_base cp>v")
//	return this->clone();
//}
//
//// move to var
////
//CONSTEXPR
//operator VB1::var() && {
//	EXO_SNITCH("var_base mv>v")
//	return this->move();
//}

// ::clone to var
//
RETVAR VB1::clone() const {

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
RETVAR VB1::move() {

    THISIS("var  var::move()")
    assertAssigned(function_sig);

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
//	var_typ = VARTYP_UNA;
	var_typ = VARTYP_STR;

	// TODO
	// Clear int and dbl?

	return rvo;
}

// Synonym for length for compatibility with pick's len() which is bytes
var  VB1::len() const {

	THISIS("var  var::len() const")
	assertString(function_sig);

	return var_str.size();
}

bool VB1::empty() const {

	THISIS("var  var::empty() const")
	assertString(function_sig);

	return var_str.empty();
}

// ::first_
//
// utility string function on var_base for throwing errors
// TODO refactor out
//
RETVAR VB1::first_(int nchars) const {
	assertString(__PRETTY_FUNCTION__);
	if (nchars < static_cast<int>(var_str.size()))
		return var_str.substr(0, nchars);
	return var_str;
}

// ::dump
//
RETVAR VB1::dump() const {
	var nrvo = "var:";

	// Append the var's address
	const var var_addr = std::int64_t(static_cast<const void*>(this));
	nrvo ^= "0x" ^ var_addr.oconv("MX").lcase();

	// Append the var typ as a single int.
//	nrvo ^= "typ:" ^ (var_base(int(var_typ)).oconv("MB").trimfirst("0") + 0);
	nrvo ^= " typ:" ^ (var_base(int(var_typ))/*.oconv("MB").trimfirst("0") + 0*/);

	if (var_typ & VARTYP_OSFILE) {
		nrvo ^= " osfile:";
		nrvo ^= " int:" ^ var_base(var_int);
		nrvo ^= " dbl:" ^ var_base(var_dbl);
	} else {

		if (var_typ & VARTYP_INT)
			nrvo ^= " int:" ^ var_base(var_int);

		if (var_typ & VARTYP_DBL)
			nrvo ^= " dbl:" ^ var_base(var_dbl);
	}

	if (var_typ & VARTYP_NAN)
		nrvo ^= " nan:";

	// Append the string if present
	if (var_typ & VARTYP_STR) {

		nrvo ^= " str:";

		// Append the string's address if not within the var e.g. it on the heap. not SSO
		const char* str_start = var_str.c_str();
		const char* obj_start = reinterpret_cast<const char*>(&var_str);
		std::ptrdiff_t diff = str_start - obj_start;
		if (std::abs(diff) > 32) {
			const var str_addr = std::int64_t(static_cast<const void*>(var_str.c_str()));
			nrvo ^= "0x" ^ str_addr.oconv("MX").lcase();
		}

		nrvo ^= " " _DQ ^ var_base(var_str).first_(1024).convert(_ALL_FMS, _VISIBLE_FMS) ^ _DQ;
	}

	return nrvo;
}

// std::string utility
inline void varb_replace_string (std::string& subject, const std::string& search, const std::string& replace) {
	std::size_t pos = 0;
//	while
	if ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
//		pos += replace.length();
		}
	}

// var friend binary ops implementations
//
// Must be after definition of clone to avoid
// error: explicit specialization of 'clone' after instantiation
#undef VAR_FRIEND
#define VAR_FRIEND
#include "varb_friends_impl.h"

}  // namespace exo
