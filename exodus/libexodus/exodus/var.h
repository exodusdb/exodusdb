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

///////////////////////////////////////////////////////////////////////////////////////////////////
////                                         IOSTREAM FRIENDS
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	// OSTREAM
//	//////////
//
//
//	// rvalue
//	friend std::ostream& operator<<(std::ostream& ostream1, const var&& outvar) {
//
//		outvar.assertString(__PRETTY_FUNCTION__);
//
//		CONSTEXPR std::array VISIBLE_FMS_EXCEPT_ESC {VISIBLE_ST_, TM_, VISIBLE_SM_, VISIBLE_VM_, VISIBLE_FM_, VISIBLE_RM_};
//
//		// Replace various unprintable field marks with unusual ASCII chars
//		// Leave ESC as \x1B because it is used to control ANSI terminal control sequences
//		// std::string str = "\x1A\x1B\x1C\x1D\x1E\x1F";
//		// |\x1B}]^~  or in high to low ~^]}\x1B|	 or in TRACE() ... ~^]}_|
//		for (auto iter = outvar.var_str.begin(); iter != outvar.var_str.end(); iter++) {
//			if (*iter <= RM_ && *iter >= ST_) {
//				UNLIKELY
//				*iter = VISIBLE_FMS_EXCEPT_ESC[std::size_t(*iter - ST_)];
//			}
//		}
//
//		ostream1 << outvar.var_str;
//		return ostream1;
//	}

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

	///// STRING CREATION:
	/////////////////////

	// Convert a number into a string after rounding it to a given number of decimal places.
	// var: The number to be converted.
	// ndecimals: Determines how many decimal places are shown to the right of the decimal point or, if ndecimals is negative, how many 0's to the left of it.
	// Returns: A var containing an ASCII string of digits with a leading "-" if negative, and a decimal point "." if ndecimals is > 0.
	// Trailing zeros are not omitted. A leading "0." is shown where appropriate.
    // 0.5 always rounds away from zero. i.e. 1.5 -> 2 and -1.5 -> -2
	// obj is varnum
	//
    // `let v1 = var(0.295).round(2);  //  "0.30"
    //  // or
    //  let v2 = round(1.295, 2);      //  "1.30"
	//
    //  var v3 = var(-0.295).round(2); // "-0.30"
    //  // or
    //  var v4 = round(-1.295, 2);     // "-1.30"`
	//
	//  var v4 = round(0, 1);           // "0.0";
	//  var v5 = round(0, 0);           // "0";
	//  var v6 = round(0, -1);          // "0";
	//
	// Negative number of decimals rounds to the left of the decimal point
	// `let v1 = round(123456.789,  0); // "123457"
	//  let v2 = round(123456.789, -1); // "123460"
	//  let v3 = round(123456.789, -2); // "123500"`
    ND var  round(const int ndecimals = 0) const;

	// obj is var()

	// Get a char given an integer 0-255.
	// Returns: A string containing a single char
	// 0-127 -> ASCII, 128-255 -> invalid UTF-8 which cannot be written to the database or used in many exodus string operations
	//
	// `let v1 = var().chr(0x61); // "a"
	//  // or
	//  let v2 = chr(0x61);`
	ND var  chr(const int num) const;

	// Get a Unicode character given a Unicode Code Point (Number)
	// Returns: A single Unicode character in UTF8 encoding.
	// To get UTF code points > 2^63 you must provide negative ints because var doesnt provide an implicit constructor to unsigned int due to getting ambigious conversions because int and unsigned int are parallel priority in c++ implicit conversions.
	//
	// `let v1 = var().textchr(171416); // "𩶘" // or "\xF0A9B698"
	//  // or
	//  let v2 = textchr(171416);`
	ND var  textchr(const int num) const;

	// Get a string of repeated substrings.
	// var: The substring to be repeated
	// num: How many times to repeat the substring
	// Returns: A string
	//
	// `let v1 = "ab"_var.str(3); // "ababab"
	//  // or
	//  let v2 = str("ab", 3);`
	ND var  str(const int num) const;

	// Get a string containing a given number of spaces.
	// var: The number of spaces required.
	// Returns: A string of space chars.
	// obj is varnum
	//
	// `let v1 = var(3).space(); // "␣␣␣"
	//  // or
	//  let v2 = space(3);`
	ND var  space() const;

	// Returns: A string representing a given number written in words instead of digits.
	// locale: Something like en_GB, ar_AE, el_CY, es_US, fr_FR etc.
	// obj is varnum
	//
	// `let softhyphen = "\xc2\xad";
	//  let v1 = var(123.45).numberinwords("de_DE").replace(softhyphen, " "); // "ein␣hundert␣drei␣und␣zwanzig␣Komma␣vier␣fünf"`
	ND var  numberinwords(in languagename_or_locale_id = "");

	////////////
	// STRING // All UTF8 unless char or byte mentioned
	////////////

	///// STRING SCANNING:
	/////////////////////

	//  obj is strvar

	// Get the char number of a char
	// Returns: A number between 0 and 255.
	// If given a string, then only the first char is considered.
	//
	// `let v1 = "abc"_var.seq(); // 0x61 // decimal 97
	//  // or
	//  let v2 = seq("abc");`
	ND var  seq() const;

	// Get the Unicode Code Point of a Unicode character.
	// var: A UTF-8 string. Only the first Unicode character is considered.
	// Returns: A number.
	//
	// `let v1 = "Γ"_var.textseq(); // 915 // U+0393: Greek Capital Letter Gamma (Unicode character)
	//  // or
	//  let v2 = textseq("Γ");`
	ND var  textseq() const;

	// Get the length of a source string in number of chars
	// Returns: A number
	//
	// `let v1 = "abc"_var.len(); // 3
	//  // or
	//  let v2 = len("abc");`
	ND var  len() const;

	// Checks if the var is an empty string.
	// Returns: True if it is empty amd false if not.
	// This is a shorthand and more expressive way of writing 'if (var == "")' or 'if (var.len() == 0)' or 'if (not var.len())'
	// Note that 'if (var.empty())' is not the same as 'if (not var)' because 'if (var("0.0")' is defined as false because the string can be converted to a 0 which is always considered to be false. Compare thia with common scripting languages where 'if (var("0"))' is defined as true.
	//
	// `let v1 = "0";
	//  if (not v1.empty()) ... ok /// true
	//  // or
	//  if (not empty(v1)) ... ok // true`
	ND bool empty() const;

	// Count the number of output columns required for a given source string.
	// Returns: A number
	// Allows wide multi-column Unicode characters that occupy more than one space in a text file or terminal screen.
	// Reduces combining characters to a single column. e.g. "e" followed by grave accent is multiple bytes but only occupies one output column.
	// Does not properly calculate all possible combining sequences of graphemes e.g. face followed by colour
	//
	// `let v1 = "🤡x🤡"_var.textwidth(); // 5
	//  // or
	//  let v2 = textwidth("🤡x🤡");`
	ND var  textwidth() const;

	// Count the number of Unicode code points in a source string.
	// Returns: A number.
	//
	// `let v1 = "Γιάννης"_var.textlen(); // 7
	//  // or
	//  let v2 = textlen("Γιάννης");`
	ND var  textlen() const;

	// Count the number of fields in a source string.
	// sepstr: The separator character or substr that delimits individual fields.
	// Returns: The count of the number of fields
	// This is similar to "var.count(sepstr) + 1" but it returns 0 for an empty source string.
	//
	// `let v1 = "aa**cc"_var.fcount("*"); // 3
	//  // or
	//  let v2 = fcount("aa**cc", "*");`
	ND var  fcount(SV sepstr) const;

	// Count the number of occurrences of a given substr in a source string.
	// substr: The substr to count.
	// Returns: The count of the number of sepstr found.
	// Overlapping substrings are not counted.
	//
	// `let v1 = "aa**cc"_var.count("*"); // 2
	//  // or
	//  let v2 = count("aa**cc", "*");`
	ND var  count(SV sepstr) const;

	// Exodus   Javascript   PHP             Python       Go          Rust          C++
	// starts   startsWith() str_starts_with startswith() HasPrefix() starts_with() starts_with
	// ends     endsWith     str_ends_with   endswith     HasSuffix() ends_with()   ends_with
	// contains includes()   str_contains    contains()   Contains()  contains()    contains

	// Checks if a source string starts with a given prefix (substr).
	// prefix: The substr to check for.
	// Returns: True if the source string starts with the given prefix.
	// Returns: False if prefix is "". DIFFERS from c++, javascript, python3. See contains() for more info.
	//
	// `if ("abc"_var.starts("ab")) ... true
	//  // or
	//  if (starts("abc", "ab")) ... true`
	ND bool starts(SV prefix) const;

	// Checks if a source string ends with a given suffix (substr).
	// suffix: The substr to check for.
	// Returns: True if the source string ends with given suffix.
	// Returns: False if suffix is "". DIFFERS from c++, javascript, python3. See contains() for more info.
	//
	// `if ("abc"_var.ends("bc")) ... true
	//  // or
	//  if (ends("abc", "bc")) ... true`
	ND bool ends(SV suffix) const;

	// Checks if a given substr exists in a source string.
	// substr: The substr to check for.
	// Returns: True if the source string starts with, ends with or contains the given substr.
	// Returns: False if suffix is "". DIFFERS from c++, javascript, python3
    // Human logic: "" is not equal to "x" therefore x does not contain "".
    // Human logic: Check each item (character) in the list for equality with what I am looking for and return success if any are equal.
    // Programmer logic: Compare as many characters as are in the search string for presence in the list of characters and return success if there are no failures.
    //
	// `if ("abcd"_var.contains("bc")) ... true
	//  // or
	//  if (contains("abcd", "bc")) ... true`
	ND bool contains(SV substr) const;

	//https://en.wikipedia.org/wiki/Comparison_of_programming_languages_(string_functions)#Find

	// Find a substr in a source string.
	// substr: The substr to search for.
	// startchar1: The char position (1 based) to start the search at. The default is 1, the first char.
	// Returns: The char position (1 based) that the substr is found at or 0 if not present.
	//
	// `let v1 = "abcd"_var.index("bc"); // 2
	//  // or
	//  let v2 = index("abcd", "bc");`
	ND var  index(SV substr, const int startchar1 = 1) const;

	// Find the nth occurrence of a substr in a source string.
	// substr: The string to search for.
	// Returns: char position (1 based) or 0 if not present.
	//
	// `let v1 = "abcabc"_var.index("bc", 2); // 2
	//  // or
	//  let v2 = index("abcabc", "bc", 2);`
	ND var  indexn(SV substr, const int occurrence) const;

	// Find the position of substr working backwards from the end of the string towards the beginning.
	// substr: The string to search for.
	// Returns: The char position of the substr if found, or 0 if not.
	// startchar1: defaults to -1 meaning start searching from the last char. Positive start1char1 counts from the beginning of the source string and negative startchar1 counts backwards from the last char.
	//
	// `let v1 = "abcabc"_var.indexr("bc"); // 5
	//  // or
	//  let v2 = indexr("abcabc", "bc");`
	ND var  indexr(SV substr, const int startchar1 = -1) const;

	// Finds all matches of a given regular expression.
	// Returns: Zero or more matching substrings separated by FMs. Any groups are in VMs.
	//
	// `let v1 = "abc1abc2"_var.match("BC(\\d)", "i"); // "bc1]1^bc2]2"_var
	//  // or
	//  let v2 = match("abc1abc2", "BC(\\d)", "i");`
	//
	// regex_options:
	// <pre>
    // l - Literal (any regex chars are treated as normal chars)
    // i - Case insensitive
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
	// f - First only. Only for replace() (not match() or search())
    // w - Wildcard glob style (e.g. *.cfg) not regex style. Only for match() and search(). Not replace().
	// </pre>
	ND var  match(SV regex_str, SV regex_options = "") const;

	// Ditto
	ND var  match(const rex& regex) const;

	// Search for the first match of a regular expression.
	// startchar1: [in] char position to start the search from
	// startchar1: [out] char position to start the next search from
	// Returns: The 1st match like match()
	// regex_options as for match()
	//
	// `var startchar1 = 1;
	//  let v1 = "abc1abc2"_var.search("BC(\\d)", startchar1, "i"); // "bc1]1"_var // startchar1 -> 5 /// Ready for the next search
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

	// Get a hash of a source string.
	// modulus: The result is limited to [0, modulus)
	// Returns: A 64 bit signed integer.
	// MurmurHash3 is used.
	//
	// `let v1 = "abc"_var.hash(); assert(v1 == var(6'715'211'243'465'481'821));
	//  // or
	//  let v2 = hash("abc");`
	ND var  hash(const std::uint64_t modulus = 0) const;

	///// STRING CONVERSION - Non-mutating - Chainable:
	//////////////////////////////////////////////////

	//  obj is strvar

	// Convert to upper case
	//
	// `let v1 = "Γιάννης"_var.ucase(); // "ΓΙΆΝΝΗΣ"
	//  // or
	//  let v2 = ucase("Γιάννης");`
	ND var  ucase() const&;

	// Convert to lower case
	//
	// `let v1 = "ΓΙΆΝΝΗΣ"_var.lcase(); // "γιάννης"
	//  // or
	//  let v2 = lcase("ΓΙΆΝΝΗΣ");`
	ND var  lcase() const&;

	// Convert to title case.
	// Returns: Original source string with the first letter of each word is capitalised.
	//
	// `let v1 = "γιάννης παππάς"_var.tcase(); // "Γιάννης Παππάς"
	//  // or
	//  let v2 = tcase("γιάννης παππάς");`
	ND var  tcase() const&;

	// Convert to folded case.
	// Returns the source string standardised in a way to enable consistent indexing and searching,
	// Case folding is the process of converting text to a case independent representation.
	// https://www.w3.org/International/wiki/Case_folding
	// Accents can be significant. As in French cote, coté, côte and côté.
	// Case folding is not locale-dependent.
	//
	// `let v1 = "Grüßen"_var.fcase(); // "grüssen"
	//  // or
	//  let v2 = tcase("Grüßen");`
	ND var  fcase() const&;

	// Replace Unicode character sequences with their standardised NFC form.
	// Unicode normalization is the process of converting Unicode strings to a standard form, making them binary comparable and suitable for text processing and comparison. It is an important part of Unicode text processing.
	// For example, Unicode character "é" can be represented by either a single Unicode character, which is Unicode Code Point (\u00E9" - Latin Small Letter E with Acute), or a combination of two Unicode code points i.e. the ASCII letter "e" and a combining acute accent (Unicode Code Point "\u0301"). Unicode NFC definition converts the pair of code points to the single code point.
	// Normalization is not locale-dependent.
	//
	// `let v1 = "cafe\u0301"_var.normalize(); // "caf\u00E9" // "café"
	//  // or
	//  let v2 = normalize("cafe\u0301");`
	ND var  normalize() const&;

	// Simple reversible disguising of string text.
	// It works by treating the string as UTF8 encoded Unicode code points and inverting the first 8 bits of their Unicode Code Points.
	// Returns: A string.
	// invert(invert()) returns to the original text.
	// ASCII bytes become multibyte UTF-8 so string sizes increase.
	// Inverted characters remain on their original Unicode Code Page but are jumbled up.
	// Non-existant Unicode Code Points may be created but UTF8 encoding remains valid.
	//
	// `let v1 = "abc"_var.invert(); // "\xC2" "\x9E" "\xC2" "\x9D" "\xC2" "\x9C"
	//  // or
	//  let v2 = invert("abc");`
	ND var  invert() const&;

	// Reduce all types of field mark chars by one level.
	// Convert all FM to VM, VM to SM etc.
	// Returns: The converted string.
	// Note that subtext ST chars are not converted because they are already the lowest level.
	// String size remains identical.
	//
	// `let v1 = "a1^b2^c3"_var.lower(); // "a1]b2]c3"_var
	//  // or
	//  let v2 = lower("a1^b2^c3"_var);`
	ND var  lower() const&;

	// Increase all types of field mark chars by one level.
	// Convert all VM to FM, SM to VM etc.
	// Returns: The converted string.
	// The record mark char RM is not converted because it is already the highest level.
	// String size remains identical.
	//
	// `let v1 = "a1]b2]c3"_var.raise(); // "a1^b2^c3"_var
	//  // or
	//  let v2 = "a1]b2]c3"_var;`
	ND var  raise() const&;

	// Remove any redundant FM, VM etc. chars (Trailing FM; VM before FM etc.)
	//
	// `let v1 = "a1^b2]]^c3^^"_var.crop(); // "a1^b2^c3"_var
	//  // or
	//  let v2 = crop("a1^b2]]^c3^^"_var);`
	ND var  crop() const&;

	// Wrap in double quotes.
	//
	// `let v1 = "abc"_var.quote(); // "\"abc\""
	//  // or
	//  let v2 = quote("abc");`
	ND var  quote() const&;

	// Wrap in single quotes.
	//
	// `let v1 = "abc"_var.squote(); // "'abc'"
	//  // or
	//  let v2 = squote("abc");`
	ND var  squote() const&;

	// Remove one pair of surrounding double or single quotes.
	//
	// `let v1 = "'abc'"_var.unquote(); // "abc"
	//  // or
	//  let v2 = unquote("'abc'");`
	ND var  unquote() const&;

	// Remove all leading, trailing and excessive inner bytes.
	// trimchars: The chars (bytes) to remove. The default is space.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trim(); // "a1␣b2␣c3"
	//  // or
	//  let v2 = trim("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trim(SV trimchars = " ") const&;

	// Ditto but only leading.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimfirst(); // "a1␣␣b2␣c3␣␣"
	//  // or
	//  let v2 = trimfirst("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trimfirst(SV trimchars = " ") const&;

	// Ditto but only trailing.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimlast(); // "␣␣a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimlast("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trimlast(SV trimchars = " ") const&;

	// Ditto but only leading and trailing, not inner.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimboth(); // "a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimboth("␣␣a1␣␣b2␣c3␣␣");`
	ND var  trimboth(SV trimchars = " ") const&;

	// Get the first char of a string.
	// Returns: A char, or "" if empty.
	// Equivalent to var.substr(1,length) or var[1, length] in Pick OS
	//
	// `let v1 = "abc"_var.first(); // "a"
	//  // or
	//  let v2 = first("abc");`
	ND var  first() const&;

	// Get the last char of a string.
	// Returns: A char, or "" if empty.
	// Equivalent to var.substr(-1, 1) or var[-1, 1] in Pick OS
	//
	// `let v1 = "abc"_var.last(); // "c"
	//  // or
	//  let v2 = last("abc");`
	ND var  last() const&;

	// Get the first n chars of a source string.
	// length: The number of chars (bytes) to get.
	// Returns: A string of up to n chars.
	// Equivalent to var.substr(1, length) or var[1, length] in Pick OS
	//
	// `let v1 = "abc"_var.first(2); // "ab"
	//  // or
	//  let v2 = first("abc", 2);`
	ND var  first(const std::size_t length) const&;

	// Extract up to length trailing chars
	// Equivalent to var.substr(-length, length) or var[-length, length] in Pick OS
	//
	// `let v1 = "abc"_var.last(2); // "bc"
	//  // or
	//  let v2 = last("abc", 2);`
	ND var  last(const std::size_t length) const&;

	// Remove n chars (bytes) from the source string.
	// length: Positive to remove first n chars or negative to remove the last n chars.
	// If the absolute value of length is >= the number of chars in the source string then all chars will be removed.
	// Equivalent to var.substr(length) or var[1, length] = "" in Pick OS
	//
	// `let v1 = "abcd"_var.cut(2); // "cd"
	//  // or
	//  let v2 = cut("abcd", 2);`
	ND var  cut(const int length) const&;

	// Insert a substr at an given position after removing a given number of chars.
	//
	// pos1: 0 or 1 : Remove length chars from the beginning and insert at the beginning.
	// pos1: > than the length of the source string. Insert after the last char.
	// pos1: -1 : Remove up to length chars before inserting.Insert on or before the last char.
	// pos1: -2 : Insert on or before the penultimate char.
	// Equivalent to var[pos1, length] = substr in Pick OS
	//
	// `let v1 = "abcd"_var.paste(2, 2, "XYZ"); // "aXYZd"
	//  // or
	//  let v2 = paste("abcd", 2, 2, "XYZ");`
	ND var  paste(const int pos1, const int length, SV insertstr) const&;

	// Insert text at char position without overwriting any following chars
	// Equivalent to var[pos1, 0] = substr in Pick OS
	//
	// `let v1 = "abcd"_var.paste(2, "XYZ"); // "aXYZbcd"
	//  // or
	//  let v2 = paste("abcd", 2, "XYZ");`
	ND var  paste(const int pos1, SV insertstr) const&;

	// Insert text at the beginning
	// Equivalent to var[0, 0] = substr in Pick OS
	//
	// `let v1 = "abc"_var.prefix("XYZ"); // "XYZabc"
	//  // or
	//  let v2 = prefix("abc", "XYZ");`
	ND var  prefix(SV insertstr) const&;

	template <typename... ARGS>
	// Append anything at the end of a string
	//
	// `let v1 = "abc"_var.append(" is ", 10, " ok", '.'); // "abc is 10 ok."
	//  // or
	//  let v2 = append("abc", " is ", 10, " ok", '.');`
	ND var  append(const ARGS&... appendable) const& {
		var nrvo = *this;
		(nrvo ^= ... ^= appendable);
		return nrvo;
	}

	// Remove one trailing char.
	// Equivalent to var[-1, 1] = "" in Pick OS
	//
	// `let v1 = "abc"_var.pop(); // "ab"
	//  // or
	//  let v2 = pop("abc");`
	ND var  pop() const&;

	// Copies one or more consecutive fields from a string given a delimiter
	// delimiter: A Unicode character.
	// fieldno: The first field is 1, the last field is -1.
	// Returns: A substring
	//
	// `let v1 = "aa*bb*cc"_var.field("*", 2); // "bb"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", 2);`
	//
	// `let v1 = "aa*bb*cc"_var.field("*", -1); // "cc"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", -1);`
	ND var  field(SV delimiter, const int fieldnx = 1, const int nfieldsx = 1) const;

	// field2 is a version that treats fieldn -1 as the last field, -2 the penultimate field etc. -
	// TODO Should probably make field() do this (since -1 is basically an erroneous call) and remove field2
	// Same as var.field() but negative fieldnos work backwards from the last field.
	//
	[[deprecated ("Just use field() which now has the same behaviour as field()")]]
	ND var  field2(SV separator, const int fieldno, const int nfields = 1) const
	{
		if (fieldno >= 0) LIKELY
			return field(separator, fieldno, nfields);
		return field(separator, this->count(separator) + 1 + fieldno + 1, nfields);
	}

	// fieldstore() replaces, inserts or deletes subfields in a string.
	// fieldno: The field number to replace or, if not 1, the field number to start at. Negative fieldno counts backwards from the last field.
	// nfields: The number of fields to replace or, if negative, the number of fields to delete first. Can be 0 to cause simple insertion of fields.
	// replacement: A string that is the replacement field or fields.
	// Returns: A modified copy of the original string.
	// There is no way to simply delete n fields because the replacement argument cannot be omitted, however one can achieve the same result by replacing n+1 fields with the n+1th field.
	//
	// The replacement can contain multiple fields itself. If replacing n fields and the replacement contains < n fields then the remaining fields become "", and if the replacement contains more fields than are required, they are unused.
	//
	// `let v1 = "aa,bb,cc,dd,ee"_var.fieldstore(",", 2, 3, "11,22"); // "aa,11,22,,ee"
	//  // or
	//  let v2 = fieldstore("aa,bb,cc,dd,ee", ",", 2, 3, "11,22");`
	//
	// If nfields is 0 then insert the replacement field(s) before fieldno
	//
	// `let v1 = "aa,bb,cc,dd,ee"_var.fieldstore(",", 2, 0, "11,22"); // "aa,11,22,bb,cc,dd,ee"`
	//
	// If nfields is negative then delete abs(n) fields before inserting whatever fields the replacement has.
	//
	// `let v1 = "aa,bb,cc,dd,ee"_var.fieldstore(",", 2, -2, "11"); // "aa,11,dd,ee"`
	//
	// If nfields exceeds the number of fields in the input then additional empty fields are added.
	//
	// `let v1 = "aa,bb,cc"_var.fieldstore(",", 6, 2, "11"); // "aa,bb,cc,,,11,"`
	//
	ND var  fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const&;

	// substr version 1.
	// Copies a substr of length chars from a given a starting char position.
	// Returns: A substr or "".
	// pos1: The char position to start at. If negative then start from a position counting backwards from the last char
	// length: The number of chars to copy. If negative then copy backwards. This reverses the order of the chars in the returned substr.
	// Equivalent to var[start, length] in Pick OS
	// Not Unicode friendly.
	//
	// `let v1 = "abcd"_var.substr(2, 2); // "bc"
	//  // or
	//  let v2 = substr("abcd", 2, 2);`
	//
	// If pos1 is negative then start counting backwards from the last char
	//
	// `let v1 = "abcd"_var.substr(-3, 2); // "bc"
	//  // or
	//  let v2 = substr("abcd", -3, 2);`
	//
	// If length is negative then work backwards and return chars reversed
	//
	// `let v1 = "abcd"_var.substr(3, -2); // "cb"
	//  // or
	//  let v2 = substr("abcd", 3, -2); // "cb"`
	ND var  substr(const int pos1, const int length) const&;

	// Abbreviated alias of substr version 1.
	ND var  b(const int pos1, const int length) const& {return this->substr(pos1, length);}

	// substr version 2.
	// Copies a substr from a given char position up to the end of the source string
	// Returns: A substr or "".
	// pos1: The char position to start at. If negative then start from a position counting backwards from the last char
	// Equivalent to var[pos1, 9999999] in Pick OS
	// Partially Unicode friendly but pos1 is in chars.
	//
	// `let v1 = "abcd"_var.substr(2); // "bcd"
	//  // or
	//  let v2 = substr("abcd", 2);`
	ND var  substr(const int pos1) const&;

	// Shorthand alias of substr version 2.
	ND var  b(const int pos1) const& {return this->substr(pos1);}

	// substr version 3.
	// Copies a substr from a given char position up to (but excluding) any one of some given delimiter chars
	// Returns: A substr or "".
	// pos1: [in] The position of the first char to copy. Negative positions count backwards from the last char of the string.
	// pos2: [out] The position of the next delimiter char, or one char position after the end of the source string if no subsequent delimiter chars are found.
	// COL2: is a predefined variable that can be used for pos2 instead of declaring a variable.
	// An empty string may be returned if pos1 [in] points to one of the delimiter chars or points beyond the end of the source string.
	// Equivalent to var[pos1, ",."] in Pick OS (non-numeric length).
	// Works with any encoding including UTF8 for the source string but the delimiter chars are bytes.
	// Add 1 to pos2 to skip over the next delimiter char to copy the next substr
	// Works with any encoding including UTF8 for the source string but the delimiter chars are bytes.
	// This function is similar to std::string::find_first_of but that function only returns pos2.
	//
	// `var pos1 = 4;
	//  let v1 = "12,45 78"_var.substr(pos1, ", ", COL2);  // v1 -> "45" // COL2 -> 6 // 6 is the position of the next delimiter char found.
	//  // or
	//  let v2 = substr("12,45 78", COL2 + 1, ", ", COL2); // v2 -> "78" // COL2 -> 9 // 9 is one after the end of the string meaning that none of the delimiter chars were found.`
	   var  substr(const int pos1, SV delimiterchars, out pos2) const;

	// Shorthand alias of substr version 3.
	   var  b(const int pos1, SV delimiterchars, out pos2) const {return this->substr(pos1, delimiterchars, pos2);}

	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// offset to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(io pos1, io
	// delimiterno) const;

	// substr version 4.
	// Copies a substr from a given char position up to (but excluding) the next field mark char (RM, FM, VM, SM, TM, ST).
	// Returns: A substr or "".
	// pos1: [in] The position of the first char to copy. Negative positions count backwards from the last char of the string.
	// pos1: [out] The position of the first char of the next substr after whatever field mark char is found, or one char position after the end of the source string if no subsequent field mark char is found.
	// field_mark_no: [out] A number (1-6) indicating which of the standard field mark chars was found, or 0 if not.
	// An empty string may be returned if the pos1 [in] points to one of the field marks or beyond the end of the source string.
	// pos1 [out] is correctly positioned to copy the next substr.
	// Works with any encoding including UTF8. Was called "remove" in Pick OS.
	// The equivalent in Pick OS was the statement "Remove variable From string At column Setting flag"
	// ...
	// This function is valuable for high performance processing of dynamic arrays.
	// It is notably used in "list" to print parallel columns of mixed combinations of multivalues/subvalues and text marks correctly lined up mv to mv, sv to sv, tm to tm even when particular values, subvalues and text fragments are missing from particular columns.
	// It is similar to version 3 of substr - substr(pos1, delimiterchars, pos2) except that in this version the delimiter chars are hard coded as the standard field mark chars (RM, FM, VM, SM, TM, ST) and it returns the first char position of the next substr, not the char position of the next field mark char.
	//
	// `var pos1 = 4, field_mark_no;
	//  let v1 = "12^45^78"_var.substr2(pos1, field_mark_no);  // "45" // pos1 -> 7 // field_mark_no -> 2 // field_mark_no 2 means that a FM was found.
	//  // or
	//  let v2 = substr2("12^45^78"_var, pos1, field_mark_no); // "78" // pos1 -> 9 // field_mark_no -> 0 // field_mark_no 0 means that none of the standard field marks were found.`
	   var  substr2(io pos1, out delimiterno) const;

	// Shorthand alias of substr version 4.
	   var  b2(io pos1, out field_mark_no) const {return this->substr2(pos1, field_mark_no);}

	// Convert or delete chars one for one to other chars
	// from_chars: chars to convert. If longer than to_chars then delete those characters instead of converting them.
	// to_chars: chars to convert to
	// Not UTF8 compatible.
	//
	// `let v1 = "abcde"_var.convert("aZd", "XY"); // "Xbce" // a is replaced and d is removed
	//  // or
	//  let v2 = convert("abcde", "aZd", "XY");`
	ND var  convert(SV from_chars, SV to_chars) const&;

	// Ditto for Unicode code points.
	//
	// `let v1 = "a🤡b😀c🌍d"_var.textconvert("🤡😀", "👋"); // "a👋bc🌍d"
	//  // or
	//  let v2 = textconvert("a🤡b😀c🌍d", "🤡😀", "👋");`
	ND var  textconvert(SV from_characters, SV to_characters) const&;

	// Replace all occurrences of one substr with another.
	// Case sensitive.
	//
	// `let v1 = "Abc.Abc"_var.replace("bc", "X"); // "AX.AX"
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
	// `let v1 = "a1^b2^a1^c2"_var.unique(); // "a1^b2^c2"_var
	//  // or
	//  let v2 = unique("a1^b2^a1^c2"_var);`
	ND var  unique() const&;

	// Reorder fields in an FM or VM etc. separated list in ascending order
	// Numeric data:
	// `let v1 = "20^10^2^1^1.1"_var.sort(); // "1^1.1^2^10^20"_var
	//  // or
	//  let v2 = sort("20^10^2^1^1.1"_var);`
	// Alphabetic data:
	//  `let v1 = "b1^a1^c20^c10^c2^c1^b2"_var.sort(); // "a1^b1^b2^c1^c10^c2^c20"_var
	//  // or
	//  let v2 = sort("b1^a1^c20^c10^c2^c1^b2"_var);`
	ND var  sort(SV delimiter = _FM) const&;

	// Reorder fields in an FM or VM etc. separated list in descending order
	//
	// `let v1 = "20^10^2^1^1.1"_var.reverse(); // "1.1^1^2^10^20"_var
	//  // or
	//  let v2 = reverse("20^10^2^1^1.1"_var);`
	ND var  reverse(SV delimiter = _FM) const&;

	// Randomise the order of fields in an FM, VM separated list
	//
	// `let v1 = "20^10^2^1^1.1"_var.shuffle(); /// e.g. "2^1^20^1.1^10" (random order depending on initrand())
	//  // or
	//  let v2 = shuffle("20^10^2^1^1.1"_var);`
	ND var  shuffle(SV delimiter = _FM) const&;

	// Split a delimited string with embedded quotes into a dynamic array.
	// Can be used to process CSV data.
	// Replaces separator chars with FM chars except inside double or single quotes and ignoring escaped quotes &bsol;" &bsol;'
	//
	// `let v1 = "abc,\"def,\"123\" fgh\",12.34"_var.parse(','); // "abc^\"def,\"123\" fgh\"^12.34"_var
	//  // or
	//  let v2 = parse("abc,\"def,\"123\" fgh\",12.34", ',');`
	ND var  parse(char sepchar = ' ') const&;

	// Split a delimited string into a dim array.
	// The delimiter can be multibyte Unicode.
	//
	// `dim d1 = "a^b^c"_var.split(); // A dimensioned array with three elements (vars)
	//  // or
	//  dim d2 = split("a^b^c"_var);`
	ND dim  split(SV delimiter = _FM) const;

	// SAME ON TEMPORARIES - CALL MUTATORS FOR SPEED (not documenting since programmer interface is the same)
	/////////////////////////////////////////

	// UTF8/byte as for accessors

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
			template <typename... ARGS>
	ND io   append(const ARGS&... appendable) && {
				((*this) ^= ... ^= appendable);
				return *this;
			}
	ND io   pop() &&;

	ND io   fieldstore(SV delimiter, const int fieldno, const int nfields, in replacement) &&;
	ND io   substr(const int pos1, const int length) &&;
	ND io   substr(const int pos1) &&;

	ND io   convert(SV from_chars, SV to_chars) &&;
	ND io   textconvert(SV from_characters, SV to_characters) &&;
	ND io   replace(const rex& regex, SV tostr) &&;
	ND io   replace(SV fromstr, SV tostr) &&;
//	ND io   regex_replace(SV regex_str, SV replacement, SV regex_options = "") &&;

	ND io   unique() &&;
	ND io   sort(SV delimiter = _FM) &&;
	ND io   reverse(SV delimiter = _FM) &&;
	ND io   shuffle(SV delimiter = _FM) &&;
	ND io   parse(char sepchar = ' ') &&;

	///// STRING MUTATION - Standalone commands:
	////////////////////////////////////////////

	//  obj is strvar

	// Upper case
	// All string mutators follow the same pattern as ucaser.<br>See the non-mutating functions for details.
	//
	// `var v1 = "abc";
	//  v1.ucaser(); // "ABC"
	//  // or
	//  ucaser(v1);`
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
			template <typename... ARGS>
	   IO   appender(const ARGS&... appendable) REF {
				((*this) ^= ... ^= appendable);

				false && (*this);
//				return void;
			}

	   IO   popper() REF ;

	// TODO look at using erase to speed up

	   IO   fieldstorer(SV delimiter, const int fieldno, const int nfields, in replacement) REF ;
	   IO   substrer(const int pos1, const int length) REF ;
	   IO   substrer(const int pos1) REF {return this->substrer(pos1, this->len());}
	   IO   converter(SV from_chars, SV to_chars) REF;
	   IO   textconverter(SV from_characters, SV to_characters) REF;
	   IO   replacer(const rex& regex, SV tostr) REF;
	   IO   replacer(SV fromstr, SV tostr) REF;
//	   IO   regex_replacer(SV regex, SV replacement, SV regex_options = "") REF ;

	   IO   uniquer() REF ;
	   IO   sorter(SV delimiter = _FM) REF ;
	   IO   reverser(SV delimiter = _FM) REF ;
	   IO   shuffler(SV delimiter = _FM) REF ;
	   IO   parser(char sepchar = ' ') REF ;

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
////        assert(x.format("{:.2f}") == "12.35");
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
	//  // U+0404 Cyrillic Capital Letter Ukrainian Ie Unicode character`
	ND var  from_codepage(const char* codepage) const;

	// Converts to codepage encoded text from UTF-8 encoded text
	//
	// `let v1 = "Є"_var.to_codepage("CP1124").oconv("HEX"); // "A4"
	//  // or
	//  let v2 = to_codepage("Є", "CP1124").oconv("HEX");`
	ND var  to_codepage(const char* codepage) const;

	///// DYNAMIC ARRAY FUNCTIONS:
	/////////////////////////////

	//  obj is strvar

	// EXTRACT() AND F()

	// Pick OS
	//   xxx=yyy<10>;
	//   xxx=extract(yyy,10);
	// becomes c++
	//   xxx=yyy.f(10);

	// f() is a highly abbreviated alias for the Pick OS field/value/subvalue extract() function.
	// "f()" can be thought of as "field" although the function can extract values and subvalues as well.
	// The convenient Pick OS angle bracket syntax for field extraction (e.g. xxx<20>) is not available in C++.
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
	//  // For brevity the function alias "f()" (standing for "field") is normally used instead of "extract()" as follows:
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
	// `let v1 = "1]2]3^4]5]6"_var.sum(); // "6^15"_var
	//  // or
	//  let v2 = sum("1]2]3^4]5]6"_var);`
	ND var  sum() const;

	// Sum up all levels into a single figure
	//
	// `let v1 = "1]2]3^4]5]6"_var.sumall(); // 21
	//  // or
	//  let v2 = sumall("1]2]3^4]5]6"_var);`
	ND var  sumall() const;

	// Ditto allowing commas etc.
	//
	// `let v1 = "10,20,30"_var.sum(","); // 60
	//  // or
	//  let v2 = sum("10,20,30", ",");`
	ND var  sum(SV delimiter) const;

	// Binary ops (+, -, *, /) in parallel on multiple values
	//
	// `let v1 = "10]20]30"_var.mv("+","2]3]4"_var); // "12]23]34"_var`
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
	//  v1.r(2, "X"); // "f1^X^f3"_var`
	   IO   r(const int fieldno, in replacement) REF {this->r(fieldno, 0, 0, replacement); return THIS;}

	// Ditto for specific value in a specific field.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.r(2, 2, "X"); // "f1^v1]X^f3"_var`
	   IO   r(const int fieldno, const int valueno, in replacement) REF {this->r(fieldno, valueno, 0, replacement); return THIS;}

	// Ditto for a specific subvalue in a specific value of a specific field
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.r(2, 2, 2, "X"); // "f1^v1]v2}X}s3^f3"_var`
	   IO   r(const int fieldno, const int valueno, const int subvalueno, in replacement) REF;

	// Insert a specific field in a dynamic array, moving all other fields up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, "X"); // "f1^X^v1]v2}s2}s3^f3"_var
	//  // or
	//  inserter(v1, 2, "X");`
	   IO   inserter(const int fieldno, in insertion) REF {this->inserter(fieldno, 0, 0, insertion); return THIS;}

	// Ditto for a specific value in a specific field, moving all other values up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, "X"); // "f1^v1]X]v2}s2}s3^f3"_var
	//  // or
	//  inserter(v1, 2, 2, "X");`
	   IO   inserter(const int fieldno, const int valueno, in insertion) REF {this->inserter(fieldno, valueno, 0, insertion); return THIS;}

	// Ditto for a specific subvalue in a dynamic array, moving all other subvalues up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, 2, "X"); // "f1^v1]v2}X}s2}s3^f3"_var
	//  // or
	//  v1.inserter(2, 2, 2, "X");`
	   IO   inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF;

	// Remove a specific field (or value, or subvalue) from a dynamic array, moving all other fields (or values, or subvalues)  down.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.remover(2, 2); // "f1^v1^f3"_var
	//  // or
	//  remover(v1, 2, 2);`
	   IO   remover(const int fieldno, const int valueno = 0, const int subvalueno = 0) REF;

	//-er version could be extract and erase in one go
	// IO   extracter(int fieldno, int valueno=0, int subvalueno=0) const;

	///// DYNAMIC ARRAY SEARCH:
	//////////////////////////

	//  obj is strvar

	// LOCATE

	// locate() with only the target substr argument provided searches unordered values separated by any of the field mark chars.
	// Returns: The field, value, subvalue etc. number if found or 0 if not.
	// Searching for empty fields, values etc. (i.e. "") will work. Locating "" in "]yy" will return 1, in "xx]]zz" 2, and in "xx]yy]" 3, however, locating "" in "xx" will return 0 because there is conceptually no empty value in "xx". Locate "" in "" will return 1.
	//
	// `if ("UK^US^UA"_var.locate("US")) ... ok // 2
	//  // or
	//  if (locate("US", "UK^US^UA"_var)) ... ok`
	ND var locate(in target) const;

	// locate() with only the target substr provided and setting returned searches unordered values separated by any type of field mark chars.
	// Returns: True if found
	// Setting: Field, value, subvalue etc. number if found or the max number + 1 if not. Suitable for additiom of new values
	//
	// `var setting;
	//  if ("UK]US]UA"_var.locate("US", setting)) ... ok // setting -> 2
	//  // or
	//  if (locate("US", "UK]US]UA"_var, setting)) ... ok`
	ND bool locate(in target, out valueno) const;

	// locate() the target in unordered fields if fieldno is 0, or values if a fieldno is specified, or subvalues if the valueno argument is provided.
	// Returns: True if found and with the field, value or subvalue number in setting.
	// Returns: False if not found and with the max field, value or subvalue number found + 1 in setting. Suitable for replacement of new fields, values or subvalues.
	//
	// `var setting;
	//  if ("f1^f2v1]f2v2]s1}s2}s3}s4^f3^f4"_var.locate("s4", setting, 2, 3)) ... ok // setting -> 4 // returns true`
	ND bool locate(in target, out setting, const int fieldno, const int valueno = 0) const;

	// LOCATE BY

	// locateby() without fieldno or valueno arguments searches ordered values separated by VM chars.
	// The order code can be AL, DL, AR, DR meaning Ascending Left, Descending Right, Ascending Right, Ascending Left.
	// Left is used to indicate alphabetic order where 10 < 2.
	// Right is used to indicate numeric order where 10 > 2.
	// Data must be in the correct order for searching to work properly.
	// Returns: True if found.
	// In case the target is not exactly found then the correct value no for inserting the target is returned in setting.
	//
	// `var valueno; if ("aaa]bbb]ccc"_var.locateby("AL", "bb", valueno)) ... // valueno -> 2 // returns false and valueno = where it could be correctly inserted.`
	ND bool locateby(const char* ordercode, in target, out valueno) const;

	// locateby() ordered as above but in fields if fieldno is 0, or values in a specific fieldno, or subvalues in a specific valueno.
	//
	// `var setting;
	//  if ("f1^f2^aaa]bbb]ccc^f4"_var.locateby("AL", "bb", setting, 3)) ... // setting -> 2 // return false and where it could be correctly inserted.`
	ND bool locateby(const char* ordercode, in target, out setting, const int fieldno, const int valueno = 0) const;

	// LOCATE USING

	// locate() a target substr in the whole unordered string using a given delimiter char returning true if found.
	// `if ("AB,EF,CD"_var.locateusing(",", "EF")) ... ok`
	ND bool locateusing(const char* usingchar, in target) const;

	// locate() the target in a specific field, value or subvalue using a specified delimiter and unordered data
	// Returns: True If found and returns in setting the number of the delimited field found.
	// Returns: False if not found and returns in setting the maximum number of delimited fields + 1 if not found.
	// This is similar to the main locate command but the delimiter char can be specified e.g. a comma or TM etc.
	//
	// `var setting;
	//  if ("f1^f2^f3c1,f3c2,f3c3^f4"_var.locateusing(",", "f3c2", setting, 3)) ... ok // setting -> 2 // returns true`
	ND bool locateusing(const char* usingchar, in target, out setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// LOCATE BY, USING

	// locatebyusing() supports all the above features in a single function.
	// Returns: True if found.
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
	//  if (not conn.connect(conninfo)) ...;
	//  // or
	//  if (not connect()) ...
	//  // or
	//  if (not connect("exodus")) ...`
	ND bool connect(in conninfo = "");

	// Attach (connect) specific files by name to specific connections.
	// It is not necessary to attach files before opening them. Attach is meant to control the defaults.
	// For the remainder of the session, opening the db file by name without specifying a connection will automatically use the specified connection applies during the attach command.
	// If conn is not specified then filename will be attached to the default connection.
	// Multiple file names must be separated by FM
	//
	// `let filenames = "xo_clients^dict.xo_clients"_var, conn = "exodus";
	//  if (conn.attach(filenames)) ... ok
	//  // or
	//  if (attach(filenames)) ... ok`
	ND bool attach(in filenames) const;

	// Detach (disconnect) files that have been attached using attach().
	   void detach(in filenames);

	// Begin a db transaction.
	//
	// `if (not conn.begintrans()) ...
	//  // or
	//  if (not begintrans()) ...`
	ND bool begintrans() const;

	// Check if a db transaction is in progress.
	//
	// `if (conn.statustrans()) ... ok
	//  // or
	//  if (statustrans()) ... ok`
	ND bool statustrans() const;

	// Rollback a db transaction.
	//
	// `if (conn.rollbacktrans()) ... ok
	//  // or
	//  if (rollbacktrans()) ... ok`
	ND bool rollbacktrans() const;

	// Commit a db transaction.
	// Returns: True if successfully committed or if there was no transaction in progress, otherwise false.
	//
	// `if (conn.committrans()) ... ok
	//  // or
	//  if (committrans()) ... ok`
	ND bool committrans() const;

	// Execute an sql command.
	// Returns: True if there was no sql error otherwise lasterror() returns a detailed error message.
	//
	// `if (conn.sqlexec("select 1")) ... ok
	//  // or
	//  if (sqlexec("select 1")) ... ok`
	ND bool sqlexec(in sqlcmd) const;

	// Execute an SQL command and capture the response.
	// Returns: True if there was no sql error otherwise response contains a detailed error message.
	// response: Any rows and columns returned are separated by RM and FM respectively. The first row is the column names.
	// Recommended: Don't use sql directly unless you must to manage or configure a database.
	//
	// `let sqlcmd = "select 'xxx' as col1, 'yyy' as col2";
	//  var response;
	//  if (conn.sqlexec(sqlcmd, response)) ... ok // response -> "col1^col2\x1fxxx^yyy"_var /// \x1f is the Record Mark (RM) char. The backtick char is used here by gendoc to deliminate source code.
	//  // or
	//  if (sqlexec(sqlcmd, response)) ... ok`
	ND bool sqlexec(in sqlcmd, io response) const;

	// Closes db connection and frees process resources both locally and in the database server.
	//
	// `conn.disconnect();
	//  // or
	//  disconnect();`
	   void disconnect();

	// Closes all connections and frees process resources both locally and in the database server(s).
	// All connections are closed automatically when a process terminates.
	//
	// `conn.disconnectall();
	//  // or
	//  disconnectall();`
	   void disconnectall();

	// Returns: The last os or db error message.
	// `var v1 = var().lasterror();
	//  // or
	//  var v2 = lasterror();`
	ND var  lasterror() const;

	// Log the last os or db error message.
	// Output: to stdlog
	// Prefixes the output with source if provided.
	//
	// `var().loglasterror("main:");
	//  // or
	//  loglasterror("main:");`
	   var  loglasterror(in source = "") const;

	///// DATABASE MANAGEMENT:
	/////////////////////////

	// obj is conn

	// Create a named database on a particular connection.
	// The target database cannot already exist.
	// Optionally copies an existing database from the same connection and which cannot have any current connections.
	//
	// `var conn = "exodus";
    //  if (not dbdelete("xo_gendoc_testdb")) {}; // Cleanup first
	//  if (conn.dbcreate("xo_gendoc_testdb")) ... ok
	//  // or
	//  if (dbcreate("xo_gendoc_testdb")) ...`
	ND bool dbcreate(in new_dbname, in old_dbname = "") const;

	// Create a named database as a copy of an existing database.
	// The target database cannot already exist.
	// The source database must exist on the same connection and cannot have any current connections.
	//
	// `var conn = "exodus";
    //  if (not dbdelete("xo_gendoc_testdb2")) {}; // Cleanup first
	//  if (conn.dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) ... ok
	//  // or
	//  if (dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) ...`
	ND bool dbcopy(in from_dbname, in to_dbname) const;

	// Returns: A list of available databases on a particular connection.
	//
	// `let v1 = conn.dblist();
	//  // or
	//  let v2 = dblist();`
	ND var  dblist() const;

	// Delete (drop) a named database.
	// The target database must exist and cannot have any current connections.
	//
	// `var conn = "exodus";
	//  if (conn.dbdelete("xo_gendoc_testdb2")) ... ok
	//  // or
	//  if (dbdelete("xo_gendoc_testdb2")) ...`
	ND bool dbdelete(in dbname) const;

	// Create a named db file.
	//
	// `let filename = "xo_gendoc_temp", conn = "exodus";
	//  if (conn.createfile(filename)) ... ok
	//  // or
	//  if (createfile(filename)) ...`
	ND bool createfile(in filename) const;

	// Rename a db file.
	//
	// `let conn = "exodus", filename = "xo_gendoc_temp", new_filename = "xo_gendoc_temp2";
	//  if (conn.renamefile(filename, new_filename)) ... ok
	//  // or
	//  if (renamefile(filename, new_filename)) ...`
	ND bool renamefile(in filename, in newfilename) const;

	// Returns: A list of all files in a database
	//
	// `var conn = "exodus";
	//  if (not conn.listfiles()) ...
	//  // or
	//  if (not listfiles()) ...`
	ND var  listfiles() const;

	// Delete all records in a db file
	//
	// `let conn = "exodus", filename = "xo_gendoc_temp2";
	//  if (not conn.clearfile(filename)) ...
	//  // or
	//  if (not clearfile(filename)) ...`
	ND bool clearfile(in filename) const;

	// Delete a db file
	//
	// `let conn = "exodus", filename = "xo_gendoc_temp2";
	//  if (conn.deletefile(filename)) ... ok
	//  // or
	//  if (deletefile(filename)) ...`
	ND bool deletefile(in filename) const;

	// obj is conn_or_file

	// Returns: The approx. number of records in a db file
	//
	// `let conn = "exodus", filename = "xo_clients";
	//  var nrecs1 = conn.reccount(filename);
	//  // or
	//  var nrecs2 = reccount(filename);`
	ND var  reccount(in filename = "") const;

	// Calls db maintenance function (vacuum)
	// This doesnt actually flush any indexes but does make sure that reccount() function is reasonably accurate.
	   bool flushindex(in filename = "") const;

	///// DATABASE FILE I/O:
	///////////////////////

	// obj is file

	// Opens a db file to a var which can be used in subsequent functions to work on the specified file and database connection.
	//
	// `var file, filename = "xo_clients";
	//  if (not file.open(filename)) ...
	//  // or
	//  if (not open(filename to file)) ...`
	ND bool open(in dbfilename, in connection = "");

	// Closes db file var
	// Does nothing currently since database file vars consume no resources
	//
	// `var file = "xo_clients";
	//  file.close();
	//  // or
	//  close(file);`
	   void close() const;

	// Creates a secondary index for a given db file and field name.
	// The fieldname must exist in a dictionary file. The default dictionary is "dict." ^ filename.
	// Returns: False if the index cannot be created for any reason.
	// * Index already exists
	// * File does not exist
	// * The dictionary file does not have a record with a key of the given field name.
	// * The dictionary file does not exist. Default is "dict." ^ filename.
	// * The dictionary field defines a calculated field that uses an exodus function. Using a psql function is OK.
	//
	// `var filename = "xo_clients", fieldname = "DATE_CREATED";
	//  if (not deleteindex("xo_clients", "DATE_CREATED")) {}; // Cleanup first
	//  if (filename.createindex(fieldname)) ... ok
	//  // or
	//  if (createindex(filename, fieldname)) ...`
	ND bool createindex(in fieldname, in dictfile = "") const;

	// Lists secondary indexes in a database or for a db file
	// Returns: False if the db file or fieldname are given and do not exist
	// obj is file|conn
	//
	// `var conn = "exodus";
	//  if (conn.listindex()) ... ok // includes "xo_clients__date_created"
	//  // or
	//  if (listindex()) ... ok`
	ND var  listindex(in file_or_filename = "", in fieldname = "") const;

	// Deletes a secondary index for a db file and field name.
	// Returns: False if the index cannot be deleted for any reason
	// * File does not exist
	// * Index does not already exists
	//
	// `var file = "xo_clients", fieldname = "DATE_CREATED";
	//  if (file.deleteindex(fieldname)) ... ok
	//  // or
	//  if (deleteindex(file, fieldname)) ...`
	ND bool deleteindex(in fieldname) const;

	// Places a metaphorical db lock on a particular record given a db file and key.
	// This is a advisory lock, not a physical lock, since it makes no restriction on the access or modification of data by other connections.
	// Neither the db file nor the record key need to actually exist since a lock is just a hash of the db file name and key combined.
	// If another connection attempts to place an identical lock on the same database it will be denied.
	// Locks can be removed by unlock() or unlockall() or will be automatically removed at the end of a transaction or when the connection is closed.
	// If the same process attempts to place an identical lock more than once it may be denied (if not in a transaction) or succeed but be ignored (if in a transaction).
	// Locks can be used to avoid processing a transaction simultaneously with another connection only to have one of them fail due to mutually updating the same records.
	// Returns::
	// * 0: Failure: Another connection has already placed the same lock.
	// * "" Failure: The lock has already been placed.
	// * 1: Success: A new lock has been placed.
	// * 2: Success: The lock has already been placed and the connection is in a transaction.
	//
	// `var file = "xo_clients", key = "1000";
	//  if (file.lock(key)) ... ok
	//  // or
	//  if (lock(file, key)) ...`
	ND var  lock(in key) const;

	// Removes a db lock placed by the lock function.
	// Only locks placed on the specified connection can be removed.
	// Locks cannot be removed while a connection is in a transaction.
	// Returns: False if the lock is not present in a connection.
	//
	// `var file = "xo_clients", key = "1000";
	//  if (file.unlock(key)) ... ok
	//  // or
	//  if (unlock(file, key)) ...`
	   bool unlock(in key) const;

	// Removes all db locks placed by the lock function in the specified connection.
	// Locks cannot be removed while in a transaction.
	//
	// `var conn = "exodus";
	//  if (not conn.unlockall()) ...
	//  // or
	//  if (not unlockall(conn)) ...`
	   bool unlockall() const;

	// obj is rec

	// Writes a record into a db file given a unique primary key.
	// Either inserts a new record or updates an existing record.
	// It always succeeds so no result code is returned.
	// Any memory cached record is deleted.
	//
	// `let rec = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (not deleterecord("xo_clients", "GD001")) {}; // Cleanup first
	//  rec.write(file, key);
	//  // or
	//  write(rec on file, key);`
	   void write(in file, in key) const;

	// Reads a record from a db file for a given key.
	// Returns: False if the key doesnt exist
	// var: Contains the record if it exists or is unassigned if not.
	//
	// `var rec;
	//  let file = "xo_clients", key = "GD001";
	//  if (not rec.read(file, key)) ... // rec -> "Client GD^G^20855^30000^1001.00^20855.76539"_var
	//  // or
	//  if (not read(rec from file, key)) ...`
	ND bool read(in file, in key);

	// Deletes a record from a db file given a key.
	// Returns: False if the key doesnt exist
	// Any memory cached record is deleted.
	// obj is file
	//
	// `let file = "xo_clients", key = "GD001";
	//  if (file.deleterecord(key)) ... ok
	//  // or
	// if (deleterecord(file, key)) ...`
	   bool deleterecord(in key) const;

	// Inserts a new record in a db file.
	// Returns: False if the key already exists
	// Any memory cached record is deleted.
	//
	// `let rec = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (rec.insertrecord(file, key)) ... ok
	//  // or
	//  if (insertrecord(rec on file, key)) ...`
	ND bool insertrecord(in file, in key) const;

	// Updates an existing record in a db file.
	// Returns: False if the key doesnt already exist
	// Any memory cached record is deleted.
	//
	// `let rec = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (not rec.updaterecord(file, key)) ...
	//  // or
	//  if (not updaterecord(rec on file, key)) ...`
	ND bool updaterecord(in file, in key) const;

	//  obj is strvar

	// "Read field" Same as read() but only returns a specific field number from the record.
	//
	// `var field, file = "xo_clients", key = "GD001", fieldno = 2;
	//  if (not field.readf(file, key, fieldno)) ... // field -> "G"
	//  // or
	//  if (not readf(field from file, key, fieldno)) ...`
	ND bool readf(in file, in key, const int fieldno);

	// "write field" Same as write() but only writes to a specific field number in the record
	//
	// `var field = "f3", file = "xo_clients", key = "1000", fieldno = 3;
	//  field.writef(file, key, fieldno);
	//  // or
	//  writef(field on file, key, fieldno);`
	   void writef(in file, in key, const int fieldno) const;

	// obj is rec

	// "Write cache" Writes a record and key into a memory cached "db file".
	// The actual database file is NOT updated.
	// writec() either updates an existing cache record if the key already exists or otherwise inserts a new record into the cache.
	// It always succeeds so no result code is returned.
	// Neither the db file nor the record key need to actually exist in the actual db.
	//
    // `let rec = "Client XD^X^20855^30000^1001.00^20855.76539"_var;
    //  let file = "xo_clients", key = "XD001";
	//  rec.writec(file, key);
	//  // or
	//  writec(rec on file, key);`
	   void writec(in file, in key) const;

	// "Read cache" Same as "read() but first reads from a memory cache.
	// 1. Tries to read from a memory cache. Returns true if successful.
	// 2a. Tries to read from the actual db file and returns false if unsuccessful.
	// 2b. Writes the record and key to the memory cache and returns true.
	// Cached db file data lives in exodus process memory and is lost when the process terminates or cleardbcache() is called.
	//
    // `var rec;
    //  let file = "xo_clients", key = "XD001";
	//  if (rec.readc(file, key)) ... ok
	//  // or
	//  if (readc(rec from file, key)) ... ok
	//
	//  // Verify not in actual database file by using read() not readc()
	//  if (read(rec from file, key)) abort("Error: " ^ key ^ " should not be in the actual database file"); // error`
	ND bool readc(in file, in key);

	// obj is dbfile

	// Deletes a record and key from a memory cached "file".
	// The actual database file is NOT updated.
	// Returns: False if the key doesnt exist
	//
	// `var file = "xo_clients", key = "XD001";
	//  if (file.deletec(key)) ... ok
	//  // or
	//  if (deletec(file, key)) ...`
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
	// Arguments:
	// strvar: Used as the primary key to lookup a field in a given file and field no or field name.
	// filename: The db file in which to look up data.
	// If var key is multivalued then a multivalued field is returned.
	// fieldno: Determines which field of the record is returned.
	// * Integer returns that field number
	// * 0 means return the key unchanged.
	// * "" means return the whole record.
	// mode: Determines what is returned if the record does not exist for the given key and file.
	// * "X" returns ""
	// * "C" returns the key unconverted.
	//
	// `let key = "SB001";
	//  let client_name = key.xlate("xo_clients", 1, "X"); // "Client AAA"
	//  // or
	//  let name_and_type = xlate("xo_clients", key, "NAME_AND_TYPE", "X"); // "Client AAA (A)"`
	ND var  xlate(in filename, in fieldno, const char* mode) const;

	///// DATABASE SORT/SELECT:
	//////////////////////////

	// obj is dbfile

	// Create an active select list of keys of a database file.
	// The select(command) function searches and orders database records for subsequent processing given an English language-like command.
	// The primary job of a database, beyond mere storage and retrieval of information, is to allow rapid searching and ordering of information on demand.
	// In Exodus, searching and ordering of information is known as "sort/select" and is performed by the select() function.
	// Executing the select() function creates an "active select list" which can then be consumed by the readnext() function.
	// dbfile: A opened database file or file name, or an open connection or an empty var for default connections. Subsequent readnext calls must use the same.
	// sort_select_command: A natural language command using dictionary field names. The command can be blank if a dbfile or filename is given in dbfile or just a file name and all keys will be selected in undefined order.
	// Example: "select xo_clients with type 'B' and with balance ge 100 by type by name"
	// Option: "(R)" appended to the sort_select_command acquires the database records as well.
	// Returns: True if any records are selected or false if none.
	// Throws: VarDBException in case of any syntax error in the command.
	// Active select lists created using var.select()'s member function syntax cannot be consumed by the free function form of readnext() and vice versa.
	//
	// `var clients = "xo_clients";
	//  if (clients.select("with type 'B' and with balance ge 100 by type by name"))
	//      while (clients.readnext(ID))
	//          println("Client code is {}", ID);
	//  // or
	//  if (select("xo_clients with type 'B' and with balance ge 100 by type by name"))
	//      while (readnext(ID))
	//          println("Client code is {}", ID);`
	ND bool select(in sort_select_command = "");

	// Create an active select list from a string of keys.
	// Similar to select() but creates the list directly from a var.
	// keys: An FM separated list of keys or key^VM^valueno pairs.
	// Returns: True if any keys are provided or false if not.
	//
	// `var dbfile = "";
	//  let keys = "A01^B02^C03"_var;
	//  if (dbfile.selectkeys(keys)) ... ok
	//  assert(dbfile.readnext(ID) and ID == "A01");
	//  // or
	//  if (selectkeys(keys)) ... ok
	//  assert(readnext(ID) and ID == "A01");`
	ND bool selectkeys(in keys);

	// Checks if a select list is active.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// Returns: True if a select list is active and false if not.
	// If it returns true then a call to readnext() will return a database record key, otherwise not.
	//
	// `var clients = "xo_clients", key;
	//  if (clients.select()) {
	//      assert(clients.hasnext());
	//  }
	//  // or
	//  if (select("xo_clients")) {
	//      assert(hasnext());
	//  }`
	ND bool hasnext();

	// Acquires and consumes one key from an active select list of database record keys.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// key: Returns the first (next) key present in an active select list or "" if no select list is active.
	// Returns: True if a list is active and a key is available, false if not.
	// Each call to readnext consumes one key from the list.
	// Once all the keys in an active select list have been consumed by calls to readnext, the list becomes inactive.
	// See select() for example code.
	ND bool readnext(out key);

	// Similar to readnext(key) but multivalued.
	// If the active list was ordered by multivalued database fields then pairs of key and multivalue number will be available to the readnext function.
	ND bool readnext(out key, out valueno);

	// Similar to readnext(key) but acquires the database record as well.
	// record: Returns the next database record from the select list assuming that the select list was created with the (R) option otherwise "" if not.
	// key: Returns the next database record key in the select list.
	// valueno: The multivalue number if the select list was ordered on multivalued database record fields or 1 if not.
	//
	// `var clients = "xo_clients";
	//  if (clients.select("with type 'B' and with balance ge 100 by type by name (R)"))
	//      while (clients.readnext(RECORD, ID, MV))
	//          println("Code is {}, Name is {}", ID, RECORD.f(1));
	//  // or
	//  DICT = "dict.xo_clients";
	//  if (select("xo_clients with type 'B' and with balance ge 100 by type by name (R)"))
	//      while (readnext(RECORD, ID, MV))
	//          println("Code is {}, Name is {}", calculate("CODE"), calculate("NAME"));`
	ND bool readnext(out record, out key, out valueno);

	// Deactivates an active select list.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// Returns: Nothing
	// Has no effect if no select list is active for dbfile.
	//
	// `var clients = "xo_clients";
	//  clients.clearselect();
	//  if (not clients.hasnext()) ... ok
	//  // or
	//  clearselect();
	//  if (not hasnext()) ... ok`
	   void clearselect();

	// Stores an active select list for later retrieval.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// listname: A suitable name that will be required for later retrieval.
	// Returns: True if saved successfully or false if there was no active list to be saved.
	// Any existing list with the same name will be overwritten.
	// Only the remaining unconsumed part of the active select list is saved.
	// Saved lists are stand-alone and are not tied to specific database files although they usually hold keys related to specific files.
	// Saved lists can be created from one file and used to access another.
	// savelist() merely writes an FM separated string of keys as a record in the "lists" database file using the list name as the key of the record.
	// If a saved list is very long, additional blocks of keys for the same list may be stored with keys like listname*2, listname*3 etc.
	// Select lists saved in the lists database file may be created, deleted and listed like database records in any other database file.
	//
	// `var clients = "xo_clients";
	//  if (clients.select("with type 'B' by name")) {
	//  }
	//  // or
	//  if (select("xo_clients with type 'B' by name")) {
	//      if (savelist("mylist")) ... ok
	//  }`
	   bool savelist(SV listname);

	// Retrieve and reactivate a saved select list.
	// dbfile: A file or connection var to be used by subsequent readnext function calls.
	// listname: The name of an existing list in the "lists" database file, either created by savelist or manually.
	// Returns: True if the list was successfully retrieved and activated, or false if the list name doesnt exist.
	// Any currently active select list is replaced.
	// Retrieving a list does not delete it and a list can be retrieved more than once until specifically deleted.
	//
	// `var file = "";
	//  if (file.getlist("mylist")) {
	//      while (file.readnext(ID))
	//          println("Key is {}", ID);
	//  }
	//  // or
	//  if (getlist("mylist")) {
	//      while (readnext(ID))
	//          println("Key is {}", ID);
	//  }`
	ND bool getlist(SV listname);

	// Delete a saved select list.
	// dbfile: A file or connection to the desired database.
	// listname: The name of an existing list in the "lists" database file.
	// Returns: True if successful or false if the list name doesnt exist.
	//
	// `var conn = "";
	//  if (conn.deletelist("mylist")) ... ok
	//  // or
	//  if (deletelist("mylist")) ... ok`
	   bool deletelist(SV listname) const;

//	// Create a saved list from a string of keys.
//	// Any existing list with the same name will be overwritten.
//	// keys: An FM separated list of keys or key^VM^valueno pairs.
//	// Returns: True if successful or false if no keys were provided.
//	// If the listname is empty then selectkeys() is called instead. This is obsolete and deprecated behaviour.
//	//
//	// `var conn = ""; let keys = "A01^B02^C03"_var;
//	//  if (conn.makelist("mylist", keys)) ... ok
//	//  // or
//	//  if (makelist("mylist", keys)) ... ok`
//	ND bool makelist(SV listname, in keys);
//
//	// Create an active select list from a database file record.
//	//
//	// `var lists = "lists";
//	//  if (lists.formlist("mylist")) ... ok
//	//  while (lists.readnext(ID))
//	//      println("Key is {}", ID);
//	//  // or
//	//  if (formlist("lists", "mylist")) ... ok
//	//  while (readnext(ID))
//	//      println("Key is {}, MV is {}", ID, MV);`
//	ND bool formlist(in keys, const int fieldno = 0);

	///// OS TIME/DATE:
	//////////////////

	// obj is var()

	// Number of whole days since pick epoch 1967-12-31 00:00:00 UTC. Negative for dates before.
	// e.g. was 20821 from 2025-01-01 00:00:00 UTC for 24 hours
	//
	// `let today1 = var().date();
	//  // or
	//  let today2 = date();`
	ND var  date() const;

	// Number of whole seconds since last 00:00:00 (UTC).
	// e.g. 43200 if time is 12:00
	// Range 0 - 86399 since there are 24*60*60 (86400) seconds in a day.
	//
	// `let now1 = var().time();
	//  // or
	//  let now2 = time();`
	ND var  time() const;

	// Number of fractional seconds since last 00:00:00 (UTC).
	// A floating point with approx. nanosecond resolution depending on hardware.
	// e.g. 23343.704387955 approx. 06:29:03 UTC
	//
	// `let now1 = var().ostime();
	//  // or
	//  let now2 = ostime();`
	ND var  ostime() const;

	// Number of fractional days since pick epoch 1967-12-31 00:00:00 UTC. Negative for dates before.
	// A floating point with approx. nanosecond resolution depending on hardware.
	// e.g. Was 20821.99998842593 around 2025-01-01 23:59:59 UTC
	//
	// `let now1 = var().timestamp();
	//  // or
	//  let now2 = timestamp();`
	ND var  timestamp() const;

	// Construct a timestamp from a date and time
	//
	// `let idate = iconv("2025-01-01", "D"), itime = iconv("23:59:59", "MT");
	//  let ts1 = idate.timestamp(itime); // 20821.99998842593
	//  // or
	//  let ts2 = timestamp(idate, itime);`
	ND var  timestamp(in ostime) const;

	// Sleep/pause/wait for a number of milliseconds
	// Releases the processor if not needed for a period of time or a delay is required.
	//
	// `var().ossleep(100); // sleep for 100ms
	//  // or
	//  ossleep(100);`
	   void ossleep(const int milliseconds) const;

	// Sleep/pause/wait up to a given number of milliseconds or until any changes occur in an FM delimited list of directories and/or files.
	// Any terminal input (e.g. a key press) will also terminate the wait.
	// An FM array of event information is returned. See below.
	// Multiple events are returned in multivalues.
	// obj is file_dir_list
	//
	// `let v1 = ".^/etc/hosts"_var.oswait(100); /// e.g. "IN_CLOSE_WRITE^/etc^hosts^f"_var
	//  // or
	//  let v2 = oswait(".^/etc/hosts"_var, 100);`
	//
	// Returned array fields
	// 1. Event type codes
	// 2. dirpaths
	// 3. filenames
	// 4. d=dir, f=file
	// <pre>
	// Possible event type codes are as follows:
	// * IN_CLOSE_WRITE⋅- A file opened for writing was closed
	// * IN_ACCESS⋅⋅⋅⋅⋅⋅- Data was read from file
	// * IN_MODIFY⋅⋅⋅⋅⋅⋅- Data was written to file
	// * IN_ATTRIB⋅⋅⋅⋅⋅⋅- File attributes changed
	// * IN_CLOSE⋅⋅⋅⋅⋅⋅⋅- File was closed (read or write)
	// * IN_MOVED_FROM⋅⋅- File was moved away from watched directory
	// * IN_MOVED_TO⋅⋅⋅⋅- File was moved into watched directory
	// * IN_MOVE⋅⋅⋅⋅⋅⋅⋅⋅- File was moved (in or out of directory)
	// * IN_CREATE⋅⋅⋅⋅⋅⋅- A file was created in the directory
	// * IN_DELETE⋅⋅⋅⋅⋅⋅- A file was deleted from the directory
	// * IN_DELETE_SELF⋅- Directory or file under observation was deleted
	// * IN_MOVE_SELF⋅⋅⋅- Directory or file under observation was moved
	// </pre>
	   var  oswait(const int milliseconds) const;

	///// OS FILE I/O:
	/////////////////

	// obj is osfilevar

	// Initialises an os file handle var that can be used in subsequent random access osbread and osbwrite functions.
	// osfilename: The name of an existing os file name including path.
	// utf8: Defaults to true which causes trimming of partial utf-8 Unicode byte sequences from the end of osbreads. For raw untrimmed osbreads pass tf8 = false;
	// Returns: True if successful or false if not possible for any reason. e.g. Target doesnt exist, permissions etc.
	// The file will be opened for writing if possible otherwise for reading.
	//
	// `let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  if (oswrite("" on osfilename)) ... ok /// Create an empty os file
	//  var ostempfile;
	//  if (ostempfile.osopen(osfilename)) ... ok
	//  // or
	//  if (osopen(osfilename to ostempfile)) ... ok`
	ND bool osopen(in osfilename, const bool utf8 = true) const;

	// Writes data to an existing os file starting at a given byte offset (0 based).
	// See osbread for more info.
	//
	// `let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  let text = "aaa=123\nbbb=456\n";
	//  var offset = osfile(osfilename).f(1); /// Size of file therefore append
	//  if (text.osbwrite(osfilename, offset)) ... ok // offset -> 16
	//  // or
	//  if (not osbwrite(text on osfilename, offset)) ...`
	ND bool osbwrite(in osfilevar, io offset) const;

	// Reads length bytes from an existing os file starting at a given byte offset (0 based).
	// The osfilevar file handle may either be initialised by osopen or be just be a normal string variable holding the path and name of the os file.
	// After reading, the offset is updated to point to the correct offset for a subsequent sequential read.
	// If reading UTF8 data (the default) then the length of data actually returned may be a few bytes shorter than requested in order to be a complete number of UTF-8 code points.
	//
	// `let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  var text, offset = 0;
	//  if (text.osbread(osfilename, offset, 8)) ... ok // text -> "aaa=123\n" // offset -> 8
	//  // or
	//  if (osbread(text from osfilename, offset, 8)) ... ok // text -> "bbb=456\n" // offset -> 16`
	ND bool osbread(in osfilevar, io offset, const int length);

	// Removes an osfilevar handle from the internal memory cache of os file handles. This frees up both exodus process memory and operating system resources.
	// It is advisable to osclose any file handles after use, regardless of whether they were specifically opened using osopen or not, especially in long running programs. Exodus performs caching of internal os file handles per thread and os file. If not closed, then the operating system will probably not flush deleted files from storage until the process is terminated. This can potentially create an memory issue or file system resource issue especially if osopening/osreading/oswriting many perhaps temporary files in a long running process.
	//
	// `osfilevar.osclose();
	//  // or
	//  osclose(osfilevar);`
	   void osclose() const;

	//  obj is strvar

	// Create a complete os file from a var.
	// Any existing os file is removed first.
	// Returns: True if successful or false if not possible for any reason.
	// e.g. Path is not writeable, permissions etc.
	// If codepage is specified then output is converted from utf-8 to that codepage. Otherwise no conversion is done.
	//
	// `let text = "aaa = 123\nbbb = 456";
	//  let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  if (text.oswrite(osfilename)) ... ok
	//  // or
	//  if (oswrite(text on osfilename)) ... ok`
	ND bool oswrite(in osfilename, const char* codepage = "") const;

	// Read a complete os file into a var.
	// If codepage is specified then input is converted from that codepage to utf-8 otherwise no conversion is done.
	// Returns: True if successful or false if not possible for any reason.
	// e.g. File doesnt exist, permissions etc.
	//
	// `var text;
	//  let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  if (text.osread(osfilename)) ... ok // text -> "aaa = 123\nbbb = 456"
	//  // or
	//  if (osread(text from osfilename)) ... ok`
	ND bool osread(const char* osfilename, const char* codepage = "");

	// TODO check if it calls osclose on itself in case removing a varfile

	// obj is osfile_or_dirname

	// Renames an os file or dir in the OS file system.
	// Will not overwrite an existing os file or dir.
	// Source and target must exist in the same storage device.
	// Returns: True if successful or false if not possible for any reason.
	// e.g. Target already exists, path is not writeable, permissions etc.
	// Uses std::filesystem::rename internally.
	//
	// `let from_osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  let to_osfilename = from_osfilename ^ ".bak";
	//  if (not osremove(ostempdirpath() ^ "xo_gendoc_test.conf.bak")) {}; // Cleanup first
	//
	//  if (from_osfilename.osrename(to_osfilename)) ... ok
	//  // or
	//  if (osrename(from_osfilename, to_osfilename)) ...`
	ND bool osrename(in new_dirpath_or_filepath) const;

	// "Moves" an os file or dir within the os file system.
	// Will not overwrite an existing os file or dir.
	// Returns: True if successful or false if not possible for any reason.
	// e.g. Source doesnt exist or cannot be accessed, target already exists, source or target is not writeable, permissions etc.
	// Attempts osrename first then oscopy followed by osremove original.
	//
	// `let from_osfilename = ostempdirpath() ^ "xo_gendoc_test.conf.bak";
	//  let to_osfilename = from_osfilename.cut(-4);
	//
	//  if (not osremove(ostempdirpath() ^ "xo_gendoc_test.conf")) {}; // Cleanup first
	//  if (from_osfilename.osmove(to_osfilename)) ... ok
	//  // or
	//  if (osmove(from_osfilename, to_osfilename)) ...`
	ND bool osmove(in to_osfilename) const;

	// Copies an os file or directory recursively within the os file system.
	// Will overwrite an existing os file or dir.
	// Uses std::filesystem::copy internally with recursive and overwrite options
	//
	// `let from_osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  let to_osfilename = from_osfilename ^ ".bak";
	//
	//  if (from_osfilename.oscopy(to_osfilename)) ... ok;
	//  // or
	//  if (oscopy(from_osfilename, to_osfilename)) ... ok`
	ND bool oscopy(in to_osfilename) const;

	// obj is osfilename

	// Removes/deletes an os file from the OS file system given path and name.
	// Will not remove directories. Use osrmdir() to remove directories
	// Returns: True if successful or false if not possible for any reason.
	// e.g. Target doesnt exist, path is not writeable, permissions etc.
	//
	// `let osfilename = ostempdirpath() ^ "xo_gendoc_test.conf";
	//  if (osfilename.osremove()) ... ok
	//  // or
	//  if (osremove(osfilename)) ...`
	ND bool osremove() const;

	///// OS DIRECTORIES:
	////////////////////

	// List files and directories

	// obj is dirpath

	// Returns: A FM delimited string containing all matching dir entries given a dir path
	// A glob pattern (e.g. *.conf) can be appended to the path or passed as argument.
	//
	// `var entries1 = "/etc/"_var.oslist("*.cfg"); /// e.g. "adduser.conf^ca-certificates.con^... etc."
	//  // or
	//  var entries2 = oslist("/etc/" "*.conf");`
	ND var  oslist(SV globpattern = "", const int mode = 0) const;

	// Same as oslist for files only
	ND var  oslistf(SV globpattern = "") const;

	// Same as oslist for files only
	ND var  oslistd(SV globpattern = "") const;

	// Returns: Dir info for any dir entry or "" if it doesnt exist
	// A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time
	// mode 0 default
	// mode 1 returns "" if not an os file
	// mode 2 returns "" if not an os dir
	// See also osfile() and osdir()
	//
	// `var info1 = "/etc/hosts"_var.osinfo(); /// e.g. "221^20597^78309"_var
	//  // or
	//  var info2 = osinfo("/etc/hosts");`
	// obj is osfile_or_dirpath
	ND var  osinfo(const int mode = 0) const;

	// Returns: Dir info for a os file
	// A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time
	// Alias for osinfo(1)
	//
	// `var fileinfo1 = "/etc/hosts"_var.osfile(); /// e.g. "221^20597^78309"_var
	//  // or
	//  var fileinfo2 = osfile("/etc/hosts");`
	// obj is osfilename
	ND var  osfile() const;

	// Returns: Dir info for a dir.
	// A short string containing FM ^ modified_time ^ FM ^ modified_time
	// Alias for osinfo(2)
	//
	// `var dirinfo1 = "/etc/"_var.osdir(); /// e.g. "^20848^44464"_var
	//  // or
	//  var dirinfo2 = osfile("/etc/");`
	ND var  osdir() const;

	// Makes a new directory and returns true if successful.
	// Including parent dirs if necessary.
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osrmdir("xo_test/aaa")) {}; // Cleanup first
	//  if (osdirname.osmkdir()) ... ok
	//  // or
	//  if (osmkdir(osdirname)) ...`
	ND bool osmkdir() const;

	// Changes the current working dir and returns true if successful.
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osdirname.oscwd()) ... ok
	//  // or
	//  if (oscwd(osdirname)) ... ok`
	ND bool oscwd(in newpath) const;

	// Returns: The current working directory
	// e.g. "/root/exodus/cli/src/xo_test/aaa"
	//
	// `var cwd1 = var().oscwd();
	//  // or
	//  var cwd2 = oscwd();`
	ND var  oscwd() const;

	// Removes a os dir and returns true if successful.
	// Optionally even if not empty. Including subdirs.
	//
	// `let osdirname = "xo_test/aaa";
	//  if (oscwd("../..")) ... ok /// Change up before removing because cannot remove dir while it is current
	//  if (osdirname.osrmdir()) ... ok
	//  // or
	//  if (osrmdir(osdirname)) ...`
	ND bool osrmdir(bool evenifnotempty = false) const;

	///// OS SHELL/ENVIRONMENT:
	//////////////////////////

	// Execute a shell command.
	// Returns: True if the process terminates with error status 0 and false otherwise.
	// Append "&>/dev/null" to the command to suppress terminal output.
	// obj is command
	//
	// `let cmd = "echo $HOME";
	//  if (cmd.osshell()) ... ok
	//  // or
	//  if (osshell(cmd)) ... ok`
	ND bool osshell() const;

	// Same as osshell but captures and returns stdout
	// Returns: The stout of the shell command.
	// Append "2>&1" to the command to capture stderr/stdlog output as well.
	// obj is instr
	//
	// `let cmd = "echo $HOME";
	//  var text;
	//  if (text.osshellread(cmd)) ... ok
	//
	//  // or capturing stdout but ignoring exit status
	//  text = osshellread(cmd);`
	ND bool osshellread(in oscmd);

	// Same as osshell but provides stdin to the process
	// Returns: True if the process terminates with error status 0 and false otherwise.
	// Append "&> somefile" to the command to suppress and/or capture output.
	// obj is outstr
	//
	// `let outtext = "abc xyz";
	//  if (outtext.osshellwrite("grep xyz")) ... ok
	//  // or
	//  if (osshellwrite(outtext, "grep xyz")) ... ok`
	ND bool osshellwrite(in oscmd) const;

	// Returns: The path of the tmp dir
	// e.g. "/tmp/"
	// obj is var()
	//
	// `let v1 = var().ostempdirpath();
	//  // or
	//  let v2 = ostempdirpath();`
	ND var  ostempdirpath() const;

	// Returns: The name of a new temporary file
	// e.g. Something like "/tmp/~exoEcLj3C"
	// obj is var()
	//
	// `var temposfilename1 = var().ostempfilename();
	//  // or
	//  var temposfilename2 = ostempfilename();`
	ND var  ostempfilename() const;

	// obj is envvalue

	// Set the value of an environment variable code
	// obj is envvalue
	//
	// `let envcode = "EXO_ABC", envvalue = "XYZ";
	//  envvalue.ossetenv(envcode);
	//  // or
	//  ossetenv(envcode, envvalue);`
	   void ossetenv(SV envcode) const;

	// Get the value of an environment variable.
	// envcode: The code of the desired environment variable or "" for all.
	// Returns: True if set or false if not.
	// var: If envcode exists: var is set to the value of the environment variable
	// var: If envcode doesnt exist: var is set to ""
	// var: If envcode is "": var is set to an dynamic array of all environment variables LIKE CODE1=VALUE1^CODE2=VALUE2...
	// osgetenv and ossetenv work with a per thread copy of the process environment. This avoids multithreading issues but not actually changing the process environment.
	// For the actual system environment, see "man environ". extern char **environ; // environ is a pointer to an array of pointers to char* env pairs like xxx=yyy and the last pointer in the array is nullptr.
	//
	// `var envvalue1;
	//  if (envvalue1.osgetenv("HOME")) ... ok // e.g. "/home/exodus"
	//  // or
	//  var envvalue2 = osgetenv("EXO_ABC"); // "XYZ"`
	ND bool osgetenv(SV envcode);

	// TODO check for threadsafe

	// obj is var()

	// Get the os process id
	//
	// `let pid1 = var().ospid(); /// e.g. 663237
	//  // or
	//  let pid2 = ospid();`
	ND var  ospid() const;

	// Get the os thread process id
	//
	// `let tid1 = var().ostid(); /// e.g. 663237
	//  // or
	//  let tid2 = ostid();`
	ND var  ostid() const;

	// Get the libexodus build date and time
	//
	// `let v1 = var().version(); /// e.g. "29 JAN 2025 14:56:52"`
	ND var  version() const;

	// obj is var

	// Sets the current thread's default locale codepage code
	// True if successful
	// obj is strvar
	//
	// `if ("en_US.utf8"_var.setxlocale()) ... ok
	//  // or
	//  if (setxlocale("en_US.utf8")) ... ok`
	   bool setxlocale() const;

	// Returns: The current thread's default locale codepage code
	//
	// `let v1 = var().getxlocale(); // "en_US.utf8"
	//  // or
	//  let v2 = getxlocale();`
	ND var  getxlocale();

	///// OUTPUT:
	////////////

	// obj is varstr

	// Output to stdout with optional prefix.
	// Appends an NL char.
	// Is FLUSHED, not buffered.
	// The raw string bytes are output. No character or byte conversion is performed.
	//
	// `"abc"_var.outputl("xyz = "); /// Sends "xyz = abc\n" to stdout and flushes.
	//  // or
	//  outputl("xyz = ", "abc"); /// Any number of arguments is allowed. All will be output.`
	   CVR outputl(in prefix = "") const;
	   CVR output(in prefix = "") const;  // Same as outputl() but doesnt append an NL char and is BUFFERED, not flushed.
	   CVR outputt(in prefix = "") const; // Same as outputl() but appends a tab char instead of an NL char and is BUFFERED, not flushed.

	// Output to stdlog with optional prefix.
	// Appends an NL char.
	// Is BUFFERED not flushed.
	// Any of the six types of field mark chars present are converted to their visible versions,
	//
	// `"abc"_var.logputl("xyz = "); /// Sends "xyz = abc\n" to stdlog buffer and is not flushed.
	//  // or
	//  logputl("xyz = ", "abc");; /// Any number of arguments is allowed. All will be output.`
	   CVR logputl(in prefix = "") const;
	   CVR logput(in prefix = "") const; // Same as logputl() but doesnt append an NL char.

	// Output to stderr with optional prefix.
	// Appends an NL char.
	// Is FLUSHED not buffered.
	// Any of the six types of field mark chars present are converted to their visible versions,
	//
	// `"abc"_var.errputl("xyz = "); /// Sends "xyz = abc\n" to stderr
	//  // or
	//  errputl("xyz = ", "abc"); /// Any number of arguments is allowed. All will be output.`
	   CVR errputl(in prefix = "") const;
	   CVR errput(in prefix = "") const; // Same as errputl() but doesnt append an NL char and is BUFFERED not flushed.

	// Output to a given stream.
	// Is BUFFERED not flushed.
	// The raw string bytes are output. No character or byte conversion is performed.
	   CVR put(std::ostream& ostream1) const;

	// Flush any and all buffered output to stdout and stdlog.
	// obj is var()
	//
	// `var().osflush();
	//  // or
	//  osflush();`
	   void osflush() const;

	///// INPUT:
	///////////

	// obj is var

	// Returns one line of input from stdin.
	// Returns: True if successful or false if EOF or user pressed Esc or Ctrl+X in a terminal.
	// var: [in] The default value for terminal input and editing. Ignored if not a terminal.
	// var: [out] Raw bytes up to but excluding the first new line char. In case of EOF or user pressed Esc or Ctrl+X in a terminal it will be changed to "".
	// Prompt: If provided, it will be displayed on the terminal.
	// Multibyte/UTF8 friendly.
	//
	// `// var v1 = "defaultvalue";
	//  // if (v1.input("Prompt:")) ... ok
	//  // or
	//  // var v2 = input();`
	ND bool input(in prompt = "");

    // Get raw bytes from standard input.
	// Any new line chars are treated like any other bytes.
	// Care must be taken to handle incomplete UTF8 byte sequences at the end of one block and the beginning of the next block.
	// Returns: The requested number of bytes or fewer if not available.
	// nchars:
	// 99 : Get up to 99 bytes or fewer if not available. Caution required with UTF8.
	// ⋅0 : Get all bytes presently available.
	// ⋅1 : Same as keypressed(true). Deprecated.
	// -1 : Same as keypressed(). Deprecated.
	   out  inputn(const int nchars);

	// Return the code of the current terminal key pressed.
	// wait: Defaults to false. True means wait for a key to be pressed if not already pressed.
	// Returns: ASCII or key code defined according to terminal protocol.
	// Returns: "" if stdin is not a terminal.
	// e.g. The PgDn key if pressed might return an escape sequence like "\x1b[6~"
	// It only takes a few µsecs to return false if no key is pressed.
	// `var v1; v1.keypressed();
	//  // or
	//  var v2 = keypressed();`
	   out  keypressed(const bool wait = false);

	// obj is var()

	// Checks if one of stdin, stdout, stderr is a terminal or a file/pipe.
	// arg: 0 - stdin, 1 - stdout (Default), 2 - stderr.
	// Returns: True if it is a terminal or false if it is a file or pipe.
	// Note that if the process is at the start or end of a pipeline, then only stdin or stdout will be a terminal.
	// The type of stdout terminal can be obtained from the TERM environment variable.
	// `var v1 = var().isterminal(); /// 1 or 0
	//  // or
	//  var v2 = isterminal();`
	ND bool isterminal(const int arg = 1) const;

	// Checks if stdin has any bytes available for input.
	// If no bytes are immediately available, the process sleeps for up to the given number of milliseconds, returning true immediately any bytes become available or false if the period expires without any bytes becoming available.
	// Returns: True if any bytes are available otherwise false.
	// It only takes a few µsecs to return false if no bytes are available and no wait time has been requested.
	ND bool hasinput(const int milliseconds = 0) const;

	// True if stdin is at end of file
	ND bool eof() const;

	// Sets terminal echo on or off.
	// "On" causes all stdin data to be reflected to stdout if stdin is a terminal.
	// Turning terminal echo off can be used to prevent display of confidential information.
	// Returns: True if successful.
	   bool echo(const bool on_off = true) const;

	// Install various interrupt handlers.
	// Automatically called in program/thread initialisation by exodus_main.
	// SIGINT - Ctrl+C -> "Interrupted. (C)ontinue (Q)uit (B)acktrace (D)ebug (A)bort ?"
	// SIGHUP - Sets a static variable "RELOAD_req" which may be handled or ignored by the program.
	// SIGTERM - Sets a static variable "TERMINATE_req" which may be handled or ignored by the program.
	   void breakon() const;

	// Disable keyboard interrupt.
	// Ctrl+C becomes inactive in terminal.
	   void breakoff() const;

	///// MATH/BOOLEAN:
	//////////////////

	// obj is varnum

    // Absolute value
    // `let v1 = -12.34;
	//  let v2 = v1.abs(); // 12.34
    //  // or
    //  let v3 = abs(v1);`
    ND var  abs() const;

    // Power
    // `let v1 = var(2).pwr(8); // 256
    //  // or
    //  let v2 = pwr(2, 8);`
    ND var  pwr(in exponent) const;

    // Initialise the seed for rnd()
	// Allows the stream of pseudo random numbers generated by rnd() to be reproduced.
	// Seeded from std::chrono::high_resolution_clock::now() if the argument is 0;
	//
    // `var(123).initrnd(); /// Set seed to 123
    //  // or
    //  initrnd(123);`
           void initrnd() const;

    // Pseudo random number generator
	// Returns: a pseudo random integer between 0 and the provided maximum minus 1.
	// Uses std::mt19937 and std::uniform_int_distribution<int>
	//
    // `let v1 = var(100).rnd(); /// Random 0 to 99
    //  // or
    //  let v2 = rnd(100);`
    ND var  rnd()     const;

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
	// Returns: Floating point ver (double)
    // `let v1 = var(2.718281828459045).loge(); // 1
    //  // or
    //  let v2 = loge(2.718281828459045);`
    ND var  loge()    const;

    //  ND var  int() const;//reserved word

    // Truncate decimal numbers towards zero
	// Returns: An integer var
    // `let v1 = var(2.9).integer(); // 2
    //  // or
    //  let v2 = integer(2.9);
	//
    //  var v3 = var(-2.9).integer(); // -2
    //  // or
    //  var v4 = integer(-2.9);`
    ND var  integer() const;

    // Truncate decimal numbers towards negative
	// Returns: An integer var
    // `let v1 = var(2.9).floor(); // 2
    //  // or
    //  let v2 = floor(2.9);
	//
    //  var v3 = var(-2.9).floor(); // -3
    //  // or
    //  var v4 = floor(-2.9);`
    ND var  floor() const;

	/* Actually implemented in var_base but documented here
	// Modulus function
	// Identical to C++ % operator only for positive numbers and modulus
	// Negative denominators are considered as periodic with positiive numbers
	// Result is between [0, modulus) if modulus is positive
	// Result is between (modulus, 0] if modulus is negative (symmetric)
	// Throws: VarDivideByZero if modulus is zero.
	// Floating point works.
	// `let v1 = var(11).mod(5); // 1
	//  // or
	//  let v2 = mod(11, 5); // 1
	//  let v3 = mod(-11, 5); // 4
	//  let v4 = mod(11, -5); // -4
	//  let v5 = mod(-11, -5); // -1`
	ND var  mod(in modulus) const;

	// Not documenting the overloaded versions
	//ND var  mod(double modulus) const;
	//ND var  mod(const int modulus) const;
    */

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

	// Internal primitive oconvs

	// MR: Character replacement // WHY is this replacer, io, non-const
	ND io   oconv_MR(const char* conversion);

	/// I/O Conversion Codes :

	// Date output: Convert internal date format to human readable date or calendar info in text format.
	// Returns: Human readable date or calendar info, or the original value unconverted if non-numeric.
	// Flags: See examples below.
	// Any multifield/multivalue structure is preserved.
	// obj is vardate
	//
	// `let v1 = 12345;
	//  assert( v1.oconv( "D"   ) == "18 OCT 2001"  ); // Default
	//  assert( v1.oconv( "D/"  ) == "10/18/2001"   ); // / separator
	//  assert( v1.oconv( "D-"  ) == "10-18-2001"   ); // - separator
	//  assert( v1.oconv( "D2"  ) == "18 OCT 01"    ); // 2 digit year
	//  assert( v1.oconv( "D/E" ) == "18/10/2001"   ); // International order with /
	//  assert( v1.oconv( "DS"  ) == "2001 OCT 18"  ); // ISO Year first
	//  assert( v1.oconv( "DS-" ) == "2001-10-18"   ); // ISO Year first with -
	//  assert( v1.oconv( "DM"  ) == "10"           ); // Month number
	//  assert( v1.oconv( "DMA" ) == "OCTOBER"      ); // Month name
	//  assert( v1.oconv( "DY"  ) == "2001"         ); // Year number
	//  assert( v1.oconv( "DY2" ) == "01"           ); // Year 2 digits
	//  assert( v1.oconv( "DD"  ) == "18"           ); // Day number in month (1-31)
	//  assert( v1.oconv( "DW"  ) == "4"            ); // Weekday number (1-7)
	//  assert( v1.oconv( "DWA" ) == "THURSDAY"     ); // Weekday name
	//  assert( v1.oconv( "DQ"  ) == "4"            ); // Quarter number
	//  assert( v1.oconv( "DJ"  ) == "291"          ); // Day number in year
	//  assert( v1.oconv( "DL"  ) == "31"           ); // Last day number of month (28-31)
	//
	//  // Multifield/multivalue
	//  var v2 = "12345^12346]12347"_var;
	//  assert(v2.oconv("D") == "18 OCT 2001^19 OCT 2001]20 OCT 2001"_var);
	//
	//   // or
	//   assert( oconv(v1, "D"   ) == "18 OCT 2001"  );`
	ND std::string oconv_D(const char* conversion) const;

	// Date input: Convert human readable date to internal date format.
	// Returns: Internal date or "" if the input is an invalid date.
	// Internal date format is whole days since 1967-12-31 00:00:00 which is day 0.
	// Any multifield/multivalue structure is preserved.
	// obj is varstr
	//
	// `// International order "DE"
	//  assert(            oconv(19005, "DE") == "12 JAN 2020");
	//  assert(   "12/1/2020"_var.iconv("DE") == 19005);
	//  assert(   "12 1 2020"_var.iconv("DE") == 19005);
	//  assert(   "12-1-2020"_var.iconv("DE") == 19005);
	//  assert( "12 JAN 2020"_var.iconv("DE") == 19005);
	//  assert( "jan 12 2020"_var.iconv("DE") == 19005);
	//
	//  // American order "D"
	//  assert(            oconv(19329, "D") == "01 DEC 2020");
	//  assert(   "12/1/2020"_var.iconv("D") == 19329);
	//  assert(  "DEC 1 2020"_var.iconv("D") == 19329);
	//  assert(  "1 dec 2020"_var.iconv("D") == 19329);
	//
	//  // Reverse order
	//  assert(  "2020/12/1"_var.iconv("DE") == 19329);
	//  assert(   "2020-12-1"_var.iconv("D") == 19329);
	//  assert(  "2020 1 dec"_var.iconv("D") == 19329);
	//
	//  //Invalid date
	//  assert(   "2/29/2021"_var.iconv("D") == "");
	//  assert(  "29/2/2021"_var.iconv("DE") == "");
	//
	//  // or
	//  assert(iconv("12/1/2020"_var, "DE") == 19005);`
	ND var  iconv_D(const char* conversion) const;

	// Time output: Convert internal time format to human readable time e.g. "10:30:59".
	// Returns: Human readable time or the original value unconverted if non-numeric.
	// Conversion code (e.g. "MTHS") is "MT" + flags ...
	// Flags:
	// "H" - Show AM/PM otherwise 24 hour clock is used.
	// "S" - Output seconds
	// "2" = Ignored (used in iconv)
	// ":" - Any other flag is used as the separator char instead of ":"
	// Any multifield/multivalue structure is preserved.
	// obj is vartime
	//
	// `var v1 = 234800;
	//  assert( v1.oconv( "MT"   ) == "17:13"      ); // Default
	//  assert( v1.oconv( "MTH"  ) == "05:13PM"    ); // 'H' flag for AM/PM
	//  assert( v1.oconv( "MTS"  ) == "17:13:20"   ); // 'S' flag for seconds
	//  assert( v1.oconv( "MTHS" ) == "05:13:20PM" ); // Both flags
	//
	//  var v2 = 0;
	//  assert( v2.oconv( "MT"   ) == "00:00"      );
	//  assert( v2.oconv( "MTH"  ) == "12:00AM"    );
	//  assert( v2.oconv( "MTS"  ) == "00:00:00"   );
	//  assert( v2.oconv( "MTHS" ) == "12:00:00AM" );
	//
	//  // Multifield/multivalue
	//  var v3 = "234800^234860]234920"_var;
	//  assert(v3.oconv("MT") == "17:13^17:14]17:15"_var);
	//
	//  // or
	//  assert( oconv(v1, "MT"   ) == "17:13"      );`
	ND std::string oconv_MT(const char* conversion) const;

	// Time input: Convert human readable time (e.g. "10:30:59") to internal time format.
	// Returns: Internal time or "" if the input is an invalid time.
	// Internal time format is whole seconds since midnight.
	// Accepts: Two or three groups of digits surrounded and separated by any non-digits char(s).
	// Any multifield/multivalue structure is preserved.
	// obj is varstr
	//
	// `assert(      "17:13"_var.iconv( "MT" ) == 61980);
	//  assert(    "05:13PM"_var.iconv( "MT" ) == 61980);
	//  assert(   "17:13:20"_var.iconv( "MT" ) == 62000);
	//  assert( "05:13:20PM"_var.iconv( "MT" ) == 62000);
	//
	//  assert(      "00:00"_var.iconv( "MT" ) == 0);
	//  assert(    "12:00AM"_var.iconv( "MT" ) == 0);     // Midnight
	//  assert(    "12:00PM"_var.iconv( "MT" ) == 43200); // Noon
	//  assert(   "00:00:00"_var.iconv( "MT" ) == 0);
	//  assert( "12:00:00AM"_var.iconv( "MT" ) == 0);
	//
	//  // Multifield/multivalue
	//  assert("17:13^05:13PM]17:13:20"_var.iconv("MT") == "61980^61980]62000"_var);
	//
	//  // or
	//  assert(iconv("17:13", "MT") == 61980);`
	ND var  iconv_MT(bool strict) const;

	// Number output: Convert internal numbers to external text format after rounding and optional scaling.
	// Returns: A string or, if the value is not numeric, then no conversion is performed and the original value is returned.
	// Conversion code (e.g. "MD20") is "MD" or "MC", 1st digit, 2nd digit, flags ...
	//
	// MD outputs like 123.45 (International)
	// MC outputs like 123,45 (European)
	//
	// 1st digit = Decimal places to display. Also decimal places to move if 2nd digit not present and no P flag present.
	// 2nd digit = Optional decimal places to move left if P flag not present.
	//
	// Flags:
	// "P" - Preserve decimal places. Same as 2nd digit = 0;
	// "Z" - Zero flag - return "" if zero.
	// "X" - No conversion - return as is.
	// "." or "," - Separate thousands depending on MD or MC.
	// "-" means suffix negatives with "-" and positives with " " (space).
	// "<" means wrap negatives in "<" and ">" chars.
	// "C" means suffix negatives with "CR" and positives or zero with "DB".
	// "D" means suffix negatives with "DB" and positives or zero with "CR".
	//
	//  Any multifield/multivalue structure is preserved.
	// obj is varnum
	//
	// `var v1 = -1234.567;
	//  assert( v1.oconv( "MD20"   ) ==  "-1234.57"   );
	//  assert( v1.oconv( "MD20,"  ) == "-1,234.57"   ); // , flag
	//  assert( v1.oconv( "MC20,"  ) == "-1.234,57"   ); // MC code
	//  assert( v1.oconv( "MD20,-" ) ==  "1,234.57-"  ); // - flag
	//  assert( v1.oconv( "MD20,<" ) == "<1,234.57>"  ); // < flag
	//  assert( v1.oconv( "MD20,C" ) ==  "1,234.57CR" ); // C flag
	//  assert( v1.oconv( "MD20,D" ) ==  "1,234.57DB" ); // D flag
	//
	//  // Multifield/multivalue
	//  var v2 = "1.1^2.1]2.2"_var;
	//  assert( v2.oconv( "MD20"   ) == "1.10^2.10]2.20"_var);
	//
	//  // or
	//  assert( oconv(v1, "MD20"   ) ==  "-1234.57"   );`
	ND std::string oconv_MD(const char* conversion) const;

	//  assert( v1.oconv( "MD2"   ) ==    "12.34" );

	// Text justification: Left, right and center. Padding and truncating. See Procrustes.
	// e.g. "L#10", "R#10", "C#10"
	// Useful when outputting to terminal devices where spaces are used for alignment.
	// Multifield/multivalue structure is preserved.
	// ASCII only.
	// obj is var
	//
	// `assert(     "abcde"_var.oconv( "L#3" ) == "abc" ); // Truncating
	//  assert(     "abcde"_var.oconv( "R#3" ) == "cde" );
	//  assert(     "abcde"_var.oconv( "C#3" ) == "abc" );
	//
	//  assert(     "ab"_var.oconv( "L#6" ) == "ab␣␣␣␣" ); // Padding
	//  assert(     "ab"_var.oconv( "R#6" ) == "␣␣␣␣ab" );
	//  assert(     "ab"_var.oconv( "C#6" ) == "␣␣ab␣␣" );
	//
	//  assert(      var(42).oconv( "L(0)#5" ) == "42000" ); // Padding char (x)
	//  assert(      var(42).oconv( "R(0)#5" ) == "00042" );
	//  assert(      var(42).oconv( "C(0)#5" ) == "04200" );
	//  assert(      var(42).oconv( "C(0)#5" ) == "04200" );
	//
	//  // Multifield/multivalue
	//  assert(      "f1^v1]v2"_var.oconv("L(_)#5") == "f1___^v1___]v2___"_var);
	//
	//  // Fail for non-ASCII (Should be 5)
	//  assert(     "🐱"_var.oconv("L#5").textwidth() == 3);
	//
	//  // or
	//  assert(     oconv("abcd", "L#3" ) == "abc" );`
	ND std::string oconv_LRC(in format) const;

	// Text folding and justification.
	// e.g. T#20
	// Useful when outputting to terminal devices where spaces are used for alignment.
	// Splits text into multiple fixed length lines by inserting spaces and TM chars.
	// ASCII only.
	// obj is varstr
	//
	// `var v1 = "Have a nice day";
	//  assert(  v1.oconv("T#10") == "Have a␣␣␣␣|nice day␣␣"_var);
	//  // or
	//  assert( oconv(v1, "T#10") == "Have a␣␣␣␣|nice day␣␣"_var );`
	ND std::string oconv_T(in format) const;

	// Convert a string of bytes to a string of hexadecimal digits. The size of the output is precisely double that of the input.
	// Multifield/multivalue structure is not preserved. Field marks are converted to HEX as for all other bytes.
	// obj is varstr
	//
	// `assert(     "ab01"_var.oconv( "HEX" ) == "61" "62" "30" "31" );
	//  assert( "\xff\x00"_var.oconv( "HEX" ) == "FF" "00"           ); // Any bytes are ok.
	//  assert(        var(10).oconv( "HEX" ) == "31" "30"           ); // Uses ASCII string equivalent of 10 i.e. "10".
	//  assert(   "\u0393"_var.oconv( "HEX" ) == "CE" "93"           ); // Greek capital Gamma in UTF8 bytes.
	//  assert(     "a^]b"_var.oconv( "HEX" ) == "61" "1E" "1D" "62" ); // Field and value marks.
	//  // or
	//  assert(      oconv("ab01"_var, "HEX") == "61" "62" "30" "31");`
	ND std::string oconv_HEX(const int ioratio) const;

	// Convert a string of hexadecimal digits to a string of bytes. After prefixing a "0" to an odd sized input, the size of the output is precisely half that of the input.
	// Reverse of oconv("HEX") above.
	// obj is varstr
	ND var  iconv_HEX(const int ioratio) const;

/* fake code to generate documentation

	// Numeric hex format: Convert number to hexadecimal string
	// If the value is not numeric then no conversion is performed and the original value is returned.
	// obj is varnum
	//
	// `assert( var("255").oconv("MX") == "FF");
	//  // or
	//  assert( oconv(var("255"), "MX") == "FF");`
	ND std::string oconv_MX() const;

	// Numeric binary format: Convert number to strings of 1s and 0s
	// If the value is not numeric then no conversion is performed and the original value is returned.
	// obj is varnum
	//
	// `assert( var(255).oconv("MB") == 1111'1111);
	//  // or
	//  assert( oconv(var(255), "MB") == 1111'1111);`
	ND std::string oconv_MB() const;
*/

	// Convert dynamic arrays to standard text format.
	// Useful for using text editors on dynamic arrays.
	// FMs -> NL after escaping any embedded NL
	// obj is varstr
	//
	// `  // backslash in text remains backslash
	//    assert(var(_BS).oconv("TX") == _BS);
	//
	//    // 1. Double escape any _BS "n" -> _BS _BS "n"
	//    assert(var(_BS "n").oconv("TX") == _BS _BS "n");
	//
	//    // 2. Single escape any _NL -> _BS "n"
	//    assert(var(_NL).oconv("TX") == _BS "n");
	//
	//    // 3. FMs -> _NL (⏎)
	//    assert("🌍^🌍"_var.oconv("TX") == "🌍" _NL "🌍");
	//
	//    // 4. VMs -> _BS _NL (\⏎)
	//    assert("🌍]🌍"_var.oconv("TX") == "🌍" _BS _NL "🌍");
	//
	//    // 5. SMs -> _BS _BS _NL (\\⏎)
	//    assert("🌍}🌍"_var.oconv("TX") == "🌍" _BS _BS _NL "🌍");
	//
	//    // 6. TMs -> _BS _BS _BS _NL (\\\⏎)
	//    assert("🌍|🌍"_var.oconv("TX") == "🌍" _BS _BS _BS _NL "🌍");
	//
	//    // 7. STs -> _BS _BS _BS _BS _NL (\\\\⏎)
	//    assert("🌍~🌍"_var.oconv("TX") == "🌍" _BS _BS _BS _BS _NL "🌍");`
	ND std::string oconv_TX(const char* conversion) const;

	// Convert standard text format to dynamic array.
	// Reverse of oconv("TX") above.
	// obj is varstr
	ND var  iconv_TX(const char* conversion) const;

	/////////////////////
	// Stop documentation
	/// :
	/////////////////////

	// MD_ Decimal <- Decimal - Not implemented yet
	//  Any multifield/multivalue structure is preserved.
	ND var  iconv_MD(const char* conversion) const;

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

//	// Operator []. Allow char extraction.
//	// xxx(fn)[cn]
//	// DONT change deprecation wording without also changing it in cli/fixdeprecated
//	[[deprecated ("EXODUS: Replace single char accessors like xxx[n] with xxx.at(n)")]]
//	ND RETVAR operator[](const int pos1) const {
//		return this->at(pos1);
//	}

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

//	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
//	[[deprecated ("EXODUS: Replace single char accessors like xxx[n] with xxx.at(n)")]]
//	ND var  operator[](const int pos1) const {
//		return this->at(pos1);
//	}

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

//	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
//	[[deprecated ("EXODUS: Replace single char accessors like xxx[n] with xxx.at(n)")]]
//	ND var  operator[](const int pos1) const {
//		return this->at(pos1);
//	}

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
// var versions of various field mark chars
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
// A global flag used in vardb:
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
