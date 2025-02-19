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

//#include <math.h>    //for abs(double) and stod
//#include <cstdlib>  //for exit

//#include <utility> //for move
//#include <mutex> //for lock_guard
//module #include <string>

#if __has_include(<signal.h>)
//#include <signal.h>	 //for raise(SIGTRAP)
#endif

/* using fastfloat instead of ryu because it is > 3 time faster
	but it doesnt prevent excessively long decimal ASCII input
	and unconfirmed if it does supports round trip ascii->double->ascii
	//from test_perf.cpp
    var v("1234.5678");  //23ns
    //v.isnum();         //+ 56ns ( 79ns using fastfloat)
    //v.isnum();         //+184ns (207ns using ryu)
    //v.isnum();         //+194ns (217ns using std::stod)
    //v.isnum();         //+???ns (???ns using <charconv> from_chars)
*/

//fastfloat  "1234.5678" -> 1234.5678  56ns (but does it round trip?)
//ryu        "1234.5678" -> 1234.5678 184ns
//std::stod  "1234.5678" -> 1234.5678 192ns (but does it round trip?)
//from_chars "1234.5678" -> 1234.5678 ???ns (but does it round trip?)

//Eisel-Lemire algorithm seems to be fastest "parse chars to double" according to Lavavej at Microsoft
//Feb 2021 https://reviews.llvm.org/D70631 "There have been a couple of algorithmic developments very recently
// - for parsing (from_chars), the Eisel-Lemire algorithm is much faster than the technique used here.
// I would have used it if it were available back then!"
//
//double only available in gcc 11 onwards. msvc has it from 2017 or 19
//https://github.com/fastfloat/fast_float
#if __has_include(<fast_float/fast_float.h>)
#define HAS_FASTFLOAT
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#pragma clang diagnostic ignored "-Wreserved-identifier"
#include <fast_float/fast_float.h>
#pragma clang diagnostic pop
#define STD_OR_FASTFLOAT fast_float

#elif __GNUC__ >= 11 || __clang_major__ >= 14
#define USE_CHARCONV
#define STD_OR_FASTFLOAT std

//#elif __has_include(<ryu/ryu.h>)
//#define HAS_RYU
//#include <ryu/ryu.h>
#endif

//gcc 10 doesnt include conv from and to floating point
//#include <charconv>	 // for from_chars and to_chars

//#include <sstream>
//#include <iomanip>    //for setprecision

//#include <iostream> //cin and cout
//#include <memory>   //for unique_ptr

#include <boost/locale.hpp>
//#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include <exodus/varimpl.h>
//#include <exodus/mvutf.h>
//#include <exodus/varlocale.h>

// Keep doxygen happy
#undef DEFAULT_SPACE
#define DEFAULT_SPACE
#undef DEFAULT_EMPTY
#define DEFAULT_EMPTY
#undef DEFAULT_FM
#define DEFAULT_FM

// std::ios::sync_with_stdio(false);
//static bool desynced_with_stdio = false;

// TODO check that all string increase operations dont crash the system

namespace exo {

// output/errput/logput not threadsafe but probably not a problem
//inline std::mutex global_mutex_threadstream;
//#define LOCKIOSTREAM_OR_NOT std::lock_guard guard(global_mutex_threadstream);
#define LOCKIOSTREAM_OR_NOT

// exodus uses one locale per thread
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
static thread_local std::locale thread_boost_locale1;
#pragma clang diagnostic pop

static void init_boost_locale1() {
	if (thread_boost_locale1.name() != "*") {
		boost::locale::generator generator1;
		thread_boost_locale1 = generator1("");
	}
}

// Utility to check char ASCII (0-127)

// On x86, comparing a char for equality with an int is generally quicker than comparing it with another char.

// This is because on x86, int is the native word size, so operations on ints are usually faster than operations
// on smaller types like char. When comparing a char with an int, the char is usually sign-extended to an int
// (if it's a signed char) or zero-extended to an int (if it's an unsigned char),
// and then the comparison is performed as an int comparison. This can be done with a single instruction.

static inline bool char_is_ASCII(char c1) {
	// c++ char type signedness is implementation defined. typically unsigned on x86 and signed on arm except IOS
	// Therefore must NOT do comparisons < unless we know the chars are ASCII 0-127, nor do arithmetic outside 0-127.
	// 8th/top bit indicates non-ASCII. Could be signed or unsigned depending on compiler
	return (c1 & 0b1000'0000) == 0;
}

// Utility to check std::string for ASCII (0-127)

static inline bool sv_is_ASCII(std::string_view str1) {
	// Optimise for ASCII
	// try ASCII uppercase to start with for speed
	// this may not be correct for all locales. eg Turkish I i İ ı mixing Latin and Turkish
	// letters.
	//for (const char& c : str1) {
	for (const char c : str1) {
		if (! char_is_ASCII(c))
			return false;
	}
	return true;
}

// Generic helper to handle char and u32_char wise conversion (mapping)

template<typename T1, typename T2, typename T3>
static void string_converter(T1& var_str, const T2 fromchars, const T3 tochars) {
	typename T1::size_type pos = T1::npos;

	// Optimise for single character replacement
	// No observable speedup
	//if (fromchars.len() == 1 and tochars.len() == 1) {
	//	std::replace(var_str.begin(), var_str.end(), fromchars[0], tochars[0]);
	//}

	auto tochars_size = tochars.size();
	while (true) {
		// locate (backwards) any of the from characters
		// because we might be removing characters
		// and it is faster to remove last character first
		pos = var_str.find_last_of(fromchars, pos);

		if (pos == T1::npos)
			break;

		// find which from character we have found
		//int fromcharn = static_cast<int>(fromchars.find(var_str[pos]));
		auto fromcharn = fromchars.find(var_str[pos]);

		if (fromcharn < tochars_size)
			//var_str.replace(pos, 1, tochars.substr(fromcharn, 1));
			var_str[pos] = tochars[fromcharn];
			//var_str.replace(pos, 1, tochars[fromcharn]);
		else
			var_str.erase(pos, 1);

		if (pos == 0)
			break;

		pos--;
	}
	return;
}

template<> PUBLIC bool VARBASE1::assigned() const {
	// THISIS("bool var::assigned() const")

	// treat undefined as unassigned
	// undefined is a state where we are USING the variable before its contructor has been
	// called! which is possible (in syntax like var xx.osread()?) and also when passing default
	// variables to functions in the callables on ISDEFINED(gcc)

	if (var_typ & VARTYP_MASK)
		return false;

	return var_typ != VARTYP_UNA;
}

template<> PUBLIC bool VARBASE1::unassigned() const {
	// see explanation above in assigned
	// THISIS("bool var::unassigned() const")
	// assertDefined(function_sig);

	if (var_typ & VARTYP_MASK)
		return true;

	return !var_typ;
}

template<> PUBLIC std::u32string VARBASE1::to_u32string() const {

	 THISIS("std::u32string var::to_u32string() const")
	 assertString(function_sig);

	// 1.4 secs per 10,000,000 var=var copies of 3 byte ASCII strings
	// simple var=var copy of the following data

	// 14.9 secs round trips u8->u32->u8 per 10,000,000 on vm7
	// SKIPS/TRIMS OUT any bad utf8
	return boost::locale::conv::utf_to_utf<char32_t>(var_str);
}

template<> PUBLIC std::wstring VARBASE1::to_wstring() const {

	 THISIS("std::wstring var::to_wstring() const")
	 assertString(function_sig);

	// 1.4 secs per 10,000,000 var=var copies of 3 byte ASCII strings
	// simple var=var copy of the following data

	// 14.9 secs round trips u8->w->u8 per 10,000,000 on vm7
	// SKIPS/TRIMS OUT any bad utf8
	return boost::locale::conv::utf_to_utf<wchar_t>(var_str);
}

template<> PUBLIC void VARBASE1::from_u32string(std::u32string u32str) const {
	// for speed, dont validate
	// THISIS("void var::from_u32tring() const")
	// assertDefined(function_sig);
	var_typ = VARTYP_STR;

	var_str = boost::locale::conv::utf_to_utf<char>(u32str);
}

// CONSTRUCTOR from const std::u32string converts to utf-8
template<> PUBLIC VARBASE1::var_base(const std::wstring& wstr1) {
	var_typ = VARTYP_STR;
	var_str = boost::locale::conv::utf_to_utf<char>(wstr1);
}

//int var::localeAwareCompare(const std::string& str1, const std::string& str2) {
template<> PUBLIC int VARBASE1::localeAwareCompare(const std::string& str1, const std::string& str2) {
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
	//#define COMP_LEVEL identical

#define COMP_LEVEL identical

//	boost::string_view str1b(str1.data(), str1.size());
//	boost::string_view str2b(str2.data(), str2.size());

	int result = std::use_facet<boost::locale::collator<char>>(thread_boost_locale1)
#if BOOST_VERSION < 108300
					.compare(boost::locale::collator_base::COMP_LEVEL, str1, str2);
#else
					.compare(boost::locale::collate_level::COMP_LEVEL, str1, str2);
#endif

	//var(str1).outputl("str1=");
	//var(str2).outputl("str2=");
	//var(result).outputl("comp=");

	return result;
}

var  var::version() const {
	return var(__DATE__).iconv("D").oconv("D") ^ " " ^ var(__TIME__);
}

bool var::eof() const {
	// THISIS("bool var::eof() const")
	// assertDefined(function_sig);

	return std::cin.eof();
}

bool var::hasinput(int milliseconds) const {
	//declare in haskey.cpp
	bool haskey(int milliseconds);

	//LOCKIOSTREAM_OR_NOT

	return haskey(milliseconds);
}

// Not in a terminal - Binary safe except for \n (\r\n for MS)
//
// In a terminal - Backspace and \n (\r\n for MS) are lost.
//                 Pressing Ctrl+d (Ctrl+z fro MS) indicates eof
//
io   var::input() {

	THISIS("bool var::input()")
	assertDefined(function_sig);

	var_str.clear();
	var_typ = VARTYP_STR;

	//LOCKIOSTREAM_OR_NOT

	if (!std::cin.eof())
		std::getline(std::cin, var_str);

	return *this;
}

// input with prompt allows default value and editing if isterminal
io   var::input(in prompt) {

	THISIS("bool var::input(in prompt")
	assertDefined(function_sig);
	ISSTRING(prompt)

	//LOCKIOSTREAM_OR_NOT

	var default_input = this->assigned() ? (*this) : "";

	var_str.clear();
	var_typ = VARTYP_STR;

	//output any prompt and flush
	if (prompt.len())
		prompt.output().osflush();

	//windows currently doesnt allow line editing
	if (not this->isterminal() or SLASH_IS_BACKSLASH) {
		this->input();
	}

	//linux terminal input line editing
	else {
		//replace double quotes with \"
		default_input.replacer("\"", "\\\"");
		var cmd = "bash -c 'read -i " ^ default_input.quote() ^ " -e EXO_TEMP_READ && printf \"%s\" \"$EXO_TEMP_READ\"'";
		//cmd.outputl("cmd=");
		if (not this->osshellread(cmd))
			lasterror().logputl();
		if ((*this) == "")
			std::cout << std::endl;
	}

	return *this;
}

// for nchars, use int instead of var to trigger error at point of calling not here
// not binary safe if nchars = 0 because we allow line editing assuming terminal console
io   var::inputn(const int nchars) {

	THISIS("bool var::inputn(const int nchars")
	assertDefined(function_sig);

	//LOCKIOSTREAM_OR_NOT

	var_str.clear();
	var_typ = VARTYP_STR;

	//declare function in getkey.cpp
	char getkey(void);

	//input whatever characters are available into this var a return true if more than none
	// quit if error or EOF
	if (nchars < 0) {

		for (;;) {
			char char1;
			{
				//LOCKIOSTREAM_OR_NOT
				char1 = getkey();
			}

			// Quit if no (more) characters available
			// really should test for EOF which is usually -1
			// Ctrl+D usually terminates input in posix
			if (char1 < 0)
				break;

			//var_str += char1;
			var_str.push_back(char1);
		}
	}

	//input a certain number of characters input this var and return true if more than none
	else if (nchars > 0) {

		while (!eof()) {

			char char1;
			{
				//LOCKIOSTREAM_OR_NOT
				char1 = getkey();
			}

			// try again after a short delay if no key and not enough characters yet
			// TODO implement as poll/epoll/select
			// Really should test for EOF which is usually -1
			// Ctrl+D usually terminates input in posix
			if (char1 < 0) {
				this->ossleep(100);
				continue;
			}

			// Enter/Return key always returns whatever has been entered so far
			//if (char1 < 0 || char1 == 0x0d)
			//	break;

			// add the character to the output
			//var_str += char1;
			var_str.push_back(char1);

			// quit if got the desired number of characters
			//nchars cannot be negative at this point
			if (var_str.size() >= static_cast<unsigned int>(nchars))
				break;
		}

	} else {
		this->input();
	}

	return *this;
}

template<> PUBLIC void VARBASE1::default_to(CBX defaultvalue) {

	// see explanation above in assigned
	// assertDefined(function_sig);

	THISIS("io   var::default_to(in defaultvalue) const")
	ISASSIGNED(defaultvalue)

	//?allow undefined usage like var xyz=xyz.readnext();
	// if (var_typ & VARTYP_MASK)

	if (this->unassigned()) UNLIKELY{
		*this = defaultvalue;
	}
	return;// *this;
}

template<> PUBLIC RETVAR VARBASE1::default_from(CBX defaultvalue) const {

	THISIS("io   var::default_from(in defaultvalue)")
	ISASSIGNED(defaultvalue)

	if (this->unassigned()) {
		return *static_cast<const exo::var*>(&defaultvalue);
	} else {
		return *static_cast<const exo::var*>(this);
	}
}

template<> PUBLIC const char* VARBASE1::c_str() const& {

	THISIS("const char* var::c_str() const")
	assertString(function_sig);

	return var_str.c_str();
}

template<> PUBLIC char VARBASE1::toChar() const {

	THISIS("char var::toChar() const")
	assertString(function_sig);

	if (var_str.empty())
		return '\0';

	return var_str[0];
}

// temporary var can return move its string into the output
template<> PUBLIC std::string VARBASE1::toString() && {

	THISIS("str  var::toString() &&")
	assertString(function_sig);

	return std::move(var_str);
}

// non-temporary var can return a const ref to its string
template<> PUBLIC const std::string& VARBASE1::toString() const& {

	THISIS("str  var::toString() const&")
	assertString(function_sig);

	return var_str;
}

// synonym for length for compatibility with pick's len() which is bytes
var  var::len() const {

	THISIS("var  var::len() const")
	assertString(function_sig);

	return var_str.size();
}

// Implementation of os wcswidth for terminal output of wide characters
// https://mitchellh.com/writing/grapheme-clusters-in-terminals
// list of terminal program support. Need switch on/off for programming?
// https://www.man7.org/linux/man-pages/man3/wcswidth.3.html
// The behavior of wcswidth() depends on the LC_CTYPE category of
// the current locale.
var  var::textwidth() const {

	THISIS("var  var::textwidth() const")
	assertString(function_sig);

	// If all ASCII then return count of non-control chars
	auto size = var_str.size();
	for (const unsigned char c : var_str) {
		if (!char_is_ASCII(c)) UNLIKELY
			goto unicode;
		if (c < '\x20') UNLIKELY
			size--;
	}
	return size;

    //int wcswidth(const wchar_t *s, std::size_t n);
unicode:
	auto wstr1 = this->to_wstring();
	auto width = wcswidth(wstr1.data(),std::string::npos);

	// If any control chars present then replace them with spaces and try again
	if (width < 0) {
		int ncontrolchars = 0;
		std::replace_if(
			wstr1.begin(),
			wstr1.end(),
			// lambda function to replace and count control chars (0x00 to 0x1f)
			[&ncontrolchars](const char ch) {
				if (ch >= 32) LIKELY
					return false;
				ncontrolchars++;
				return true;
			},
			' '
		);
		width = wcswidth(wstr1.data(),std::string::npos) - ncontrolchars;

		// If still cannot get length then report an error and terminate
		if (width < 0) {
			if (not this->oswrite("error_textwidth.txt")) {
				this->lasterror().errputl();
			}
			throw VarError("EXODUS: Error in " ^ var(__PRETTY_FUNCTION__) ^ " wcswidth failed - see error_textwidth.txt");
		}
	}

	return width;
}

//const char* var::data() const {
//
//	THISIS("const char* var::data() const")
//	assertString(function_sig);
//
//	return var_str.data();
//}

var  var::textlen() const {

	THISIS("var  var::textlen()")
	assertString(function_sig);

	var result = 0;
	for (const char c : var_str) {
		// Count bytes that dont start with 10 in the high end two bits, since these are utf-8 continuation bytes
		result.var_int += (c & 0b1100'0000) != 0b1000'0000;
		//std::cout << c << " " << std::bitset<8>(c) << " " << result.var_int << std::endl;
	}
	return result;
}

///////////////////////////////////////////////////////////////////////
// trim - remove leading, trailing and excess internal spaces/character
///////////////////////////////////////////////////////////////////////

// trim

// Const
var  var::trim(SV trimchars DEFAULT_SPACE) const& {

	THISIS("io   var::trim(SV trimchars)")
	assertString(function_sig);

	return boost::algorithm::trim_all_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
io   var::trimmer(SV trimchars DEFAULT_SPACE) {

	// TODO reimplement with boost string trim_if algorithm
	// http://www.boost.org/doc/libs/1_39_0/doc/html/string_algo/reference.html

	THISIS("io   var::trimmer(SV trimchars)")
	assertStringMutator(function_sig);

//	trimmerlast_helper(var_str, trimchars);
//	trimmerfirst_helper(var_str, trimchars);
//	//trimmerinner_helper only works after first and last trimchars are removed
//	trimmerinner_helper(var_str, trimchars);
	boost::algorithm::trim_all_if(var_str, boost::algorithm::is_any_of(trimchars));

	return *this;
}

// trimfirst - remove leading/left bytes

// Const
var  var::trimfirst(SV trimchars DEFAULT_SPACE) const& {

	THISIS("io   var::trimfirst(SV trimchars) const&")
	assertString(function_sig);

	return boost::algorithm::trim_left_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
io   var::trimmerfirst(SV trimchars DEFAULT_SPACE) {

	THISIS("io   var::trimmerfirst(SV trimchars)")
	assertStringMutator(function_sig);

	boost::algorithm::trim_left_if(var_str, boost::algorithm::is_any_of(trimchars));

	return *this;
}

// trimlast() - trim trailing/right bytes

// Const
var  var::trimlast(SV trimchars DEFAULT_SPACE) const& {

	THISIS("io   var::trimlast(SV trimchars) const&")
	assertString(function_sig);

	return boost::algorithm::trim_right_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
io   var::trimmerlast(SV trimchars DEFAULT_SPACE) {

	THISIS("io   var::trimmerlast(SV trimchars)")
	assertStringMutator(function_sig);

	//trimmerlast_helper(var_str, trimchars);
	boost::algorithm::trim_right_if(var_str, boost::algorithm::is_any_of(trimchars));

	return *this;
}

//trimboth() - remove leading and trailing spaces/characters but not inner

// Const
var  var::trimboth(SV trimchars DEFAULT_SPACE) const& {

	THISIS("io   var::trimboth(SV trimchars) const&")
	assertString(function_sig);

	return boost::algorithm::trim_copy_if(var_str, boost::algorithm::is_any_of(trimchars));
}

// Mutate
io   var::trimmerboth(SV trimchars DEFAULT_SPACE) {

	THISIS("io   var::trimmerboth(SV trimchars)")
	assertStringMutator(function_sig);

	boost::algorithm::trim_if(var_str, boost::algorithm::is_any_of(trimchars));

	return *this;
}

// invert() - inverts lower 8 bits of UTF8 codepoints (not bytes)

// Mutate
io   var::inverter() {

	THISIS("io   var::inverter()")
	assertStringMutator(function_sig);

	// xor each unicode code point, with the bits we want to toggle ... ie the bottom 8
	// since we will keep inversion within the same 256 byte pages of unicode codepoints
	// TODO invert directly in the UTF8 bytes - requires some cleverness

	// convert to char32.t string - four bytes per code point
	//std::u32string u32string1 = this->to_u32string();
	std::u32string u32_str1(*this);

	// invert only the lower 8 bits to keep the resultant code points within the same unicode
	// 256 byte page
	for (auto& c : u32_str1)
		c ^= char32_t(255);

	// convert back to utf8
	//this->from_u32string(u32_str1);
	*this = var(u32_str1);

	return *this;
}

// ucase() - upper case

// Const
var  var::ucase() const& {

// Performance testing indicates 20% faster to
// bulk clone a string and then amend it char by char.
// than copy the characters one by one to the new location.
// When there are few char conversions required, the
// performance advantage seems to be up to 50%.
//
// so we will use clone and ucaser.
//
//#if __cpp_lib_string_resize_and_overwrite < 202110L
	return this->clone().ucaser();
//#else
//
//	THISIS("var  var::ucase() const&")
//	assertString(function_sig);
//
//	var rvo;
//	rvo.var_typ = VARTYP_STR;
//
//	rvo.var_str.resize_and_overwrite(
//		var_str.size(),
//		[this](char* buf, std::size_t /*buf_size*/) noexcept {
//
////			assert(buf_size == var_str.size());
//
//			for (char c : var_str) {
//
//				// Quit if any non-ASCII
//				if (!char_is_ASCII(c))
//					return std::size_t(0);
//
//				// ASCII ucase
//				if (c >= 'a' && c <= 'z')
//					c -= 'z' - 'Z';
//
//				// rvo.var_str
//				*buf++ = c;
//			}
//
//			// Flag to resize_and_overwrite its new string size
//			// which we arrange to be the same as the requested buffer size.
//			return var_str.size();
//		}
//	);
//
//	//  If any unicode then run full unicode uppercase
//
////	// On unconverted remainder of var_str
////	if (rvo.var_str.size() != var_str.size())
////		rvo.var_str += unicode_to_upper(var_str.substr(rvo.var_str.size()));
//
//	if (rvo.var_str.size() != var_str.size()) {
//
//		// ~6ms! For very first call for non-ASCII string ucaser or lcaser
//		// ~ 70ns for a one char non-ASCII string
//		// ~ 2.5ns per additional char in non-ASCII string
//		init_boost_locale1();
//
//		// Using boost (which calls icu)
//		rvo.var_str = boost::locale::to_upper(var_str, thread_boost_locale1);
//	}
//
//	return rvo;
//#endif
}

// Mutate
io   var::ucaser() {

	THISIS("io   var::ucaser()")
	assertStringMutator(function_sig);

#define EXO_ULCASE_TRY_ASCII
#ifdef EXO_ULCASE_TRY_ASCII

	// Optimise for ASCII
	// Attempt fast byte-wise conversion of ASCII
	// Try ASCII uppercase to start with for speed
	// TODO This may not be correct for all locales.
	// e.g. Turkish I i İ ı mixing ASCII Latin and UNICODE Turkish letters.
	// Perhaps only detect ASCII if the default locale is currently C.

	// If pure ASCII
	//
	// ~ 10 times faster than unicode case conversion
	//
	// 'in function' time measured using timebank/RAII)

	// 100,000 chars with no conversion required.

	// gcc 14 Ubuntu 24.04
	// 0.45 ns/char ucase and lcase regardless of if need to convert or not

	// clang 18 Ubuntu 24.04
	//      ~ 1ns to "convert" an empty string
	// + ~ 0.33ns per ASCII character unconverted (clang 18)
	// + ~ 0.45ns per ASCII character converted (all other compilers and versions)

	// Strangely clang 18 can ucase faster when no conversion is required
	// but lcase does not show the same ability.
	// Putting LIKELY destroys this optimisation.

	// return localeAwareChangeCase(1);

	auto allASCII = true;
	for (char& c : var_str) {

		// C/C++ standard says that it is implementation defined whether char is signed or unsigned.
		// On x86 char is generally signed and char is numerically from 0 up to +255
		// On ARM char is generally unsigned (except IOS) and char is numerically -128 up to +127
		// So char < char is implementation defined.

		// Check for ASCII by setting the lower 7 bits to zero and checking if 0.
		// Checking for ASCII will eliminate the need for static_cast<signed char>(c) below

		if (!(allASCII = char_is_ASCII(c)))
			break;

		// Using std::toupper - 2ns/char
		// toupper returns an int despite being given a char
		// Presumably safe to cast back to char
		// c = static_cast<char>(std::toupper(c));

		// Hand written code ~.33ns/op 5-6 times faster than std::toupper (which returns an int)
//		if (std::islower(c))
//			c = std::toupper(c);
		// Focus on lower case ASAP by testing >= 'a' 1st
		// 0x41 = A
		// 0x61 = a
		if (c >= 'a' && c <= 'z') {
			// LIKELY   // Stops clang 18 optimising ucase when no conversion is required
			// UNLIKELY // No change in performance
			c -= '\x20'; // 'a' - 'A';
		}
	}
	if (allASCII)
		return *this;
#endif

	// ~6ms! For very first call for non-ASCII string ucaser or lcaser
	// ~ 70ns for a one char non-ASCII string
	// ~ 2.5ns per additional char in non-ASCII string
	init_boost_locale1();

	// boost calls icu

//	// Using boost locale
//	// gcc 14 100,000 chars with no conversion required and no ASCII preconv above
//	// 5.2 ns/char ucase
//	// 5.0 ns/char lcase
	var_str = boost::locale::to_upper(var_str, thread_boost_locale1);

	// Using boost algorithm - doesnt work
	// gcc 14 100,000 chars with no conversion required and no ASCII preconv above.
	// 4.2 ns/char ucase
	// 4.0 ns/char lcase
//	boost::algorithm::to_upper(var_str, thread_boost_locale1);

	return *this;
}

// lcase() - lower case

// Mutate
io   var::lcaser() {

	THISIS("io   var::lcaser()")
	assertStringMutator(function_sig);

	// For comments, see ucaser above.

#ifdef EXO_ULCASE_TRY_ASCII

	auto allASCII = true;
	for (char& c : var_str) {

		if (!(allASCII = char_is_ASCII(c)))
			break;

		// Skip already lower case ASAP by testing <= 'Z' 1st
		if (c <= 'Z' && c >= 'A') {
			c += '\x20';
			continue;
		}

	}
	if (allASCII)
		return *this;
#endif

	init_boost_locale1();

	var_str = boost::locale::to_lower(var_str, thread_boost_locale1);
//	boost::algorithm::to_lower(var_str, thread_boost_locale1);

	return *this;
}


// tcase() - title case

// Mutate
io   var::tcaser() {

	THISIS("io   var::tcaser()")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	init_boost_locale1();

	// should not title 1 2 or 3 letter words or perhaps just a list of exceptions as follows:
	// a abaft about above afore after along amid among an apud as aside at atop below but by
	// circa down for from given in into lest like mid midst minus near next of off on onto out
	// over pace past per plus pro qua round sans save since than thru till times to under until
	// unto up upon via vice with worth the and nor or yet so or a an as at but by circa for
	// from in into like mid of on onto out over per pro qua sans than thru to until unto up
	// upon via vice with the and nor or yet so

	var_str = boost::locale::to_title(var_str, thread_boost_locale1);

	return *this;
}


// fcase()
// fold case - standardise text for indexing/searching
// https://www.w3.org/International/wiki/Case_folding
// Note that accents are sometimes significant and sometime not. e.g. in French
//  cote (rating)
//  coté (highly regarded)
//  côte (coast)
//  côté (side)
// Case Folding - is a process of converting a text to case independent representation.
// For example case folding for a word "Grüßen" is "grüssen"
// where the letter "ß" is represented in case independent way as "ss".

// Mutate
io   var::fcaser() {

	THISIS("io   var::fcaser()")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	init_boost_locale1();

	var_str = boost::locale::fold_case(var_str, thread_boost_locale1);

	return *this;
}

// *** USED TO NORMALISE ALL KEYS BEFORE READING OR WRITING ***
// *** otherwise can have two record with similar keys á and á
// postgres gives FALSE for the following:
// SELECT 'á' = 'á';

// normalise()
// normalise (unicode NFC, C=Compact ... norm_nfc)
// see "Unicode Normalization Forms" https://unicode.org/reports/tr15/
//"It is crucial that Normalization Forms remain stable over time. That is, if a string that does
// not
// have any unassigned characters is normalized under one version of Unicode,
// it must remain normalized under all future versions of Unicode."

// Mutate
io   var::normalizer() {

	THISIS("io   var::normalizer()")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	// optimise for ASCII which needs no normalisation
	if (sv_is_ASCII(var_str))
		return (*this);

	init_boost_locale1();

	// TODO see if the quick way to check if already in NFC format
	// is available in some library or if it is already built into boost locale normalize
	// because checking for being in normal form is very fast according to unicode docs

	// norm_nfc
	var_str = boost::locale::normalize(var_str, boost::locale::norm_nfc, thread_boost_locale1);

	return *this;
}


// There is no memory or performance advantage for mutable call, only a consistent syntax for user

// Mutate
io   var::uniquer() {
	*this = this->unique();
	return *this;
}

// Constant
var  var::unique() const& {

	THISIS("var  var::unique()")
	assertString(function_sig);

	// linemark
	var result = "";
	var start = 0;
	var bit;
	var delimiter;
	var sepchar = VM;
	int RMseq_plus1 = RM.seq() + 1;
	// bool founddelimiter = false;
	while (true) {

		// bit=this->remove(start, delimiter);
		bit = this->substr2(start, delimiter);

		// if (!founddelimiter && delimiter)
		if (delimiter)
			// sepchar=RM_-static_cast<int>(delimiter)+1;
			sepchar = var().chr(RMseq_plus1 - delimiter);

		if (bit.len()) {
			if (not(result.locateusing(sepchar, bit))) {
				//if (delimiter)
				result ^= bit ^ sepchar;
			}
		}
		if (not delimiter)
			break;
	}  // loop;
	//result.popper();
	if (not result.var_str.empty())
		result.var_str.pop_back();

	return result;
}

// BINARY - 1st byte
var  var::seq() const {

	THISIS("var  var::seq() const")
	assertString(function_sig);

	if (var_str.empty())
		return "";

	int byteno = var_str[0];
	if (byteno >= 0)
		return byteno;
	else
		return byteno + 256;
}

// UTF8 - 1st UTF code point
var  var::textseq() const {

	THISIS("var  var::textseq() const")
	assertString(function_sig);

	if (var_str.empty())
		return "";

	// get four bytes from input string since in UTF8 a unicode code point may occupy up to 4
	// bytes
	std::u32string str1 = boost::locale::conv::utf_to_utf<char32_t>(var_str.substr(0, 4));

	return uint32_t(str1[0]);
}

// only returns BINARY bytes 0-255 (128-255) cannot be stored in the database unless with other
// bytes making up UTF8
var  var::chr(const int charno) const {
	return static_cast<char>(charno % 256);
}

// returns unicode 1-4 byte sequences (in utf8)
// returns empty string for some invalid unicode points like 0xD800 to 0xDFFF which is reserved for
// UTF16 0x110000 ... is invalid too
var  var::textchr(const int utf_codepoint) const {
	// doesnt use *this at all (do we need a version that does?)

	// return var((char) int1);

	if (!utf_codepoint)
		return std::string("\0", 1);

	std::wstring wstr1;
	wstr1.push_back(wchar_t(uint32_t(utf_codepoint)));
	return boost::locale::conv::utf_to_utf<char>(wstr1);
}


////////
// QUOTE - wrap with double quotes
////////

// Const
var var::quote() const& {

	THISIS("var  var::quote() const&")
	assertString(function_sig);

	var rvo = DQ_;
	rvo.var_str.append(var_str);
	rvo.var_str.push_back(DQ_);

	return rvo;
}

// Mutate
io   var::quoter() {

	THISIS("io   var::quoter()")
	assertStringMutator(function_sig);

	// Use std::string "replace" to insert
	var_str.replace(0, 0, _DQ);
	var_str.push_back(DQ_);

	return *this;
}


/////////
// SQUOTE - wrap with single quotes
/////////

// Const
var  var::squote() const& {

	THISIS("var  var::squote() const&")
	assertString(function_sig);

	var rvo = SQ_;
	rvo.var_str.append(var_str);
	rvo.var_str.push_back(SQ_);

	return rvo;
}

// Mutate
io   var::squoter() {

	THISIS("io   var::squoter()")
	assertStringMutator(function_sig);

	// std::string "replace" to insert
	var_str.replace(0, 0, _SQ);
	var_str.push_back('\'');

	return *this;
}


//////////
// UNQUOTE - remove outer double or single quotes
//////////

// Const
var   var::unquote() const& {

	THISIS("var  var::unquote() const&")
	assertString(function_sig);

	// Removes MATCHING beginning and terminating " or ' characters.
	// Also removes a SINGLE " or ' on the grounds that you probably want to eliminate all such
	// characters.

	var rvo;
	rvo.var_typ = VARTYP_STR;
	char char0;

	// No change if no length
	std::size_t len = var_str.size();
	if (len < 2) {
return_this:
		rvo.var_str = var_str;
		return rvo;
	}

	char0 = var_str[0];

	// No change if not starting " or '
	if (char0 != DQ_ && char0 != SQ_)
		goto return_this;

	// No change if terminating character ne starting character
	if (var_str.back() != char0)
		goto return_this;

	// Copy over all except the first character
	rvo.var_str = var_str.substr(1);

	// Remove the last char if any
	if (!rvo.var_str.empty())
		rvo.var_str.pop_back();

	return rvo;
}

// Mutate
io   var::unquoter() {

	THISIS("io   var::unquoter()")
	assertStringMutator(function_sig);

	// removes MATCHING beginning and terminating " or ' characters
	// also removes a SINGLE " or ' on the grounds that you probably want to eliminate all such
	// characters

	// no change if no length
	std::size_t len = var_str.size();
	if (len < 2)
		return *this;

	char char0 = var_str[0];

	// no change if not starting " or '
	if (char0 != '\"' && char0 != '\'')
		return *this;

	// no change if terminating character ne starting character
	if (var_str[len - 1] != char0)
		return *this;

	// erase first (and last character if more than one)
	var_str.erase(0, 1);
	if (len)
		var_str.erase(len - 2, 1);

	return *this;
}


////////
// PASTE
////////

// 1. paste replace

// Mutate
io   var::paster(const int pos1, const int length, SV insertstr) {

	THISIS("io   var::paster(const int pos1, const int length, SV insertstr)")
	assertStringMutator(function_sig);
	//ISSTRING(insertstr)

	std::size_t start0;
	std::size_t lengthb;

	// First work out start index from start position
	// Depends on length of string, if position is negative
	if (pos1 <= 0) {
		if (pos1 == 0 || static_cast<std::size_t>(-pos1) > var_str.size())
			// Negative start index means 0
			// abcdef[-8,2] -> abcdef[1,2] ie ab will be replaced
			start0 = 0;
		else
			// abcdef[-3,2] -> abcdef[4,2] ie de will be replaced
			start0 = var_str.size() + pos1;
	}
	else
		start0 = pos1 - 1;

	// Negative length simply moves the start char backwards
	// and the length is up to and including the start char
	if (length < 0) {

		// abcdef[4,-2]  -> abcdef[3,2]
		// abcdef[4,-99] -> abcdef[1,4]

		//int start0_save = start0;
		lengthb = start0 + 1;

		//start0 = start0 + length + 1;
		if (static_cast<std::size_t>(-length) > start0) {
			lengthb = start0 + 1;
			start0 = 0;
		} else {
			start0 = start0 + length + 1;
			lengthb = -length;
		}
//		if (start0 < 0) {
//			start0 = 0;
//			//lengthb = start0_save + 1;
//		} else
//			lengthb = -length;

		//std::cerr << "start0  = " << start0  << std::endl;
		//std::cerr << "lengthb = " << lengthb << std::endl;
	} else
		lengthb = length;

	if (static_cast<unsigned int>(start0) >= var_str.size()) {
		//if (newstr.var_str.size())
			var_str += insertstr;
	} else {
		//if (insertstr.var_str.size())
			var_str.replace(start0, lengthb, insertstr);
		//else
		//	var_str.erase(start0,lengthb);
	}

	return *this;
}
//
//// 2. paste over to end
//
//// Constant
//var  var::pasteall(const int pos1, SV insertstr) const& {
//	// TODO avoid copy
//	return var(*this).pasterall(pos1, insertstr);
//}
//
//// Mutate
//io   var::pasterall(const int pos1, SV insertstr) {
//
//	THISIS("io   var::pasterall(const int pos1, SV insertstr)")
//	assertStringMutator(function_sig);
//	//ISSTRING(insertstr)
//
//	// TODO make sure start and length work like pickos and HANDLE NEGATIVE LENGTH!
//	int pos1b;
//	if (pos1 > 0)
//		pos1b = pos1;
//	else if (pos1 < 0) {
//		pos1b = static_cast<int>(var_str.size()) + pos1 + 1;
//		if (pos1b < 1)
//			pos1b = 1;
//	} else
//		pos1b = 1;
//
//	if (static_cast<unsigned int>(pos1b) > var_str.size())
//		var_str += insertstr;
//	else
//		var_str.replace(pos1b - 1, var_str.size(), insertstr);
//
//	return *this;
//}

// 3. paste insert at

// Mutate
io   var::paster(const int pos1, SV insertstr) {

	THISIS("io   var::paster(const int pos1, SV insertstr)")
	assertStringMutator(function_sig);
	//ISSTRING(insertstr)

	if (pos1 > 0) {
		if (static_cast<unsigned int>(pos1) > var_str.size())
			// abc(4, x) -> abcx
			// abc(5, x) -> abcx
			var_str += insertstr;
		else
			// abc(1, x) -> xabc
			// abc(2, x) -> axbc
			// abc(3, x) -> abxc
			var_str.insert(pos1 - 1, insertstr);
	}

	else if (pos1 < 0) {
		if (static_cast<unsigned int>(-pos1) >= var_str.size())
			// abc(-4, x) -> xabc
			// abc(-3, x) -> xabc
			var_str.insert(0, insertstr);
		else
			// abc(-2, x) -> axbc
			// abc(-1, x) -> abxc
			var_str.insert(var_str.size() + pos1, insertstr);
	}
	else
		// abc(0, x) -> xabc
		var_str.insert(0, insertstr);

	return *this;
}

/////////
// PREFIX - insert at beginning
/////////

// Constant
var  var::prefix(SV insertstr) const& {

	THISIS("var  var::prefix(SV insertstr)")
	assertString(function_sig);

	var nrvo = insertstr;

	nrvo.var_str.append(this->var_str);

	return nrvo;
}

// Mutate
io   var::prefixer(SV insertstr) {

	THISIS("io   var::prefixer(SV insertstr)")
	assertStringMutator(function_sig);

	var_str.insert(0, insertstr);

	return *this;
}


//////
// POP -remove last byte of string
//////

// Mutate
io   var::popper() {

	THISIS("io   var::popper()")
	assertStringMutator(function_sig);

	if (!var_str.empty())
		var_str.pop_back();

	return *this;
}


template<> PUBLIC VBR1 VARBASE1::move(VBX tovar) {

	THISIS("io   var::move(io tovar)")
	assertAssigned(function_sig);
	ISDEFINED(tovar)

	// move the string
	tovar.var_str = std::move(var_str);

	// set source var to empty string
	var_str.clear();
	var_typ = VARTYP_STR;

	// copy the rest over
	tovar.var_typ = var_typ;
	tovar.var_int = var_int;
	tovar.var_dbl = var_dbl;

	return tovar;
}

// Const version needed in calculatex
template<> PUBLIC CBR VARBASE1::swap(CBX var2) const {

	THISIS("CVR  var::swap(in var2) const")
	// Works on unassigned vars
	assertDefined(function_sig);
	ISDEFINED(var2)

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

	return *this;
}

// non-const version
template<> PUBLIC VBR1 VARBASE1::swap(VBX var2) {

	THISIS("io   var::swap(io var2)")
	// Works on unassigned vars
	assertDefined(function_sig);
	ISDEFINED(var2)

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

	return *this;
}

var  var::str(const int num) const {

	THISIS("var  var::str(const int num) const")
	assertString(function_sig);

	var newstr = "";

	//negative num returns "" in loop below
	if (num < 0)
		return newstr;

	int basestrlen = static_cast<int>(var_str.size());
	if (basestrlen == 1) {
		newstr.var_str.resize(num, var_str.at(0));
	}
	else if (basestrlen) {
		for (int ii = num; ii > 0; --ii)
			newstr.var_str.append(var_str);
	}
	return newstr;
}

var  var::space() const {

	THISIS("var  var::space() const")
	assertNumeric(function_sig);

	var newstr = "";
	int nspaces = this->round().toInt();
	if (nspaces > 0)
		newstr.var_str.resize(nspaces, ' ');

	return newstr;
}


//crop() - Remove superfluous FM, VM. e.g. VM before FM etc.

// Mutate
io   var::cropper() {

	THISIS("io   var::cropper()")
	assertStringMutator(function_sig);

	auto cur_iter = var_str.begin();
	auto end_iter = var_str.end();

	auto beg_iter = var_str.begin();
	auto out_iter = beg_iter;

	while (cur_iter != end_iter) {

		char charx = *cur_iter++;

		// Simply append ordinary characters
		if (!char_is_ASCII(charx) || charx > RM_ || charx < ST_) {
			*out_iter++ = charx;
			continue;
		}

		// Found one of the fms

		// "Remove" all lower separators from the end of the current output
		// by decrementing the output iterator
		// e.g. aaa]]]^bbb -> aaa^bbb
		while (out_iter != beg_iter) {
			char lastchar = *(out_iter-1);
			if (char_is_ASCII(lastchar) && lastchar >= ST_ && lastchar < charx)
				out_iter--;
			else
				break;
		}

		// "Append" the separator by incrementing the output iterator
		*out_iter++ = charx;
	}

	// Trim the string to its new size
	// Determined by the output iterator
	var_str.resize(static_cast<std::size_t>(out_iter - var_str.begin()));

	// Remove all trailing fms.
	// The above algorithm only trims superfluous fms within the string.
	auto last_not_fm = var_str.find_last_not_of(_RM _FM _VM _SM _TM _ST);
	if (last_not_fm == std::string::npos)
		var_str.clear();
	else
		var_str.resize(last_not_fm + 1);

	return *this;
}


// lower() drops FM to VM, VM to SM etc.

// Mutate
io   var::lowerer() {

	THISIS("io   var::lowerer()")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	// note: rotate lowest sep to highest
	//this->converter(_RM _FM _VM _SM _TM _ST, _FM _VM _SM _TM _ST _RM);

	//bottom marks get crushed together but ST is infrequently used
	// reversible by raiser only if no ST chars are present - which are not common
//	this->converter(_RM _FM _VM _SM _TM, _FM _VM _SM _TM _ST);
//	string_converter(var_str, _RM _FM _VM _SM _TM, _FM _VM _SM _TM _ST);

	for (char& c : var_str) {

		// static_cast<unsigned char>(c) is well-defined ONLY if the value of c is within the range of unsigned char which is 0-127.
		// If the value is outside the range 0-127, the behavior is implementation-defined.
		// In other words you cannot work with non-ASCII characters reliably.

		// Skip non-ASCII
		if (!char_is_ASCII(c))
			continue;

		// Skip > RM_
		if (c > RM_) LIKELY
			continue;

		// Bump DOWN all field marks except the bottom one (ST_)
		if (c > ST_)
			c -=1;
	}

	return *this;
}

// raise() lifts VM to FM, SM to VM etc.

// Mutate
io   var::raiser() {

	THISIS("io   var::raiser()")
	assertStringMutator(function_sig);
	//assertString(function_sig);

	// note: rotate highest sep to lowest
	// advantage is it is reversible by lowerer but the problem is that the smallest delimiter becomes the largest
	//this->converter(_FM _VM _SM _TM _ST _RM, _RM _FM _VM _SM _TM _ST);

	// top two marks get crushed together but RM is rarely used
	// reversible by lowerer only if no RM are present - which are rare
//	this->converter(_FM _VM _SM _TM _ST, _RM _FM _VM _SM _TM);
//	string_converter(var_str, _FM _VM _SM _TM _ST, _RM _FM _VM _SM _TM);

	for (char& c : var_str) {

		// static_cast<unsigned char>(c) is well-defined ONLY if the value of c is within the range of unsigned char which is 0-127.
		// If the value is outside the range 0-127, the behavior is implementation-defined.
		// In other words you cannot work with non-ASCII characters reliably.

		// Skip non-ASCII
		if (!char_is_ASCII(c))
			continue;

		// Skip > FM_
		if (c > FM_) LIKELY
			continue;

		// Bump UP all field marks except the top one (RT_)
		if (c >= ST_)
			c +=1;
	}

	return *this;
}


// convert() - replaces one by one in string, a list of characters with another list of characters
// if the target list is shorter than the source list of characters then characters are deleted
//var  var::convert(in fromchars, in tochars) const& {

// Mutate
//io   var::converter(in fromchars, in tochars) {
io   var::converter(SV fromchars, SV tochars) {

	THISIS("io   var::converter(SV fromchars, SV tochars)")
	assertStringMutator(function_sig);

	//string_converter(var_str, fromchars.var_str, tochars.var_str);
	string_converter(var_str, fromchars, tochars);

	return *this;
}


//// Mutate for const char*
//io   var::converter(const char* fromchars, const char* tochars) {
//
//	THISIS("io   var::converter(const char* fromchars, const char* tochars)")
//	assertStringMutator(function_sig);
//
//	string_converter(var_str, std::string(fromchars), std::string(tochars));
//
//	return *this;
//}

// Mutate
io   var::textconverter(SV fromchars, SV tochars) {

	THISIS("io   var::converter(in fromchars, in tochars)")
	assertStringMutator(function_sig);

	// all ASCII -> bytewise conversion for speed
	if (sv_is_ASCII(fromchars) && sv_is_ASCII(tochars)) {
		string_converter(var_str, fromchars, tochars);
	}

	// any non-ASCI -> convert to wide before conversion, then back again
	else {

		// convert everything to from UTF8 to wide string
		std::u32string u32_str1 = this->to_u32string();
		//std::u32string u32_fromchars = var(fromchars).to_u32string();
		//std::u32string u32_tochars = var(tochars).to_u32string();
		//std::u32string u32_fromchars = boost::locale::conv::utf_to_utf<char32_t>(fromchars);
		//std::u32string u32_tochars = boost::locale::conv::utf_to_utf<char32_t>(tochars);
		std::u32string u32_fromchars = boost::locale::conv::utf_to_utf<char32_t>(std::string(fromchars));
		std::u32string u32_tochars = boost::locale::conv::utf_to_utf<char32_t>(std::string(tochars));

		// convert the wide characters
		string_converter(u32_str1, u32_fromchars, u32_tochars);

		// convert the string back to UTF8 from wide string
		//this->from_u32string(u32_str1);
		*this = var(u32_str1);
	}
	return *this;
}

// parse() - replaces seps with FMs except inside double and single quotes. Backslash escapes.


// Mutate
io   var::parser(char sepchar) {

	THISIS("io   var::parser(char sepchar)")
	assertStringMutator(function_sig);

//	//std::string s = "abc 'def gh'qwe";
//	std::string s1 = R"___(abc 'def gh\' q'w "' e)___";
//	//std::string s1 = R"___(a 'b c' d")___";
//	std::string s2 = s1;

	std::size_t len = var_str.size();
	if (!len)
		return *this;

	// The following bytes can be escaped so spaces, double and single quotes are ignored
	std::size_t pos = 0;
	char ch;

next_unquoted:
	while (pos < len) {

		// Replace separator with FM and skip to the next char
		ch = var_str[pos];
		if (ch == sepchar) {
			var_str[pos] = FM_;
			pos++;
			continue;
		}

		switch (ch) {

			// Switch into double quoted mode
			case DQ_:

				// Skip over all following characters up to the next double quote
				// Ignore escaped double quote
				pos++;
				while (pos < len) {
					switch (var_str[pos]) {

						case DQ_:
							// Skip over double quote and looping inside quote
							pos++;
							goto next_unquoted;

						case BS_:
							// Skip over the BS and the following character if any
							pos++;
							pos++;
							break;

						default:
							pos++;
					}
				} // inside quotes

				continue;

			// Switch into single quoted mode
			case SQ_:

				// Skip over all following characters up to the next single quote
				// Ignore escaped single quote
				pos++;
				while (pos < len) {
					switch (var_str[pos]) {

						case SQ_:
							// Skip over single quote and looping inside quote
							pos++;
							goto next_unquoted;

						case BS_:
							// Skip over the BS and the following character if any
							pos++;
							pos++;
							break;

						default:
							pos++;
					}
				} // inside quotes

				continue;

			case BS_:
				pos++;
				pos++;
				continue;

			default:
				pos++;

		} // switch
	}
	return *this;
}

////////
// FCOUNT
////////
// TODO make a char and char version for speed
var  var::fcount(SV sep) const {

	THISIS("var  var::fcount(SV sep) const")
	assertString(function_sig);
	//ISSTRING(sep)

	if (var_str.empty())
		return 0;

	if (sep.empty())
		return "";

	return this->count(sep) + 1;
}
////////
// COUNT
////////

var  var::count(SV str) const {

	THISIS("var  var::count(SV str) const")
	assertString(function_sig);

	if (str.empty())
		return "";

	std::size_t substr_len = str.size();

	// find the starting position of the field or return ""
	std::size_t start_pos = 0;
	int fieldno = 0;
	while (true) {
		start_pos = var_str.find(str, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return fieldno;
		// start_pos++;
		start_pos += substr_len;
		fieldno++;
	}
}

////////
// INDEX
////////

//// 1 based starting byte no of an occurrence or 0 if not present
//var  var::index(SV substr) const {
//	return this->index2(substr, 1);
//}

// 1 based starting byte no of first occurrence starting from byte no, or 0 if not present
var  var::index(SV substr, const int startindex) const {

	THISIS("var  var::index(SV substr, const int startindex) const")
	assertString(function_sig);

	if (substr.empty())
		return "";

	// find the starting position of the field or return ""
	std::size_t start_pos = startindex - 1;
	start_pos = var_str.find(substr, start_pos);

	// not found, return 0
	if (start_pos == std::string::npos)
		return 0;

	return var(static_cast<int>(start_pos) + 1);
}

// reverse search
// 1 based starting byte no of first occurrence starting from byte no, or 0 if not present
var  var::indexr(SV substr, const int startindex) const {

	THISIS("var  var::indexr(SV substr, const int startindex) const")
	assertString(function_sig);

	if (substr.empty())
		return "";

	std::size_t start_pos;

	if (startindex == 0) {
		start_pos = std::string::npos;

	} else if (startindex > 0) {
		start_pos = startindex - 1;

	} else {

		// startindex < 1

		// prevent negative index starting before beginning of string
		// e.g. -startindex(-2) > strsize(1) is true
		if (static_cast<std::size_t>(-startindex) > var_str.size())
			return 0;

		// e.g. startpos(0) = strsize(1) + startindex(-1)
		start_pos = var_str.size() + startindex;
	}

	// find the starting position of the target
	start_pos = var_str.rfind(substr, start_pos);

	// not found, return 0
	if (start_pos == std::string::npos)
		return 0;

	return var(static_cast<int>(start_pos) + 1);
}

// 1 based starting byte no of an occurrence or 0 if not present
var  var::indexn(SV substr, const int occurrenceno) const {

	//THISIS("var  var::index(SV substr, const int occurrenceno) const")
	THISIS("var  var::index(SV substr) const")
	assertString(function_sig);

	//TODO implement negative occurenceno as meaning backwards from the end
	//eg -1 means the last occurrence

	if (substr.empty())
		return "";

	std::size_t start_pos = 0;
	std::size_t substr_len = substr.size();

	// negative and 0th occurrence mean the first
	int countdown = occurrenceno >= 1 ? occurrenceno : 1;

	for (;;) {

		// find the starting position of the field or return ""
		start_pos = var_str.find(substr, start_pos);

		// past of of string?
		if (start_pos == std::string::npos)
			return 0;

		--countdown;

		// found the right occurrence
		if (countdown == 0)
			return (static_cast<int>(start_pos) + 1);

		// skip to character after substr (not just next character)
		// start_pos++;
		start_pos += substr_len;
	}

	// should never get here
	//std::unreachable();
	//return 0;
}

// fieldno can be "" to return the whole record (0 returns the key)
// TODO provide a version with int fieldno to handle the most frequent case
// although may also support dictid (of target file) instead of fieldno

var  var::xlate(in filename, in fieldno, const char* mode) const {

	THISIS("var  var::xlate(in filename, in fieldno, const char* mode) const")
	assertString(function_sig);
	ISSTRING(filename)
	// fieldnames are supported as exoprog::xlate
	// but not here in var::xlate which only supports field numbers since it has no
	// access to dictionaries
	ISNUMERIC(fieldno)

	// open the file (skip this for now since sql doesnt need "open"
	var file;
	// if (!file.open(filename))
	//{
	//	_STATUS=filename^" does not exist";
	//	record="";
	//	return record;
	//}
	//file MUST be lower case in order to detect "dict."
	file = filename.lcase();

	char sep = fieldno.len() ? VM_ : RM_;

	var response = "";
	int nmv = this->fcount(_VM);
	for (int vn = 1; vn <= nmv; ++vn) {

		//test every time instead of always appending and removing at the end
		//because the vast majority of xlate are single valued so it is faster
		if (vn > 1)
			response ^= sep;

		// read the record
		var key = this->f(1, vn);
		var record;
		if (!record.readc(file, key)) {
			// if record doesnt exist then "", or original key if mode is "C"

			// no record and mode C returns the key
			// gcc warning: comparison with string literal results in unspecified
			// behaviour if (mode=="C")
			if (*mode == 'C')
				response ^= key;

			// no record and mode X or anything else returns ""
			continue;
		}

		// extract the field or field 0 means return the whole record
		if (fieldno) {

			// numeric fieldno not zero return field
			// if (fieldno.isnum())

			// throw non-numeric error if fieldno not numeric
			response ^= record.f(fieldno);

			// non-numeric fieldno - cannot call calculate from here
			// return calculate(fieldno,filename,mode);
			continue;
		}

		// fieldno "" returns whole record
		if (!fieldno.len()) {
			response ^= record;
			continue;
		}

		// field no 0 returns key
		response ^= key;
	}
	//response.convert(FM^VM,"^]").outputl("RESPONSE=");
	return response;
}

var  var::numberinwords(in langname_or_locale_id) {

	THISIS("var  var::numberinwords(in number, in langname_or_locale_id)")
	assertNumeric(function_sig);
	langname_or_locale_id.assertString(function_sig);

	var exo_languages =
		"english" _VM
		"arabic" _VM
		"greek" _VM
		"spanish" _VM
		"french";

	var iso_languages =
		"en_GB" _VM
		"ar_AE" _VM
		"el_CY" _VM
		"es_US" _VM
		"fr_FR";

	// Determine locale_id
	//const char* locale_name = "ar_AE.utf8";
	var langn;
	var langcode;
	if (exo_languages.locate(langname_or_locale_id.lcase(),langn))
		langcode = iso_languages(1, langn);
	else if (langname_or_locale_id == "")
		langcode = "en_GB";
	else
		langcode = langname_or_locale_id;

	var locale_id = langcode;
	if (not locale_id.ends(".utf8"))
		locale_id ^= ".utf8";

	// skip locale-gen since
	// 1. It is not necessary since boost::locale:generator seems to manage creation of all languages
	// 2. It is slow and doesnt take effect until a new process is started

	// Verify locale_id exists and create it
	//var locale_id = langcode ^ ".UTF-8";
//	var ok;
//	try {
//		std::locale mylocale(locale_id.toString());
//		ok = true;
//	} catch (std::runtime_error& re) {
//		ok = false;
//		//var(re.what()).errputl();
//	}
//
//	// otherwise try to generate it and try again
//	// otherwise use default probably english
//	if (not ok) {
//		var("number_in_words:get_locale: locale_id " ^ locale_id.quote() ^ " does not exist. Trying to generate it.").errputl();
//		var cmd = "locale-gen " ^ locale_id;
//		cmd.errputl();
//		if (not cmd.osshell())
//			this->lasterror().logputl();
//		try {
//			// Try again after attempted generation of locale
//			std::locale mylocale(locale_id.toString());
//		} catch (std::runtime_error& re) {
//			// Fall back to default locale
//			//std::locale mylocale("");
//			throw VarError("number_in_words:get_locale: " ^ var(re.what()) ^ ". get_locale cannot create locale for " ^ locale_id);
//		}
//	}

	// Create the right language locale
	//TRACE(locale_id);
	boost::locale::generator locale_generator1;
	var result;
	try {
		//TRACE(locale_id)
		std::locale boost_generated_locale1=locale_generator1(locale_id.toString());
		// create a locale imbued stringstream
		std::ostringstream ss;
		ss.imbue(boost_generated_locale1);

	    ss << boost::locale::as::spellout << this->toDouble();

		result = ss.str();
	}
	catch (...) {
			throw VarError("number_in_words: boost_locale_generator failed for " ^ locale_id);
	}

	return result;

}

// clang-format off

// Many of the non-mutating functions are forwarded with a clone to the mutating function

//var  var::ucase()                               const& {return this->clone().ucaser();}
var  var::lcase()                               const& {return this->clone().lcaser();}
var  var::tcase()                               const& {return this->clone().tcaser();}
var  var::fcase()                               const& {return this->clone().fcaser();}
var  var::normalize()                           const& {return this->clone().normalizer();}
var  var::invert()                              const& {return this->clone().inverter();}

var  var::lower()                               const& {return this->clone().lowerer();}
var  var::raise()                               const& {return this->clone().raiser();}
var  var::crop()                                const& {return this->clone().cropper();}

//var  var::quote()                               const& {return this->clone().quoter();}
//var  var::squote()                              const& {return this->clone().squoter();}
//var  var::unquote()                             const& {return this->clone().unquoter();}

var  var::convert(SV fromchars, SV tochars)     const& {return this->clone().converter(fromchars,tochars);}
var  var::textconvert(SV fromchars, SV tochars) const& {return this->clone().textconverter(fromchars,tochars);}
var  var::parse(char sepchar)                   const& {return this->clone().parser(sepchar);}

var  var::pop()                                 const& {return this->clone().popper();}

var  var::paste(const int pos1, const int length, SV insertstr)
                                               const& {return this->clone().paster(pos1, length, insertstr);}
var  var::paste(const int pos1, SV insertstr)   const& {return this->clone().paster(pos1, insertstr);}

// on temporaries the mutator function is called to avoid creating a temporary in many cases

ND io   var::ucase()                                   && {return ucaser();}
ND io   var::lcase()                                   && {return lcaser();}
ND io   var::tcase()                                   && {return tcaser();}
ND io   var::fcase()                                   && {return fcaser();}
ND io   var::normalize()                               && {return normalizer();}
ND io   var::invert()                                  && {return inverter();}

ND io   var::lower()                                   && {return lowerer();}
ND io   var::raise()                                   && {return raiser();}
ND io   var::crop()                                    && {return cropper();}

ND io   var::quote()                                   && {return quoter();}
ND io   var::squote()                                  && {return squoter();}
ND io   var::unquote()                                 && {return unquoter();}

ND io   var::trim(     SV trimchars DEFAULT_SPACE)     && {return trimmer(trimchars);}
ND io   var::trimfirst(SV trimchars DEFAULT_SPACE)     && {return trimmerfirst(trimchars);}
ND io   var::trimlast( SV trimchars DEFAULT_SPACE)     && {return trimmerlast(trimchars);}
ND io   var::trimboth( SV trimchars DEFAULT_SPACE)     && {return trimmerboth(trimchars);}

ND io   var::first()                                   && {return firster();}
ND io   var::last()                                    && {return laster();}
ND io   var::first(const std::size_t length)           && {return firster(length);}
ND io   var::last( const std::size_t length)           && {return laster(length);}
ND io   var::cut(  const int    length)                && {return cutter(length);}
ND io   var::paste(const int    pos1, const int length,
                                        SV insertstr)    && {return paster(pos1, length, insertstr);}
ND io   var::paste(const int    pos1, SV insertstr)    && {return paster(pos1, insertstr);}
ND io   var::prefix(                  SV insertstr)    && {return prefixer(insertstr);}
//ND io append(SV appendstr)                         && {return appender(appendstr);}
ND io   var::pop()                                     && {return popper();}

ND io   var::fieldstore(SV sepchar, const int fieldno, const int nfields, in replacement)
                                                         && {return fieldstorer(sepchar, fieldno, nfields, replacement);}

ND io   var::substr(const int pos1, const int length)  && {return substrer(pos1, length);}
ND io   var::substr(const int pos1)                    && {return substrer(pos1);}

ND io   var::convert(    SV fromchars, SV tochars)     && {return converter(fromchars, tochars);}
ND io   var::textconvert(SV fromchars, SV tochars)     && {return textconverter(fromchars, tochars);}
ND io   var::replace(    SV fromstr,   SV tostr)       && {return replacer(fromstr, tostr);}
ND io   var::replace(const rex& regex, SV replacement) && {return replacer(regex, replacement);}

ND io   var::unique()                                  && {return uniquer();}
ND io   var::sort(   SV sepchar DEFAULT_FM)            && {return sorter(sepchar);}
ND io   var::reverse(SV sepchar DEFAULT_FM)            && {return reverser(sepchar);}
ND io   var::shuffle(SV sepchar DEFAULT_FM)            && {return shuffler(sepchar);}
ND io   var::parse(char sepchar DEFAULT_FM)            && {return parser(sepchar);}

// clang-format on

}  // namespace exo
