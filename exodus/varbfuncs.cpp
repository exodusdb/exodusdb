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

/* UTF-8 bytewise encoding
Binary    Hex          Comments
0xxxxxxx  0x00..0x7F   Only byte of a 1-byte character encoding
10xxxxxx  0x80..0xBF   Continuation bytes (1-3 continuation bytes)
110xxxxx  0xC0..0xDF   First byte of a 2-byte character encoding
1110xxxx  0xE0..0xEF   First byte of a 3-byte character encoding
11110xxx  0xF0..0xF4   First byte of a 4-byte character encoding
*/

#ifdef EXO_MODULE
	import std;
#else
#	include <string>
#	include <utility> // For std::move
#	include <locale>  // For std::use_facet
#endif

#include <boost/version.hpp>
#include <boost/locale.hpp> // For locate aware compare

#include <exodus/varimpl.h>

namespace exo {

// exodus uses one locale per thread
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
thread_local std::locale thread_boost_locale1;
#pragma clang diagnostic pop

static void init_boost_locale1() {
	if (thread_boost_locale1.name() != "*") {
		boost::locale::generator generator1;
		thread_boost_locale1 = generator1("");
	}
	return;
}

bool VB1::assigned() const {

	// THISIS("bool var::assigned() const")

	// treat undefined as unassigned
	// undefined is a state where we are USING the variable before its contructor has been
	// called! which is possible (in syntax like var xx.osread()?) and also when passing default
	// variables to functions in the callables on ISVAR(gcc)

	if (var_typ & VARTYP_MASK)
		return false;

	return var_typ != VARTYP_UNA;
}

bool VB1::unassigned() const {
	// see explanation above in assigned
	// THISIS("bool var::unassigned() const")
	// assertVar(function_sig);

	if (var_typ & VARTYP_MASK)
		return true;

	return !var_typ;
}

std::u32string VB1::to_u32string() const {

	 THISIS("std::u32string var::to_u32string() const")
	 assertString(function_sig);

	// 1.4 secs per 10,000,000 var=var copies of 3 byte ASCII strings
	// simple var=var copy of the following data

	// 14.9 secs round trips u8->u32->u8 per 10,000,000 on vm7
	// SKIPS/TRIMS OUT any bad utf8
	return boost::locale::conv::utf_to_utf<char32_t>(var_str);
}

std::wstring VB1::to_wstring() const {

	 THISIS("std::wstring var::to_wstring() const")
	 assertString(function_sig);

	// 1.4 secs per 10,000,000 var=var copies of 3 byte ASCII strings
	// simple var=var copy of the following data

	// 14.9 secs round trips u8->w->u8 per 10,000,000 on vm7
	// SKIPS/TRIMS OUT any bad utf8
	return boost::locale::conv::utf_to_utf<wchar_t>(var_str);
}

void VB1::from_u32string(std::u32string u32str) const {
	// for speed, dont validate
	// THISIS("void var::from_u32tring() const")
	// assertVar(function_sig);
	var_typ = VARTYP_STR;

	var_str = boost::locale::conv::utf_to_utf<char>(u32str);
	return;
}

// CONSTRUCTOR from const std::u32string converts to utf-8
// Not inline to avoid including boost headers in varb.h
VB1::var_base(const std::wstring& wstr1) {
	var_typ = VARTYP_STR;
	var_str = boost::locale::conv::utf_to_utf<char>(wstr1);
}

int VB1::localeAwareCompare(const std::string& str1, const std::string& str2) {

	// https://www.boost.org/doc/libs/1_70_0/libs/locale/doc/html/collation.html
	// eg ensure lower case sorts before uppercase (despite "A" \x41 is less than "a" \x61)
	init_boost_locale1();

	// Level
	// 0 = primary – ignore accents and character case, comparing base letters only. For example "facade"
	// and "Façade" are the same.
	// 1 = secondary – ignore character case but consider accents. "facade" and
	// "façade" are different but "Façade" and "façade" are the same.
	// 3 = tertiary – consider both case and
	// accents: "Façade" and "façade" are different. Ignore punctuation.
	// 4 = quaternary – consider all case,
	// accents, and punctuation. The words must be identical in terms of Unicode representation.
	// 5 = identical – as quaternary, but compare code points as well.
	int result = std::use_facet<boost::locale::collator<char>>(thread_boost_locale1)
#define COMP_LEVEL identical
#if BOOST_VERSION < 108300
				.compare(boost::locale::collator_base::COMP_LEVEL, str1, str2);
#else
				.compare(boost::locale::collate_level::COMP_LEVEL, str1, str2);
#endif

	return result;
}

void VB1::defaulter(CBR defaultvalue) {

	// See explanation above in assigned
	// assertVar(function_sig);

	THISIS("void var::defaulter(in defaultvalue)")
	ISASSIGNED(defaultvalue)

	// Allow undefined usage like var xyz=xyz.readnext();
	// if (var_typ & VARTYP_MASK)

	if (this->unassigned()) UNLIKELY{
		*this = defaultvalue;
	}
	return;
}

RETVAR VB1::or_default(CBR defaultvalue) const {

	THISIS("var  var::or_default(in defaultvalue) const")
	ISASSIGNED(defaultvalue)

	if (this->unassigned()) {
		return *static_cast<const exo::var*>(&defaultvalue);
	} else {
		return *static_cast<const exo::var*>(this);
	}
}

const char* VB1::c_str() const& {

	THISIS("const char* var::c_str() const")
	assertString(function_sig);

	return var_str.c_str();
}

char VB1::toChar() const {

	THISIS("char var::toChar() const")
	assertString(function_sig);

	if (var_str.empty())
		return '\0';

	return var_str[0];
}

// temporary var can return move its string into the output
std::string VB1::toString() && {

	THISIS("str  var::toString() &&")
	assertString(function_sig);

	return std::move(var_str);
}

// non-temporary var can return a const ref to its string
const std::string& VB1::toString() const& {

	THISIS("str  var::toString() const&")
	assertString(function_sig);

	return var_str;
}

void VB1::move(VARBASEREF tovar) {

	THISIS("void var::move(io tovar)")
	assertAssigned(function_sig);
	ISVAR(tovar)

	// move the string
	tovar.var_str = std::move(var_str);

	// set source var to empty string
	var_str.clear();
	var_typ = VARTYP_STR;

	// copy the rest over
	tovar.var_typ = var_typ;
	tovar.var_int = var_int;
	tovar.var_dbl = var_dbl;

	return;
}

// Const version needed in calculatex
// Identical code except signature is not const
void VB1::swap(CBR var2) const {

	THISIS("CVR  var::swap(in var2) const")

	// Works on unassigned vars
	assertVar(function_sig);
	ISVAR(var2)

	// copy var2 to temp
	auto mvtypex = var2.var_typ;
	auto mvintx = var2.var_int;
	auto mvdblx = var2.var_dbl;

	// swap strings
	var_str.swap(var2.var_str);

	// copy var1 to var2
	var2.var_typ = var_typ;
	var2.var_int = var_int;
	var2.var_dbl = var_dbl;

	// copy temp to var1
	var_typ = mvtypex;
	var_int = mvintx;
	var_dbl = mvdblx;

	return;
}

// non-const version
//VBR1 VB1::swap(VARBASEREF var2) {
void VB1::swap(VARBASEREF var2) {

	THISIS("io   var::swap(io var2)")

	// Works on unassigned vars
	assertVar(function_sig);
	ISVAR(var2)

	// copy var2 to temp
	auto mvtypex = var2.var_typ;
	auto mvintx = var2.var_int;
	auto mvdblx = var2.var_dbl;

	// swap strings
	var_str.swap(var2.var_str);

	// copy var1 to var2
	var2.var_typ = var_typ;
	var2.var_int = var_int;
	var2.var_dbl = var_dbl;

	// copy temp to var1
	var_typ = mvtypex;
	var_int = mvintx;
	var_dbl = mvdblx;

	return;
}

}  // namespace exo
