#ifndef EXODUS_LIBEXODUS_EXODUS_VAR_H_
#define EXODUS_LIBEXODUS_EXODUS_VAR_H_
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

// clang-format off

// Purely #define so can done before any declarations
// Always need to be run because modules dont export #define
#include <exodus/vardefs.h>

#if EXO_MODULE
	import std;
#else
#	include <iostream>
#	include <string>
#	include <string_view>
#	include <array> // used in operator<<
#	include <vector>
#endif

#include <exodus/format.h>

// var_base provides the basic var-like functionality for var
#include <exodus/vartyp.h>
#include <exodus/varb.h>

namespace exo {

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                               VAR_MID
///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename var>
class var_mid : public var_base<var_mid<var>> {

public:

	//friend class var;// Does this achieve anything?

	// Inherit constructors
	using var_base<var_mid<var>>::var_base;

	// var_base::clone and move, like most var_base functions return a var
	// since a var is usable as a var_base but not vice versa.

//	// Implicitly convert var_base to var
//	operator var() &{
//		return this->clone();
//	}
//
//	operator var() && {
//		return this->move();
//	}

}; // class var_mid

// Forward declarations
//
class var;
class rex;
class dim;
class var_iter;
class var_proxy1;
class var_proxy2;
class var_proxy3;

// Note:
// Inside class var_base
// using VAR    =       var_base;
// using VARREF =       var_base&;
// using CVR    = const var_base&;
// using TVR    =       var_base&&;

	using VAR    =       var;
	using VARREF =       var&;
	using CVR    = const var&;
	using TVR    =       var&&;

	using in     = const var&;
	using out    =       var&;
	using io     =       var&;

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                               VAR
///////////////////////////////////////////////////////////////////////////////////////////////////

// class var
// using CRTP to capture a customised base class that knows what a var is
class PUBLIC var : public var_mid<var> {

	// Apparently the "using CVR - const var&;" declared in exo namespace scope
	// isnt isnt exactly the same inside the actual var class definition
	// for some reason. So we will simply redeclare it.
	using CVR    = const var&;

	friend class dim;
	friend class rex;

	friend var  operator""_var(const char* cstr, std::size_t size);

public:

	// Inherit all constructors from var_base
//	using var_base::var_base;
	using var_mid<var>::var_mid;

	////////////////////////////////////////////
	// GENERAL CONSTRUCTOR FROM INITIALIZER LIST
	////////////////////////////////////////////

	// var_base{...}

	// initializer lists can only be of one type int, double, cstr, char etc.
	template <class T>
	CONSTEXPR var(std::initializer_list<T> list)
	// This constructor assumes FM delimiter so keep it out of var_base.
	//Therefore it cannot use var_base initializers
	//:
	//var_typ(VARTYP_STR) {
	//:
	{
		var_typ = VARTYP_STR;

		for (auto item : list) {
			//(*this) ^= item;
			var_str += std::string_view(var_base(item));
			var_str.push_back(FM_);
		}
		if (!var_str.empty())
			var_str.pop_back();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                                         IOSTREAM FRIENDS
/////////////////////////////////////////////////////////////////////////////////////////////////

	// OSTREAM
	//////////

	// WARNING: MUST PASS A COPY of var_base in since it will may be modified before being output
	// TODO Take a reference and make an internal copy if any FM need to be converted
	// Causes ambiguous overload for some unknown reason despite being a hidden friend
	// friend std::ostream& operator<<(std::ostream& ostream1, TVR var1);
	friend std::ostream& operator<<(std::ostream& ostream1, const var&& outvar) {

		outvar.assertString(__PRETTY_FUNCTION__);

		CONSTEXPR std::array VISIBLE_FMS_EXCEPT_ESC {VISIBLE_ST_, TM_, VISIBLE_SM_, VISIBLE_VM_, VISIBLE_FM_, VISIBLE_RM_};

		// Replace various unprintable field marks with unusual ASCII characters
		// Leave ESC as \x1B because it is used to control ANSI terminal control sequences
		// std::string str = "\x1A\x1B\x1C\x1D\x1E\x1F";
		// |\x1B}]^~  or in high to low ~^]}\x1B|	 or in TRACE() ... ~^]}_|

		// Default configuration: Output the incoming var_str if no conversion is required
		const std::string* outstr = &outvar.var_str;

		// Check if any conversion required
		std::string converted_str;
		char charx;
		for (auto citer = outvar.var_str.cbegin(); (charx = *citer); citer++) {

			// Conversion required - Copy, convert and break out
			if (charx <= 0x1F && charx >= 0x1A) {
				UNLIKELY

				// Replicate the output str
				converted_str = std::forward<CVR>(outvar).var_str;

				// Reconfigure output of the copied and converted
				outstr = &converted_str;

				// Convert the first char that needs converting and bump the iterator
				auto iter2 = converted_str.begin() + std::distance(outvar.var_str.cbegin(), citer);
				*iter2 = VISIBLE_FMS_EXCEPT_ESC[std::size_t(charx - 0x1A)];

				// Convert the rest of the chars
				while ((charx = *++iter2)) {
					if (charx <= RM_ && charx >= ST_) {
						UNLIKELY
						*iter2 = VISIBLE_FMS_EXCEPT_ESC[std::size_t(charx - ST_)];
					}
				}
				break;
			}
		}

		ostream1 << *outstr;
		return ostream1;
	}

	/////////////////
	// PARENTHESIS ()
	/////////////////

	// extract using () int int int (alternative to .f() and extract())
	// instead of xyz=abc.extract(1,2,3);
	// sadly there is no way to use pick/mv angle brackets like "abc<1,2,3>"
	// and [] brackets would only allow one dimension eg abc[1] (c++23 allows more than one)

	//SADLY no way to get a different operator() function called when on the left hand side of assign
	//http://codepad.org/MzzhlRkb

	//subscript operators often come in pairs
	//
	// 1. const returning a value
	// 2. non-const returning a reference or a proxy object

	// 1. () on const vars will extract the desired field/value/subvalue as a proper var
	// Note that all  function "in" arguments are const vars
	// so will work perfectly with () extraction
	ND var  operator()(int fieldno, int valueno = 0, int subvalueno = 0) const {return this->f(fieldno, valueno, subvalueno);}
//	ND var  operator()(int fieldno, int valueno = 0, int subvalueno = 0) &&      {return a(fieldno, valueno, subvalueno);}

	// DONT declare this so we force use of the above const version that produces a temporary
	//RETVARREF operator()(int fieldno, int valueno = 0, int subvalueno = 0);

	// 2. () on non-const vars produces a proxy which can be assigned to or converted to a var implicitly
	// sadly the implicit conversion does not allow OO orientated syntax eg xyz(1,2).oconv("D")
	//
	//  var x = y(3);              will compile and work regardless of if y is const or not
	//  var x = oconv(y(3), "D");  will also compile and work perfectly
	//
	//  var x = y(3).oconv("D");"  will not compile UNLESS y is const e.g function arg of type "in"
	//
	// The proxy object created by (1,2,3) on a non-const var does not have the member function "oconv"
	//
	//
	friend class var_proxy1;
	friend class var_proxy2;
	friend class var_proxy3;
	ND var_proxy1 operator()(int fieldno);
	ND var_proxy2 operator()(int fieldno, int valueno);
	ND var_proxy3 operator()(int fieldno, int valueno, int subvalueno);

	// Prefix operators must be replicated in var
	// //////////////////////////////////////////
	//
	// Cannot use the var_base version because it returns a var_base which
	// cannot be used in place of a var and would stop the use of ++xxx and --xxx
	// in places where a var is required. e.g. some function arguments.
	//
	// Will be forwarded to var_base behind the scenes
	//
	// Must have postfix operators if prefix operators are defined

	   var  operator++(int) &;
	   var  operator--(int) &;

	   io   operator++() &;
	   io   operator--() &;

	// Tabular documentation is generated for comments starting /// or more and ending with a colon

	///// MATH/BOOLEAN:
	//////////////////

	// obj is num

    // Absolute value
    // `let v1 = var(-12.34).abs(); // 12.34
    //  // or
    //  let v2 = abs(-12.34);`
    ND var  abs() const;

    // Power
    // `let v1 = var(2).pwr(8); // 256
    //  // or
    //  let v2 = pwr(2, 8);`
    ND var  pwr(in exponent) const;

    // Pseudo random number generator
    // `let v1 = var(100).rnd(); // 0 to 99 pseudo random
    //  // or
    //  let v2 = rnd(100);`
    ND var  rnd()     const;

    // Initialise the seed for rnd()
    // `var(123).initrnd(); // Set seed to 123
    //  // or
    //  initrnd(123);`
           void initrnd() const;

    // Power of e
    // `let v1 = var(1).exp(); // 2.718281828459045
    //  // or
    //  let v2 = exp(1);`
    ND var  exp()     const;

    // Square root
    // `let v1 = var(100).sqrt(); // 10
    //  // or
    //  let v2 = sqrt(100);`
    ND var  sqrt()    const;

    // Sine of degrees
    // `let v1 = var(30).sin(); // 0.5
    //  // or
    //  let v2 = sin(30);`
    ND var  sin()     const;

    // Cosine of degrees
    // `let v1 = var(60).cos(); // 0.5
    //  // or
    //  let v2 = cos(60);`
    ND var  cos()     const;

    // Tangent of degrees
    // `let v1 = var(45).tan(); // 1
    //  // or
    //  let v2 = tan(45);`
    ND var  tan()     const;

    // Arctangent of degrees
    // `let v1 = var(1).atan(); // 45
    //  // or
    //  let v2 = atan(1);`
    ND var  atan()    const;

    // Natural logarithm
    // `let v1 = var(2.718281828459045).loge(); // 1
    //  // or
    //  let v2 = loge(2.718281828459045);`
    ND var  loge()    const;

    //  ND var  int() const;//reserved word

    // Truncate decimal numbers towards zero
    // `let v1 = var(2.9).integer(); // 2
    //  // or
    //  let v2 = integer(2.9);
	//
    //  var v3 = var(-2.9).integer(); // -2
    //  // or
    //  var v4 = integer(-2.9);`
    ND var  integer() const;

    // Truncate decimal numbers towards negative
    // `let v1 = var(2.9).floor(); // 2
    //  // or
    //  let v2 = floor(2.9);
	//
    //  var v3 = var(-2.9).floor(); // -3
    //  // or
    //  var v4 = floor(-2.9);`
    ND var  floor() const;

    // Round decimal numbers to a desired number of decimal places
    // .5 always rounds away from zero.
    // `let v1 = var(0.455).round(2); // 0.46
    //  // or
    //  let v2 = round(1.455, 2);
	//
    //  var v3 = var(-0.455).round(2); // -0.46
    //  // or
    //  var v4 = round(-1.455, 2);`
    ND var  round(const int ndecimals = 0) const;

	/* Actually implemented in var_base but documented here
	// Modulus function
	// Identical to C++ % operator only for positive numbers and modulus
	// Negative denominators are considered as periodic with positiive numbers
	// Result is between [0 , modulus) if modulus is positive
	// Result is between (modulus, 0] if modulus is negative (symmetric)
	// mod(11, 5); // "1"
	// mod(-11, 5); // "4"
	// mod(11, -5); // "-4"
	// mod(-11, -5); // "-1"
	// `let v1 = var(11).mod(5); // 1
	//  // or
	//  let v2 = mod(11, 5);`
	ND var  mod(in modulus) const;

	// Not documenting the overloaded versions
	//ND var  mod(double modulus) const;
	//ND var  mod(const int modulus) const;
    */

	///// LOCALE:
	////////////

	// obj is var

	// Gets the current thread's default locale codepage code
	//
	// `let v1 = var().getxlocale(); // e.g. "en_US.utf8"
	//  // or
	//  let v2 = getxlocale();`
	ND out  getxlocale();

	// Sets the current thread's default locale codepage code
	//  obj is strvar
	//
	// `if (not "de_DE.utf8"_var.setxlocale()) ... // true if successful
	//  // or
	//  if (setxlocale("de_DE.utf8")) ...`
	   bool setxlocale() const;

	///// STRING CREATION:
	/////////////////////

	// obj is var()

	// Create a string of a single char (byte) given an integer 0-255.
	// 0-127 -> ASCII, 128-255 -> invalid UTF-8 so cannot be written to database or used various exodus string operations
	//
	// `let v1 = var().chr(0x61); // "a"
	//  // or
	//  let v2 = chr(0x61);`
	ND var  chr(const int num) const;

	// Create a string of a single unicode code point in utf8 encoding.
	// To get utf codepoints > 2^63 you must provide negative ints
	// Not providing implicit constructor from var to unsigned int due to getting ambigious conversions
	// since int and unsigned int are parallel priority in c++ implicit conversions
	//
	// `let v1 = var().textchr(171416); // "𩶘" or "\xF0A9B698"
	//  // or
	//  let v2 = textchr(171416);`
	ND var  textchr(const int num) const;

	// Create a string by repeating a given character or string
	//
	// `let v1 = "ab"_var.str(3); // "ababab"
	//  // or
	//  let v2 = str("ab", 3);`
	ND var  str(const int num) const;

	// Create string of space characters.
	// obj is num
	//
	// `let v1 = var(3).space(); // "␣␣␣"
	//  // or
	//  let v2 = space(3);`
	ND var  space() const;

	// Create a string describing a given number in words
	// obj is num
	//
	// `let v1 = var(123.45).numberinwords("de_DE"); // "ein­hundert­drei­und­zwanzig Komma vier fünf"`
	ND var  numberinwords(in languagename_or_locale_id = "");

	////////////
	// STRING // All utf8 unless char or byte mentioned
	////////////

	///// STRING SCANNING:
	/////////////////////

	//  obj is strvar

	// <em>Returns:</em> The character number of the first char.
	//
	// `let v1 = "abc"_var.seq(); // 0x61 97
	//  // or
	//  let v2 = seq("abc");`
	ND var  seq() const;

	// <em>Returns:</em> The Unicode character number of the first unicode code point.
	//
	// `let v1 = "Γ"_var.textseq(); // 915 U+0393: Greek Capital Letter Gamma (Unicode Character)
	//  // or
	//  let v2 = textseq("Γ");`
	ND var  textseq() const;

	// <em>Returns:</em> The length of a string in number of chars
	//
	// `let v1 = "abc"_var.len(); // 3
	//  // or
	//  let v2 = len("abc");`
	ND var  len() const;

	// <em>Returns:</em> The number of output columns.
	// Allows multi column unicode and reduces combining characters etc. like e followed by grave accent
	// Possibly does not properly calculate combining sequences of graphemes e.g. face followed by colour
	//
	// `let v1 = "🤡x🤡"_var.textwidth(); // 5
	//  // or
	//  let v2 = textwidth("🤡x🤡");`
	ND var  textwidth() const;

	// <em>Returns:</em> The number of Unicode code points
	//
	// `let v1 = "Γιάννης"_var.textlen(); // 7
	//  // or
	//  let v2 = textlen("Γιάννης");`
	ND var  textlen() const;

	// <em>Returns:</em> The number of fields determined by presence of sepstr.
	// It is the same as var.count(sepstr) + 1 except that fcount returns 0 for an empty string.
	//
	// `let v1 = "aa**cc"_var.fcount("*"); // 3
	//  // or
	//  let v2 = fcount("aa**cc", "*");`
	ND var  fcount(SV sepstr) const;

	// Return the number of sepstr found
	//
	// `let v1 = "aa**cc"_var.count("*"); // 2
	//  // or
	//  let v2 = count("aa**cc", "*");`
	ND var  count(SV sepstr) const;

	// Exodus   Javascript   PHP             Python       Go          Rust          C++
	// starts   startsWith() str_starts_with startswith() HasPrefix() starts_with() starts_with
	// ends     endsWith     str_ends_with   endswith     HasSuffix() ends_with()   ends_with
	// contains includes()   str_contains    contains()   Contains()  contains()    contains

	// <em>Returns:</em> True if starts with prefix
	//
	// `let v1 = "abc"_var.starts("ab"); // true
	//  // or
	//  let v2 = starts("abc", "ab");`
	ND bool starts(SV prefix) const;

	// <em>Returns:</em> True if ends with suffix
	//
	// `let v1 = "abc"_var.ends("bc"); // true
	//  // or
	//  let v2 = ends("abc", "bc");`
	ND bool ends(SV suffix) const;

	// <em>Returns:</em> True if starts, ends or contains substr
	//
	// `let v1 = "abcd"_var.contains("bc"); // true
	//  // or
	//  let v2 = contains("abcd", "bc");`
	ND bool contains(SV substr) const;

	//https://en.wikipedia.org/wiki/Comparison_of_programming_languages_(string_functions)#Find

	// <em>Returns:</em> Char no if found or 0 if not. startchar1 is byte no to start at.
	//
	// `let v1 = "abcd"_var.index("bc"); // 2
	//  // or
	//  let v2 = index("abcd", "bc");`
	ND var  index(SV substr, const int startchar1 = 1) const;

	// ditto. Occurrence 1 = find first occurrence
	//
	// `let v1 = "abcabc"_var.index("bc", 2); // 5
	//  // or
	//  let v2 = index("abcabc", "bc", 2);`
	ND var  indexn(SV substr, const int occurrence) const;

	// ditto. Reverse search.
	// startchar1 defaults to -1 meaning start searching from the last byte (towards the first byte).
	//
	// `let v1 = "abcabc"_var.indexr("bc"); // 5
	//  // or
	//  let v2 = indexr("abcabc", "bc");`
	ND var  indexr(SV substr, const int startchar1 = -1) const;

	// <em>Returns:</em> All results of regex matching
	// Multiple matches are in fields. Groups are in values
	//
	// `let v1 = "abc1abc2"_var.match("BC(\\d)", "i"); // "bc1]1^bc2]2"
	//  // or
	//  let v2 = match("abc1abc2", "BC(\\d)", "i");`
	//
	// regex_options:
	//
    // l - Literal (any regex chars are treated as normal chars)
	//
    // i - Case insensitive
	//
    // p - ECMAScript/Perl (the default)
    // b - Basic POSIX (same as sed)
    // e - Extended POSIX
    // a - awk
    // g - grep
    // eg - egrep or grep -E
	//
    // char ranges like a-z are locale sensitive if ECMAScript
	//
    // m - Multiline. Default in boost (and therefore exodus)
    // s - Single line. Default in std::regex
	//
	// f - First only. Only for replace() (not match() or search())
	//
    // w - Wildcard glob style (e.g. *.cfg) not regex style. Only for match() and search(). Not replace().
	ND var  match(SV regex_str, SV regex_options = "") const;

	// Ditto
	ND var  match(const rex& regex) const;

	// Search for first match of a regular expression starting at startchar1
	// Updates startchar1 ready to search for the next match
	// regex_options as for match()
	//
	// `var startchar1 = 1;
	//  let v1 = "abc1abc2"_var.search("BC(\\d)", startchar1, "i"); // returns "bc1]1" and startchar1 is updated to 5 ready for the next search
	//  // or
	//  startchar1 = 1;
	//  let v2 = search("abc1abc2", "BC(\\d)", startchar1, "i");`
	ND var  search(SV regex_str, io startchar1, SV regex_options = "") const;

	// Ditto starting from first char
	ND var  search(SV regex_str) const {var startchar1 = 1; return this->search(regex_str, startchar1);}

	// Ditto given a rex
	ND var  search(const rex& regex, io startchar1) const;

	// Ditto starting from first char.
	ND var  search(const rex& regex) const {var startchar1 = 1; return this->search(regex, startchar1);}

//	//static member for speed on std strings because of underlying boost implementation
//	static int localeAwareCompare(const std::string& str1, const std::string& str2);
//	//int localeAwareCompare(const std::string& str2) const;

	///// STRING CONVERSION - Chainable. Non-mutating:
	//////////////////////

	//  obj is strvar

	// Upper case
	//
	// `let v1 = "Γιάννης"_var.ucase(); // "ΓΙΆΝΝΗΣ"
	//  // or
	//  let v2 = ucase("Γιάννης");`
	ND var  ucase() const&;

	// Lower case
	//
	// `let v1 = "ΓΙΆΝΝΗΣ"_var.lcase(); // "γιάννης"
	//  // or
	//  let v2 = lcase("ΓΙΆΝΝΗΣ");`
	ND var  lcase() const&;

	// Title case (first letters)
	//
	// `let v1 = "γιάννης"_var.tcase(); // "Γιάννης"
	//  // or
	//  let v2 = tcase("γιάννης");`
	ND var  tcase() const&;

	// Fold case (lower case and remove accents for indexing)
	ND var  fcase() const&;

	// Normalise Unicode to NFC to eliminate different code combinations of the same character
	ND var  normalize() const&;

	// Simple reversible disguising of text
	//
	// `let v1 = "abc"_var.invert(); // "\x{C29EC29DC29C}"
	//  // or
	//  let v2 = invert("abc");`
	ND var  invert() const&;

	// Convert all FM to VM, VM to SM etc.
	//
	// `let v1 = "a1^b2^c3"_var.lower(); // "a1]b2]c3"
	//  // or
	//  let v2 = lower("a1^b2^c3"_var);`
	ND var  lower() const&;

	// Convert all VM to FM, SM to VM etc.
	//
	// `let v1 = "a1]b2]c3"_var.raise(); // "a1^b2^c3"
	//  // or
	//  let v2 = "a1]b2]c3"_var;`
	ND var  raise() const&;

	// Remove any redundant FM, VM etc. characters (Trailing FM; VM before FM etc.)
	//
	// `let v1 = "a1^b2]]^c3^^"_var.crop(); // "a1^b2^c3"
	//  // or
	//  let v2 = crop("a1^b2]]^c3^^"_var);`
	ND var  crop() const&;

	// Wrap in double quotes
	//
	// `let v1 = "abc"_var.quote(); // ""abc""
	//  // or
	//  let v2 = quote("abc");`
	ND var  quote() const&;

	// Wrap in single quotes
	//
	// `let v1 = "abc"_var.squote(); // "'abc'"
	//  // or
	//  let v2 = squote("abc");`
	ND var  squote() const&;

	// Remove one pair of double or single quotes
	//
	// `let v1 = "'abc'"_var.unquote(); // "abc"
	//  // or
	//  let v2 = unquote("'abc'");`
	ND var  unquote() const&;

	// Remove leading, trailing and excessive inner bytes
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trim(); // "a1␣b2␣c3"
	//  // or
	//  let v2 = trim("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trim(SV trimchars = " ") const&;

	// Ditto leading
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimfirst(); // "a1␣␣b2␣c3␣␣"
	//  // or
	//  let v2 = trimfirst("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trimfirst(SV trimchars = " ") const&;

	// Ditto trailing
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimlast(); // "␣␣a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimlast("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trimlast(SV trimchars = " ") const&;

	// Ditto leading, trailing but not inner
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimboth(); // "a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimboth("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trimboth(SV trimchars = " ") const&;

	// Extract first char or "" if empty
	//
	// `let v1 = "abc"_var.first(); // "a"
	//  // or
	//  let v2 = first("abc");`
	ND var  first() const&;

	// Extract last char or "" if empty
	//
	// `let v1 = "abc"_var.last(); // "c"
	//  // or
	//  let v2 = last("abc");`
	ND var  last() const&;

	// Extract up to length leading chars
	//
	// `let v1 = "abc"_var.first(2); // "ab"
	//  // or
	//  let v2 = first("abc", 2);`
	ND var  first(const std::size_t length) const&;

	// Extract up to length trailing chars
	//
	// `let v1 = "abc"_var.last(2); // "bc"
	//  // or
	//  let v2 = last("abc", 2);`
	ND var  last(const std::size_t length) const&;

	// Remove length leading chars
	//
	// `let v1 = "abcd"_var.cut(2); // "cd"
	//  // or
	//  let v2 = cut("abcd", 2);`
	ND var  cut(const int length) const&;

	// Insert text at char position overwriting length chars
	//
	// `let v1 = "abcd"_var.paste(2, 2, "XYZ"); // "aXYZd"
	//  // or
	//  let v2 = paste("abcd", 2, 2, "XYZ");`
	ND var  paste(const int pos1, const int length, SV insertstr) const&;

	// Insert text at char position without overwriting any following characters
	//
	// `let v1 = "abcd"_var.paste(2, "XYZ"); // "aXYbcd"
	//  // or
	//  let v2 = paste("abcd", 2, "XYZ");`
	ND var  paste(const int pos1, SV insertstr) const&;

	// Insert text at the beginning
	//
	// `let v1 = "abc"_var.prefix("XY"); // "XYabc"
	//  // or
	//  let v2 = prefix("abc", "XY");`
	ND var  prefix(SV insertstr) const&;

	// ND var  append(SV appendstr) const&;

	// Remove one trailing char
	//
	// `let v1 = "abc"_var.pop(); // "ab"
	//  // or
	//  let v2 = pop("abc");`
	ND var  pop() const&;

	// fieldstore() replaces n fields of subfield(s) in a string.
	//
	// `let v1 = "aa*bb*cc*dd"_var.fieldstore("*", 2, 3, "X*Y"); // "aa*X*Y*"
	//  // or
	//  let v2 = fieldstore("aa*bb*cc*dd", "*", 2, 3, "X*Y");`
	// If nfields is 0 then insert fields before fieldno
	//
	// `let v1 = "a1*b2*c3*d4"_var.fieldstore("*", 2, 0, "X*Y"); // "a1*X*Y*b2*c3*d4"
	//  // or
	//  let v2 = fieldstore("a1*b2*c3*d4", "*", 2, 0, "X*Y");`
	// If nfields is negative then delete abs(n) fields before inserting.
	//
	// `let v1 = "a1*b2*c3*d4"_var.fieldstore("*", 2, -3, "X*Y"); // "a1*X*Y"
	//  // or
	//  let v2 = fieldstore("a1*b2*c3*d4", "*", 2, -3, "X*Y");`
	ND var  fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;

	// substr version 1. Extract length chars starting at pos1
	//
	// `let v1 = "abcd"_var.substr(2, 2); // "bc"
	//  // or
	//  let v2 = substr("abcd", 2, 2);`
	// If length is negative then work backwards and return chars reversed
	//
	// `let v1 = "abcd"_var.substr(3, -2); // "cb"`
	//  // or
	//  let v2 = substr("abcd", 3, -2);`
	ND var  substr(const int pos1, const int length) const&;

	// substr version 2. Extract all chars from pos1 up to the end
	//
	// `let v1 = "abcd"_var.substr(2); // "bcd"
	//  // or
	//  let v2 = substr("abcd", 2);`
	ND var  substr(const int pos1) const&;

	//[[deprecated("EXODUS: Replace all xxx.b(pos1, len) with xxx.subst(pos1, len)")]]
	// Same as substr version 1.
	ND var  b(const int pos1, const int length) const& {return this->substr(pos1, length);}

	//[[deprecated("EXODUS: Replace all xxx.b(pos1) with xxx.substr(pos1)")]]
	// Same as substr version 2.
	ND var  b(const int pos1) const& {return this->substr(pos1);}

	// Convert chars to other chars one for one or delete where tochars is shorter.
	//
	// `let v1 = "abcde"_var.convert("aZd", "XY"); // "Xbce" (a is replaced and d is removed)
	//  // or
	//  let v2 = convert("abcde", "aZd", "XY");`
	ND var  convert(SV fromchars, SV tochars) const&;

	// Ditto for Unicode code points.
	//
	// `let v1 = "a🤡b😀c🌍d"_var.textconvert("🤡😀", "👋"); // "a👋bc🌍d"
	//  // or
	//  let v2 = textconvert("a🤡b😀c🌍d", "🤡😀", "👋");`
	ND var  textconvert(SV fromchars, SV tochars) const&;

	// Replace all occurrences of a substr with another. Case sensitive
	//
	// `let v1 = "Abc Abc"_var.replace("bc", "X"); // "AX AX"
	//  // or
	//  let v2 = replace("Abc Abc", "bc", "X");`
	ND var  replace(SV fromstr, SV tostr) const&;

	// Replace substring(s) using a regular expression.
	// Use $0, $1, $2 in tostr to refer to groups defined in the regex.
	//
	// `let v1 = "A a B b"_var.replace("[A-Z]"_rex, "'$0'"); // "'A' a 'B' b"
	//  // or
	//  let v2 = replace("A a B b", "[A-Z]"_rex, "'$0'");`
	ND var  replace(const rex& regex, SV tostr) const&;

	// Remove duplicate fields in an FM or VM etc. separated list
	//
	// `let v1 = "a1^b2^a1^c2"_var.unique(); // "a1^b2^c2"
	//  // or
	//  let v2 = unique("a1^b2^a1^c2"_var);`
	ND var  unique() const&;

	// Reorder fields in an FM or VM etc. separated list in ascending order
	// Numerical
	//
	// `let v1 = "20^10^2^1^1.1"_var.sort(); // "1^1.1^2^10^20"
	//  // or
	//  let v2 = sort("20^10^2^1^1.1"_var);`
	// Alphabetical
	//  `let v1 = "b1^a1^c20^c10^c2^c1^b2"_var.sort(); // "a1^b1^b2^c1^c10^c2^c20"
	//  // or
	//  let v2 = sort("b1^a1^c20^c10^c2^c1^b2"_var);`
	ND var  sort(SV sepchar = _FM) const&;

	// Reorder fields in an FM or VM etc. separated list in descending order
	//
	// `let v1 = "20^10^2^1^1.1"_var.reverse(); // "1.1^1^2^10^20"
	//  // or
	//  let v2 = reverse("20^10^2^1^1.1"_var);`
	ND var  reverse(SV sepchar = _FM) const&;

	// Randomise the order of fields in an FM, VM separated list
	//
	// `let v1 = "20^10^2^1^1.1"_var.shuffle(); // "2^1^20^1.1^10" (random order depending on initrand())`
	//  // or
	//  let v2 = shuffle("20^10^2^1^1.1"_var);`
	ND var  shuffle(SV sepchar = _FM) const&;

	// Replace separator characters with FM char except inside double or single quotes ignoring escaped quotes &bsol;" &bsol;'
	//
	// `let v1 = "abc,\"def,\"123\" fgh\",12.34"_var.parse(','); // "abc^"def,"123" fgh"^12.34"
	//  // or
	//  let v2 = parse("abc,\"def,\"123\" fgh\",12.34", ',');`
	ND var  parse(char sepchar = ' ') const&;

	// SAME ON TEMPORARIES - CALL MUTATORS FOR SPEED (not documenting since programmer interface is the same)
	/////////////////////////////////////////

	// utf8/byte as for accessors

	ND io   ucase() &&;
	ND io   lcase() &&;
	ND io   tcase() &&;
	ND io   fcase() &&;
	ND io   normalize() &&;
	ND io   invert() &&;

	ND io   lower() &&;
	ND io   raise() &&;
	ND io   crop() &&;

	ND io   quote() &&;
	ND io   squote() &&;
	ND io   unquote() &&;

	ND io   trim(SV trimchars = " ") &&;
	ND io   trimfirst(SV trimchars = " ") &&;
	ND io   trimlast(SV trimchars = " ") &&;
	ND io   trimboth(SV trimchars = " ") &&;

	ND io   first() &&;
	ND io   last() &&;
	ND io   first(const std::size_t length) &&;
	ND io   last(const std::size_t length) &&;
	ND io   cut(const int length) &&;
	ND io   paste(const int pos1, const int length, SV replacestr) &&;
	ND io   paste(const int pos1, SV insertstr) &&;
	ND io   prefix(SV insertstr) &&;
//	ND io   append(SV appendstr) &&;
	ND io   pop() &&;

	ND io   fieldstore(SV sepchar, const int fieldno, const int nfields, in replacement) &&;
	ND io   substr(const int pos1, const int length) &&;
	ND io   substr(const int pos1) &&;

	ND io   convert(SV fromchars, SV tochars) &&;
	ND io   textconvert(SV fromchars, SV tochars) &&;
	ND io   replace(const rex& regex, SV tostr) &&;
	ND io   replace(SV fromstr, SV tostr) &&;
//	ND io   regex_replace(SV regex_str, SV replacement, SV regex_options = "") &&;

	ND io   unique() &&;
	ND io   sort(SV sepchar = _FM) &&;
	ND io   reverse(SV sepchar = _FM) &&;
	ND io   shuffle(SV sepchar = _FM) &&;
	ND io   parse(char sepchar = ' ') &&;

	///// STRING MUTATOR standalone commands. All similar to Non-Mutators:
	/////////////////////////////////////////////////////////////////////

	//  obj is strvar

	// Upper case
	// All following string mutators follow the same pattern. See the non-mutating functions for details.
	//
	// `var v1 = "abc";
	// v1.ucaser(); // "ABC"
	// // or
	// ucaser(v1);`
	   IO   ucaser() REF ;

	   IO   lcaser() REF ;
	   IO   tcaser() REF ;
	   IO   fcaser() REF ;
	   IO   normalizer() REF ;
	   IO   inverter() REF ;

	   IO   quoter() REF ;
	   IO   squoter() REF ;
	   IO   unquoter() REF ;

	   IO   lowerer() REF ;
	   IO   raiser() REF ;
	   IO   cropper() REF ;

	   IO   trimmer(SV trimchars = " ") REF ;
	   IO   trimmerfirst(SV trimchars = " ") REF ;
	   IO   trimmerlast(SV trimchars = " ") REF ;
	   IO   trimmerboth(SV trimchars = " ") REF ;

	   IO   firster() REF ;
	   IO   laster() REF ;
	   IO   firster(const std::size_t length) REF ;
	   IO   laster(const std::size_t length) REF ;
	   IO   cutter(const int length) REF ;
	   IO   paster(const int pos1, const int length, SV insertstr) REF ;
	   IO   paster(const int pos1, SV insertstr) REF ;
	   IO   prefixer(SV insertstr) REF ;
//	   IO   appender(SV appendstr) REF ;

	   IO   popper() REF ;

	// TODO look at using erase to speed up

	   IO   fieldstorer(SV sepchar, const int fieldno, const int nfields, in replacement) REF ;
	   IO   substrer(const int pos1, const int length) REF ;
	   IO   substrer(const int pos1) REF {return this->substrer(pos1, this->len());}
	   IO   converter(SV fromchars, SV tochars) REF;
	   IO   textconverter(SV fromchars, SV tochars) REF;
	   IO   replacer(const rex& regex, SV tostr) REF;
	   IO   replacer(SV fromstr, SV tostr) REF;
//	   IO   regex_replacer(SV regex, SV replacement, SV regex_options = "") REF ;

	   IO   uniquer() REF ;
	   IO   sorter(SV sepchar = _FM) REF ;
	   IO   reverser(SV sepchar = _FM) REF ;
	   IO   shuffler(SV sepchar = _FM) REF ;
	   IO   parser(char sepchar DEFAULT_CSPACE) REF ;

	///// OTHER STRING ACCESS:
	/////////////////////////

	//  obj is strvar

	// Hash by default returns a 64 bit signed integer as a var.
	// If a modulus is provided then the result is limited to [0, modulus)
	// MurmurHash3 is used.
	//
	// `let v1 = "abc"_var.hash(); // 6715211243465481821
	//  // or
	//  let v2 = hash("abc");`
	ND var  hash(const std::uint64_t modulus = 0) const;

	// Split a FM etc. separated string into a dim array.
	//
	// `dim d1 = "a^b^c"_var.split(); //a dimensioned array with three elements (vars)
	//  // or
	//  dim d1 = split("a^b^c"_var);`
	ND dim  split(SV sepchar = _FM) const;

	// v3 - returns bytes from some byte number upto the first of a given list of bytes
	// this is something like std::string::find_first_of but doesnt return the delimiter found

	// substr version 3.
	// Extract a substr starting from pos1 up to any one of some given delimiter chars
	// Also returns in pos2 the pos of the following delimiter or one past the end of the string if not found.
	// Add 1 to pos2 start the next search if continuing.
	//
	// `var pos1a = 4, pos2a; let v1 = "aa,bb,cc"_var.substr(pos1a, ",", pos2a); // "bb" and pos2 -> 6
	//  // or
	//  var pos1b = 4, pos2b; let v2 = substr("aa,bb,cc", pos1b, ",", pos2b);`
	   var  substr(const int pos1, SV delimiterchars, io pos2) const;

	// Alias of substr version 3.
	   var  b(const int pos1, SV delimiterchars, io pos2) const {return this->substr(pos1, delimiterchars, pos2);}

	// v4 - like v3. was named "remove" in pick. notably used in nlist to print parallel columns
	// of mixed combinations of multivalues/subvalues and text marks
	// correctly lined up mv to mv, sv to sv, tm to tm even when particular columns were missing
	// some vm/sm/tm
	// it is like substr(pos1,delimiterbytes,pos2) except that the delimiter bytes are
	// hard coded as the usual RM/FM/VM/SM/TM/ST
	// except that it updates the offset to point one after found delimiter byte and
	// returns the delimiter no (1-6)
	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// offset to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(io pos1, io
	// delimiterno) const;

	// substr version 4.
	// <em>Returns:</em> A substr from a given pos1  up to the next RM/FM/VM/SM/TM/STM delimiter char. Also returns the next index/offset and the delimiter no. found 1-6 or 0 if not found.
	//
	// `var pos1a = 4, delim1;
	//  let v1 = "aa^bb^cc"_var.substr2(pos1a, delim1); // "bb", pos1a -> 7, delim -> 2 (FM)
	//  // or
	//  var pos1b = 4, delim2;
	//  let v2 = substr2("aa^bb^cc"_var, pos1b, delim2);`
	   var  substr2(io pos1, out delimiterno) const;

	// Alias of substr version 4
	   var  b2(io pos1, out delimiterno) const {return this->substr2(pos1, delimiterno);}

	// Extract one or more consecutive fields given a delimiter char or substr.
	//
	// `let v1 = "aa*bb*cc"_var.field("*", 2); // "bb"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", 2);`
	ND var  field(SV strx, const int fieldnx = 1, const int nfieldsx = 1) const;

	// field2 is a version that treats fieldn -1 as the last field, -2 the penultimate field etc. -
	// TODO Should probably make field() do this (since -1 is basically an erroneous call) and remove field2
	// Same as var.field() but negative fieldnos work backwards from the last field.
	//
	// `let v1 = "aa*bb*cc"_var.field2("*", -1); // "cc"
	//  // or
	//  let v2 = field2("aa*bb*cc", "*", -1);`
	ND var  field2(SV separator, const int fieldno, const int nfields = 1) const
	{
		if (fieldno >= 0) LIKELY
			return field(separator, fieldno, nfields);
		return field(separator, this->count(separator) + 1 + fieldno + 1, nfields);
	}

	///// I/O CONVERSION:
	////////////////////

	// obj is var

	// Converts internal data to output external display format according to a given conversion code or pattern
	// If the internal data is invalid and cannot be converted then most conversions return the ORIGINAL data unconverted
	// Throws a runtime error VarNotImplemented if convstr is invalid
	// See [[#ICONV/OCONV PATTERNS]]
	//
	// `let v1 = var(30123).oconv("D/E"); // "21/06/2050"
	//  // or
	//  let v2 = oconv(30123, "D/E");`
	ND var  oconv(const char* convstr) const;

	// Converts external data to internal format according to a given conversion code or pattern
	// If the external data is invalid and cannot be converted then most conversions return the EMPTY STRING ""
	// Throws a runtime error VarNotImplemented if convstr is invalid
	// See [[#ICONV/OCONV PATTERNS]]
	//
	// `let v1 = "21 JUN 2050"_var.iconv("D/E"); // 30123
	//  // or
	//  let v2 = iconv("21 JUN 2050", "D/E");`
	ND var  iconv(const char* convstr) const;

#ifdef EXO_FORMAT

//template<class... Args>
//	ND CONSTEXPR
//	   var  format(EXO_FORMAT_STRING_TYPE1&& fmt_str, Args&&... args) const {
//
////error: call to consteval function 'fmt::basic_format_string<char, exo::var &>
////::basic_format_string<fmt::basic_format_string<char, const exo::var &>, 0>' is not a constant expression
//// 2033 |                         return fmt::format(fmt_str, *this, args... );
////      |                                            ^
//
//#if __clang_major__ == 0
//#if __cpp_if_consteval >= 202106L
//		if consteval {
//#else
//		if (std::is_constant_evaluated()) {
//#endif
//// OK in 2404 g++ but not OK in 2404 clang
//// clang on 22.04 cannot accept compile time format string even if a cstr
/////root/exodus/test/src/test_format.cpp:14:18: error: call to consteval function 'fmt::basic_format_string<char, exo::var>::basic_format_string<ch
////ar[7], 0>' is not a constant expression
////        assert(x.format("{:.2f}").outputl() == "12.35");
//
//			THISIS("var  format(SV fmt_str, Args&&... args) const")
//			return fmt::format(std::forward<EXO_FORMAT_STRING_TYPE1>(fmt_str), *this, std::forward<Args>(args)... );
//		} else
//#endif
//
//		{
////#pragma message "EXO_FORMAT_STRING_TYPE1 == " DUMPDEFINE0(EXO_FORMAT_STRING_TYPE1)
////			THISIS("var  format(" DUMPDEFINE0(EXO_FORMAT_STRING_TYPE1) "&& fmt_str, Args&&... args) const")
//			THISIS("var  format(SV fmt_str, Args&&... args) const")
//			assertString(function_sig);
//			return fmt::vformat(fmt_str, fmt::make_format_args(*this, std::forward<Args>(args)...) );
//		}
//	}
//
//	template<class... Args>
////	ND var  format(in fmt_str, Args&&... args) const {
//	ND var  vformat(SV fmt_str, Args&&... args) const {
//		THISIS("var  vformat(SV fmt_str, Args&&... args) const")
//		assertString(function_sig);
//		return fmt::vformat(fmt_str, fmt::make_format_args(*this, args...) );
//	}

	template<class... Args>
	// Classic format function in printf style
	// vars can be formatted either with C++ format codes e.g. {:_>8.2f}
	// or with exodus oconv codes e.g. {::MD20P|R(_)#8} as in the below example.
	//
	// `let v1 = var(12.345).format("'{:_>8.2f}'"); // "'___12.35'"
	//  let v2 = var(12.345).format("'{::MD20P|R(_)#8}'");
	//  // or
	//  var v3 = format("'{:_>8.2f}'", var(12.345)); // "'___12.35'"
	//  var v4 = format("'{::MD20P|R(_)#8}'", var(12.345));`
	ND var  format(in fmt_str, Args&&... args) const
	{
		THISIS("var  var::format(SV fmt_str, Args&&... args) const")
		assertString(function_sig);
		// *this becomes the first format argument and any additional arguments become additionl format arguments
		return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(*this, args...) );
	}

#endif //EXO_FORMAT

	//  obj is strvar

	// Converts from codepage encoded text to UTF-8 encoded text
	// e.g. Codepage "CP1124" (Ukrainian).
	// Use Linux command "iconv -l" for complete list of code pages and encodings.
	//
	// `let v1 = "\xa4"_var.from_codepage("CP1124"); // "Є"
	//  // or
	//  let v2 = from_codepage("\xa4", "CP1124");
	//  // U+0404 Cyrillic Capital Letter Ukrainian Ie Unicode Character`
	ND var  from_codepage(const char* codepage) const;

	// Converts to codepage encoded text from UTF-8 encoded text
	//
	// `let v1 = "Є"_var.to_codepage("CP1124").oconv("HEX"); // "A4"
	//  // or
	//  let v2 = to_codepage("Є", "CP1124").oconv("HEX");`
	ND var  to_codepage(const char* codepage) const;

	///// BASIC DYNAMIC ARRAY FUNCTIONS:
	///////////////////////////////////

	//  obj is strvar

	// EXTRACT() AND F()

	// Pick OS
	//   xxx=yyy<10>;
	//   xxx=extract(yyy,10);
	// becomes c++
	//   xxx=yyy.f(10);

	// f() is a highly abbreviated alias for the PICK OS field/value/subvalue extract() function.
	// "f()" can be thought of as "field" although the function can extract values and subvalues as well.
	// The convenient PICK OS angle bracket syntax for field extraction (e.g. xxx<20>) is not available in C++.
	// The abbreviated exodus field extraction function (e.g. xxx.f(20)) is provided instead since field access is extremely heavily used in source code.
	//
	// `let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
	//  let v2 = v1.f(2, 2); // "f2v2"`
	ND var  f(const int fieldno, const int valueno = 0, const int subvalueno = 0)            const;

	// Extract a specific field, value or subvalue from a dynamic array.
	//
	// `let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
	//  let v2 = v1.extract(2, 2); // "f2v2"
	//  //
	//  // The alias "f" is normally used instead for brevity
	//  var v3 = v1.f(2, 2);`
	ND var  extract(const int fieldno, const int valueno = 0, const int subvalueno = 0)      const {return this->f(fieldno, valueno, subvalueno);}

	// PICKREPLACE

	// This function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better. It was called replace() in Pick Basic but we are now using "replace()" to
	// change substrings using regex (similar to the old Pick Basic replace function) its mutator function
	// is .r()y
	// Replaces a specific subvalue in a dynamic array. Normally one uses the r() function to replace in place.

	// Same as var.r() function but returns a new string instead of updating a variable in place.<br>Rarely used.
	ND var  pickreplace(const int fieldno, const int valueno, const int subvalueno, in replacement) const&;

	// Ditto for a specific multivalue
	ND var  pickreplace(const int fieldno, const int valueno, in replacement)                       const&;

	// Ditto for a specific field
	ND var  pickreplace(const int fieldno, in replacement)                                          const&;

	// INSERT

	// Same as var.inserter() function but returns a new string instead of updating a variable in place.
	ND var  insert(const int fieldno, const int valueno, const int subvalueno, in insertion) const&;

	// Ditto for a specific multivalue
	ND var  insert(const int fieldno, const int valueno, in insertion)                       const&;

	// Ditto for a specific field
	ND var  insert(const int fieldno, in insertion)                                          const&;

	// REMOVE

	// Same as var.remover() function but returns a new string instead of updating a variable in place.
	// "remove" was called "delete" in Pick OS.
	ND var  remove(const int fieldno, const int valueno = 0, const int subvalueno = 0)       const&;

	// SAME AS ABOVE ON TEMPORARIES TO USE MUTATING (not documented because used difference in implementation is irrelevant to exodus users)
	///////////////////////////////////////////////

	ND io   pickreplace(const int fieldno, const int valueno, const int subvalueno, in replacement) && {this->r(fieldno, valueno, subvalueno, replacement); return *this;}
	ND io   pickreplace(const int fieldno, const int valueno, in replacement)                       && {this->r(fieldno, valueno, 0, replacement); return *this;}
	ND io   pickreplace(const int fieldno, in replacement)                                          && {this->r(fieldno, 0, 0, replacement); return *this;}

	ND io   insert(const int fieldno, const int valueno, const int subvalueno, in insertion)        && {this->inserter(fieldno, valueno, subvalueno, insertion); return *this;}
	ND io   insert(const int fieldno, const int valueno, in insertion)                              && {this->inserter(fieldno, valueno, 0, insertion); return *this;}
	ND io   insert(const int fieldno, in insertion)                                                 && {this->inserter(fieldno, 0, 0, insertion); return *this;}

	ND io   remove(const int fieldno, const int valueno = 0, const int subvalueno = 0)              && {this->remover(fieldno, valueno, subvalueno); return *this;}

	///// DYNAMIC ARRAY FILTERS:
	///////////////////////////

	//  obj is strvar

	// Sum up multiple values into one higher level
	//
	// `let v1 = "1]2]3^4]5]6"_var.sum(); // "6^15"
	//  // or
	//  let v2 = sum("1]2]3^4]5]6"_var);`
	ND var  sum() const;

	// Sum up all levels into a single figure
	//
	// `let v1 = "1]2]3^4]5]6"_var.sumall(); // "21"
	//  // or
	//  let v2 = sumall("1]2]3^4]5]6"_var);`
	ND var  sumall() const;

	// Ditto allowing commas etc.
	//
	// `let v1 = "10,20,33"_var.sum(","); // "60"
	//  // or
	//  let v2 = sum("10,20,33");`
	ND var  sum(SV sepchar) const;

	// Binary ops (+, -, *, /) in parallel on multiple values
	//
	// `let v1 = "10]20]30"_var.mv("+","2]3]4"); // "12]23]34"`
	ND var  mv(const char* opcode, in var2) const;

	///// DYNAMIC ARRAY MUTATORS Standalone commands:
	////////////////////////////

	//  obj is strvar

	// Mutable versions update lvalue vars and dont return anything so that they cannot be chained. This is to prevent accidental misuse and bugs.
	//
	// r() stands for "replacer" abbreviated due to high incidience in code.
	// However the actual exodus replacer function has a different function replacing substrings, not fields, values and subvalues.
	//
	// Pick Basic
	//   xyz<10> = "abc";
	// becomes c++
	//   xyz.r(10, "abc");

	// Replaces a specific field in a dynamic array
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.r(2, 2, "X"); // v1 -> "f1^X^f3"`
	   IO   r(const int fieldno, in replacement) REF {this->r(fieldno, 0, 0, replacement); return THIS;}

	// Ditto for specific value in a specific field.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.r(2, 2, "X"); // v1 -> "f1^v1]X^f3"`
	   IO   r(const int fieldno, const int valueno, in replacement) REF {this->r(fieldno, valueno, 0, replacement); return THIS;}

	// Ditto for a specific subvalue in a specific value of a specific field
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.r(2, 2, 2, "X"); // v1 -> "f1^v1]v2}X}s3^f3"`
	   IO   r(const int fieldno, const int valueno, const int subvalueno, in replacement) REF;

	// Insert a specific field in a dynamic array, moving all other fields up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, "X"); // v1 -> "f1^X^v1]v2}s2}s3^f3"
	//  // or
	//  inserter(v1, 2, "X");`
	   IO   inserter(const int fieldno, in insertion) REF {this->inserter(fieldno, 0, 0, insertion); return THIS;}

	// Ditto for a specific value in a specific field, moving all other values up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, "X"); // v1 -> "f1^v1]X]v2}s2}s3^f3"
	//  // or
	//  inserter(v1, 2, 2, "X");`
	   IO   inserter(const int fieldno, const int valueno, in insertion) REF {this->inserter(fieldno, valueno, 0, insertion); return THIS;}

	// Ditto for a specific subvalue in a dynamic array, moving all other subvalues up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, 2, "X"); // v1 -> "f1^v1]v2}X}s2}s3^f3"
	//  // or
	//  v1.inserter(2, 2, 2, "X");`
	   IO   inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF;

	// Remove a specific field (or value, or subvalue) from a dynamic array, moving all other fields (or values, or subvalues)  down.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.remover(2, 2); // v1 -> "f1^v1^f3"
	//  // or
	//  remover(v1, 2, 2);`
	   IO   remover(const int fieldno, const int valueno = 0, const int subvalueno = 0) REF;

	//-er version could be extract and erase in one go
	// IO   extracter(int fieldno, int valueno=0, int subvalueno=0) const;

	///// DYNAMIC ARRAY SEARCH:
	//////////////////////////

	//  obj is strvar

	// LOCATE

	// locate() with only the target substr argument provided searches unordered values separated by VM chars.
	// <em>Returns:</em> True if found and false if not.
	// `if ("UK]US]UA"_var.locate("US")) ... // true
	//  // or
	//  if (locate("US", "UK]US]UA"_var)) ...`
	ND bool locate(in target) const;

	// locate() with only the target substr and valueno arguments provided searches unordered values separated by VM chars.
	// <em>Returns:</em> True if found and with the value number in valueno.
	// <em>Returns:</em> False if not found and with the max value number + 1 in setting. Suitable for additiom of new values
	//
	// `var valueno;
	//  if ("UK]US]UA"_var.locate("US", valueno)) ... // returns true and valueno = 2`
	ND bool locate(in target, out valueno) const;

	// locate() the target in unordered fields if fieldno is 0, or values if a fieldno is specified, or subvalues if the valueno argument is provided.
	// <em>Returns:</em> True if found and with the field, value or subvalue number in setting.
	// <em>Returns:</em> False if not found and with the max field, value or subvalue number found + 1 in setting. Suitable for replacement of new fields, values or subvalues.
	//
	// `var setting;
	//  if ("f1^f2v1]f2v2]s1}s2}s3}s4^f3^f4"_var.locate("s4", setting, 2, 3)) ... // returns true and setting = 4`
	ND bool locate(in target, out setting, const int fieldno, const int valueno = 0) const;

	// LOCATE BY

	// locateby() without fieldno or valueno arguments searches ordered values separated by VM chars.
	// The order code can be AL, DL, AR, DR meaning Ascending Left, Descending Right, Ascending Right, Ascending Left.
	// Left is used to indicate alphabetic order where 10 < 2.
	// Right is used to indicate numeric order where 10 > 2.
	// Data must be in the correct order for searching to work properly.
	// <em>Returns:</em> True if found.
	// In case the target is not exactly found then the correct value no for inserting the target is returned in setting.
	//
	// `var valueno; if ("aaa]bbb]ccc"_var.locateby("AL", "bb", valueno)) ... // returns false and valueno = 2 where it could be correctly inserted.`
	ND bool locateby(const char* ordercode, in target, out valueno) const;

	// locateby() ordered as above but in fields if fieldno is 0, or values in a specific fieldno, or subvalues in a specific valueno.
	//
	// `var setting;
	//  if ("f1^f2^aaa]bbb]ccc^f4"_var.locateby("AL", "bb", setting, 3)) ... // returns false and setting = 2 where it could be correctly inserted.`
	ND bool locateby(const char* ordercode, in target, out setting, const int fieldno, const int valueno = 0) const;

	// LOCATE USING

	// locate() a target substr in the whole unordered string using a given delimiter char returning true if found.
	// `if ("AB,EF,CD"_var.locateusing(",", "EF")) ... // true`
	ND bool locateusing(const char* usingchar, in target) const;

	// locate() the target in a specific field, value or subvalue using a specified delimiter and unordered data
	// <em>Returns:</em> True If found and returns in setting the number of the delimited field found.
	// <em>Returns:</em> False if not found and returns in setting the maximum number of delimited fields + 1 if not found.
	// This is similar to the main locate command but the delimiter char can be specified e.g. a comma or TM etc.
	//
	// `var setting;
	//  if ("f1^f2^f3c1,f3c2,f3c3^f4"_var.locateusing(",", "f3c2", setting, 3)) ... // returns true and setting = 2`
	ND bool locateusing(const char* usingchar, in target, out setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// LOCATE BY, USING

	// locatebyusing() supports all the above features in a single function.
	// <em>Returns:</em> True if found.
	ND bool locatebyusing(const char* ordercode, const char* usingchar, in target, out setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	///// DATABASE ACCESS:
	/////////////////////

	// obj is conn

	// For all db operations, the operative var can either be a db connection created with dbconnect() or be any var and a default connection will be established on the fly.
	// The db connection string (conninfo) parameters are merged from the following places in descending priority.
	// 1. Provided in connect()'s conninfo argument. See 4. for the complete list of parameters.
	// 2. Any environment variables EXO_HOST EXO_PORT EXO_USER EXO_DATA EXO_PASS EXO_TIME
	// 3. Any parameters found in a configuration file at ~/.config/exodus/exodus.cfg
	// 4. The default conninfo is "host=127.0.0.1 port=5432 dbname=exodus user=exodus password=somesillysecret connect_timeout=10"
	// Setting environment variable EXO_DBTRACE=1 will cause tracing of db interface including SQL commands.
	//
	// `let conninfo = "dbname=exodus user=exodus password=somesillysecret";
	//  var db;
	//  if (not db.connect(conninfo)) abort(lasterror());
	//  db.disconnect();
	//  // or
	//  if (not connect("exodus")) ...
	//  // or
	//  if (not connect()) ...
	//  disconnect();
	ND bool connect(in conninfo = "");

	// Closes db connection and frees process resources both locally and in the database server.
	//
	// `conn.disconnect();
	//  // or
	//  disconnect();
	   void disconnect();

	// Closes all connections and frees process resources both locally and in the database server(s).
	// All connections are closed automatically when a process terminates.
	//
	// `conn.disconnectall();
	//  // or
	//  disconnectall();
	   void disconnectall();

	// Attach (connect) specific files by name to specific connections.
	// It is not necessary to attach files before opening them. Attach is meant to control the defaults.
	// For the remainder of the session, opening the file by name without specifying a connection will automatically use the specified connection applies during the attach command.
	// If conn is not specified then filename will be attached to the default connection.
	// Multiple file names must be separated by FM
	//
	// `let filenames = "definitions^dict.definitions"_var;
	//  if (not conn.attach(filenames)) ...
	//  // or
	//  if (not attach(filenames)) ...`
	ND bool attach(in filenames);

	// Detach (disconnect) files that have been attached using attach().
	   void detach(in filenames);

	// Begin a db transaction.
	//
	// `if (not conn.begintrans()) ...
	//  // or
	//  if (not begintrans()) ...`
	ND bool begintrans() const;

	// Rollback a db transaction.
	//
	// `if (not conn.rollbacktrans()) ...
	//  // or
	//  if (not rollbacktrans()) ...`
	ND bool rollbacktrans() const;

	// Commit a db transaction.
	//
	// `if (not conn.committrans()) ...
	//  // or
	//  if (not committrans()) ...`
	ND bool committrans() const;

	// Check if a db transaction is in progress.
	//
	// `if (not conn.statustrans()) ...
	//  // or
	//  if (not statustrans()) ...`
	ND bool statustrans() const;

	// Execute an sql command.
	//
	// `var sqlcmd = "select 2 + 2;", response;
	//  if (not conn.sqlexec("select 2 + 2;")) ... // True
	//  // or
	//  if (not sqlexec(sqlcmd)) ...`
	ND bool sqlexec(in sqlcmd) const;

	// Execute an SQL command and capture the response.
	//
	// `var sqlcmd = "select 2 + 2;", response;
	//  if (not conn.sqlexec(sqlcmd, response)) ... // True and response = 4
	//  // or
	//  if (not sqlexec(sqlcmd, response)) ...`
	ND bool sqlexec(in sqlcmd, io response) const;

	// Get the last os or db error message.
	ND var  lasterror() const;

	// Log the last os or db error message.
	   var  loglasterror(in source = "") const;

	///// DATABASE MANAGEMENT:
	/////////////////////////

	// obj is conn

	// Create a named database on a particular connection.
	//
	// `if (not conn.dbcreate("mydb")) ...
	//  // or
	//  if (not dbcreate("mydb")) ...`
	ND bool dbcreate(in dbname) const;

	// Return a list of available databases on a particular connection.
	//
	// `let v1 = conn.dblist();
	//  // or
	//  let v2 = dblist();`
	ND var  dblist() const;

	// Create a named database from an existing database.
	// The source database cannot have any current connections.
	//
	// `if (not conn.dbcopy("mydb", "mydb2")) ...
	//  // or
	//  if (not dbcopy("mydb", "mydb2")) ...`
	ND bool dbcopy(in from_dbname, in to_dbname) const;

	// Delete (drop) a named database.
	// The target database cannot have any current connections.
	//
	// `if (not conn.dbdelete("mydb")) ...
	//  // or
	//  if (not dbdelete("mydb")) ...`
	ND bool dbdelete(in dbname) const;

	// Create a named db file.
	//
	// `if (not conn.createfile("myfile")) ...
	//  // or
	//  if (not createfile("myfile")) ...`
	ND bool createfile(in filename) const;

	// Rename a db file.
	//
	// `if (not conn.renamefile("myfile", "myfile2")) ...
	//  // or
	//  if (not renamefile("myfile", "myfile2")) ...`
	ND bool renamefile(in filename, in newfilename) const;

	// Delete (drop) a db file
	//
	// `if (not conn.deletefile("myfile")) ...
	//  // or
	//  if (not deletefile("myfile")) ...`
	ND bool deletefile(in filename) const;

	// Delete all records in a db file
	//
	// `if (not conn.clearfile("myfile")) ...
	//  // or
	//  if (not clearfile("myfile")) ...`
	ND bool clearfile(in filename) const;

	// Return a list of all files in a database
	//
	// `if (not conn.listfiles()) ...
	//  // or
	//  if (not listfiles()) ...`
	ND var  listfiles() const;

	// obj is conn_or_file

	// <em>Returns:</em> The approx. number of records in a file
	//
	// `let file = "definitions";
	//  var nrecs1 = file.reccount();
	//  // or
	//  var nrecs2 = reccount("myfile");`
	ND var  reccount(in filename = "") const;

	// Calls db maintenance function (vacuum)
	// This doesnt actually flush any indexes but does make sure that reccount() function is reasonably accurate.
	   bool flushindex(in filename = "") const;

	///// DATABASE FILE I/O:
	///////////////////////

	// obj is file

	// Opens database file to a var which can be used in subsequent functions to work on the specified file and database connection.
	//
	// `var file, filename = "definitions";
	//  if (not file.open(filename)) ...
	//  // or
	//  if (not open(filename to file)) ...`
	ND bool open(in dbfilename, in connection = "");

	// Closes database file var
	// Does nothing since database file vars consume no resources
	//
	// `file.close();
	//  // or
	//  close(file);`
	   void close() const;

	// Creates a secondary index for a given file and field name.
	// The fieldname must exist in a dictionary file. The default dictionary is "dict." ^ filename.
	// <em>Returns:</em> False if the index cannot be created for any reason.
	// * Index already exists
	// * File does not exist
	// * The dictionary file does not have a record with a key of the given field name.
	// * The dictionary file does not exist. Default is "dict." ^ filename.
	// * The dictionary field defines a calculated field that uses an exodus function. Using a psql function is OK.
	//
	// `var filename = "definitions", fieldname;
	//  if (not file.createindex(fieldname)) ...
	//  // or
	//  if (not createindex(filename, fieldname)) ...`
	ND bool createindex(in fieldname, in dictfile = "") const;

	// Deletes a secondary index for a file and field name.
	// <em>Returns:</em> False if the index cannot be deleted for any reason
	// * File does not exist
	// * Index does not already exists
	//
	// `var file = "definitions", fieldname = "some_name";
	//  if (not file.deleteindex(fieldname)) ...
	//  // or
	//  if (not deleteindex(file, fieldname)) ...`
	ND bool deleteindex(in fieldname) const;

	// Lists secondary indexes in a database or file
	// <em>Returns:</em> False if the file or fieldname are given and do not exist
	// obj is file|conn
	//
	// `if (not conn.listindex()) ...
	//  // or
	//  if (not listindex()) ...`
	ND var  listindex(in file_or_filename = "", in fieldname = "") const;

	// Places a metaphorical lock on a particular file and key in a database.
	// This is a advisory lock, not a physical lock, since it makes no restriction on the access or modification of data by other connections.
	// Neither the file nor the record key need to actually exist since a lock is just a hash of the filename and key combined.
	// If another connection attempts to place an identical lock on the same database it will be denied.
	// Locks can be removed by unlock() or unlockall() or will be automatically removed at the end of a transaction or when the connection is closed.
	// If the same process attempts to place an identical lock more than once it may be denied (if not in a transaction) or succeed but be ignored (if in a transaction).
	// Locks can be used to avoid processing a transaction simultaneously with another connection only to have one of them fail due to mutually updating the same records.
	// <em>Returns:</em>:
	// * 0: Failure: Another connection has already placed the same lock.
	// * "" Failure: The lock has already been placed.
	// * 1: Success: A new lock has been placed.
	// * 2: Success: The lock has already been placed and the connection is in a transaction.
	//
	// `var file = "definitions", key = "1000";
	//  if (not file.lock(key)) ...
	//  // or
	//  if (not lock(file, key)) ...`
	ND var  lock(in key) const;

	// Removes a lock placed by the lock function.
	// Only locks placed on the specified connection can be removed.
	// Locks cannot be removed while a connection is in a transaction.
	// <em>Returns:</em> False if the lock is not present in a connection.
	//
	// `var file = "definitions", key = "1000";
	//  if (not file.unlock(key)) ...
	//  // or
	//  if (not unlock(file, key)) ...`
	   bool unlock(in key) const;

	// Removes all locks placed by the lock function in the specified connection.
	// Locks cannot be removed while in a transaction.
	//
	// `if (not conn.unlockall()) ...
	//  // or
	//  if (not unlockall(conn)) ...`
	   bool unlockall() const;

	// obj is rec

	// Reads a record from a file given its unique primary key.
	// <em>Returns:</em> False if the key doesnt exist
	//
	// `var rec, file = "definitions", key = "1000";
	//  if (not rec.read(file, key)) ...
	//  // or
	//  if (not read(rec from file, key)) ...`
	ND bool read(in file, in key);

	// Writes a record to a file.
	// Updates an existing record if the key already exists or inserts a new record.
	// It always succeeds so no result code is returned.
	// Any memory cached record is deleted.
	//
	// `var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
	// `rec.write(file, key);
	//  // or
	//  write(rec on file, key);`
	   void write(in file, in key) const;

	// Updates an existing record in a file.
	// <em>Returns:</em> False if the key doesnt already exist
	// Any memory cached record is deleted.
	//
	// `var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
	//  if (not rec.updaterecord(file, key)) ...
	//  // or
	//  if (not updaterecord(rec on file, key)) ...`
	ND bool updaterecord(in file, in key) const;

	// Inserts a new record in a file.
	// <em>Returns:</em> False if the key already exists
	// Any memory cached record is deleted.
	//
	// `var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
	//  if (not rec.insertrecord(file, key)) ...
	//  // or
	//  if (not insertrecord(rec on file, key)) ...`
	ND bool insertrecord(in file, in key) const;

	// Deletes a record from a file given its key.
	// <em>Returns:</em> False if the key doesnt exist
	// Any memory cached record is deleted.
	// obj is file
	//
	// `var file = "definitions", key = "1000";
	//  if (not file.deleterecord(key)) ...
	//  // or
	// if (not deleterecord(file, key)) ...`
	   bool deleterecord(in key) const;

	//  obj is strvar

	// Same as read() but only returns a specific field no from the record
	//
	// `var field, file = "definitions", key = "1000", fieldno = 3;
	//  if (not field.readf(file, key, fieldno)) ...
	//  // or
	//  if (not readf(field from file, key, fieldno)) ...`
	ND bool readf(in file, in key, const int fieldno);

	// Same as write() but only writes a specific field no to the record
	//
	// `var field = "f3", file = "definitions", key = "1000", fieldno = 3;
	//  field.writef(file, key, fieldno);
	//  // or
	//  writef(field on file, key, fieldno);`
	   void writef(in file, in key, const int fieldno) const;

	// obj is rec

	// Cache read a record from a memory cache "file" given its key.
	// 1. Tries to read from a memory cache and returns true if successful.
	// 2. Reads to read from the actual file and returns false if unsuccessful.
	// 3. Writes the record and key to the memory cache and returns true.
	// Cached db file data lives in exodus process memory not the database.
	//
	// `var rec, file = "definitions", key = "1000";
	//  if (not rec.readc(file, key)) ...
	//  // or
	//  if (not readc(rec from file, key)) ...`
	ND bool readc(in file, in key);

	// Cache write a record and key into a memory cached "file".
	// The actual file is NOT updated.
	// It either updates an existing record if the key already exists or otherwise inserts a new record.
	// It always succeeds so no result code is returned.
	//
	// `var rec = "f1^f2^f3"_var, file = "definitions", key = "1000";
	//  rec.writec(file, key);
	//  // or
	//  writec(rec on file, key);`
	   void writec(in file, in key) const;

	// obj is dbfile

	// Deletes a record and key from a memory cached "file".
	// The actual file is NOT updated.
	// <em>Returns:</em> False if the key doesnt exist
	//
	// `var file = "definitions", key = "1000";
	//  if (not file.deletec(key)) ...
	//  // or
	//  if (not deletec(file, key)) ...`
	   bool deletec(in key) const;

	// obj is conn

	// Clears the memory cache of all records for the given connection
	// All future cache readc() function calls will be forced to obtain records from the actual database and refresh the cache.
	//
	// `conn.cleardbcache();
	//  // or
	// cleardbcache(conn);`
	   void cleardbcache() const;

	//  obj is strvar

	// The xlate ("translate") function is similar to readf() but, when called as an exodus program member function, it can be used efficiently with exodus file dictionaries using column names and functions and multivalued data.
	// ''Arguments:''
	// '''str:''' Used as the primary key to lookup a field in a given file and field no or field name.
	// '''filename:''' The file in which to look up data.
	// If var key is multivalued then a multivalued field is returned.
	// '''fieldno:''' Determines which field of the record is returned.
	// * Integer returns that field number
	// * 0 means return the key unchanged.
	// * "" means return the whole record.
	// '''mode:''' Determines what is returned if the record does not exist for the given key and file.
	// * "X" returns ""
	// * "C" returns the key unconverted.
	//
	// `var partno = "A1000-1";
	//  var partname1 = partno.xlate("PARTS", 3, "X");
	//  // or
	//  var partname2 = xlate(partno, "PARTS", "PART_NAME", "X");`
	ND var  xlate(in filename, in fieldno, const char* mode) const;

	///// DATABASE SORT/SELECT:
	//////////////////////////

	// obj is file

	ND bool select(in sortselectclause = "");
	   void clearselect();
	ND bool hasnext();
	ND bool readnext(out key);
	ND bool readnext(out key, out valueno);
	ND bool readnext(out record, out key, out valueno);

	   bool savelist(in listname);
	ND bool getlist(in listname);
	ND bool makelist(in listname, in keys);
	   bool deletelist(in listname) const;
	ND bool formlist(in keys, in fieldno = 0);

	///// OS TIME/DATE:
	//////////////////

	// obj is var()

	// Number of whole days since pick epoch 1967-12-31 00:00:00 UTC. Negative for dates before.
	//
	// `var today1 = var().date(); // e.g. was 20821 from 2025-01-01 00:00:00 UTC
	//  // or
	//  var today2 = date();`
	ND var  date() const;

	// Number of whole seconds since last 00:00:00 (UTC).
	//
	// `var now1 = var().time(); // range 0 - 86399 since there are 24*60*60 (86400) seconds in a day.
	//  // or
	//  var now2 = time();`
	ND var  time() const;

	// Number of fractional seconds since last 00:00:00 (UTC).
	// A floating point with approx. nanosecond resolution depending on hardware.
	//
	// `var now1 = var().ostime(); // e.g. 23343.704387955 approx. 06:29:03 UTC
	//  // or
	//  var now2 = ostime();`
	ND var  ostime() const;

	// Number of fractional days since pick epoch 1967-12-31 00:00:00 UTC. Negative for dates before.
	// A floating point with approx. nanosecond resolution depending on hardware.
	//
	// `var now1 = var().timestamp(); // was 20821.99998842593 around 2025-01-01 23:59:59 UTC
	//  // or
	//  var now2 = timestamp();`
	ND var  timestamp() const;

	// Construct a timestamp from a date and time
	//
	// `var idate = iconv("2025-01-01", "D"), itime = iconv("23:59:59", "MT");
	//  var ts1 = idate.timestamp(itime); // 20821.99998842593
	//  // or
	// var ts2 = timestamp(idate, itime);`
	ND var  timestamp(in ostime) const;

	// Sleep/pause/wait for a number of milliseconds
	//
	// `var().ossleep(3000); // sleep for 3 seconds
	//  // or
	//  ossleep(3000);`
	   void ossleep(const int milliseconds) const;

	// Sleep/pause/wait up to a given number of milliseconds or until any changes occur in an FM delimited list of directories and/or files.
	// Any terminal input (e.g. a key press) will also terminate the wait.
	// An FM array of event information is returned. See below.
	// Multiple events are returned in multivalues.
	// obj is file_dir_list
	//
	// `let v1 = ".^/etc/hosts"_var.oswait(3000); // e.g. "IN_CLOSE_WRITE^/etc^hosts^f"_var
	//  // or
	//  let v2 = oswait(".^/etc/hosts"_var, 3000);`
	//
	// Returned array fields
	// 1. Event type codes
	// 2. dirpaths
	// 3. filenames
	// 4. d=dir, f=file
	//
	// Possible event type codes are as follows:
	// * IN_CLOSE_WRITE - A file opened for writing was closed
	// * IN_ACCESS      - Data was read from file
	// * IN_MODIFY      - Data was written to file
	// * IN_ATTRIB      - File attributes changed
	// * IN_CLOSE       - File was closed (read or write)
	// * IN_MOVED_FROM  - File was moved away from watched directory
	// * IN_MOVED_TO    - File was moved into watched directory
	// * IN_MOVE        - File was moved (in or out of directory)
	// * IN_CREATE      - A file was created in the directory
	// * IN_DELETE      - A file was deleted from the directory
	// * IN_DELETE_SELF - Directory or file under observation was deleted
	// * IN_MOVE_SELF   - Directory or file under observation was moved
	   var  oswait(const int milliseconds) const;

	///// OS FILE I/O:
	/////////////////

	// obj is osfilevar

	// Given the name of an existing file name including path, initialises a file handle var that can be used in random access osbread and osbwrite functions.
	// The utf8 option defaults to true which causes trimming of partial utf-8 unicode byte sequences from the end of osbreads. For raw untrimmed osbreads pass utf8 = false;
	// File will be opened for writing if possible otherwise for reading.
	// <em>Returns:</em> True if successful or false if not possible for any reason.
	// e.g. Target doesnt exist, permissions etc.
	//
	// `var hostsfile;
	//  if (not hostsfile.osopen("/etc/hosts")) ...
	//  // or
	//  if (not osopen("/etc/hosts" to hostsfile)) ...`
	ND bool osopen(in osfilename, const bool utf8 = true) const;

	// Reads length bytes from an existing os file starting at a given byte offset (0 based).
	// The osfilevar file handle may either be initialised by osopen or be just be a normal string variable holding the path and name of the os file.
	// After reading, the offset is updated to point to the correct offset for a subsequent sequential read.
	// If reading utf8 data (the default) then the length of data actually returned may be a few bytes shorter than requested in order to be a complete number of UTF-8 code points.
	//
	// `var text, hostsfile = "/etc/hosts", offset = 0;
	//  if (not text.osbread(hostsfile, offset, 1024)) ...
	//  // or
	//  if (not osbread(text from hostsfile, offset, 1024)) ...`
	ND bool osbread(in osfilevar, io offset, const int length);

	// Writes data to an existing os file starting at a given byte offset (0 based).
	// See osbread for more info.
	//
	// `let text = "\n#comment", xyzfile = "../xyz.conf";
	//  var offset = osfile(xyzfile).f(1); // size of file -> append
	//  if (not text.osbwrite(xyzfile, offset)) abort(lasterror());
	//  // or
	//  if (not osbwrite(text on xyzfile, offset)) ...`
	ND bool osbwrite(in osfilevar, io offset) const;

	// Removes an osfilevar handle from the internal memory cache of os file handles. This frees up both exodus process memory and operating system resources.
	// It is advisable to osclose any file handles after use, regardless of whether they were specifically opened using osopen or not, especially in long running programs. Exodus performs caching of internal os file handles per thread and os file. If not closed, then the operating system will probably not flush deleted files from storage until the process is terminated. This can potentially create an memory issue or file system resource issue especially if osopening/osreading/oswriting many perhaps temporary files in a long running process.
	//
	// `osfilevar.osclose();
	//  // or
	//  osclose(osfilevar);`
	   void osclose() const;

	//  obj is strvar

	// Read a complete os file into a var.
	// If codepage is specified then input is converted from that codepage to utf-8 otherwise no conversion is done.
	// <em>Returns:</em> True if successful or false if not possible for any reason.
	// e.g. File doesnt exist, permissions etc.
	//
	// `var text;
	//  if (not text.osread("/etc/hosts")) ...
	//  // or
	//  if (not osread(text from "/etc/hosts")) ...`
	ND bool osread(const char* osfilename, const char* codepage = "");

	// Create a complete os file from a var.
	// Any existing file is removed first.
	// <em>Returns:</em> True if successful or false if not possible for any reason.
	// e.g. Path is not writeable, permissions etc.
	// If codepage is specified then output is converted from utf-8 to that codepage. Otherwise no conversion is done.
	//
	// `let text = "xyz = 123\n", osfilename="../xyz.conf";
	//  if (not text.oswrite(osfilename)) abort(lasterror());
	//  // or
	//  if (not oswrite(text on osfilename)) ...`
	ND bool oswrite(in osfilename, const char* codepage = "") const;

	// TODO check if it calls osclose on itself in case removing a varfile

	// obj is osfilename

	// Removes/deletes an os file from the OS file system given path and name.
	// Will not remove directories. Use osrmdir() to remove directories
	// <em>Returns:</em> True if successful or false if not possible for any reason.
	// e.g. Target doesnt exist, path is not writeable, permissions etc.
	//
	// `if (not "../xyz.conf"_var.osremove()) abort(lasterror());
	//  // or
	//  if (not osremove("../xyz.conf")) ...`
	ND bool osremove() const;

	// obj is osfileordirname

	// Renames an os file or dir in the OS file system.
	// Will not overwrite an existing file or dir.
	// Source and target must exist in the same storage device.
	// <em>Returns:</em> True if successful or false if not possible for any reason.
	// e.g. Target already exists, path is not writeable, permissions etc.
	// Uses std::filesystem::rename internally.
	//
	// `if (not "../xyz.conf"_var.osrename("../abc.conf")) abort(lasterror());
	//  // or
	//  if (not osrename("../xyz.conf", "../abc.conf")) ...`
	ND bool osrename(in new_dirpath_or_filepath) const;

	// Copies a file or directory recursively within the file system.
	// Uses std::filesystem::copy internally with recursive and overwrite options
	//
	// `if (not "../xyz.conf"_var.oscopy("../abc.conf")) abort(lasterror());
	//  // or
	//  if (not oscopy("../xyz.conf", "../abc.conf")) ...`
	ND bool oscopy(in to_osfilename) const;

	// "Moves" a file or dir within the os file system.
	// Will not overwrite an existing file or dir.
	// <em>Returns:</em> True if successful or false if not possible for any reason.
	// e.g. Source doesnt exist or cannot be accessed, target already exists, source or target is not writeable, permissions etc.
	// Attempts osrename first then oscopy followed by osremove original.
	//
	// `if (not "../xyz.conf"_var.osmove("../abc.conf")) abort(lasterror());
	//  // or
	//  if (not osmove("../xyz.conf", "../abc.conf")) ...`
	ND bool osmove(in to_osfilename) const;

	///// OS DIRECTORIES:
	////////////////////

	// List files and directories

	// obj is dirpath

	// <em>Returns:</em> A FM delimited string containing all matching dir entries given a dir path
	// A glob pattern (e.g. *.conf) can be appended to the path or passed as argument.
	//
	// `var entries1 = "/etc/"_var.oslist("*.cfg"); // "adduser.conf^ca-certificates.conf^ ..."
	//  // or
	//  var entries2 = oslist("/etc/" "*.conf");`
	ND var  oslist(SV globpattern = "", const int mode = 0) const;

	// Same as oslist for files only
	ND var  oslistf(SV globpattern = "") const;

	// Same as oslist for files only
	ND var  oslistd(SV globpattern = "") const;

	// Return dir info for any dir entry or "" if it doesnt exist
	// A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time
	// mode 0 default
	// mode 1 returns "" if not a file
	// mode 2 returns "" if not a dir
	// See also osfile() and osdir()
	//
	// `var info1 = "/etc/hosts"_var.osinfo(); // "221^20597^78309"
	//  // or
	//  var info2 = osinfo("/etc/hosts");`
	// obj is osfileordirpath
	ND var  osinfo(const int mode = 0) const;

	// Return dir info for a file
	// A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time
	// Alias for osinfo(1)
	//
	// `var fileinfo1 = "/etc/hosts"_var.osfile(); // "221^20597^78309"
	//  // or
	//  var fileinfo2 = osfile("/etc/hosts");`
	// obj is osfilename
	ND var  osfile() const;

	// Return dir info for a dir.
	// A short string containing FM ^ modified_time ^ FM ^ modified_time
	// Alias for osinfo(2)
	//
	// `var dirinfo1 = "/etc/"_var.osdir(); // "^20848^44464"
	//  // or
	//  var dirinfo2 = osfile("/etc/");`
	ND var  osdir() const;

	// Makes a new directory and returns true if successful.
	// Including parent dirs if necessary.
	//
	// `if (not "abc/def"_var.osmkdir()) ...
	//  // or
	//  if (not osmkdir("abc/def")) ...`
	ND bool osmkdir() const;

	// Removes a os dir and returns true if successful.
	// Optionally even if not empty. Including subdirs.
	//
	// `if (not "abc/def"_var.osrmdir()) ...
	//  // or
	//  if (not osrmdir("abc/def")) ...`
	ND bool osrmdir(bool evenifnotempty = false) const;

	// obj is var()

	// <em>Returns:</em> The current working directory
	//
	// `var cwd1 = var().oscwd(); // "/home/exodus"
	//  // or
	//  var cwd2 = oscwd();`
	ND var  oscwd() const;

	// Changes the current working dir and returns true if successful.
	//
	// `if (not "abc/def"_var.oscwd()) ...
	//  // or
	//  if (not oscwd("abc/def")) ...`
	ND bool oscwd(in newpath) const;

	///// OS SHELL/ENVIRONMENT:
	//////////////////////////

	// Execute a shell command
	// <em>Returns:</em> True if the process terminates with error status 0 and false otherwise.
	//
	// `let cmd = "ls -l xyz";
	//  if (not cmd.osshell()) ...
	//  // or
	//  if (not osshell(cmd)) ...`
	// obj is command
	ND bool osshell() const;

	// Same as osshell but captures stdout
	//
	// `var intext;
	//  if (not intext.osshellread("ls -l xyz")) ...
	//
	//  // or capturing stdout but ignoring exit status
	//  intext = osshellread("ls -l xyz");`
	// obj is instr
	ND bool osshellread(in oscmd);

	// Same as osshell but provides stdin to the process
	//
	// `var outtext = ...
	//  if (not outtext.osshellwrite("grep xyz")) ...
	//  // or
	//  if (not osshellwrite(outtext, "grep xyz")) ...`
	// obj is outstr
	ND bool osshellwrite(in oscmd) const;

	// Get the path of the tmp dir
	//
	// `let v1 = var().ostempdirpath(); // "/tmp/"
	//  // or
	//  let v2 = ostempdirpath();`
	// obj is var()
	ND var  ostempdirpath() const;

	// Create and get a temporary file
	//
	// `var tempfilename1 = var().ostempfilename(); // "/tmp/~exoEcLj3C"
	//  // or
	//  var tempfilename2 = ostempfilename();`
	// obj is var()
	ND var  ostempfilename() const;

	// obj is envvalue

	// Get the value of an environment variable
	//
	// `var envvalue1;
	//  if (not envvalue1.osgetenv("HOME")) ... // "/home/exodus"
	//  // or
	//  var envvalue2 = osgetenv("HOME");`
	ND bool osgetenv(SV envcode);

	// Set the value of an environment variable code
	//
	// `var envvalue, envcode = "HOME";
	//  envvalue.ossetenv(envcode);
	//  // or
	//  ossetenv(envcode, envvalue);`
	// obj is envvalue
	   void ossetenv(SV envcode) const;

	// TODO check for threadsafe

	// obj is var()

	// Get the os process id
	//
	// `let pid1 = var().ospid(); // 663237
	//  // or
	//  let pid2 = ospid();`
	ND var  ospid() const;

	// Get the os thread process id
	//
	// `let tid1 = var().ostid(); // 663237
	//  // or
	//  let tid2 = ostid();`
	ND var  ostid() const;

	// Get the libexodus build date and time
	//
	// `let v1 = var().version(); // "29 JAN 2025 14:56:52"`
	ND var  version() const;

	///// OUTPUT:
	////////////

	// obj is var

	// To stdout/cout Buffered.
	   CVR output() const;      // To stdout. No new line. Buffered.
	   CVR outputl() const;     // To stdout. Starts a new line. Flushed.
	   CVR outputt() const;     // To stdout. Adds a tab. Buffered.

	// To stdlog/clog Buffered.
	   CVR logput() const;  // To stdlog. No new line. Buffered.
	   CVR logputl() const; // To stdlog. Starts a new line. Flushed.

	// To stderr/cerr usually unBuffered.
	   CVR errput() const;  // To stderr. No new line. Flushed.
	   CVR errputl() const; // To stderr. Starts a new line. Flushed.

	// As above but with a prefix
	   CVR output(in prefix) const;  // To stdout. With a prefix. No new line. Buffered.
	   CVR outputl(in prefix) const; // To stdout. With a prefix. Starts a new line. Flushed.
	   CVR outputt(in prefix) const; // To stdout. With a prefix. Adds a tab. Buffered.

	// As above but with a prefix
	   CVR logput(in prefix) const;  // To stdlog. With a prefix. No new line. Buffered.
	   CVR logputl(in prefix) const; // To stdlog. With a prefix. Starts a new line. Flushed.

	// As above but with a prefix
	   CVR errput(in prefix) const;  // To stderr. With a prefix. No new line. Flushed.
	   CVR errputl(in prefix) const; // To stderr. With a prefix. Starts a new line. Flushed.

	// Output to a given stream
	   CVR put(std::ostream& ostream1) const;

	// Flushes any Buffered. output to stdout/cout
	//
	// `var().osflush();
	//  // or
	//  osflush();`
	// obj is var()
	   void osflush() const;

	///// STANDARD INPUT:
	////////////////////

	   out  input();                  // Wait for stdin until cr or eof
	   out  input(in prompt);         // Ditto after outputting prompt to stdout
	   out  inputn(const int nchars); // Wait for nbytes from stdin

	ND bool isterminal() const;                   // true if terminal is available
	ND bool hasinput(int milliseconds = 0) const; // true if stdin bytes available within milliseconds
	ND bool eof() const;                          // true if stdin is at end of file
	   bool echo(const int on_off) const;         // Reflect all stdin to stdout if terminal available

	   void breakon() const;  // Allow interrupt Ctrl+C
	   void breakoff() const; // Prevent interrupt Ctr+C

	// Close documentation
	/// :
	////////////////////////

	// Iterator over fields
	///////////////////////

	// friend class to iterate over the fields of a var
	friend class var_iter;

	ND var_iter begin() const;
	ND var_iter end() const;

	///////////////////////////
	// PRIVATE MEMBER FUNCTIONS
	///////////////////////////

 private:

	ND bool cursorexists(); //database, not terminal
	ND bool selectx(in fieldnames, in sortselectclause);

	   var  setlasterror(in msg) const;

	// TODO check if can speed up by returning reference to converted self like MC

	// Faster primitive oconv
	// L/R/C: Text -> left/right/center padded and truncated
	ND std::string oconv_LRC(in format) const;
	// T: Text -> justified and folded
	ND std::string oconv_T(in format) const;
	// D: Int -> Date
	ND std::string oconv_D(const char* conversion) const;
	// MT: Int -> Time
	ND std::string oconv_MT(const char* conversion) const;
	// MD: Decimal -> Decimal
	ND std::string oconv_MD(const char* conversion) const;
	// MR: Character replacement // WHY is this replacer, io, non-const
	ND io   oconv_MR(const char* conversion);
	// HEX: Chars -> Hex
	ND std::string oconv_HEX(const int ioratio) const;
	// TX: Record (FM) -> text (\n) and \ line endings
	ND std::string oconv_TX(const char* conversion) const;

	// Faster primitive iconv
	// D: Int <- Date
	ND var  iconv_D(const char* conversion) const;
	// MT: Int <- Time
	ND var  iconv_MT() const;
	// MD_ Decimal <- Decimal
	ND var  iconv_MD(const char* conversion) const;
	// Chars <- Hex
	ND var  iconv_HEX(const int ioratio) const;
	// TX: Record (FM) <- text (\n) and \ line endings
	ND var  iconv_TX(const char* conversion) const;

	ND std::fstream* osopenx(in osfilename, const bool utf8 = true) const;

	   bool THIS_IS_OSFILE() const { return ((var_typ & VARTYP_OSFILE) != VARTYP_UNA); }

};  // class "var"

// NB we should probably NEVER add operator^(io var1, bool)
// this is a trick to avoid a problem that exodus concat operator
// has the wrong precedence versus the logical operators
// a ^ b > c //we should prevent this from compiling because
// exodus will treat this like a^(b>c) which is WRONG
// multivalue basic requires it to be (a^b)>c
// however c++ ^ operator has lower priority than >
// whereas multivalue concatenate operator should have HIGHER priority than >
// and no way to change priority of operators in c++
// therefore to prevent silent non-standard behaviour from causing
// insidious bugs it is better to refuse to compile a request to
// concatenate boolean as "1" or "0" in Exodus
// a refusal to compile can be solved explicitly by putting brackets
// to force compiler to do the right thing multivalue wise
//(a^b)>c
// or, if you do want to concatenate the result of a comparison do this
// a^var(b>c)

class PUBLIC var_iter {

/////////////////////////////////
// var_iter - iterate over fields
/////////////////////////////////

	const var* pvar_;
	mutable std::size_t startpos_ = 0;
	mutable std::size_t endpos_ = std::string::npos;

 public:
	using value_type = var*;

	// Default constructor
	var_iter() = default;

	// Construct from var
	var_iter(in v);

//	var_iter(var_iter&) = delete;
//	var_iter(var_iter&&) = delete;

	// Check iter != iter (i.e. iter != string::npos)
	   bool operator!=(const var_iter& vi);

	// Convert to var (copy)
	var  operator*() const;

//	// Convert to var reference
//	var&  operator*();

	// ++Iter prefix
	var_iter operator++();

	// Iter++ postfix
	var_iter operator++(int);

	// --Iter prefix
	var_iter operator--();

	// iter++ postfix
	var_iter operator--(int);

	// assign
	//void operator=(SV replacement);

};

///////////////////////////////
// dim_iter - iterate over vars
///////////////////////////////
class PUBLIC dim_iter {
private:
//	var* ptr_;
	std::vector<var>::iterator ptr_;
public:

	using iterator_category = std::random_access_iterator_tag;
	using value_type        = var;
	using reference         = var&;
	using pointer           = var*;
	using difference_type   = unsigned long long;

//	dim_iter(var* ptr) : ptr_(ptr) {}
	dim_iter(std::vector<var>::iterator iter) : ptr_(iter) {}

//	operator std::vector<var>::iterator() {
//		return std::vector<var>(*ptr_);
//	}

	var& operator*() {
		return *ptr_;
	}

	std::vector<var>::iterator operator->() {
		return ptr_;
	}

	dim_iter& operator++() {
		++ptr_;
		return *this;
	}

	dim_iter operator++(int) {
		dim_iter tmp(*this);
		++ptr_;
		return tmp;
	}

	dim_iter& operator--() {
		--ptr_;
		return *this;
	}

	dim_iter operator--(int) {
		dim_iter tmp(*this);
		--ptr_;
		return tmp;
	}

	bool operator==(const dim_iter& rhs) const {
		return ptr_ == rhs.ptr_;
	}

	bool operator!=(const dim_iter& rhs) const {
		return !(*this == rhs);
	}

	var& operator[](int n) {
		return ptr_[n];
	}

	dim_iter& operator+=(int n) {
		ptr_ += n;
		return *this;
	}

	dim_iter operator+(int n) const {
		dim_iter tmp(*this);
		tmp += n;
		return tmp;
	}

	dim_iter& operator-=(int n) {
		ptr_ -= n;
		return *this;
	}

	dim_iter operator-(int n) const {
		dim_iter tmp(*this);
		tmp -= n;
		return tmp;
	}

	long int operator-(const dim_iter& rhs) const {
		return ptr_ - rhs.ptr_;
	}

	bool operator<(const dim_iter& rhs) const {
		return ptr_ < rhs.ptr_;
	}

	bool operator>(const dim_iter& rhs) const {
		return ptr_ > rhs.ptr_;
	}

	bool operator<=(const dim_iter& rhs) const {
		return ptr_ <= rhs.ptr_;
	}

	bool operator>=(const dim_iter& rhs) const {
		return ptr_ >= rhs.ptr_;
	}

};

///////////////////////////////////////////////////////
// var_proxy1 - replace or extract fields by fn, vn, sv
///////////////////////////////////////////////////////
class PUBLIC var_proxy1 {

 private:

	exo::var& var_;
	mutable int fn_;

	// Prevent default construction
	var_proxy1() = delete;

 public:

	// Constructor for object holding a var& and a fn
	var_proxy1(var& var1, int fn) : var_(var1), fn_(fn) {}

	// Implicit conversion to var
	//
	// Allows easy extraction of fields, values and subvalues
	//
	//  var x = y(1);     // extract field 1
	//  var x = y(1,2);   // extract field 1, value 2
	//  var x = y(1,2,3); // extract field 1, value 2, subvalue 3
	//
	// Equivalent in Pick Basic
	//
	//  x = y<1,2,3>
	//
	operator var() const {
		return var_.f(fn_);
	}

	// Operator assign using =
	//
	// Allows easy replacement of fields
	//
	// Also values and subvalues. See var_proxy2 and var_proxy3
	//
	//  x(1)    = y; // Replace field 1
	//  x(1,2)  = y; // Replace field 1, value 2             (var_proxy2)
	//  x1,2,3) = y; // Replace field 1, value 2, subvalue 3 (var_proxy3)
	//
	// and easy appending of fields, values and subvalues
	//
	//  x(-1)       = y; // Append a field
	//  x(1, -1)    = y; // Append a value    (var_proxy2)
	//  x(1, 2, -1) = y; // Append a subvalue (var_proxy3)
	//
	// Equivalent in Pick Basic:
	//
	//  x<1> = y     // Replace field 1
	//  x<1,2> = y   // Replace field 1, value 2             (var_proxy2)
	//  x<1,2,3> = y // Replace field 1, value 2, subvalue 3 (var_proxy3)
	//  x<-1>    = y // Append a field
	//
	void operator=(in replacement) {
		var_.r(fn_, replacement);
	}

	void operator=(var_proxy1 rhs);
	void operator=(var_proxy2 rhs);
	void operator=(var_proxy3 rhs);

//	void operator=(var_proxy2 rhs) {
//		var_.r(fn_, var(rhs));
//	}
//
//	void operator=(var_proxy3 rhs) {
//		var_.r(fn_, var(rhs));
//	}

	// Operator bool
	//
	// Allows usage in if statements etc.
	//
	// if (x(1))
	//
	// Equivalent in Pick Basic:
	//
	// if x<fn> then
	//
	explicit operator bool() const {
		return var_.f(fn_);
	}

	// Operator []. Allow character extraction.
	// xxx(fn)[cn]
	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND RETVAR operator[](const int pos1) const {
		return this->at(pos1);
	}

	// TODO replace .f with a version of .f that returns a string_view
	// instead of wasting time constructing a temporary var only to extract a single char from it
	ND RETVAR at(const int pos1) const;

};

// class var_proxy2 - replace or extract fn, vn, sv
///////////////////////////////////////////////////
class PUBLIC var_proxy2 {
 
 private:

	exo::var& var_;
	mutable int fn_;
	mutable int vn_;

	// Prevent default construction
	var_proxy2() = delete;

 public:

	var_proxy2(var& var1, int fn, int vn) : var_(var1), fn_(fn), vn_(vn) {}

	operator var() const {
		return var_.f(fn_, vn_);
	}

	void operator=(in replacement) {
		var_.r(fn_, vn_, replacement);
	}

	void operator=(var_proxy1 rhs);
	void operator=(var_proxy2 rhs);
	void operator=(var_proxy3 rhs);

	explicit operator bool() const {
		return var_.f(fn_, vn_);
	}

	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND var  operator[](const int pos1) const {
		return this->at(pos1);
	}

	ND var  at(const int pos1) const;

}; // class var

// class var_proxy3 - replace or extract fn, vn, sn
///////////////////////////////////////////////////
class PUBLIC var_proxy3 {

 private:

	exo::var& var_;
	mutable int fn_;
	mutable int vn_;
	mutable int sn_;

	// Prevent default construction
	var_proxy3() = delete;

 public:

	var_proxy3(var& var1, int fn, int vn = 0, int sn = 0) : var_(var1), fn_(fn), vn_(vn), sn_(sn) {}

	operator var() const {
		return var_.f(fn_, vn_, sn_);
	}

	void operator=(in replacement) {
		var_.r(fn_, vn_, sn_, replacement);
	}

	void operator=(var_proxy1 rhs);
	void operator=(var_proxy2 rhs);
	void operator=(var_proxy3 rhs);

	explicit operator bool() const {
		return var_.f(fn_, vn_, sn_);
	}

	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND var  operator[](const int pos1) const {
		return this->at(pos1);
	}

	ND var  at(const int pos1) const;

};

ND inline var_proxy1 var::operator()(int fieldno) {return var_proxy1(*this, fieldno);}
ND inline var_proxy2 var::operator()(int fieldno, int valueno) {return var_proxy2(*this, fieldno, valueno);}
ND inline var_proxy3 var::operator()(int fieldno, int valueno, int subvalueno) {return var_proxy3(*this, fieldno, valueno, subvalueno);}

   inline void var_proxy1::operator=(var_proxy1 rhs) {var_.r(fn_, var(rhs));}
   inline void var_proxy1::operator=(var_proxy2 rhs) {var_.r(fn_, var(rhs));}
   inline void var_proxy1::operator=(var_proxy3 rhs) {var_.r(fn_, var(rhs));}

   inline void var_proxy2::operator=(var_proxy1 rhs) {var_.r(fn_, vn_, var(rhs));}
   inline void var_proxy2::operator=(var_proxy2 rhs) {var_.r(fn_, vn_, var(rhs));}
   inline void var_proxy2::operator=(var_proxy3 rhs) {var_.r(fn_, vn_, var(rhs));}

   inline void var_proxy3::operator=(var_proxy1 rhs) {var_.r(fn_, vn_, sn_, var(rhs));}
   inline void var_proxy3::operator=(var_proxy2 rhs) {var_.r(fn_, vn_, sn_, var(rhs));}
   inline void var_proxy3::operator=(var_proxy3 rhs) {var_.r(fn_, vn_, sn_, var(rhs));}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wreserved-identifier"

////////////////////////////////////////////////
// var versions of various field mark characters
////////////////////////////////////////////////

	PUBLIC extern const var RM;
	PUBLIC extern const var FM;
	PUBLIC extern const var VM;
	PUBLIC extern const var SM;
	PUBLIC extern const var TM;
	PUBLIC extern const var ST;

	PUBLIC extern const var BS;
	PUBLIC extern const var DQ;
	PUBLIC extern const var SQ;

	PUBLIC extern const char* const _OS_NAME;
	PUBLIC extern const char* const _OS_VERSION;

#ifdef EXO_VAR_CPP
	CONSTINIT_VAR const var RM = RM_;
	CONSTINIT_VAR const var FM = FM_;
	CONSTINIT_VAR const var VM = VM_;
	CONSTINIT_VAR const var SM = SM_;
	CONSTINIT_VAR const var TM = TM_;
	CONSTINIT_VAR const var ST = ST_;

	CONSTINIT_VAR const var BS = BS_;
	CONSTINIT_VAR const var DQ = DQ_;
	CONSTINIT_VAR const var SQ = SQ_;

#ifdef EXO_OS_NAME
	const char* const _OS_NAME = EXO_OS_NAME;
#endif

#ifdef EXO_OS_VERSION
	const char* const _OS_VERSION = EXO_OS_VERSION;
#endif

#endif // EXO_VAR_CPP

/////////////////////////////////////
// A global flag used in mvdbpostgres
/////////////////////////////////////
[[maybe_unused]] static inline int DBTRACE = var().osgetenv("EXO_DBTRACE");

//
/////////////////////
// _var user literals
/////////////////////

// "abc^def"_var
PUBLIC var  operator""_var(const char* cstr, std::size_t size);

// 123456_var
PUBLIC var  operator""_var(unsigned long long int i);

// 123.456_var
PUBLIC var  operator""_var(long double d);

}  // namespace exo

// clang-format on

#endif //EXODUS_LIBEXODUS_EXODUS_VAR_H_
