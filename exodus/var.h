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

#if EXO_MODULE
	import std;
#else
#	include <fstream>
#	include <string>
#	include <string_view>
//#	include <array> // used in operator<<
#	include <vector>
#endif

#include <exodus/format.h>

// Purely #define so can done before any declarations
// Always need to be run because modules dont export #define
#include <exodus/vardefs.h>

// var_base provides the basic var-like functionality for var
//#include <exodus/vartyp.h>
#include <exodus/varb.h>

namespace exo {

	// Concept to constrain replace()'s callback function
	template <typename F>
	concept ReplacementFunction  = requires(F f, const var& v) {
		{ f(v) } -> std::same_as<var>; // Must take const var& and return var
	};

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

// class var
// using CRTP to capture a customised base class that knows what a var is
class PUBLIC var : public var_mid<var> {

	// Apparently the "using CVR - const var&;" declared in exo namespace scope
	// isnt isnt exactly the same inside the actual var class definition
	// for some reason. So we will simply redeclare it.
	using CVR    = const var&;

	friend class dim;
	friend class rex;

public:

	// Inherit all constructors from var_base
//	using var_base::var_base;
	using var_mid<var>::var_mid;

	// Tabular documentation is generated for comments starting /// or more and ending with a colon

	////////////////////
	///// var creation :
	////////////////////

	/* fake for gendoc

	// Create an unassigned var.
	// Unassigned vars can be conditionally assigned in if/else statements or used as outbound arguments in function calls.
	// A runtime error is thrown if a var is used before being assigned, preventing silent "use before assign" bugs.
	//
	// `var client; // Unassigned var
	//  if (not read(client from "xo_clients", "SB001")) ...`
	//
	// `//             Exodus keywords:     C++ declarations:
	//  //             Non-Const   Const    Non-Const   Const
	//  //             -----------------    ----------------------
	//  // Variable:   var         let      var         const var
	//  // Reference:  out         in       var&        const var&`
	//
    var() = default;

	// Assign a var using a literal or an expression.
	// Use "let" instead of "var" wherever possible as a shorthand way of writing "const var".
	//
	// `var v1 = 42;                 // Integer
	//  var v2 = 42.3;               // Double
	//  var v3 = "abc";              // String
	//  var v4 = 'x';                // char
	//  var v5 = true;               // bool
	//
	//  var v6 = v1 + 100;           // Arithmetic
	//  var v7 = v3 ^ "xyz";         // Concatenation
	//  var v8 = oslist(".").sort(); // Built in functions
	//
	//  let v9 = 12345;              // A const var
	//
	//  var v10 = 12'345_var;        // A literal var integer
	//  var v11 = 123.45_var;        // A literal var double
	//  var v12 = "f1^v1]v2^f3"_var; // A literal var string
	//
	//  var x = 0.1, y = "0.2", z = x + y; // z -> 0.3`
	//
	void operator=(expression) &;

	// obj is v1

	// Check if a var has been assigned a value.
	// return: True if the var is assigned, otherwise false
	ND bool assigned() const;

	// Check if a var has not been assigned a value;
	// return: True if the var is unassigned, otherwise false
	ND bool unassigned() const;

	// Copy a var or, if it is unassigned, copy a default value.
	// return: A copy of the var if it is assigned or the default value if it is not.
	// Can be used to handle optional arguments in functions.
	// defaultvalue: Cannot be unassigned.
	// obj is v2
	//
	// `var v1; // Unassigned
	//  var v2 = v1.or_default("abc"); // v2 -> "abc"
	//  // or
	//  var v3 = or_default(v1, "abc");`
	//
	// Mutator: defaulter()
	//
	ND var or_default(in defaultvalue) const;

	// If a var is unassigned, assign a default value.
	// If the var is unassigned then assign the default value to it, otherwise do nothing.
	// defaultvalue: Cannot be unassigned.
	//
	// `var v1; // Unassigned
	//  v1.defaulter("abc"); // v1 -> "abc"
	//  // or
	//  defaulter(v1, "abc");`
	//
	void defaulter(CVR defaultvalue);

	// Swap the contents of one var with another.
    // Useful for stashing large strings quickly. They are moved using pointers without making copies or allocating memory.
	// Either or both variables may be unassigned.
	//
	// `var v1 = space(65'536);
	//  var v2 = "";
	//  v1.swap(v2); // v1 -> "" // v2.len() -> 65'536
	//  // or
	//  swap(v1, v2);`
	//
	void swap(io v2);

	// Move a var into another.
	// Performs a shallow copy of the var's data and transfers ownership of its string, if any. The moved var is set to an empty string.
	// Enables efficient handling of large strings by moving pointers without copying or allocating memory.
	// throw: VarUnassigned if the moved var is unassigned before the move.
	// obj is v2
	//
	// `var v1 = space(65'536);
	//  var v2 = v1.move(); // v2.len() -> 65'536 // v1 -> ""
	//  // or
	//  var v3 = move(v2);`
	//
	ND var move();

    // Return a copy of the var.
    // The cloned var may be unassigned, in which case the copy will be unassigned too.
	// obj is v2
	// `var v1 = "abc";
	//  var v2 = v1.clone(); // "abc"
	//  // or
	//  var v3 = clone(v2);`
	//
	ND var clone() const;

	// Return a string describing internal data of a var.
	// If the var holds an internal std::string using heap storage then its its heap storage address is given.
	// typ: Multiple typs may exist simultaneously.
    // * 0x01 * str is available.
    // * 0x02 * int is available.
    // * 0x04 * dbl is available.
    // * 0x08 * nan: str is not a number.
    // * 0x16 * osfile (nan is true, str, int and dbl have special meaning).
	//
	// `var v1 = str("x", 32);
	//  v1.dump().outputl(); /// e.g. var:0x7ffea7462cd0 typ:1 str:0x584d9e9f6e70 "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	//  // or
	//  outputl(dump(v1));`
	//
	ND var dump() const;

	*/

	//////////////////////////////
	///// Arithmetical operators :
	//////////////////////////////

	/* fake for gendoc Defined for var_base in varb.h

	// Check if a var is numeric.
	// return: True if a var holds a double, integer, or a string representing a numeric value.
	// A string is considered numeric if it is:
	// * Empty (treated as zero), or
	// * Composed of one or more digits (0-9), an optional leading '+' or '-' sign, and an optional single decimal point ('.') placed before, within, or after the digits.
	// * Optionally includes an exponential suffix ('e' or 'E', optionally followed by '+' or '-', and 1-3 digits).
	//
	// `if (   "+123.45"_var.isnum()) ... ok
	//  if ("+1.2345e+2"_var.isnum()) ... ok
	//  if (          ""_var.isnum()) ... ok
	//  if (not      "."_var.isnum()) ... ok
	//  // or
	//  if (isnum("123.")) ... ok`
	//
	bool isnum() const;

    // Return a copy of the var if it is numeric or 0 otherwise.
	// Allow working numerically with data that may be non-numeric.
	// return: A guaranteed numeric var
	//
	// `var v1 = "123.45"_var.num();    // 123.45
	//  var v2 = "abc"_var.num() + 100; // 100`
	//
	ND var num() const;

	// Addition
	// Any attempt to perform numeric operations on non-numeric strings will throw a runtime error VarNonNumeric.
	// Floating point numbers are implicitly converted to strings with no more than 12 significant digits of precision. This practically eliminates all floatng point rounding errors.
	// Internally, 0.1 + 0.2 looks like this using doubles.
	// 0.10000000000000003 + 0.20000000000000004 -> 0.30000000000000004
	//
	// `var v1 = 0.1;
	//  var v2 = v1 + 0.2; // 0.3`
	//
	ND var operator+(var);

	// Subtraction
	ND var operator-(var);

	// Multiplication
	ND var operator*(var);

	// Division
	ND var operator/(var);

	// Modulus
	ND var operator%(var);

	// Self addition
	//
	// `var v1 = 0.1;
	//  v1 += 0.2; // 0.3`
	//
	ND var operator+=(var);

	// Self subtraction
	ND var operator-=(var);

	// Self multiplication
	ND var operator*=(var);

	// Self division
	ND var operator/=(var);

	// Self modulus
	ND var operator%=(var);

	*/

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

	// Post increment
	//
	// `var v1 = 3;
	//  var v2 = v1 ++; // v2 -> 3 // v1 -> 4`
	//
	   var  operator++(int) &;

	// Post decrement
	//
	// `var v1 = 3;
	//  var v2 = v1 --; // v2 -> 3 // v1 -> 2`
	//
	   var  operator--(int) &;

	// Pre increment
	//
	// `var v1 = 3;
	//  var v2 = ++ v1; // v2 -> 4 // v1 -> 4`
	//
	   io   operator++() &;

	// Pre decrement
	//
	// `var v1 = 3;
	//  var v2 = -- v1; // v2 -> 2 // v1 -> 2`
	//
	   io   operator--() &;

	////////////////////////////////////////////////
	///// Dynamic array creation, access and update:
	////////////////////////////////////////////////

	// Literal suffix "_var".
	// Allow dynamic arrays to be seamlessly embedded in code using a predefined set of visible equivalents of unprintable field mark characters as follows:
	// Visible equivalents:
	// * {backtick} * RM  Record mark
	// * ^     * FM  Field mark
	// * ]     * VM  Value mark
	// * }     * SM  Subvalue mark
	// * |     * TM  Text mark
	// * ~     * ST  Subtext mark
	//
	// `var v1 = "f1^f2^v1]v2^f4"_var; // "f1" _FM "f2" _FM "v1" _VM "v2" _FM "f4"`
	//
	friend var  operator""_var(const char* cstr, std::size_t size);

	template <class T>
    // Code a dynamic array var as as list.
	// C++ constrains list elements to be all the same type: var, string, double, int, etc. but they all end up as fields of a dynamic array string.
	//
	// `var v1 = {11, 22, 33}; // "11^22^33"_var`
	//
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

	/* fake for gendoc

	// Dynamic array field extraction, update and append:
	// See also inserter() and remover().
	//
	// `var v1 = "aa^bb"_var;
	//  v1(4) = 44; // v1 -> "aa^bb^^44"_var
	//  // Field number -1 causes appending a field when updating.
	//  v1(-1) = "55"; // v1 -> "aa^bb^^44^55"_var`
	//
	// Field access:
	// It is recommended to use "v1.f(fieldno)" syntax using a ".f(" prefix to access fields in expressions instead of plain "v1(fieldno)". The former syntax (using .f()) will always compile whereas the latter does not compile in all contexts. It will compile only if being called on a constant var or in a location which requires a var. This is due to C++ not making a clear distinction between usage on the left and right side of assignment operator =.
	// Furthermore using plain round brackets without the leading .f can be confused with function call syntax.
	//
	// `var v1 = "aa^bb^cc"_var;
	//  var v2 = v1.f(2); // "bb" /// .f() style access. Recommended.
	//  var v3 =   v1(2); // "bb" ///   () style access. Not recommended.`
	//
	ND var  operator()(int fieldno) const;

	// Dynamic array value update and append
	// See also inserter() and remover().
	//
	// `var v1 = "aa^b1]b2^cc"_var;
	//  v1(2, 4) = "44"; // v1 -> "aa^b1]b2]]44^cc"_var
	//  // value number -1 causes appending a value when updating.
	//  v1(2, -1) = 55; // v1 -> "aa^b1]b2]]44]55^cc"_var`
	//
	// Value access:
	//
	// `var v1 = "aa^b1]b2^cc"_var;
	//  var v2 = v1.f(2,2); // "b2" /// .f() style access. Recommended.
	//  var v3 =   v1(2,2); // "b2" ///   () style access. Not recommended.`
	//
	ND var  operator()(int fieldno, valueno) const;

	*/

	/////////////////
	// PARENTHESIS ()
	/////////////////

	// Extract fields, values and subvalues using () int int int
	// An alternative to .f() and extract()
	// Equvalent to var v1 = v2.extract(1,2,3);
	// Sadly there is no way to use pick/mv angle brackets like "abc<1,2,3>"

	//SADLY no way to get a different operator() function called when on the left hand side of assign
	//http://codepad.org/MzzhlRkb

	//subscript operators often come in pairs
	//
	// 1. const returning a value
	// 2. non-const returning a reference or a proxy object

	// 1. () on const vars will extract the desired field/value/subvalue as a proper var
	// Note that all  function "in" arguments are const vars
	// so will work perfectly with () extraction

	// Dynamic array subvalue update and append
	// See also inserter() and remover().
	//
	// `var v1 = "aa^bb^cc"_var;
	//  v1(2, 2, 2) = "22"; // v1 -> "aa^bb]}22^cc"_var
	//  // subvalue number -1 causes appending a subvalue when updating.
	//  v1(2, 2, -1) = 33; // v1 -> "aa^bb]}22}33^cc"_var`
	//
	// Subvalue access:
	//
	// `var v1 = "aa^b1]b2}s2^cc"_var;
	//  var v2 = v1.f(2, 2, 2); // "s2" /// .f() style access. Recommended.
	//  var v3 =   v1(2, 2, 2); // "s2" ///   () style access. Not recommended.`
	//
	ND const var  operator()(int fieldno, int valueno = 0, int subvalueno = 0) const {return this->f(fieldno, valueno, subvalueno);}
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

	///////////////////////
	///// String creation :
	///////////////////////

	/* fake for gendoc

	// String concatention operator ^
	// At least one side must be a var.
	// "aa" ^ "22" will not compile but "aa" "22" will.
	// Floating point numbers are implicitly converted to strings with no more than 12 significant digits of precision. This practically eliminates all floatng point rounding errors.
	// `var v2 = "aa";
	//  var v1 = v2 ^ 22; // "aa22"`
	//
	ND var operator^(var);

	// String self concatention ^= (append)
	// `var v1 = "aa";
	//  v1 ^= 22; // v1 -> "aa22"`
	ND var operator^=(var);
	*/

	// Round a number.
	// Convert a number into a string after rounding it to a given number of decimal places.
	// Trailing zeros are not omitted. A leading "0." is shown where appropriate.
    // 0.5 always rounds away from zero. i.e. 1.5 -> 2 and -2.5 -> -3
	// var: The number to be converted.
	// ndecimals: Determines how many decimal places are shown to the right of the decimal point or, if ndecimals is negative, how many 0's to the left of it.
	// return: A var containing an ASCII string of digits with a leading "-" if negative, and a decimal point "." if ndecimals is > 0.
	// obj is varnum
	//
    // `let v1 = var(0.295).round(2);  //  "0.30"
    //  // or
    //  let v2 = round(1.295, 2);      //  "1.30"
	//
    //  var v3 = var(-0.295).round(2); // "-0.30"
    //  // or
    //  var v4 = round(-1.295, 2);     // "-1.30"
	//
	//  var v5 = round(0, 1);           // "0.0"
	//  var v6 = round(0, 0);           // "0"
	//  var v7 = round(0, -1);          // "0"`
	// Negative number of decimals rounds to the left of the decimal point
	// `let v1 = round(123456.789,  0); // "123457"
	//  let v2 = round(123456.789, -1); // "123460"
	//  let v3 = round(123456.789, -2); // "123500"`
	//
    ND var  round(const int ndecimals = 0) const;

	// obj is var()

	// Get a char.
	// num: An integer 0-255.
	// return: A string containing a single char
	// 0-127 -> ASCII, 128-255 -> invalid UTF-8 which cannot be written to the database or used in many exodus string operations
	//
	// `let v1 = var::chr(0x61); // "a"
	//  // or
	//  let v2 = chr(0x61);`
	//
	ND static var  chr(const int num);

	// Get a Unicode character.
	// num: A Unicode Code Point (Number)
	// return: A single Unicode character in UTF8 encoding.
	//
	// `let v1 = var::textchr(171416); // "𩶘" // or "\xF0A9B698"
	//  // or
	//  let v2 = textchr(171416);`
	//
	ND static var  textchr(const int num);

	// Get a Unicode character name
	// unicode_code_point: 0 - 0x10FFFF.
	// return: Text of the name or "" if not a valid Unicode Code Point
	//
	// `let v1 = var::textchrname(91); // "LEFT SQUARE BRACKET"
	//  // or
	//  let v2 = textchrname(91);`
	//
	ND static var  textchrname(const int unicode_code_point);

	// Get a string of repeated substrings.
	// var: The substring to be repeated
	// num: How many times to repeat the substring
	// return: A string
	// obj is strvar
	//
	// `let v1 = "ab"_var.str(3); // "ababab"
	//  // or
	//  let v2 = str("ab", 3);`
	//
	ND var  str(const int num) const;

	// Get a string as a given number of spaces.
	// nspaces: The number of spaces required.
	// return: A string of space chars.
	// obj is var()
	//
	// `let v1 = var::space(3); // "␣␣␣"
	//  // or
	//  let v2 = space(3);`
	//
	ND static var  space(const int nspaces);

	// Get a number written out in words insteads of digits.
	// return: A string.
	// locale: e.g. en_GB, ar_AE, el_CY, es_US, fr_FR etc or a language name e.g. "french".
	// obj is varnum
	//
	// `let softhyphen = "\xc2\xad";
	//  let v1 = var(123.45).numberinwords("de_DE").replace(softhyphen, " "); // "ein␣hundert␣drei␣und␣zwanzig␣Komma␣vier␣fünf"`
	//
	ND var  numberinwords(in locale = "");

	////////////
	// STRING // All UTF8 unless char or byte mentioned
	////////////

	///// STRING SCANNING:
	//////////////////////

	// obj is strvar

	/* fake for gendoc from var_base in varb.h

	// Get a single char from a string.
	// pos1: First char is 1. Last char is -1.
	// return: A single char if pos1 +/- the length of the string, or "" if greater. Returns the first char if pos1 is 0 or (-pos1) > length.
	// `var v1 = "abc";
	//  var v2 = v1.at(2);  // "b"
	//  var v3 = v1.at(-3); // "a"
	//  var v4 = v1.at(4);  // ""`
    ND var  at(const int pos1) const;
	*/

	// Get the char number of a char.
	// return: A number between 0 and 255.
	// If given a string, then only the first char is considered.
	// Equivalent to ord() in php
	//
	// `let v1 = "abc"_var.ord(); // 0x61 // decimal 97, 'a'
	//  // or
	//  let v2 = ord("abc");`
	//
	ND var  ord() const;

	// Get the Unicode Code Point of a Unicode character.
	// var: A UTF-8 string. Only the first Unicode character is considered.
	// return: A number 0 to 0x10FFFF.
	// Equivalent to ord() in python and ruby, mb_ord() php.
	//
	// `let v1 = "Γ"_var.textord(); // 915 // U+0393: Greek Capital Letter Gamma (Unicode character)
	//  // or
	//  let v2 = textord("Γ");`
	//
	ND var  textord() const;

	// Get the length of a source string in number of chars.
	// return: A number
	//
	// `let v1 = "abc"_var.len(); // 3
	//  // or
	//  let v2 = len("abc");`
	//
	ND var  len() const;

	// Check if the var is an empty string.
	// return: True if it is empty amd false if not.
	// This is a shorthand and more expressive way of writing 'if (var == "")' or 'if (var.len() == 0)' or 'if (not var.len())'
	// Note that 'if (var.empty())' is not exactly the same as 'if (not var)' because 'if (var("0.0")' is also defined as false. If a string can be converted to 0 then it is considered to be false. Contrast this with common scripting languages where 'if (var("0"))' is defined to be true.
	//
	// `let v1 = "0";
	//  if (not v1.empty()) ... ok // true
	//  // or
	//  if (not empty(v1)) ... ok // true`
	//
	ND bool empty() const;

	// Count the number of output columns required for a given source string.
	// return: A number
	// Allow wide multi-column Unicode characters that occupy more than one space in a text file or terminal screen.
	// Reduce combining characters to a single column. e.g. "e" followed by grave accent is multiple bytes but only occupies one output column.
	// Does not properly calculate all possible combining sequences of graphemes e.g. face followed by colour
	//
	// `let v1 = "🤡x🤡"_var.textwidth(); // 5
	//  // or
	//  let v2 = textwidth("🤡x🤡");`
	//
	ND var  textwidth() const;

	// Count the number of Unicode code points in a source string.
	// return: A number.
	//
	// `let v1 = "Γιάννης"_var.textlen(); // 7
	//  // or
	//  let v2 = textlen("Γιάννης");`
	//
	ND var  textlen() const;

	// Count the number of fields in a source string.
	// sepstr: The separator character or substr that delimits individual fields.
	// return: The count of the number of fields
	// This is similar to "var.count(sepstr) + 1" but it returns 0 for an empty source string.
	//
	// `let v1 = "aa**cc"_var.fcount("*"); // 3
	//  // or
	//  let v2 = fcount("aa**cc", "*");`
	//
	ND var  fcount(SV sepstr) const;

	// Count the number of occurrences of a given substr in a source string.
	// substr: The substr to count.
	// return: The count of the number of sepstr found.
	// Overlapping substrings are not counted.
	//
	// `let v1 = "aa**cc"_var.count("*"); // 2
	//  // or
	//  let v2 = count("aa**cc", "*");`
	//
	ND var  count(SV sepstr) const;

	// Exodus   Javascript   PHP             Python       Go          Rust          C++
	// starts   startsWith() str_starts_with startswith() HasPrefix() starts_with() starts_with
	// ends     endsWith     str_ends_with   endswith     HasSuffix() ends_with()   ends_with
	// contains includes()   str_contains    contains()   Contains()  contains()    contains

	// Check if a source string starts with a given prefix (substr).
	// prefix: The substr to check for.
	// return: True if the source string starts with the given prefix.
 	// Always returns false if suffix is "". DIFFERS from c++, javascript, python3. See contains() for more info.
	//
	// `if ("abc"_var.starts("ab")) ... true
	//  // or
	//  if (starts("abc", "ab")) ... true`
	//
	ND bool starts(SV prefix) const;

	template<typename... T>
	bool starts(SV prefix, T&&... prefix_n) const {
		if (starts(prefix)) return true;
//		static_assert((std::is_convertible_v<T, std::string_view> && ...),
//			"All prefix_n arguments must be convertible to std::string_view");
		return (... || no_check_starts(static_cast<SV>(std::forward<T>(prefix_n))));
	}

	// Check if a source string ends with a given suffix (substr).
	// suffix: The substr to check for.
	// return: True if the source string ends with given suffix.
 	// Always returns false if suffix is "". DIFFERS from c++, javascript, python3. See contains() for more info.
	//
	// `if ("abc"_var.ends("bc")) ... true
	//  // or
	//  if (ends("abc", "bc")) ... true`
	//
	ND bool ends(SV suffix) const;

	template<typename... T>
	bool ends(SV suffix, T&&... suffix_n) const {
		if (ends(suffix)) return true;
//		static_assert((std::is_convertible_v<T, std::string_view> && ...),
//			"All suffix_n arguments must be convertible to std::string_view");
		return (... || no_check_ends(static_cast<SV>(std::forward<T>(suffix_n))));
	}

	// Check if a given substr exists in a source string.
	// substr: The substr to check for.
	// return: True if the source string starts with, ends with or contains the given substr.
	// Always returns false if substr is "". DIFFERS from c++, javascript, python3. See contains() for more info.
    // Human logic: "" is not equal to "x" therefore x does not contain "".
    // Human logic: Check each item (character) in the list for equality with what I am looking for and return success if any are equal.
    // Programmer logic: Compare as many characters as are in the search string for presence in the list of characters and return success if there are no failures.
    //
	// `if ("abcd"_var.contains("bc")) ... true
	//  // or
	//  if (contains("abcd", "bc")) ... true`
	//
	ND bool contains(SV substr) const;

	template<typename... T>
	bool contains(SV substr, T&&... substr_n) const {
		if (contains(substr)) return true;
//		static_assert((std::is_convertible_v<T, std::string_view> && ...),
//			"All substr_n arguments must be convertible to std::string_view");
		return (... || no_check_contains(static_cast<SV>(std::forward<T>(substr_n))));
	}

	//https://en.wikipedia.org/wiki/Comparison_of_programming_languages_(string_functions)#Find

	// Find a substr in a source string.
	// substr: The substr to search for.
	// startchar1: The char position (1 based) to start the search at. The default is 1, the first char.
	// return: The char position (1 based) that the substr is found at or 0 if not present.
	//
	// `let v1 = "abcd"_var.index("bc"); // 2
	//  // or
	//  let v2 = index("abcd", "bc");`
	//
	ND var  index(SV substr, const int startchar1 = 1) const;

	// Find the nth occurrence of a substr in a source string.
	// substr: The string to search for.
	// return: char position (1 based) or 0 if not present.
	//
	// `let v1 = "abcabc"_var.indexn("bc", 2); // 5
	//  // or
	//  let v2 = indexn("abcabc", "bc", 2);`
	//
	ND var  indexn(SV substr, const int occurrence) const;

	// Find the position of substr working backwards
	// Start at the end and work backwards.
	// substr: The string to search for.
	// return: The char position of the substr if found, or 0 if not.
	// startchar1: Defaults to -1 meaning start searching from the last char. Positive start1char1 counts from the beginning of the source string and negative startchar1 counts backwards from the last char.
	//
	// `let v1 = "abcabc"_var.indexr("bc"); // 5
	//  // or
	//  let v2 = indexr("abcabc", "bc");`
	//
	ND var  indexr(SV substr, const int startchar1 = -1) const;

	// Check if something is in a list.
	// The list must be comma separated without spaces.
	//
	// `let v1 = "def";
	//  if (v1.listed("abc,def")) ... ok
	//  // or
	//  if (listed(v1, "abc,def")) ... ok`
	//
	ND bool listed(SV list) const;

	// Check if something is in a list and return its position.
	//
	// `let v1 = "def";
	//  var posn;
	//  if (v1.listed("abc,def", posn)) ... ok // posn -> 2
	//  // or
	//  if (listed(v1, "abc,def", posn)) ... ok`
	//
	ND bool listed(SV list, out position) const;

	// Find all matches of a given regular expression.
	// return: Zero or more matching substrings separated by FMs. Any groups are in VMs.
	//
	// `let v1 = "abc1abc2"_var.match("BC(\\d)", "i"); // "bc1]1^bc2]2"_var
	//  // or
	//  let v2 = match("abc1abc2", "BC(\\d)", "i");`
	//
	// regex_options:
    // *  l  * Literal (any regex chars are treated as normal chars)
    // *  i  * Case insensitive
    // *  p  * ECMAScript/Perl (the default)
    // *  b  * Basic POSIX (same as sed)
    // *  e  * Extended POSIX
    // *  a  * awk
    // *  g  * grep
    // *  eg * egrep or grep -E
	//
    // char ranges like a-z are locale sensitive if ECMAScript
	//
	// regex_options:
    // *  m * Multiline. Default in boost (and therefore exodus)
    // *  s * Single line. Default in std::regex
	// *  f * First only. Only for replace() (not match() or search())
    // *  w * Wildcard glob style (e.g. *.cfg) not regex style. Only for match() and search(). Not replace().
	//
	ND var  match(SV regex_str, SV regex_options = "") const;

	// Ditto
	ND var  match(const rex& regex) const;

	// Search for the first match of a regular expression.
	// startchar1: [in] char position to start the search from
	// startchar1[out]: char position to start the next search from or 0 if no more matches.
	// return: The 1st match like match()
	// regex_options as for match()
	//
	// `var startchar1 = 1;
	//  let v1 = "abc1abc2"_var.search("BC(\\d)", startchar1, "i"); // "bc1]1"_var // startchar1 -> 5 /// Ready for the next search
	//  // or
	//  startchar1 = 1;
	//  let v2 = search("abc1abc2", "BC(\\d)", startchar1, "i");`
	//
	ND var  search(SV regex_str, io startchar1, SV regex_options = "") const;

	// Ditto starting from first char
	ND var  search(SV regex_str) const {var startchar1 = 1; return this->search(regex_str, startchar1);}

	// Ditto given a rex
	ND var  search(const rex& regex, io startchar1) const;

	// Ditto starting from first char.
	ND var  search(const rex& regex) const {var startchar1 = 1; return this->search(regex, startchar1);}

	// Get a hash of a source string.
	// modulus: The result is limited to [0, modulus)
	// return: A 64 bit signed integer.
	// MurmurHash3 is used.
	//
	// `let v1 = "abc"_var.hash(); assert(v1 == var(6'715'211'243'465'481'821));
	//  // or
	//  let v2 = hash("abc");`
	//
	ND var  hash(const std::uint64_t modulus = 0) const;

	///// STRING CONVERSION - Non-mutating - Chainable:
	//////////////////////////////////////////////////

	// obj is strvar

	// Convert to upper case
	//
	// `let v1 = "Γιάννης"_var.ucase(); // "ΓΙΆΝΝΗΣ"
	//  // or
	//  let v2 = ucase("Γιάννης");`
	//
	ND var  ucase() const& {var nrvo = this->clone(); nrvo.ucaser(); return nrvo;}

	// Convert to lower case
	//
	// `let v1 = "ΓΙΆΝΝΗΣ"_var.lcase(); // "γιάννης"
	//  // or
	//  let v2 = lcase("ΓΙΆΝΝΗΣ");`
	//
	ND var  lcase() const& {var nrvo = this->clone(); nrvo.lcaser(); return nrvo;}

	// Convert to title case.
	// return: Original source string with the first letter of each word is capitalised.
	//
	// `let v1 = "γιάννης παππάς"_var.tcase(); // "Γιάννης Παππάς"
	//  // or
	//  let v2 = tcase("γιάννης παππάς");`
	//
	ND var  tcase() const& {var nrvo = this->clone(); nrvo.tcaser(); return nrvo;}

	// Convert to folded case.
	// Case folding is the process of converting text to a case independent representation.
	// return: The source string standardised in a way to enable consistent indexing and searching,
	// https://www.w3.org/International/wiki/Case_folding
	// Accents can be significant. As in French cote, coté, côte and côté.
	// Case folding is not locale-dependent.
	//
	// `let v1 = "Grüßen"_var.fcase(); // "grüssen"
	//  // or
	//  let v2 = tcase("Grüßen");`
	//
	ND var  fcase() const& {var nrvo = this->clone(); nrvo.fcaser(); return nrvo;}

	// Replace Unicode character sequences with their standardised NFC form.
	// Unicode normalization is the process of converting Unicode strings to a standard form, making them binary comparable and suitable for text processing and comparison. It is an important part of Unicode text processing.
	// For example, Unicode character "é" can be represented by either a single Unicode character, which is Unicode Code Point (\u00E9" - Latin Small Letter E with Acute), or a combination of two Unicode code points i.e. the ASCII letter "e" and a combining acute accent (Unicode Code Point "\u0301"). Unicode NFC definition converts the pair of code points to the single code point.
	// Normalization is not locale-dependent.
	//
	// `let v1 = "cafe\u0301"_var.normalize(); // "caf\u00E9" // "café"
	//  // or
	//  let v2 = normalize("cafe\u0301");`
	//
	ND var  normalize() const& {var nrvo = this->clone(); nrvo.normalizer(); return nrvo;}

	// Simple reversible disguising of string text.
	// It works by treating the string as UTF8 encoded Unicode code points and inverting the first 8 bits of their Unicode Code Points.
	// return: A string.
	// invert(invert()) returns to the original text.
	// ASCII bytes become multibyte UTF-8 so string sizes increase.
	// Inverted characters remain on their original Unicode Code Page but are jumbled up.
	// Non-existant Unicode Code Points may be created but UTF8 encoding remains valid.
	//
	// `let v1 = "abc"_var.invert(); // "\xC2" "\x9E" "\xC2" "\x9D" "\xC2" "\x9C"
	//  // or
	//  let v2 = invert("abc");`
	//
	ND var  invert() const& {var nrvo = this->clone(); nrvo.inverter(); return nrvo;}

	// Reduce all types of field mark chars by one level.
	// Convert all FM to VM, VM to SM etc.
	// return: The converted string.
	// Note that subtext ST chars are not converted because they are already the lowest level.
	// String size remains identical.
	//
	// `let v1 = "a1^b2^c3"_var.lower(); // "a1]b2]c3"_var
	//  // or
	//  let v2 = lower("a1^b2^c3"_var);`
	//
	ND var  lower() const& {var nrvo = this->clone(); nrvo.lowerer(); return nrvo;}

	// Increase all types of field mark chars by one level.
	// Convert all VM to FM, SM to VM etc.
	// return: The converted string.
	// The record mark char RM is not converted because it is already the highest level.
	// String size remains identical.
	//
	// `let v1 = "a1]b2]c3"_var.raise(); // "a1^b2^c3"_var
	//  // or
	//  let v2 = "a1]b2]c3"_var;`
	//
	ND var  raise() const& {var nrvo = this->clone(); nrvo.raiser(); return nrvo;}

	// Remove any redundant FM, VM etc. chars (Trailing FM; VM before FM etc.)
	//
	// `let v1 = "a1^b2]]^c3^^"_var.crop(); // "a1^b2^c3"_var
	//  // or
	//  let v2 = crop("a1^b2]]^c3^^"_var);`
	//
	ND var  crop() const& {var nrvo = this->clone(); nrvo.cropper(); return nrvo;}

	// Wrap in double quotes.
	//
	// `let v1 = "abc"_var.quote(); // "\"abc\""
	//  // or
	//  let v2 = quote("abc");`
	//
	ND var  quote() const&;

	// Wrap in single quotes.
	//
	// `let v1 = "abc"_var.squote(); // "'abc'"
	//  // or
	//  let v2 = squote("abc");`
	//
	ND var  squote() const&;

	// Remove one pair of surrounding double or single quotes.
	//
	// `let v1 = "'abc'"_var.unquote(); // "abc"
	//  // or
	//  let v2 = unquote("'abc'");`
	//
	ND var  unquote() const&;

	// Remove all leading, trailing and excessive inner bytes.
	// trimchars: The chars (bytes) to remove. The default is space.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trim(); // "a1␣b2␣c3"
	//  // or
	//  let v2 = trim("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trim(SV trimchars = " ") const&;

	// Ditto but only leading.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimfirst(); // "a1␣␣b2␣c3␣␣"
	//  // or
	//  let v2 = trimfirst("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trimfirst(SV trimchars = " ") const&;

	// Ditto but only trailing.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimlast(); // "␣␣a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimlast("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trimlast(SV trimchars = " ") const&;

	// Ditto but only leading and trailing, not inner.
	//
	// `let v1 = "␣␣a1␣␣b2␣c3␣␣"_var.trimboth(); // "a1␣␣b2␣c3"
	//  // or
	//  let v2 = trimboth("␣␣a1␣␣b2␣c3␣␣");`
	//
	ND var  trimboth(SV trimchars = " ") const&;

	// Get the first char of a string.
	// return: A char, or "" if empty.
	// Equivalent to var.substr(1,length) or var[1, length] in Pick OS
	//
	// `let v1 = "abc"_var.first(); // "a"
	//  // or
	//  let v2 = first("abc");`
	//
	ND var  first() const&;

	// Get the last char of a string.
	// return: A char, or "" if empty.
	// Equivalent to var.substr(-1, 1) or var[-1, 1] in Pick OS
	//
	// `let v1 = "abc"_var.last(); // "c"
	//  // or
	//  let v2 = last("abc");`
	//
	ND var  last() const&;

	// Get the first n chars of a source string.
	// length: The number of chars (bytes) to get.
	// return: A string of up to n chars.
	// Equivalent to var.substr(1, length) or var[1, length] in Pick OS
	//
	// `let v1 = "abc"_var.first(2); // "ab"
	//  // or
	//  let v2 = first("abc", 2);`
	//
	ND var  first(const std::size_t length) const&;

	// Extract up to length trailing chars
	// Equivalent to var.substr(-length, length) or var[-length, length] in Pick OS
	//
	// `let v1 = "abc"_var.last(2); // "bc"
	//  // or
	//  let v2 = last("abc", 2);`
	//
	ND var  last(const std::size_t length) const&;

	// Remove n chars (bytes) from the source string.
	// length: Positive to remove first n chars or negative to remove the last n chars.
	// If the absolute value of length is >= the number of chars in the source string then all chars will be removed.
	// Equivalent to var.substr(length) or var[1, length] = "" in Pick OS
	//
	// `let v1 = "abcd"_var.cut(2); // "cd"
	//  // or
	//  let v2 = cut("abcd", 2);`
	//
	ND var  cut(const int length) const&;

	// Insert a substr at an given position after removing a given number of chars.
	// pos1:
	// * 0 or 1   * Remove length chars from the beginning and insert at the beginning.
	// *     -1   * Insert on or before the last char after removing up to length chars starting with the last char.
	// *     -2   * Ditto on or before the penultimate char.
	// * > strlen * If pos1 > length of the source string, insert after the last char.
	// Equivalent to var[pos1, length] = substr in Pick OS
	//
	// `let v1 = "abcd"_var.paste(2, 2, "XYZ"); // "aXYZd"
	//  // or
	//  let v2 = paste("abcd", 2, 2, "XYZ");`
	//
	ND var  paste(const int pos1, const int length, SV replacestr) const& {var nrvo = this->clone(); nrvo.paster(pos1, length, replacestr); return nrvo;}

	// Insert text at char position without overwriting any following chars
	// Equivalent to var[pos1, 0] = substr in Pick OS
	//
	// `let v1 = "abcd"_var.paste(2, "XYZ"); // "aXYZbcd"
	//  // or
	//  let v2 = paste("abcd", 2, "XYZ");`
	//
	ND var  paste(const int pos1, SV insertstr) const& {var nrvo = this->clone(); nrvo.paster(pos1, insertstr); return nrvo;}

	// Insert text at the beginning
	// Equivalent to var[0, 0] = substr in Pick OS
	//
	// `let v1 = "abc"_var.prefix("XYZ"); // "XYZabc"
	//  // or
	//  let v2 = prefix("abc", "XYZ");`
	//
	ND var  prefix(SV insertstr) const&;

//	template <typename... ARGS>
//	ND var  append(const ARGS&... appendable) const& {
//		var nrvo = *this;
//		(nrvo ^= ... ^= appendable);
//		return nrvo;
//	}

	// Append anything at the end of a string.
	//
	// `let v1 = "abc"_var.append(" is ", 10, " ok", '.'); // "abc is 10 ok."
	//  // or
	//  let v2 = append("abc", " is ", 10, " ok", '.');`
    ND var  append(const auto&... appendable) const& {var nrvo = this->clone(); (nrvo ^= ... ^= appendable); return nrvo;}
	//
	// TODO perfect forwarding on argument 1 to create the initial string?

	// Remove one trailing char.
	// Equivalent to var[-1, 1] = "" in Pick OS
	//
	// `let v1 = "abc"_var.pop(); // "ab"
	//  // or
	//  let v2 = pop("abc");`
	//
	ND var  pop() const& {var nrvo = this->clone(); nrvo.popper(); return nrvo;}

	// Copy one or more consecutive fields from a string.
	// delimiter: A Unicode character.
	// fieldno: The first field is 1, the last field is -1.
	// return: A substring
	//
	// `let v1 = "aa*bb*cc"_var.field("*", 2); // "bb"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", 2);`
	//
	// `let v1 = "aa*bb*cc"_var.field("*", -1); // "cc"
	//  // or
	//  let v2 = field("aa*bb*cc", "*", -1);`
	//
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
	// return: A modified copy of the original string.
	// There is no way to simply delete n fields because the replacement argument cannot be omitted, however one can achieve the same result by replacing n+1 fields with the n+1th field.
	//
	// The replacement can contain multiple fields itself. If replacing n fields and the replacement contains < n fields then the remaining fields become "". Conversely, if the replacement contains more fields than are required, they are discarded.
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
	ND var  fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const& {var nrvo = this->clone(); nrvo.fieldstorer(separator, fieldno, nfields, replacement); return nrvo;}

	// substr version 1.
	// Copy a substr of length chars from a given a starting char position.
	// return: A substr or "".
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
	//
	ND var  substr(const int pos1, const int length) const& {var nrvo = this->clone(); nrvo.substrer(pos1, length); return nrvo;}

	// Abbreviated alias of substr version 1.
	ND var  b(const int pos1, const int length) const& {return this->substr(pos1, length);}

	// substr version 2.
	// Copy a substr from a given char position up to the end of the source string
	// return: A substr or "".
	// pos1: The char position to start at. If negative then start from a position counting backwards from the last char
	// Equivalent to var[pos1, 9999999] in Pick OS
	// Partially Unicode friendly but pos1 is in chars.
	//
	// `let v1 = "abcd"_var.substr(2); // "bcd"
	//  // or
	//  let v2 = substr("abcd", 2);`
	//
	ND var  substr(const int pos1) const& {var nrvo = this->clone(); nrvo.substrer(pos1); return nrvo;}

	// Shorthand alias of substr version 2.
	ND var  b(const int pos1) const& {return this->substr(pos1);}

	// substr version 3.
	// Copy a substr from a given char position up to (but excluding) any one of some given delimiter chars
	// return: A substr or "".
	// pos1: [in] The position of the first char to copy. Negative positions count backwards from the last char of the string.
	// pos2[out]: The position of the next delimiter char, or one char position after the end of the source string if no subsequent delimiter chars are found.
	// COL2: Is a predefined variable that can be used for pos2 instead of declaring a variable.
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
	//
	   var  substr(const int pos1, SV delimiterchars, out pos2) const;

	// Shorthand alias of substr version 3.
	   var  b(const int pos1, SV delimiterchars, out pos2) const {return this->substr(pos1, delimiterchars, pos2);}

	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// offset to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(io pos1, io
	// delimiterno) const;

	// substr version 4.
	// Copy a substr from a given char position up to (but excluding) the next field mark char (RM, FM, VM, SM, TM, ST).
	// return: A substr or "".
	// pos1: [in] The position of the first char to copy. Negative positions count backwards from the last char of the string.
	// pos1[out]: The position of the first char of the next substr after whatever field mark char is found, or one char position after the end of the source string if no subsequent field mark char is found.
	// field_mark_no[out]: A number (1-6) indicating which of the standard field mark chars was found, or 0 if not.
	// An empty string may be returned if the pos1 [in] points to one of the field marks or beyond the end of the source string.
	// pos1 [out] is correctly positioned to copy the next substr.
	// Works with any encoding including UTF8. Was called "remove" in Pick OS.
	// The equivalent in Pick OS was the statement "Remove variable From string At column Setting flag"
	// This function is valuable for high performance processing of dynamic arrays.
	// It is notably used in "list" to print parallel columns of mixed combinations of multivalues/subvalues and text marks correctly lined up mv to mv, sv to sv, tm to tm even when particular values, subvalues and text fragments are missing from particular columns.
	// It is similar to version 3 of substr - substr(pos1, delimiterchars, pos2) except that in this version the delimiter chars are hard coded as the standard field mark chars (RM, FM, VM, SM, TM, ST) and it returns the first char position of the next substr, not the char position of the next field mark char.
	//
	// `var pos1 = 4, field_mark_no;
	//  let v1 = "12^45^78"_var.substr2(pos1, field_mark_no);  // "45" // pos1 -> 7 // field_mark_no -> 2 // field_mark_no 2 means that a FM was found.
	//  // or
	//  let v2 = substr2("12^45^78"_var, pos1, field_mark_no); // "78" // pos1 -> 9 // field_mark_no -> 0 // field_mark_no 0 means that none of the standard field marks were found.`
	//
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
	//
	ND var  convert(SV fromchars, SV tochars) const& {var nrvo = this->clone(); nrvo.converter(fromchars,tochars); return nrvo;}

	// Ditto for Unicode code points.
	//
	// `let v1 = "a🤡b😀c🌍d"_var.textconvert("🤡😀", "👋"); // "a👋bc🌍d"
	//  // or
	//  let v2 = textconvert("a🤡b😀c🌍d", "🤡😀", "👋");`
	//
	ND var  textconvert(SV fromchars, SV tochars) const& {var nrvo = this->clone(); nrvo.textconverter(fromchars,tochars); return nrvo;}

	// Replace all occurrences of one substr with another.
	// Case sensitive.
	//
	// `let v1 = "Abc.Abc"_var.replace("bc", "X"); // "AX.AX"
	//  // or
	//  let v2 = replace("Abc Abc", "bc", "X");`
	//
	ND var  replace(SV fromstr, SV tostr) const&;

	// Replace substrings using a regular expression.
	// regex: A regular expression created by rex() or _rex.
	// replacement_str: A literal to replace all matched substrings.
	// The replacement string can include the following special replacement patterns:
	// Pattern
	// * $$ *    Inserts a "$".
	// * $& *    Inserts the matched substring. Equivalent to $0.
	// * ${backtick} *    Inserts the portion of the string that precedes the matched substring.
	// * $' *    Inserts the portion of the string that follows the matched substring.
	// * $n *    Inserts the nth (1-indexed) capturing group where n is a positive integer less than 100.
	//
	// `let v1 = "A a B b"_var.replace("[A-Z]"_rex, "'$0'"); // "'A' a 'B' b"
	//  // or
	//  let v2 = replace("A a B b", "[A-Z]"_rex, "'$0'");`
	//
	ND var  replace(const rex& regex, SV replacement_str) const&;

	// old non-templated version that handled functions and non-capturing lambdas without std::function
	// Replaced by templated version that can accept capturing lambdas as well
	//	var  replace(const rex& regex, SomeFunction(in match_str)) const;

	// Implementation can handle any type of function including capturing lambdas.
	// [[undocumented]]
	var replace(const rex& rex1, var (*func)(const var&, void*), void* context) const;

	// Template wrapper to accept any callable (including capturing lambdas)

	// Replace substrings using a regular expression and a custom function.
	// Allow complex string conversions.
	// repl_func: A function with arguments (in match_str) that returns a var to replace match_str. May be an inline anonymous lambda function (capturing or non-capturing).
	// e.g. [](auto match_str) {return match_str;} // Does nothing.
	// match_str: Text of a single match. If regex groups are used, match_str.f(1, 1) is the whole match, match_str.f(1, 2) is the first group, etc.
	//
	// `// Decode hex escape codes.
	//  let v1 = R"(--\0x3B--\0x2F--)";                                 // Hex escape codes.
	//  let v2 = v1.replace(
	//      R"(\\0x[0-9a-fA-F]{2,2})"_rex,                              // Find \0xFF.
	//      [](auto match_str) {return match_str.cut(3).iconv("HEX");}  // Decode to a char.
	//  );
	//  assert(v2 == "--;--/--");`
	//
	// `// Reformat dates using groups.
	//  let v3 = "Date: 03-15-2025";
	//  let v4 = v3.replace(
	//      R"((\d{2})-(\d{2})-(\d{4}))"_rex,
	//      [](auto match_str) {return match_str.f(1, 4) ^ "-" ^ match_str.f(1, 2) ^ "-" ^ match_str.f(1, 3);}
	//  );
	//  assert(v4 == "Date: 2025-03-15");`
	//
	var replace(const rex& regex, ReplacementFunction auto repl_func) const {

		// Lambda to bridge the callable to a function pointer + context
		struct Context {decltype(repl_func)* lambda;};
		Context ctx{&repl_func};

		// Non-capturing lambda to adapt the callable to the function pointer signature. [[Undocumented]]
		auto bridge = [](const var& match_str, void* ctx) -> var {
			auto* context = static_cast<Context*>(ctx);
			return (*context->lambda)(match_str); // Invoke the original lambda
		};

		return replace(regex, bridge, &ctx);
	}

	// Remove duplicate fields, values or subvalues.
	// From a dynamic array.
	//
	// `let v1 = "a1^b2^a1^c2"_var.unique(); // "a1^b2^c2"_var
	//  // or
	//  let v2 = unique("a1^b2^a1^c2"_var);`
	//
	ND var  unique() const&;

	// Reorder fields, values or subvalues.
	// In a dynamic array.
	// Numeric data:
	// `let v1 = "20^10^2^1^1.1"_var.sort(); // "1^1.1^2^10^20"_var
	//  // or
	//  let v2 = sort("20^10^2^1^1.1"_var);`
	// Alphabetic data:
	//  `let v1 = "b1^a1^c20^c10^c2^c1^b2"_var.sort(); // "a1^b1^b2^c1^c10^c2^c20"_var
	//  // or
	//  let v2 = sort("b1^a1^c20^c10^c2^c1^b2"_var);`
	//
	ND var  sort(SV delimiter = _FM) const&;

	// Reorder fields in an FM or VM etc. separated list in descending order
	//
	// `let v1 = "20^10^2^1^1.1"_var.reverse(); // "1.1^1^2^10^20"_var
	//  // or
	//  let v2 = reverse("20^10^2^1^1.1"_var);`
	//
	ND var  reverse(SV delimiter = _FM) const&;

	// Randomise the order of fields in an FM, VM separated list
	//
	// `let v1 = "20^10^2^1^1.1"_var.randomize(); /// e.g. "2^1^20^1.1^10" (random order depending on initrand())
	//  // or
	//  let v2 = randomize("20^10^2^1^1.1"_var);`
	//
	ND var  randomize(SV delimiter = _FM) const&;

	// Split a delimited string into a dynamic array.
	// Replace separator chars with FM chars except inside double or single quotes and ignoring escaped quotes \" \'
	// return: A dynamic array
	// Can be used to process CSV data.
	//
	// `let v1 = "abc,\"def,\"123\" fgh\",12.34"_var.parse(','); // "abc^\"def,\"123\" fgh\"^12.34"_var
	//  // or
	//  let v2 = parse("abc,\"def,\"123\" fgh\",12.34", ',');`
	//
	ND var  parse(char sepchar = ' ') const& {var nrvo = this->clone(); nrvo.parser(sepchar); return nrvo;}

	// Split a delimited string into a dim array.
	// Delimiter: Can be multibyte Unicode.
	// return: A dim array.
	//
	// `dim d1 = "a^b^c"_var.split(); // A dimensioned array with three elements (vars)
	//  // or
	//  dim d2 = split("a^b^c"_var);`
	//
	ND dim  split(SV delimiter = _FM) const;

	// Unpack a delimited string into N new vars.
	// Useful when marshalling multiple vars as a single string across interfaces.
	// Note the unusual syntax requiring literal angle brackets.
	// [a,b,c...]: New vars to be constructed.
	// N: Must agree with the number of vars being created or returned.
	// delim: Defaults to FM but can be multichar/unicode string. e.g. ",", VM, "| |", or "⭕".
	// return: A C++ object enabling inline creation of vars.
	// Prefix with const to make all the new vars const; partial const declaration is not supported.
	//
	// `let a = "aa", b = "bb";
	//  let pack1 = {a, b}; // "aa^bb"_var
	//  auto [a2, b2, c2] = pack1.unpack<3>(); // a2 -> "aa" // b2 -> "bb" // c2 -> unassigned
	//  // or
	//  auto [a3, b3, c3] = unpack<3>(pack1);
	//
	//  // Similar to:
	//  auto funcx = []() -> std::array<var, 2> {
	//      let a = "aa", b = "bb";
	//      return std::array{a, b};
	//  //  return unpack("aa^bb"_var);
	//  };
	//  auto [a4, b4] = funcx();`
	//
	template <size_t N>
	auto unpack/*<N>*/(SV delim = _FM) const -> std::array<var, N> {
        THISIS("auto var::unpack<N>(SV delim = _FM) const")
        assertString(function_sig);

		// std::vector<var> vv1 = this->split(delim);
		// Utility somewhere in the forest
		auto basic_split(in v1, SV delim) -> std::vector<var>;
		auto vv1 = basic_split(var_str, delim);
		return [&vv1]<size_t... Is>(std::index_sequence<Is...>) {
    		return std::array<var, N>{
        		(Is < vv1.size() ? std::move(vv1[Is]) : std::move(var()))...
    		};
		}(std::make_index_sequence<N>{});
	}

	// SAME ON TEMPORARIES - CALL MUTATORS FOR SPEED (not documenting since programmer interface is the same)
	/////////////////////////////////////////

	// UTF8/byte as for accessors

// clang-format off

// Many of the non-mutating functions are forwarded with a clone to the mutating function

// On temporaries the mutator function is called to avoid creating a temporary in many cases

	ND var  ucase()                             && {ucaser();     return std::move(*this);}
	ND var  lcase()                             && {lcaser();     return std::move(*this);}
	ND var  tcase()                             && {tcaser();     return std::move(*this);}
	ND var  fcase()                             && {fcaser();     return std::move(*this);}
	ND var  normalize()                         && {normalizer(); return std::move(*this);}
	ND var  invert()                            && {inverter();   return std::move(*this);}

	ND var  lower()                             && {lowerer();    return std::move(*this);}
	ND var  raise()                             && {raiser();     return std::move(*this);}
	ND var  crop()                              && {cropper();    return std::move(*this);}

	ND var  quote()                             && {quoter();     return std::move(*this);}
	ND var  squote()                            && {squoter();    return std::move(*this);}
	ND var  unquote()                           && {unquoter();   return std::move(*this);}

	ND var  trim(     SV trimchars = " ")       && {trimmer(trimchars);      return std::move(*this);}
	ND var  trimfirst(SV trimchars = " ")       && {trimmerfirst(trimchars); return std::move(*this);}
	ND var  trimlast( SV trimchars = " ")       && {trimmerlast(trimchars);  return std::move(*this);}
	ND var  trimboth( SV trimchars = " ")       && {trimmerboth(trimchars);  return std::move(*this);}

	ND var  first()                             && {firster();               return std::move(*this);}
	ND var  last()                              && {laster();                return std::move(*this);}
	ND var  first(const std::size_t length)     && {firster(length);         return std::move(*this);}
	ND var  last( const std::size_t length)     && {laster(length);          return std::move(*this);}
	ND var  cut(  const int length)             && {cutter(length);          return std::move(*this);}
	ND var  paste(const int pos1, const int length, SV replacestr)
                                                && {paster(pos1, length, replacestr);    return std::move(*this);}
	ND var  paste(const int pos1, SV insertstr) && {paster(pos1, insertstr);             return std::move(*this);}
	ND var  prefix(               SV prefixstr) && {prefixer(prefixstr);                 return std::move(*this);}
	ND var  pop()                               && {popper();                            return std::move(*this);}

//	ND var  append(SV appendstr)                && {appender(appendstr);                 return std::move(*this);}
//	template <typename... ARGS>
//	ND var  append(const ARGS&... appendable) && {
//				this->createString();
//				(var_str += ... += appendable);
	//
	ND var  append(const auto&... appendable)   && {((*this) ^= ... ^= appendable);      return std::move(*this);}

//    // Helper to append one argument, handling var differently
//    template<typename T>
//    void append_one(const T& value) {
//        if constexpr (std::is_same_v<std::decay_t<T>, var>) {
//            var_str += static_cast<const std::string&>(value);  // Explicit cast to string&
//        } else {
//            var_str += value;  // Other types (string, int, etc.)
//        }
//    }
//	ND var  append(const auto&... appendable) && {
////				((*this) ^= ... ^= appendable);
//				this->createString();
//				//(var_str += ... += appendable);
//				(append_one(appendable), ...);  // Comma fold—calls append_one per arg
//				return std::move(*this);
//			}

	ND var  fieldstore(SV delimiter, const int fieldno, const int nfields, in replacement)
                                                      && {fieldstorer(delimiter, fieldno, nfields, replacement);
                                                                                             return std::move(*this);}

	ND var  substr(const int pos1, const int length)  && {substrer(pos1, length);              return std::move(*this);}
	ND var  substr(const int pos1)                    && {substrer(pos1);                      return std::move(*this);}

	ND var  convert(    SV fromchars, SV tochars)     && {this->converter(fromchars, tochars); return std::move(*this);}
	ND var  textconvert(SV fromchars, SV tochars)     && {textconverter(fromchars, tochars);   return std::move(*this);}
	ND var  replace(    SV fromstr,   SV tostr)       && {replacer(fromstr, tostr);            return std::move(*this);}
	ND var  replace(const rex& regex, SV replacement) && {replacer(regex, replacement);        return std::move(*this);}
//	ND var  replace(const rex& regex, ReplacementFunction auto repl_func)
//	                                                  && {replacer(regex,repl_func);    return std::move(*this);}

	ND var  unique()                                  && {uniquer();           return std::move(*this);}
	ND var  sort(   SV delimiter = _FM)               && {sorter(delimiter);   return std::move(*this);}
	ND var  reverse(SV delimiter = _FM)               && {reverser(delimiter); return std::move(*this);}
	ND var  randomize(SV delimiter = _FM)               && {randomizer(delimiter); return std::move(*this);}
	ND var  parse(char delimiter = ' ')               && {parser(delimiter);   return std::move(*this);}

	///// STRING MUTATION - Standalone commands:
	////////////////////////////////////////////

	// obj is strvar

	// To upper case
	// All string mutators follow the same pattern as ucaser. See the non-mutating functions for details.
	//
	// `var v1 = "abc";
	//  v1.ucaser(); // "ABC"
	//  // or
	//  ucaser(v1);`
	//
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

				//false && (*this);
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
	   IO   replacer(const rex& regex, ReplacementFunction auto repl_func) REF {*this = replace(regex, repl_func);}
	   IO   replacer(SV fromstr, SV tostr) REF;
//	   IO   regex_replacer(SV regex, SV replacement, SV regex_options = "") REF ;

	   IO   uniquer() REF ;
	   IO   sorter(SV delimiter = _FM) REF ;
	   IO   reverser(SV delimiter = _FM) REF ;
	   IO   randomizer(SV delimiter = _FM) REF ;
	   IO   parser(char sepchar = ' ') REF ;

	///// I/O CONVERSION:
	////////////////////

	// obj is var

	// Convert internal data to output external display format.
	// convstr: A conversion code or pattern. See ICONV/OCONV PATTERNS
	// return: The data in external display format or, if the data is invalid and cannot converted, most conversions return the original data UNCONVERTED.
	// throw: VarNotImplemented if convstr is invalid
	//
	// `let v1 = var(30123).oconv("D/E"); // "21/06/2050"
	//  // or
	//  let v2 = oconv(30123, "D/E");`
	//
	ND var  oconv(const char* convstr) const;

	// Convert external data to internal format.
	// convstr: A conversion code or pattern. See ICONV/OCONV PATTERNS
	// return: The data in internal format or, if the data is invalid and cannot be converted, most conversions return the EMPTY STRING ""
	// throw: VarNotImplemented if convstr is invalid
	//
	// `let v1 = "21 JUN 2050"_var.iconv("D/E"); // 30123
	//  // or
	//  let v2 = iconv("21 JUN 2050", "D/E");`
	//
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

	// Classic format function in printf style
	// vars can be formatted either with C++ format codes e.g. {:_>8.2f}
	// or with exodus oconv codes e.g. {::MD20P|R(_)#8} as in the below example.
	//
	// `let v1 = var(12.345).format("'{:_>8.2f}'"); // "'___12.35'"
	//  let v2 = var(12.345).format("'{::MD20P|R(_)#8}'");
	//  // or
	//  var v3 = format("'{:_>8.2f}'", var(12.345)); // "'___12.35'"
	//  var v4 = format("'{::MD20P|R(_)#8}'", var(12.345));`
	//
	template<class... Args>
	ND var  format(in fmt_str, Args&&... args) const
	{
		THISIS("var  var::format(SV fmt_str, Args&&... args) const")
		assertString(function_sig);
		// *this becomes the first format argument and any additional arguments become additionl format arguments
		return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(*this, args...) );
	}

#endif //EXO_FORMAT

	// obj is strvar

	// Convert from codepage encoded text to UTF-8 encoded exodus text
	// codepage: e.g. Codepage "CP1124" (Ukrainian).
	// Use Linux command "iconv -l" for a complete list of code pages and encodings.
	//
	// `let v1 = "\xa4"_var.from_codepage("CP1124"); // "Є"
	//  // or
	//  let v2 = from_codepage("\xa4", "CP1124");
	//  // U+0404 Cyrillic Capital Letter Ukrainian Ie Unicode character`
	//
	ND var  from_codepage(const char* codepage) const;

	// Convert to codepage encoded text from exodus UTF-8 encoded text
	//
	// `let v1 = "Є"_var.to_codepage("CP1124").oconv("HEX"); // "A4"
	//  // or
	//  let v2 = to_codepage("Є", "CP1124").oconv("HEX");`
	//
	ND var  to_codepage(const char* codepage) const;

	///// DYNAMIC ARRAY FUNCTIONS:
	/////////////////////////////

	// obj is strvar

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
	//
	ND var  f(const int fieldno, const int valueno = 0, const int subvalueno = 0)            const;

	// Extract a specific field, value or subvalue from a dynamic array.
	//
	// `let v1 = "f1^f2v1]f2v2]f2v3^f2"_var;
	//  let v2 = v1.extract(2, 2); // "f2v2"
	//  //
	//  // For brevity the function alias "f()" (standing for "field") is normally used instead of "extract()" as follows:
	//  var v3 = v1.f(2, 2);`
	//
	ND var  extract(const int fieldno, const int valueno = 0, const int subvalueno = 0)      const {return this->f(fieldno, valueno, subvalueno);}

	// UPDATE

	// This function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better. It was called replace() in Pick Basic but we are now using "replace()" to
	// change substrings using regex (similar to the old Pick Basic replace function) its mutator function
	// is .updater()
	// Replace a specific subvalue in a dynamic array. Normally one uses the updater() function to replace in place.

	// Update (replace or insert) a specific subvalue  in a dynamic array.
	// Same as var.updater() function but returns a new string instead of updating a variable in place. Rarely used.
	// "update()" was called "replace()" in Pick OS/Basic.
	ND var  update(const int fieldno, const int valueno, const int subvalueno, in replacement) const& {var nrvo = this->clone(); nrvo.updater(fieldno, valueno, subvalueno, replacement); return nrvo;}

	// Update (replace or insert) a specific value in a dynamic array.
	ND var  update(const int fieldno, const int valueno, in replacement)                    const& {var nrvo = this->clone(); nrvo.updater(fieldno, valueno, 0, replacement); return nrvo;}

	// Update (replace or insert) a specific field a dynamic array.
	ND var  update(const int fieldno, in replacement)                                       const& {var nrvo = this->clone(); nrvo.updater(fieldno, 0, 0, replacement); return nrvo;}

	// INSERT

	// Insert a subvalue in a dynamic array.
	// Same as var.inserter() function but returns a new string instead of updating a variable in place.
	ND var  insert(const int fieldno, const int valueno, const int subvalueno, in insertion) const& {var nrvo = this->clone(); nrvo.inserter(fieldno, valueno, subvalueno, insertion); return nrvo;}

	// Insert a value in a dynamic array.
	ND var  insert(const int fieldno, const int valueno, in insertion)                      const& {var nrvo = this->clone(); nrvo.inserter(fieldno, valueno, 0, insertion); return nrvo;}

	// Insert a field in a dynamic array.
	ND var  insert(const int fieldno, in insertion)                                         const& {var nrvo = this->clone(); nrvo.inserter(fieldno, 0, 0, insertion); return nrvo;}

	// REMOVE

	// Remove a field, value or subvalue from a dynamic array.
	// Same as var.remover() function but returns a new string instead of updating a variable in place.
	// "remove()" was called "delete()" in Pick OS/Basic.
	ND var  remove(const int fieldno, const int valueno = 0, const int subvalueno = 0)      const& {var nrvo = this->clone(); nrvo.remover(fieldno, valueno, subvalueno); return nrvo;}

	// SAME AS ABOVE ON TEMPORARIES TO USE MUTATING (not documented because used difference in implementation is irrelevant to exodus users)
	///////////////////////////////////////////////

	ND var   update(const int fieldno, const int valueno, const int subvalueno, in replacement) && {this->updater(fieldno, valueno, subvalueno, replacement); return std::move(*this);}
	ND var   update(const int fieldno, const int valueno, in replacement)                       && {this->updater(fieldno, valueno, 0, replacement); return std::move(*this);}
	ND var   update(const int fieldno, in replacement)                                          && {this->updater(fieldno, 0, 0, replacement); return std::move(*this);}

	ND var   insert(const int fieldno, const int valueno, const int subvalueno, in insertion)   && {this->inserter(fieldno, valueno, subvalueno, insertion); return std::move(*this);}
	ND var   insert(const int fieldno, const int valueno, in insertion)                         && {this->inserter(fieldno, valueno, 0, insertion); return std::move(*this);}
	ND var   insert(const int fieldno, in insertion)                                            && {this->inserter(fieldno, 0, 0, insertion); return std::move(*this);}

	ND var   remove(const int fieldno, const int valueno = 0, const int subvalueno = 0)         && {this->remover(fieldno, valueno, subvalueno); return std::move(*this);}

	///// DYNAMIC ARRAY FILTERS:
	///////////////////////////

	// obj is strvar

	// Sum up multiple values in a dynamic array.
	// Whatever is the lowest level is summed up into a higher level.
	//
	// `let v1 = "1]2]3^4]5]6"_var.sum(); // "6^15"_var
	//  // or
	//  let v2 = sum("1]2]3^4]5]6"_var);`
	//
	ND var  sum() const;

	// Sum up everything in a dynamic array.
	//
	// `let v1 = "1]2]3^4]5]6"_var.sumall(); // 21
	//  // or
	//  let v2 = sumall("1]2]3^4]5]6"_var);`
	//
	ND var  sumall() const;

	// Sum all fields using a given delimiter.
	//
	// `let v1 = "10,20,30"_var.sum(","); // 60
	//  // or
	//  let v2 = sum("10,20,30", ",");`
	//
	ND var  sum(SV delimiter) const;

	// Calculate basic statistics include stddev.
	// return: An FM delimited string containing n, tot, min, max, tot, mean and stddev
	// strvar: A dynamic array containing numbers using any field, value or subvalue mark delimiters.
	//
	// `let v1 = "-11.2^0^11.5^12^13.9^14"_var.stddev(); // "6^40.2^-11.2^14^6.7^9.32344714506"_var
	//  // or
	//  let v2 = stddev("-11.2^0^11.5^12^13.9^14"_var);`
	//
	ND var  stddev() const;

	// Binary ops on parallel multivalues
	//
	// opcode:
	// * + * Addition
	// * - * Subtraction
	// * ∗ * Multiplication
	// * / * Division
	//
	// `let v1 = "10]20]30"_var.mv("+","2]3]4"_var); // "12]23]34"_var`
	//
	ND var  mv(const char* opcode, in var2) const;
	//
	// --- NOTE ABOVE --> ∗ <-- is not a * asteriskfg. It is Unicode character U+2217 ∗ ASTERISK OPERATOR
	// to enable parsing of *?* correctly

	///// DYNAMIC ARRAY MUTATORS Standalone commands:
	////////////////////////////

	// obj is strvar

	// Mutable versions update lvalue vars and dont return anything so that they cannot be chained. This is to prevent accidental misuse and bugs.
	//
	// Pick Basic
	//   xyz<10> = "abc";
	// becomes in exodus c++
	//   xyz(10) = "abc";
	// or
	//   xyz.updater(10, "abc");
	// or
	//   updater(xyz, 10, "abc");

	// Replace a specific field in a dynamic array
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.updater(2, "X"); // "f1^X^f3"_var
	//  // or
	//  v1(2) = "X"; /// Easiest.
	//  // or
	//  updater(v1, 2, "X");`
	//
	   IO   updater(const int fieldno, in replacement) REF {this->updater(fieldno, 0, 0, replacement); return THIS;}

	// Replace a specific value in a dynamic array.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.updater(2, 2, "X"); // "f1^v1]X^f3"_var
	//  // or
	//  v1(2, 2) = "X"; /// Easiest.
	//  // or
	//  updater(v1, 2, 2, "X");`
	//
	   IO   updater(const int fieldno, const int valueno, in replacement) REF {this->updater(fieldno, valueno, 0, replacement); return THIS;}

	// Replace a specific subvalue in a dynamic array.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.updater(2, 2, 2, "X"); // "f1^v1]v2}X}s3^f3"_var
	//  // or
	//  v1(2, 2, 2) = "X"; /// Easiest.
	//  // or
	//  updater(v1, 2, 2, 2, "X");`
	//
	   IO   updater(const int fieldno, const int valueno, const int subvalueno, in replacement) REF;

	// Insert a specific field in a dynamic array
	// All other fields are moved up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, "X"); // "f1^X^v1]v2}s2}s3^f3"_var
	//  // or
	//  inserter(v1, 2, "X");`
	//
	   IO   inserter(const int fieldno, in insertion) REF {this->inserter(fieldno, 0, 0, insertion); return THIS;}

	// Insert a specific value in a dynamic array.
	// All other values are moved up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, "X"); // "f1^v1]X]v2}s2}s3^f3"_var
	//  // or
	//  inserter(v1, 2, 2, "X");`
	//
	   IO   inserter(const int fieldno, const int valueno, in insertion) REF {this->inserter(fieldno, valueno, 0, insertion); return THIS;}

	// Insert a specific subvalue in a dynamic array.
	// All other subvalues are moved up.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.inserter(2, 2, 2, "X"); // "f1^v1]v2}X}s2}s3^f3"_var
	//  // or
	//  v1.inserter(2, 2, 2, "X");`
	//
	   IO   inserter(const int fieldno, const int valueno, const int subvalueno, in insertion) REF;

	// Remove a specific field, value, or subvalue from a dynamic array.
	// All other fields, values, or subvalues are moved down.
	//
	// `var v1 = "f1^v1]v2}s2}s3^f3"_var;
	//  v1.remover(2, 2); // "f1^v1^f3"_var
	//  // or
	//  remover(v1, 2, 2);`
	//
	   IO   remover(const int fieldno, const int valueno = 0, const int subvalueno = 0) REF;

	//-er version could be extract and erase in one go
	// IO   extracter(int fieldno, int valueno=0, int subvalueno=0) const;

	///// DYNAMIC ARRAY SEARCH:
	//////////////////////////

	// obj is strvar

	// LOCATE

	// locate substr in dynamic array
	// Search unordered fields, values and/or subvalues.
	// With only the substr argument provided, locate searches regardless of the field mark delimiters present.
	// return: The field, value or subvalue number if found or 0 if not. Fields are counted regardless of delimiter.
	// Searching for empty fields, values etc. (i.e. "") will work. Locating "" in "]yy" will return 1, in "xx]]zz" 2, and in "xx]yy]" 3, however, locating "" in "xx" will return 0 because there is conceptually no empty value in "xx". Locate "" in "" will return 1.
	//
	// `if ("UK^US^UA"_var.locate("US")) ... ok // 2
	//  // or
	//  if (locate("US", "UK^US^UA"_var)) ... ok`
	//
	ND var locate(in substr) const;

	// locate substr in values.
	// Search unordered VM delimited string.
	// valueno[out]: Value number if found or the max value number + 1 if not. If not found then valueno [out] is suitable for creating a new value.
	// return: True if found or False if not.
	//
	// `var valueno;
	//  if ("UK]US]UA"_var.locate("US", valueno)) ... ok // valueno -> 2
	//  // or
	//  if (locate("US", "UK]US]UA"_var,valueno)) ... ok`
	//
	ND bool locate(in substr, out valueno) const;

	// locate substr in dynamic array.
	// Search in an unordered dynamic array.
	// fieldno: If fieldno is non-zero then search the specified field number otherwise, if fieldno is 0, search using FM as delimiter.
	// valueno: If provided, search the specified value number for a subvalue.
	// num[out]: If found, the field, value or subvalue number where it was found. If not found, the max field, value or subvalue number + 1. If not found then num [out] is suitable for creating a new field, value or subvalue.
	// return: True if found or False if not.
	//
	// `var num;
	//  if ("f1^f2v1]f2v2]s1}s2}s3}s4^f3^f4"_var.locate("s4", num, 2, 3)) ... ok // num -> 4 // Return true`
	//
	ND bool locate(in substr, out num, const int fieldno, const int valueno = 0) const;

	// LOCATE BY

	// locate substr in ordered dynamic array
	// locateby() without fieldno or valueno arguments, searches ordered values separated by VM chars.
	// Data must already be in the correct order for searching to work properly.
	// ordercode:
	// * AL * Ascending  - Left Justified (Alphabetic)
	// * DL * Descending - Left Justified (Alphabetic)
	// * AR * Ascending  - Right Justified (Numeric/Natural)
	// * DR * Descending - Right Justified (Numeric/Natural)
	// return: True if found, otherwise false.
	// valueno[out]: Either the value no found or the correct value no for inserting the substr
	//
	// `var valueno; if ("aaa]bbb]ccc"_var.locateby("AL", "bb", valueno)) ... // valueno -> 2 // Return false and valueno = where it could be correctly inserted.`
	//
	ND bool locateby(const char* ordercode, in substr, out valueno) const;

	// locate substr in ordered dynamic array
	// locateby() with fieldno and/or valueno arguments, searches fields if fieldno is 0, or values in a specific fieldno, or subvalues in a specific valueno.
	// For more info, see locateby() without fieldno or valueno arguments.
	//
	// `var num;
	//  if ("f1^f2^aaa]bbb]ccc^f4"_var.locateby("AL", "bb", num, 3)) ... // num -> 2 // return false and where it could be correctly inserted.`
	//
	ND bool locateby(const char* ordercode, in substr, out num, const int fieldno, const int valueno = 0) const;

	// LOCATE USING

	// locate substr using any delimiter.
	//
	// `if ("AB,EF,CD"_var.locateusing(",", "EF")) ... ok`
	//
	ND bool locateusing(const char* usingchar, in substr) const;

	// locate substr in dynamic array using any delimiter
	// Search in a specific field, value or subvalue.
	// num[out]:  If found, the number where found, otherwise the maximum number of delimited fields + 1.
	// return: True if found and False if not.
	// This is similar to the main locate command but the delimiter char can be specified e.g. a comma or TM etc.
	//
	// `var num;
	//  if ("f1^f2^f3c1,f3c2,f3c3^f4"_var.locateusing(",", "f3c2", num, 3)) ... ok // num -> 2 // Return true`
	//
	ND bool locateusing(const char* usingchar, in substr, out num, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// LOCATE BY, USING

	// locatebyusing() supports all the locate features in a single function.
	ND bool locatebyusing(const char* ordercode, const char* usingchar, in substr, out num, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	///// DATABASE ACCESS:
	/////////////////////

	// obj is dbconn

	// Establish a connection to a database.
	// conninfo: The DB connection string parameters are merged from the following places in descending priority.
	// 1. Provided in connect()'s conninfo argument. See the last option. for the complete list of parameters.
	// 2. Any environment variables EXO_HOST EXO_PORT EXO_USER EXO_DATA EXO_PASS EXO_TIME
	// 3. Any parameters found in a configuration file at ~/.config/exodus/exodus.cfg
	// 4. The default conninfo is "host=127.0.0.1 port=5432 dbname=exodus user=exodus password=somesillysecret connect_timeout=10"
	// Setting environment variable EXO_DBTRACE=1 will cause tracing of DB interface including SQL commands.
	// dbconn[out]: Becomes a reference or handle for future functions that require a connection argument.
	// For all the various DB function calls, the dbconn or operative var can be either:
	// * A DB connection created with dbconnect().
	// * A file var created with open().
	// * Any var. A default connection will be established on the fly.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.connect("dbname=exodus user=exodus password=somesillysecret")) ...;
	//  // or
	//  if (not connect()) ...
	//  // or
	//  if (not connect("exodus")) ...`
	//
	ND bool connect(in conninfo = "");

	// Attach filename(s) to a specific DB connection.
	// Any following use of the given filename(s) without specifying a connection will be directed to the specified connection until process termination.
	// It is not necessary to attach files before opening them but the act of opening them also attaches them.
	// The files must exist in the specified connection.
	// Attachments can changed by calling attach() or open() on a different connection or they can be removed by calling detach().
	// dbconn: The connection to which the filename(s) should be attached. Defaults to the default connection.
	// filenames: FM separated list.
	// return: False if any filename does not exist and cannot be opened on the given connection. All filenames that can be opened on the connection are attached even if some cannot.
	// Internally, attach merely opens each filename on the given connection causing them to be added to an internal cache.
	//
	// `var dbconn = "exodus";
	//  let filenames = "xo_clients^dict.xo_clients"_var;
	//  if (dbconn.attach(filenames)) ... ok
	//  // or
	//  if (attach(filenames)) ... ok`
	//
	ND bool attach(in filenames) const;

	// Remove files from the internal cache created by previous open() and attach() calls.
	// filenames: FM separated list.
	//
	   void detach(in filenames);

	// Begin a DB transaction.
	// return:
	// * True  * Successfully begun.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.begintrans()) ...
	//  // or
	//  if (not begintrans()) ...`
	//
	ND bool begintrans() const;

	// Check if a DB transaction is in progress.
	// return: True or False.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.statustrans()) ... ok
	//  // or
	//  if (statustrans()) ... ok`
	//
	ND bool statustrans() const;

	// Rollback a DB transaction.
	// return:
	// * True  * Successfully rolled back or there was no transaction in progress.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.rollbacktrans()) ... ok
	//  // or
	//  if (rollbacktrans()) ... ok`
	//
	ND bool rollbacktrans() const;

	// Commit a DB transaction.
	// return:
	// * True  * Successfully committed or there was no transaction in progress.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.committrans()) ... ok
	//  // or
	//  if (committrans()) ... ok`
	//
	ND bool committrans() const;

	// Execute an sql command.
	// return:
	// * True  * Successfully executed.
	// * False * DB refused. See lasterror() for info.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.sqlexec("select 1")) ... ok
	//  // or
	//  if (sqlexec("select 1")) ... ok`
	//
	ND bool sqlexec(in sqlcmd) const;

	// Execute an SQL command and capture the response.
	// return:
	// * True  * Successfully executed.
	// * False * DB refused. See lasterror() for info.
	// response: Any rows and columns returned are separated by RM and FM respectively. The first row is the column names.
	// It is *ecommended* that you do not use sql directly unless you must, perhaps to manage or configure a database.
	//
	// `var dbconn = "exodus";
	//  let sqlcmd = "select 'xxx' as col1, 'yyy' as col2";
	//  var response;
	//  if (dbconn.sqlexec(sqlcmd, response)) ... ok // response -> "col1^col2\x1fxxx^yyy"_var /// \x1f is the Record Mark (RM) char. The backtick char is used here by gendoc to deliminate source code.
	//  // or
	//  if (sqlexec(sqlcmd, response)) ... ok`
	//
	ND bool sqlexec(in sqlcmd, io response) const;

	// Close a DB connection
	// Free process resources both locally and in the database server.
	//
	// `var dbconn = "exodus";
	//  dbconn.disconnect();
	//  // or
	//  disconnect();`
	//
	   void disconnect();

	// Close all DB connections
	// Free process resources both locally and in the database server(s).
	// All connections are closed automatically when a process terminates.
	//
	// `var dbconn = "exodus";
	//  dbconn.disconnectall();
	//  // or
	//  disconnectall();`
	//
	   void disconnectall();

	// return: The last OS or DB error message.
	// obj is var()
	//
	// `var v1 = var::lasterror();
	//  // or
	//  var v2 = lasterror();`
	//
	ND static var  lasterror();

	// Set the lasterror() message.
	   static void  setlasterror(in msg);

	// Log the last OS or DB error message.
	// Output: To stdlog
	// Prefix the output with source if provided.
	// obj is var()
	//
	// `var::loglasterror("main:");
	//  // or
	//  loglasterror("main:");`
	//
	   static void  loglasterror(in source = "");

	///// DATABASE MANAGEMENT:
	/////////////////////////

	// obj is dbconn

	// Create a named database on a particular connection.
	// Optionally copies an existing database from the same connection
	// return: True or False. See lasterror() for errors.
	// * Target database must not already exist.
	// * Source database must exist on the same connection.
	// * Source database cannot have any current connections.
	//
	// `var dbconn = "exodus";
    //  if (not dbdelete("xo_gendoc_testdb")) {}; // Cleanup first
	//  if (dbconn.dbcreate("xo_gendoc_testdb")) ... ok
	//  // or
	//  if (dbcreate("xo_gendoc_testdb")) ...`
	//
	ND bool dbcreate(in new_dbname, in old_dbname = "") const;

	// Create a named database as a copy of an existing database.
	// return: True or False. See lasterror() for errors.
	// * Target database must not already exist.
	// * Source database must exist on the same connection.
	// * Source database cannot have any current connections.
	//
	// `var dbconn = "exodus";
    //  if (not dbdelete("xo_gendoc_testdb2")) {}; // Cleanup first
	//  if (dbconn.dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) ... ok
	//  // or
	//  if (dbcopy("xo_gendoc_testdb", "xo_gendoc_testdb2")) ...`
	//
	ND bool dbcopy(in from_dbname, in to_dbname) const;

	// Get a list of available databases.
	// A list for a given connection or the default connection.
	// return: An FM delimited list.
	//
	// `var dbconn = "exodus";
	//  let v1 = dbconn.dblist();
	//  // or
	//  let v2 = dblist();`
	//
	ND var  dblist() const;

	// Delete (drop) a named database.
	// return: True or False. See lasterror() for errors.
	// * Database does not exist
	// * Database has active connections.
	//
	// `var dbconn = "exodus";
	//  if (dbconn.dbdelete("xo_gendoc_testdb2")) ... ok
	//  // or
	//  if (dbdelete("xo_gendoc_testdb2")) ...`
	//
	ND bool dbdelete(in dbname) const;

	// Create a named DB file.
	// filenames ending with "_temp" only last until the connection is closed.
	// return: True or False. See lasterror() for errors.
	// * Filename is invalid.
	// * Filename already exists.
	//
	// `let filename = "xo_gendoc_temp", dbconn = "exodus";
	//  if (dbconn.createfile(filename)) ... ok
	//  // or
	//  if (createfile(filename)) ...`
	//
	ND bool createfile(in filename) const;

	// Rename a DB file.
	// return: True or False. See lasterror() for errors.
	// * source filename does not exist.
	// * newfilename is invalid.
	// * newfilename already exists.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp", new_filename = "xo_gendoc_temp2";
	//  if (dbconn.renamefile(filename, new_filename)) ... ok
	//  // or
	//  if (renamefile(filename, new_filename)) ...`
	//
	ND bool renamefile(in filename, in newfilename) const;

	// Get a list of all files in a database.
	// return: An FM separated list.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.listfiles()) ...
	//  // or
	//  if (not listfiles()) ...`
	//
	ND var  listfiles() const;

	// Delete all records in a DB file.
	// return: True or False. See lasterror() for errors.
	// * filename does not exist.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp2";
	//  if (not dbconn.clearfile(filename)) ...
	//  // or
	//  if (not clearfile(filename)) ...`
	//
	ND bool clearfile(in filename) const;

	// Delete a DB file.
	// return: True or False. See lasterror() for errors.
	// * filename does not exist.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp2";
	//  if (dbconn.deletefile(filename)) ... ok
	//  // or
	//  if (deletefile(filename)) ...`
	//
	ND bool deletefile(in filename) const;

	// obj is conn_or_file

	// Get the approx number of records in a DB file.
	// Might return -1 if not known.
	// Not very accurate inside transactions.
	// return: An approximate number.
	//
	// `let dbconn = "exodus", filename = "xo_clients";
	//  var nrecs1 = dbconn.reccount(filename);
	//  // or
	//  var nrecs2 = reccount(filename);`
	//
	ND var  reccount(in filename = "") const;

	// Call the DB maintenance function.
	// For one file or all files.
	// Ensure that reccount() function is reasonably accurate.
	// Despite the name, this doesnt flush any index.
	// return: True or False. See lasterror() for errors.
	// * Cannot perform maintenance while a transaction is active.
	//
	   bool flushindex(in filename = "") const;

	///// DATABASE FILE I/O:
	///////////////////////

	// obj is file

	// Open a DB file.
	// To a var which can be used in subsequent DB function calls to access a specific file using a specific connection.
	// dbconn: If dbconn is *not* specified, and the filename is present in an internal cache of filenames and connections created by previous calls to open() or attach() then open() returns true. If it is not present in the cache then the default connection will be checked.
	// return: See lasterror().
	// * True  * The filename was present in the cache OR the DB connection reports that the file is present.
	// * False * The DB connection reports that the file does not exist.
	//
	// `var file, filename = "xo_clients";
	//  if (not file.open(filename)) ...
	//  // or
	//  if (not open(filename to file)) ...`
	//
	ND bool open(in dbfilename, in dbconn = "");

	// Close a DB file.
	// Does nothing currently since database file vars consume no resources
	//
	// `var file = "xo_clients";
	//  file.close();
	//  // or
	//  close(file);`
	//
	   void close() const;

	// Create a secondary index.
	// For a given DB file and field name.
	// The fieldname must exist in a dictionary file. The default dictionary is "dict." ^ filename.
	// return: False if the index cannot be created for any reason. See lasterror().
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
	//
	ND bool createindex(in fieldname, in dictfile = "") const;

	// List secondary indexes.
	// In a database or for a DB file.
	// return: An FM separated list.
	// * DB file or fieldname does not exist.
	// obj is file|dbconn
	//
	// `var dbconn = "exodus";
	//  if (dbconn.listindex()) ... ok // include "xo_clients__date_created"
	//  // or
	//  if (listindex()) ... ok`
	//
	ND var  listindex(in file_or_filename = "", in fieldname = "") const;

	// Delete a secondary index.
	// For a DB file and field name.
	// return: False if the index cannot be deleted for any reason. See lasterror().
	// * File does not exist
	// * Index does not already exists
	//
	// `var file = "xo_clients", fieldname = "DATE_CREATED";
	//  if (file.deleteindex(fieldname)) ... ok
	//  // or
	//  if (deleteindex(file, fieldname)) ...`
	//
	ND bool deleteindex(in fieldname) const;

	// Place a metaphorical DB lock.
	// On a particular record given a DB file and key.
	// This is a advisory lock, not a physical lock, since it makes no restriction on the access or modification of data by other connections.
	// Neither the DB file nor the record key need to actually exist since a lock is just a hash of the DB file name and key combined.
	// If another connection attempts to place an identical lock on the same database it will be denied.
	// Locks can be removed by unlock() or unlockall() or will be automatically removed at the end of a transaction or when the connection is closed.
	// If the same process attempts to place an identical lock more than once it may be denied (if not in a transaction) or succeed but be ignored (if in a transaction).
	// Locks can be used to avoid processing a transaction simultaneously with another connection only to have one of them fail due to mutually updating the same records.
	// Returns:
	// * 0  * Failure: Another connection has already placed the same lock.
	// * "" * Failure: The lock has already been placed.
	// * 1  * Success: A new lock has been placed.
	// * 2  * Success: The lock has already been placed and the connection is in a transaction.
	//
	// `var file = "xo_clients", key = "1000";
	//  if (file.lock(key)) ... ok
	//  // or
	//  if (lock(file, key)) ...`
	//
	ND var  lock(in key) const;

	// Remove a DB lock.
	// A lock placed by the lock function.
	// return: True or False. See lasterror().
	// * Lock cannot be removed while in a transaction.
	// * Lock is not present in the connection.
	//
	// `var file = "xo_clients", key = "1000";
	//  if (file.unlock(key)) ... ok
	//  // or
	//  if (unlock(file, key)) ...`
	//
	   bool unlock(in key) const;

	// Remove all DB locks.
	// All locks placed by the lock function in the specified connection.
	// return: True or False. See lasterror().
	// * Locks cannot be removed while in a transaction.
	//
	// `var dbconn = "exodus";
	//  if (not dbconn.unlockall()) ...
	//  // or
	//  if (not unlockall(dbconn)) ...`
	//
	   bool unlockall() const;

	// obj is record

	// Write a record into a DB file.
	// Given a unique primary key, either inserts a new record or updates an existing record.
	// return: Nothing since writes always succeed.
	// throw: VarDBException if the file does not exist.
	// Any memory cached record is deleted.
	//
	// `let record = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  //if (not "xo_clients"_var.deleterecord("GD001")) {}; // Cleanup first
	//  record.write(file, key);
	//  // or
	//  write(record on file, key);`
	//
	   void write(in file, in key) const;

	// Read a record from a DB file.
	// Given a unique primary key.
	// file: A DB filename or a var opened to a DB file.
	// key: The key of the record to be read.
	// return: False if the key doesnt exist
	// var: Contains the record if it exists or is unassigned if not.
	// A special case of the key being "%RECORDS%" results in a fictitious "record" being returned as an FM separated list of all the keys in the DB file up to a maximum size of 4Mib, sorted in natural order.
	//
	// `var record;
	//  let file = "xo_clients", key = "GD001";
	//  if (not record.read(file, key)) ... // record -> "Client GD^G^20855^30000^1001.00^20855.76539"_var
	//  // or
	//  if (not read(record from file, key)) ...`
	//
	ND bool read(in file, in key);

	// Delete a record from a DB file.
	// Given a unique primary key.
	// return: True or False.
	// * Key doesnt exist.
	// Any memory cached record is deleted.
	// obj is file
	// deleterecord(in file), a one argument free function, is available that deletes multiple records using the currently active select list.
	//
	// `let file = "xo_clients", key = "GD001";
	//  if (file.deleterecord(key)) ... ok
	//  // or
	// //if (deleterecord(file, key)) ...`
	//
	   bool deleterecord(in key) const;

	// Insert a new record in a DB file.
	// Given a unique primary key.
	// return: True or False.
	// * Key already exists
	// Any memory cached record is deleted.
	//
	// `let record = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (record.insertrecord(file, key)) ... ok
	//  // or
	//  if (insertrecord(record on file, key)) ...`
	//
	ND bool insertrecord(in file, in key) const;

	// Update an existing record in a DB file.
	// Given a unique primary key.
	// return: True or False.
	// * Key does not exists
	// Any memory cached record is deleted.
	//
	// `let record = "Client GD^G^20855^30000^1001.00^20855.76539"_var;
	//  let file = "xo_clients", key = "GD001";
	//  if (not record.updaterecord(file, key)) ...
	//  // or
	//  if (not updaterecord(record on file, key)) ...`
	//
	ND bool updaterecord(in file, in key) const;

	// Update the key of an existing record in a DB file.
	// Given two unique primary keys.
	// return: True or False.
	// * Key does not exists
	// * New key already exists.
	// Any memory cached records of either key are deleted.
	//
	// `let file = "xo_clients", key = "GD001", newkey = "GD002";
	//  if (not file.updatekey(key, newkey)) ...
	//  // or
	//  if (not updatekey(file, newkey, key)) ... // Reverse the above change.`
	//
	ND bool updatekey(in key, in newkey) const;

	// obj is strvar

	// Read a field from a DB file record.
	// Same as read() but only returns a specific field number from the record.
	// fieldno: The field number to return from the DB record.
	// return: A string var.
	//
	// `var field, file = "xo_clients", key = "GD001", fieldno = 2;
	//  if (not field.readf(file, key, fieldno)) ... // field -> "G"
	//  // or
	//  if (not readf(field from file, key, fieldno)) ...`
	//
	ND bool readf(in file, in key, const int fieldno);

	// Write a field to a DB file record.
	// Same as write() but only writes to a specific field number in the record.
	//
	// `var field = "f3", file = "xo_clients", key = "1000", fieldno = 3;
	//  field.writef(file, key, fieldno);
	//  // or
	//  writef(field on file, key, fieldno);`
	//
	   void writef(in file, in key, const int fieldno) const;

	// obj is record

	// Write a record and key into a memory cached "DB file".
	// The actual database file is NOT updated.
	// writec() either updates an existing cache record if the key already exists or otherwise inserts a new record into the cache.
	// It always succeeds so no result code is returned.
	// Neither the DB file nor the record key need to actually exist in the actual DB.
	//
    // `let record = "Client XD^X^20855^30000^1001.00^20855.76539"_var;
    //  let file = "xo_clients", key = "XD001";
	//  record.writec(file, key);
	//  // or
	//  writec(record on file, key);`
	//
	   void writec(in file, in key) const;

	// Read a DB record first looking in a memory cached "DB file".
	// Same as "read() but first reads from a memory cache held per connection.
	// 1. Tries to read from a memory cache. Returns true if successful.
	// 2a. Tries to read from the actual DB file and returns false if unsuccessful.
	// 2b. Writes the record and key to the memory cache and returns true.
	// Cached DB file data lives in exodus process memory and is lost when the process terminates or clearcache() is called.
	//
    // `var record;
    //  let file = "xo_clients", key = "XD001";
	//  if (record.readc(file, key)) ... ok
	//  // or
	//  if (readc(record from file, key)) ... ok
	//
	//  // Verify not in actual database file by using read() not readc()
	//  if (read(record from file, key)) abort("Error: " ^ key ^ " should not be in the actual database file"); // error`
	//
	ND bool readc(in file, in key);

	// obj is dbfile

	// Delete a record from a memory cached "DB file".
	// The actual database file is NOT updated.
	// return: False if the key doesnt exist
	//
	// `var file = "xo_clients", key = "XD001";
	//  if (file.deletec(key)) ... ok
	//  // or
	//  if (deletec(file, key)) ...`
	//
	   bool deletec(in key) const;

	// obj is dbconn

	// Clear the "DB file" memory cache.
	// All cached records for the given connection.
	// All future cache readc() function calls will be forced to obtain records from the actual database and refresh the cache.
	//
	// `let dbconn = "exodus";
	//  dbconn.clearcache();
	//  // or
	// clearcache(dbconn);`
	//
	   void clearcache() const;

	// obj is strvar

	// Read a field given filename, key and field number or name.
	// The xlate ("translate") function is similar to readf() but, when called as an exodus program member function, it can be used efficiently with Exodus file dictionaries using named columns, functions and multivalued data.
	// strvar: The primary key to lookup a field in a given file and field no or field name.
	// filename: The DB file in which to look up data.
	// If var key is multivalued then a multivalued field is returned.
	// fieldno: Which field of the record to return.
	// * nn * Field number nn
	// * 0  * The key.
	// * "" * The whole record.
	// mode: If the record does not exist.
	// * "X" * Returns ""
	// * "C" * Returns the key unconverted.
	//
	// `let key = "SB001";
	//  let client_name = key.xlate("xo_clients", 1, "X").squote(); // "'Client AAA'"
	//  // or
	//  let name_and_type = xlate("xo_clients", key, "NAME_AND_TYPE", "X"); // "Client AAA (A)"`
	//
	ND var  xlate(in filename, in fieldno, const char* mode) const;

	///// DATABASE SORT/SELECT:
	//////////////////////////

	// obj is dbfile

	// Create an active select list of DB keys.
	// The select(command) function searches and orders database records for subsequent processing given an English language-like command.
	// The primary job of a database, beyond mere storage and retrieval of information, is to allow rapid searching and ordering of information on demand.
	// In Exodus, searching and ordering of information is known as "sort/select" and is performed by the select() function.
	// Executing the select() function creates an "active select list" which can then be consumed by the readnext() function.
	// dbfile: A opened database file or file name, or an open connection or an empty var for default connections. Subsequent readnext calls must use the same.
	// sort_select_command: A natural language command using dictionary field names. The command can be blank if a dbfile or filename is given in dbfile or just a file name and all keys will be selected in undefined order.
	// Example: "select xo_clients with type 'B' and with balance ge 100 by type by name"
	// Option: "(R)" appended to the sort_select_command acquires the database records as well.
	// return: True if any records are selected or false if none.
	// throw: VarDBException in case of any syntax error in the command.
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
	//
	ND bool select(in sort_select_command = "");

	// Create an active select list from a string of DB keys.
	// Similar to select() but creates the list directly from a var.
	// keys: An FM separated list of keys or key^VM^valueno pairs.
	// return: True if any keys are provided or false if not.
	//
	// `var dbfile = "";
	//  let keys = "A01^B02^C03"_var;
	//  if (dbfile.selectkeys(keys)) ... ok
	//  assert(dbfile.readnext(ID) and ID == "A01");
	//  // or
	//  if (selectkeys(keys)) ... ok
	//  assert(readnext(ID) and ID == "A01");`
	//
	ND bool selectkeys(in keys);

	// Check if a select list is active.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// return: True if a select list is active and false if not.
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
	//
	ND bool hasnext();

	// Acquire and consume one key from an active select list.
	// Each call to readnext consumes one key from the list.
	// Once all the keys in an active select list have been consumed by calls to readnext, the list becomes inactive.
	// See select() for example code.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// key[out]: Returns the first (next) key present in an active select list or "" if no select list is active.
	// return: True if a list was active and a key was acquired, false if not.
	//
	ND bool readnext(out key);

	// Acquire and consume one key and valueno pair from an active select list.
	// Similar to readnext(key) but multivalued.
	// If the active list was ordered by multivalued database fields then pairs of key and multivalue number will be available to the readnext function.
	//
	ND bool readnext(out key, out valueno);

	// Similar to readnext(key, valueno) but acquire the database record as well.
	// record[out]: Returns the next database record from the select list assuming that the select list was created with the (R) option otherwise "" if not.
	// key[out]: Returns the next database record key in the select list.
	// valueno[out]: The multivalue number if the select list was ordered on multivalued database record fields or 1 if not.
	// return: True if a list was active and a key was acquired, false if not.
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
	//
	ND bool readnext(out record, out key, out valueno);

	// Deactivate an active select list.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// return: Nothing
	// Has no effect if no select list is active for dbfile.
	//
	// `var clients = "xo_clients";
	//  clients.clearselect();
	//  if (not clients.hasnext()) ... ok
	//  // or
	//  clearselect();
	//  if (not hasnext()) ... ok`
	//
	   void clearselect();

	// Save an active select list for later retrieval.
	// dbfile: A file or connection var used in a prior select, selectkeys or getlist function call.
	// listname: A suitable name that will be required for later retrieval.
	// return: True if saved successfully or false if there was no active list to be saved.
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
	//
	   bool savelist(SV listname);

	// Retrieve and reactivate a saved select list.
	// dbfile: A file or connection var to be used by subsequent readnext function calls.
	// listname: The name of an existing list in the "lists" database file, either created by savelist or manually.
	// return: True if the list was successfully retrieved and activated, or false if the list name doesnt exist.
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
	//
	ND bool getlist(SV listname);

	// Delete a saved select list.
	// dbfile: A file or connection to the desired database.
	// listname: The name of an existing list in the "lists" database file.
	// return: True if successful or false if the list name doesnt exist.
	//
	// `var dbconn = "";
	//  if (dbconn.deletelist("mylist")) ... ok
	//  // or
	//  if (deletelist("mylist")) ...`
	//
	   bool deletelist(SV listname) const;

//	// Create a saved list from a string of keys.
//	// Any existing list with the same name will be overwritten.
//	// keys: An FM separated list of keys or key^VM^valueno pairs.
//	// return: True if successful or false if no keys were provided.
//	// If the listname is empty then selectkeys() is called instead. This is obsolete and deprecated behaviour.
//	//
//	// `var dbconn = ""; let keys = "A01^B02^C03"_var;
//	//  if (dbconn.makelist("mylist", keys)) ... ok
//	//  // or
//	//  if (makelist("mylist", keys)) ... ok`
//	//
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
//	//
//	ND bool formlist(in keys, const int fieldno = 0);

	///// OS TIME/DATE:
	//////////////////

	// obj is var()

	// Get the current date in internal format.
	// Internal format is the number of whole days since pick epoch 1967-12-31 00:00:00 UTC. Dates prior to that are numbered negatively.
	// return: A number. e.g. 20821 represents 2025-01-01 00:00:00 UTC for 24 hours.
	//
	// `let today1 = var::date();
	//  // or
	//  let today2 = date();`
	//
	ND static var  date();

	// Get the current time in internal format.
	// Internal time is the number of whole seconds since the last midnight 00:00:00 (UTC).
	// return: A number in the range 0 - 86399 since there are 24*60*60 seconds in a day. e.g. 43200 if time is 12:00:00
	//
	// `let now1 = var::time();
	//  // or
	//  let now2 = time();`
	//
	ND static var  time();

	// Get the current time in high resolution internal format.
	// High resolution internal time is the number of fractional seconds since the last midnight 00:00:00 (UTC).
	// return: A floating point with approx. nanosecond resolution depending on hardware.
	// e.g. 23343.704387955 approx. 06:29:03 UTC
	//
	// `let now1 = var::ostime();
	//  // or
	//  let now2 = ostime();`
	//
	ND static var  ostime();

	// Get the current timestamp in internal format.
	// Internal timestamp is the number of fractional days since pick epoch 1967-12-31 00:00:00 UTC. Negative for dates before.
	// return: A floating point with approx. nanosecond resolution depending on hardware.
	// e.g. Was 20821.99998842593 around 2025-01-01 23:59:59 UTC
	//
	// `let now1 = var::ostimestamp();
	//  // or
	//  let now2 = ostimestamp();`
	//
	ND static var  ostimestamp();

	// Get the timestamp for a given date and time
	// vardate: Internal date from date(), iconv("D") etc.
	// ostime: Internal time from time(), ostime(), iconv("MT") etc.
	// obj is vardate
	//
	// `let idate = iconv("2025-01-01", "D"), itime = iconv("23:59:59", "MT");
	//  let ts1 = idate.ostimestamp(itime); // 20821.99998842593
	//  // or
	//  let ts2 = ostimestamp(idate, itime);`
	//
	ND var  ostimestamp(in ostime) const;

	// Sleep/pause/wait
	// milliseconds: How to long to sleep.
	// Release the processor if not needed for a period of time or a delay is required.
	//
	// `var::ossleep(100); // sleep for 100ms
	//  // or
	//  ossleep(100);`
	//
	   static void ossleep(const int milliseconds);

	// Sleep/pause/wait up for a file system event
	// file_dir_list: An FM delimited list of OS files and/or dirs to monitor.
	// milliseconds: How long to wait. Any terminal input (e.g. a key press) will also terminate the wait.
	// return: An FM array of event information is returned. See below.
	// Multiple events may be captured and are returned in multivalues.
	// obj is file_dir_list
	//
	// `let v1 = ".^/etc/hosts"_var.oswait(100); /// e.g. "IN_CLOSE_WRITE^/etc^hosts^f"_var
	//  // or
	//  let v2 = oswait(".^/etc/hosts"_var, 100);`
	//
	// Returned dynamic array fields:
	// 1. Event type codes
	// 2. dirpaths
	// 3. filenames
	// 4. d=dir, f=file
	//
	// Possible event type codes:
	// * IN_CLOSE_WRITE * A file opened for writing was closed
	// * IN_ACCESS      * Data was read from file
	// * IN_MODIFY      * Data was written to file
	// * IN_ATTRIB      * File attributes changed
	// * IN_CLOSE       * File was closed (read or write)
	// * IN_MOVED_FROM  * File was moved away from watched directory
	// * IN_MOVED_TO    * File was moved into watched directory
	// * IN_MOVE        * File was moved (in or out of directory)
	// * IN_CREATE      * A file was created in the directory
	// * IN_DELETE      * A file was deleted from the directory
	// * IN_DELETE_SELF * Directory or file under observation was deleted
	// * IN_MOVE_SELF   * Directory or file under observation was moved
	//
	   var  oswait(const int milliseconds) const;

	///// OS FILE I/O:
	/////////////////

	// Open an OS file handle.
	// Allow random read and write operations.
	// Open for writing if possible, otherwise read-only.
	// osfilevar[out]: Handle for subsequent osbread() and osbwrite() calls.
	// osfilename: Path and name of an existing OS file.
	// utf8: True (default) removes partial UTF-8 sequences from osbread() ends; false returns raw data.
	// return: True if opened successfully, false if file doesn’t exist or isn’t accessible.
	// obj is osfilevar
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (oswrite("" on osfilename)) ... ok /// Create an empty OS file
	//  var ostempfile;
	//  if (ostempfile.osopen(osfilename)) ... ok
	//  // or
	//  if (osopen(osfilename to ostempfile)) ... ok`
	//
	ND bool osopen(in osfilename, const bool utf8 = true) const;

	// Random write data to an OS file.
	// At a specified position.
	// strvar: Data to write.
	// osfilevar: Handle from osopen() or a path/filename; creates file if offset is 0 and it’s new, fails if offset isn’t 0.
	// offset: [in/out] Start position (0-based); updated to end of written data; -1 appends.
	// return: True if write succeeds, false if file isn’t accessible, updateable, or creatable.
	// obj is strvar
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  let text = "aaa=123\nbbb=456\n";
	//  var offset = -1; /// -1 means append.
	//  if (text.osbwrite(osfilename, offset)) ... ok // offset -> 16
	//  // or
	//  if (not osbwrite(text on osfilename, offset)) ...`
	//
	ND bool osbwrite(in osfilevar, io offset) const;

	// Random read data from an OS file
	// From a specified position.
	// strvar[out]: Data read.
	// osfilevar: Handle from osopen() or a path/filename.
	// offset: [in/out] Start position (0-based); updated to end of read data.
	// length: Chars to read; with utf8=true (default), may return less to ensure complete UTF-8 code points.
	// return: True if read succeeds, false if file doesn’t exist or isn’t accessible or offset >= file size.
	// obj is strvar
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  var text, offset = 0;
	//  if (text.osbread(osfilename, offset, 8)) ... ok // text -> "aaa=123\n" // offset -> 8
	//  // or
	//  if (osbread(text from osfilename, offset, 8)) ... ok // text -> "bbb=456\n" // offset -> 16`
	//
	ND bool osbread(in osfilevar, io offset, const int length);

	// Use convenient << syntax to output anything to an osfile.
	// osfile: An OS path and filename or an osfilevar opened by osopen(). The file will be appended, or created if it does not already exist. osfile can be "stdout" or "stderr" to simulate cout/cerr/clog.
	// obj is osfile
	//
	// `let txtfile = "t_temp.txt";
	//  if (not osremove(txtfile)) {} // Remove any existing file.
	//  txtfile << txtfile << " " << 123.456789 << " " << 123 << std::endl;
	//  let v1 = osread(txtfile);   // "t_temp.txt 123.457 123\n"`
	//
	// All standard c++ io manipulators may be used e.g. std::setw, setfill etc.
	//
	// `let vout = "t_std_iomanip_overview.txt";
	//  if (not osremove(vout)) {}
	//  using namespace std;
	//
	//  vout << boolalpha    << true          << "\ttrue"    << endl;
	//  vout << noboolalpha  << true          << "\t1"       << endl;
	//
	//  vout << showpoint    << 42.0          << "\t42.0000" << endl;
	//  vout << noshowpoint  << 42.0          << "\t42"      << endl;
	//
	//  vout << showpos      << 42            << "\t+42"     << endl;
	//  vout << noshowpos    << 42            << "\t42"      << endl;
	//
	//  vout << skipws       << " " << 42     << "\t 42"     << endl;
	//  vout << noskipws     << " " << 42     << "\t 42"     << endl;
	//
	//  vout << unitbuf      << "a"           << "\ta"       << endl;
	//  vout << nounitbuf    << "b"           << "\tb"       << endl;
	//
	//  vout << setw(6)      << 42            << "\t    42"  << endl;
	//
	//  vout << left         << setw(6) << 42 << "\t42    "  << endl;
	//  vout << right        << setw(6) << 42 << "\t    42"  << endl;
	//  vout << internal     << setw(6) << 42 << "\t    42"  << endl;
	//  vout << setfill('*') << setw(6) << 42 << "\t****42"  << endl;
	//
	//  vout << showbase     << hex << 255    << "\t0xff"    << endl;
	//  vout << noshowbase   << 255           << "\tff"      << endl;
	//
	//  vout << uppercase    << 255           << "\tFF"      << endl;
	//  vout << nouppercase  << 255           << "\tff"      << endl;
	//
	//  vout << oct          << 255           << "\t377"     << endl;
	//  vout << hex          << 255           << "\tff"      << endl;
	//  vout << dec          << 255           << "\t255"     << endl;
	//
	//  vout << fixed        << 42.1          << "\t42.100000"            << endl;
	//  vout << scientific   << 42.1          << "\t4.210000e+01"         << endl;
	//  vout << hexfloat     << 42.1          << "\t0x1.50ccccccccccdp+5" << endl;
	//  vout << defaultfloat << 42.1          << "\t42.1"                 << endl;
	//
	//  vout << std::setprecision(3)      << 42.1567  << "\t42.2"  << endl;
	//  vout << resetiosflags(ios::fixed) << 42.1567  << "\t42.2"  << endl;
	//  vout << setiosflags(ios::showpos) << 42       << "\t+42"   << endl;
	//
	//  // Verify actual v. expected.
	//  var act_v_exp = osread(vout);
	//  act_v_exp.converter("\n\t", FM ^ VM); /// Text to dynamic array
	//  act_v_exp = invertarray(act_v_exp);   /// Columns <-> Rows
	//  assert(act_v_exp.f(1) eq act_v_exp.f(2));`
	//
	CVR operator<<(const auto& value) const {
		std::fstream* fs = this->osopenx(*this, /*utf8*/ true, /*or_throw*/ true);
		(*fs) << value; // Write to the stream
		return *this;
	}

	// Handle all stream manipulators

	CVR operator<<(std::ostream& (*manip)(std::ostream&)) const {
		std::fstream* fs = this->osopenx(*this, /*utf8*/ true, /*or_throw*/ true);
		manip(*fs); // Apply manipulator to the output side
		return *this;
	}

	// Close an osfilevar.
	// Remove an osfilevar handle from the internal memory cache of OS file handles. This frees up both exodus process memory and operating system resources.
	// It is advisable to osclose any file handles after use, regardless of whether they were specifically opened using osopen or not, especially in long running programs. Exodus performs caching of internal OS file handles per thread and OS file. If not closed, then the operating system will probably not flush deleted files from storage until the process is terminated. This can potentially create an memory issue or file system resource issue especially if osopening/osreading/oswriting many perhaps temporary files in a long running process.
	//
	// `var osfilevar; if (osfilevar.osopen(ostempfile())) ... ok
	//  osfilevar.osclose();
	//  // or
	//  osclose(osfilevar);`
	//
	   void osclose() const;

	// obj is strvar

	// Create a complete OS file from a var.
	// strvar: The text or data to be used to create the file.
	// osfilename: Absolute or relative path and filename to be written. Any existing OS file is removed first.
	// codepage: If specified then output is converted from UTF-8 to that codepage before being written. Otherwise no conversion is done.
	// return: True if successful or false if not possible for any reason. e.g. Path is not writeable, permissions etc
	//
	// `let text = "aaa = 123\nbbb = 456";
	//  let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (text.oswrite(osfilename)) ... ok
	//  // or
	//  if (oswrite(text on osfilename)) ... ok`
	//
	ND bool oswrite(in osfilename, const char* codepage = "") const;

	// Read a complete OS file into a var.
	// osfilename: Absolute or relative path and filename to be read.
	// codepage: If specified then input is converted from that codepage to UTF-8 after being read. Otherwise no conversion is done.
	// strvar[out]: Is currently set to "" in case of any failure but this is may be changed in a future release to either force var to be unassigned or to leave it untouched. To guarantee future behaviour either add a line 'xxxx.defaulter("")' or set var manually in case osread() returns false. Or use the one argument free function version of osread() which always returns "" in case of failure to read.
	// return: True if successful or false if not possible for any reason. e.g. File doesnt exist, insufficient permissions etc.
	//
	// `var text;
	//  let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (text.osread(osfilename)) ... ok // text -> "aaa = 123\nbbb = 456"
	//  // or
	//  if (osread(text from osfilename)) ... ok
	//  let text2 = osread(osfilename);`
	//
	ND bool osread(const char* osfilename, const char* codepage = "");

	// TODO check if it calls osclose on itself in case removing a varfile

	// obj is osfile_or_dirname

	// Rename an OS file or dir.
	// In the OS file system.
	// The source and target must exist in the same storage device.
	// osfile_or_dirname: Absolute or relative path and file or dir name to be renamed.
	// new_dirpath_or_filepath: Will not overwrite an existing OS file or dir.
	// return: True if successful or false if not possible for any reason. e.g. Target already exists, path is not writeable, permissions etc.
	// Uses std::filesystem::rename internally.
	//
	// `let from_osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  let to_osfilename = from_osfilename ^ ".bak";
	//  if (not osremove(ostempdir() ^ "xo_gendoc_test.conf.bak")) {}; // Cleanup first
	//
	//  if (from_osfilename.osrename(to_osfilename)) ... ok
	//  // or
	//  if (osrename(from_osfilename, to_osfilename)) ...`
	//
	ND bool osrename(in new_dirpath_or_filepath) const;

	// "Move" an OS file or dir.
	// Within the OS file system.
	// Attempt osrename first, then oscopy plus osremove original.
	// osfile_or_dirname: Absolute or relative path and file or dir name to be moved.
	// to_osfilename: Will not overwrite an existing OS file or dir.
	// return: True if successful or false if not possible for any reason. e.g. Source doesnt exist or cannot be accessed, target already exists, source or target is not writeable, permissions, storage space etc.
	//
	// `let from_osfilename = ostempdir() ^ "xo_gendoc_test.conf.bak";
	//  let to_osfilename = from_osfilename.cut(-4);
	//
	//  if (not osremove(ostempdir() ^ "xo_gendoc_test.conf")) {}; // Cleanup first
	//  if (from_osfilename.osmove(to_osfilename)) ... ok
	//  // or
	//  if (osmove(from_osfilename, to_osfilename)) ...`
	//
	ND bool osmove(in to_osfilename) const;

	// Copy an OS file or directory.
	// Including subdirs.
	// osfile_or_dirname: Absolute or relative path and file or dir name to be copied.
	// to_osfilename: Will overwrite an existing OS file or merge into an existing dir.
	// return: True if successful or false if not possible for any reason. e.g. Source doesnt exist or cannot be accessed, target is not writeable, permissions, storage space, etc.
	// Uses std::filesystem::copy internally with recursive and overwrite options
	//
	// `let from_osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  let to_osfilename = from_osfilename ^ ".bak";
	//
	//  if (from_osfilename.oscopy(to_osfilename)) ... ok;
	//  // or
	//  if (oscopy(from_osfilename, to_osfilename)) ... ok`
	//
	ND bool oscopy(in to_osfilename) const;

	// Remove/delete an OS file.
	// From the OS file system.
	// Will not remove directories. Use osrmdir() to remove directories
	// osfilename: Absolute or relative path and file name to be removed.
	// return: True if successful or false if not possible for any reason. e.g. Target doesnt exist, path is not writeable, permissions etc.
	// If osfilename is an osfilevar then it is automatically closed.
	// obj is osfilename
	//
	// `let osfilename = ostempdir() ^ "xo_gendoc_test.conf";
	//  if (osfilename.osremove()) ... ok
	//  // or
	//  if (osremove(osfilename)) ...`
	//
	ND bool osremove() const;

	///// OS DIRECTORIES:
	////////////////////

	// List files and directories

	// obj is dirpath

	// Get a list of OS files and/or dirs.
	// dirpath: Absolute or relative dir path.
	// globpattern: e.g. *.conf to be appended to the dirpath or a complete path plus glob pattern e.g. /etc/ *.conf.
	// mode:
	// * 0 * Any regular OS file or dir (Default).
	// * 1 * Only regular OS files.
	// * 2 * Only dirs.
	// return: An FM delimited string containing all matching dir entries given a dir path
	//
	// `var entries1 = "/etc/"_var.oslist("*.cfg"); /// e.g. "adduser.conf^ca-certificates.con^... etc."
	//  // or
	//  var entries2 = oslist("/etc/" "*.conf");`
	ND var  oslist(SV globpattern = "", const int mode = 0) const;

	// Get a list of OS files.
	// See oslist() for info.
	ND var  oslistf(SV globpattern = "") const;

	// Get a list of OS dirs.
	// See oslist() for info.
	ND var  oslistd(SV globpattern = "") const;

	// Get dir info about an OS file or dir.
	// return: A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time or "" if not a regular file or dir.
	// mode: 0: Default. 1: Must be a regular OS file. 2: Must be an OS dir.
	// See also osfile() and osdir()
	// obj is osfile_or_dirpath
	//
	// `var info1 = "/etc/hosts"_var.osinfo(); /// e.g. "221^20597^78309"_var
	//  // or
	//  var info2 = osinfo("/etc/hosts");`
	//
	ND var  osinfo(const int mode = 0) const;

	// Get dir info of an OS file.
	// osfilename: Absolute or relative path and file name.
	// return: A short string containing size ^ FM ^ modified_time ^ FM ^ modified_time or "" if not a regular file.
	// Alias for osinfo(1)
	// obj is osfilename
	//
	// `var fileinfo1 = "/etc/hosts"_var.osfile(); /// e.g. "221^20597^78309"_var
	//  // or
	//  var fileinfo2 = osfile("/etc/hosts");`
	//
	ND var  osfile() const;

	// Get dir info of an OS dir.
	// dirpath: Absolute or relative path and dir name.
	// return: A short string containing FM ^ modified_time ^ FM ^ modified_time or "" if not a dir.
	// Alias for osinfo(2)
	// obj is dirpath
	//
	// `var dirinfo1 = "/etc/"_var.osdir(); /// e.g. "^20848^44464"_var
	//  // or
	//  var dirinfo2 = osfile("/etc/");`
	//
	ND var  osdir() const;

	// Create a new OS file system directory.
	// Parent dirs wil be created if necessary.
	// dirpath: Absolute or relative path and dir name.
	// return: True if successful.
	// obj is dirpath
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osrmdir("xo_test/aaa")) {}; // Cleanup first
	//  if (osdirname.osmkdir()) ... ok
	//  // or
	//  if (osmkdir(osdirname)) ...`
	//
	ND bool osmkdir() const;

	// Change the current working dir.
	// newpath: An absolute or relative dir path and name.
	// return: True if successful or false if not. e.g. Invalid dirpath, insufficient permission etc.
	// obj is var()
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osdirname.oscwd()) ... ok
	//  // or
	//  if (oscwd(osdirname)) ... ok
	//  if (oscwd("../..")) ... ok /// Change back to avoid errors in following code.`
	//
	ND static bool oscwd(SV newpath);

	// Get the current working dir path and name.
	// return: The current working dir path and name.
	// e.g. "/root/exodus/cli/src/xo_test/aaa"
	// obj is var()
	//
	// `var cwd1 = var().oscwd();
	//  // or
	//  var cwd2 = oscwd();`
	//
	ND static var  oscwd();

	// Remove (deletes) an OS dir,
	// eventifnotempty: If true any subdirs will also be removed/deleted recursively, otherwise the function will fail and return false.
	// return: Returns true if successful or false if not. e.g dir doesnt exist, insufficient permission, not empty etc.
	//
	// `let osdirname = "xo_test/aaa";
	//  if (osdirname.osrmdir()) ... ok
	//  // or
	//  if (osrmdir(osdirname)) ...`
	//
	ND bool osrmdir(bool evenifnotempty = false) const;

	///// OS SHELL/ENVIRONMENT:
	//////////////////////////

	// Execute a shell command.
	// command: An executable command to be interpreted by the default OS shell.
	// return: True if the process terminates with error status 0 and false otherwise.
	// Append "&>/dev/null" to the command to suppress terminal output.
	// obj is command
	//
	// `let cmd = "echo $HOME";
	//  if (cmd.osshell()) ... ok
	//  // or
	//  if (osshell(cmd)) ... ok`
	//
	ND bool osshell() const;

	// Execute a shell command and capture its stdout.
	// return: The stout of the shell command.
	// Append "2>&1" to the command to capture stderr/stdlog output as well.
	// obj is instr
	//
	// `let cmd = "echo $HOME";
	//  var text;
	//  if (text.osshellread(cmd)) ... ok
	//
	//  // or capturing stdout but ignoring exit status
	//  text = osshellread(cmd);`
	//
	ND bool osshellread(in oscmd);

	// Execute a shell command and provide its stdin.
	// return: True if the process terminates with error status 0 and false otherwise.
	// Append "&> somefile" to the command to suppress and/or capture output.
	// obj is outstr
	//
	// `let outtext = "abc xyz";
	//  if (outtext.osshellwrite("grep xyz")) ... ok
	//  // or
	//  if (osshellwrite(outtext, "grep xyz")) ... ok`
	//
	ND bool osshellwrite(in oscmd) const;

	// Run an OS program synchronously.
	// Provide its standard input and capture its output, errors, and exit status.
	// Shell features (e.g., pipes, redirects) are unsupported but can be invoked via an oscmd like "bash -c 'abc|yy $HOME'".
	// oscmd: Executable and arguments; must exist in OS PATH.
	// stdin_for_process: Optional; input data for the program’s standard input.
	// stdout_from_process[out]: Standard output from the program.
	// stderr_from_process[out]: Error/log output from the program.
	// exit_status[out]: Program’s exit status: 0 (normal), -1 (timeout), else (error).
	// timeout_secs: Optional; max runtime in seconds (default 0 = no timeout).
	// return: True if program ran and exited with status 0 (success) or false if program failed to start, timed out, or exited with non-zero status.
	// throw: Pipe creation failed, fork failed, poll failed.
	//
	// `var v_stdout, v_stderr, v_exit_status;
	//  if (var::osprocess("grep xyz", "abc\nxyz 123\ndef", v_stdout, v_stderr, v_exit_status)) ... ok // v_stdout -> "xyz 123\n" // v_exit_status = 0
	//  // or
	//  if (osprocess("grep xyz", "abc\nxyz 123\ndef", v_stdout, v_stderr, v_exit_status)) ... ok`
	//
	ND static bool osprocess(in oscmd, in stdin_for_process, out stdout_from_process, out stderr_from_process, out exit_status, in timeout_secs = 0);

	// Get the tmp dir path.
	// return: A string e.g. "/tmp/"
	// obj is var()
	//
	// `let v1 = var::ostempdir();
	//  // or
	//  let v2 = ostempdir();`
	//
	ND static var  ostempdir();

	// Create a temporary file.
	// return: The name of new temporary file e.g. "/tmp/~exoEcLj3C"
	// obj is var()
	//
	// `var temposfilename1 = var::ostempfile();
	//  // or
	//  var temposfilename2 = ostempfile();`
	//
	ND static var  ostempfile();

	// obj is envvalue

	// Get the value of an environment variable.
	// envcode: The code of the env variable to get or "" for all.
	// envvalue[out]: Set to the value of the env variable if set otherwise "". If envcode is "" then envvalue is set to a dynamic array of all environment variables LIKE CODE1=VALUE1^CODE2=VALUE2...
	// return: True if the envcode is set or false if not.
	// osgetenv and ossetenv work with a per thread copy of the OS process environment. This avoids multithreading issues but does not change the process environment. Child processes created by var::osshell() will not inherit any env variables set using ossetenv() so the oscommand will need to be prefixed to achieve the desired result.
	// For the actual system environment, see "man environ". extern char **environ; // environ is a pointer to an array of pointers to char* env pairs like xxx=yyy and the last pointer in the array is nullptr.
	// obj is envvalue
	//
	// `var envvalue1;
	//  if (envvalue1.osgetenv("HOME")) ... ok // e.g. "/home/exodus"
	//  // or
	//  let envvalue2 = osgetenv("EXO_ABC"); // "XYZ"`
	//
	ND bool osgetenv(SV envcode);

	// Set the value of an environment variable.
	// envcode: The code of the env variable to set.
	// envvalue: The new value to set the env code to.
	// obj is envvalue
	//
	// `let envcode = "EXO_ABC", envvalue = "XYZ";
	//  envvalue.ossetenv(envcode);
	//  // or
	//  ossetenv(envcode, envvalue);`
	//
	   void ossetenv(SV envcode) const;

	// obj is var()

	// Get the current OS process id.
	// return: A number e.g. 663237.
	// obj is var()
	//
	// `let pid1 = var::ospid(); /// e.g. 663237
	//  // or
	//  let pid2 = ospid();`
	//
	ND static var  ospid();

	// Get the current OS thread process id.
	// return: A number e.g. 663237.
	// obj is var()
	//
	// `let tid1 = var::ostid(); /// e.g. 663237
	//  // or
	//  let tid2 = ostid();`
	//
	ND static var  ostid();

	// Get the exodus library version info.
	// return: The git commit details as at the time the library was built.
	// obj is var()
	//
	// `// e.g.
	//  // Local:  doc 2025-03-19 18:15:31 +0000 219cdad8a
	//  // Remote: doc 2025-03-17 15:03:00 +0000 958f412f0
	//  // https://github.com/exodusdb/exodusdb/commit/219cdad8a
	//  // https://github.com/exodusdb/exodusdb/archive/958f412f0.tar.gz
	//  //
	//  let v1 = var::version();
	//  // or
	//  let v2 = version();`
	//
	ND static var  version();

	// obj is var

	// Set the current thread's default locale.
	// strvar: The new locale codepage code.
	// True if successful
	// obj is strvar
	//
	// `if (var::setxlocale("en_US.utf8")) ... ok
	//  // or
	//  if (setxlocale("en_US.utf8")) ... ok`
	//
	   static bool setxlocale(const char* newlocalecode);

	// Get the current thread's default locale.
	// return: A locale codepage code string.
	//
	// `let v1 = var::getxlocale(); // "en_US.utf8"
	//  // or
	//  let v2 = getxlocale();`
	//
	ND static var  getxlocale();

	///// OUTPUT:
	////////////

	// obj is strvar

	// Output to stdout with optional prefix.
	// Append an NL char.
	// Is FLUSHED, not buffered.
	// The raw string bytes are output. No character or byte conversion is performed.
	//
	// `"abc"_var.outputl("xyz = "); /// Sends "xyz = abc\n" to stdout and flushes.
	//  // or
	//  outputl("xyz = ", "abc"); /// Any number of arguments is allowed. All will be output.`
	//
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
	//
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
	//
	   CVR errputl(in prefix = "") const;
	   CVR errput(in prefix = "") const; // Same as errputl() but doesnt append an NL char and is BUFFERED not flushed.

	// Output to a given stream.
	// Is BUFFERED not flushed.
	// The raw string bytes are output. No character or byte conversion is performed.
	//
	   CVR put(std::ostream& ostream1) const;

	// Flush any and all buffered output to stdout and stdlog.
	// obj is var()
	//
	// `var().osflush();
	//  // or
	//  osflush();`
	//
	   void osflush() const;

	///// INPUT:
	///////////

	// obj is var

	// Read one line of input from stdin.
	// return: True if successful or false if EOF or user pressed Esc or Ctrl+X in a terminal.
	// var[in]: The default value for terminal input and editing. Ignored if not a terminal.
	// var[out]: Raw bytes up to but excluding the first new line char. In case of EOF or user pressed Esc or Ctrl+X in a terminal it will be changed to "".
	// Prompt: If provided, it will be displayed on the terminal.
	// Multibyte/UTF8 friendly.
	//
	// `// var v1 = "defaultvalue";
	//  // if (v1.input("Prompt:")) ... ok
	//  // or
	//  // var v2 = input();`
	//
	ND bool input(in prompt = "");

    // Read raw bytes from standard input.
	// Any new line chars are treated like any other bytes.
	// Care must be taken to handle incomplete UTF8 byte sequences at the end of one block and the beginning of the next block.
	// return: The requested number of bytes or fewer if not available.
	// nchars:
	// * nn * Get up to nn bytes or fewer if not available. Caution required with UTF8.
	// *  0 * Get all bytes presently available.
	// *  1 * Same as keypressed(true). Deprecated.
	// * -1 * Same as keypressed(). Deprecated.
	//
	   out  inputn(const int nchars);

	// Return the code of the current terminal key pressed.
	// wait: Defaults to false. True means wait for a key to be pressed if not already pressed.
	// return: ASCII or key code defined according to terminal protocol.
	// return: "" if stdin is not a terminal.
	// e.g. The PgDn key if pressed might return an escape sequence like "\x1b[6~"
	// It only takes a few µsecs to return false if no key is pressed.
	// `var v1; v1.keypressed();
	//  // or
	//  var v2 = keypressed();`
	//
	   out  keypressed(const bool wait = false);

	// obj is var()

	// Check if is a terminal or a file/pipe.
	// Can check stdin, stdout, stderr.
	// in_out_err:
	// * 0 * stdin
	// * 1 * stdout (Default)
	// * 2 * stderr.
	// return: True if it is a terminal or false if it is a file or pipe.
	// Note that if the process is at the start or end of a pipeline, then only stdin or stdout will be a terminal.
	// The type of stdout terminal can be obtained from the TERM environment variable.
	//
	// `var v1 = var().isterminal(); /// 1 or 0
	//  // or
	//  var v2 = isterminal();`
	//
	ND bool isterminal(const int in_out_err = 1) const;

	// Check if stdin has any bytes available for input.
	// If no bytes are immediately available, the process sleeps for up to the given number of milliseconds, returning true immediately any bytes become available or false if the period expires without any bytes becoming available.
	// return: True if any bytes are available otherwise false.
	// It only takes a few µsecs to return false if no bytes are available and no wait time has been requested.
	//
	ND bool hasinput(const int milliseconds = 0) const;

	// True if stdin is at end of file
	//
	ND bool eof() const;

	// Set terminal echo on or off.
	// "On" causes all stdin data to be reflected to stdout if stdin is a terminal.
	// Turning terminal echo off can be used to prevent display of confidential information.
	// return: True if successful.
	//
	   bool echo(const bool on_off = true) const;

	// Install various interrupt handlers.
	// Automatically called in program/thread initialisation by exodus_main.
	// * SIGINT  * Ctrl+C -> "Interrupted. (C)ontinue (Q)uit (B)acktrace (D)ebug (A)bort ?"
	// * SIGHUP  * Sets a static variable "RELOAD_req" which may be handled or ignored by the program.
	// * SIGTERM * Sets a static variable "TERMINATE_req" which may be handled or ignored by the program.
	//
	   void breakon() const;

	// Disable keyboard interrupt.
	// Ctrl+C becomes inactive in terminal.
	//
	   void breakoff() const;

	///// MATH/BOOLEAN:
	//////////////////

	// obj is varnum

    // Absolute value.
    // `let v1 = -12.34;
	//  let v2 = v1.abs(); // 12.34
    //  // or
    //  let v3 = abs(v1);`
	//
    ND var  abs() const;

    // Power.
    // `let v1 = var(2).pwr(8); // 256
    //  // or
    //  let v2 = pwr(2, 8);`
	//
    ND var  pwr(in exponent) const;

    // Initialise the seed for rnd().
	// Allow the stream of pseudo random numbers generated by rnd() to be reproduced.
	// Seeded from std::chrono::high_resolution_clock::now() if the argument is 0;
	//
    // `var(123).initrnd(); /// Set seed to 123
    //  // or
    //  initrnd(123);`
	//
           void initrnd() const;

    // Pseudo random number generator.
	// return: A pseudo random integer between 0 and the provided maximum minus 1.
	// Uses std::mt19937 and std::uniform_int_distribution<int>
	//
    // `let v1 = var(100).rnd(); /// Random 0 to 99
    //  // or
    //  let v2 = rnd(100);`
	//
    ND var  rnd()     const;

    // Power of e.
    // `let v1 = var(1).exp(); // 2.718281828459045
    //  // or
    //  let v2 = exp(1);`
	//
    ND var  exp()     const;

    // Square root.
    // `let v1 = var(100).sqrt(); // 10
    //  // or
    //  let v2 = sqrt(100);`
	//
    ND var  sqrt()    const;

    // Sine of degrees.
    // `let v1 = var(30).sin(); // 0.5
    //  // or
    //  let v2 = sin(30);`
	//
    ND var  sin()     const;

    // Cosine of degrees.
    // `let v1 = var(60).cos(); // 0.5
    //  // or
    //  let v2 = cos(60);`
	//
    ND var  cos()     const;

    // Tangent of degrees.
    // `let v1 = var(45).tan(); // 1
    //  // or
    //  let v2 = tan(45);`
	//
    ND var  tan()     const;

    // Arctangent of degrees.
    // `let v1 = var(1).atan(); // 45
    //  // or
    //  let v2 = atan(1);`
	//
    ND var  atan()    const;

    // Natural logarithm.
	// return: Floating point var (double)
    // `let v1 = var(2.718281828459045).loge(); // 1
    //  // or
    //  let v2 = loge(2.718281828459045);`
	//
    ND var  loge()    const;

    //  ND var  int() const;//reserved word

    // Truncate decimals.
	// Convert decimal to nearest integer towards zero.
	// Remove decimal fraction.
	// return: An integer var
    // `let v1 = var(2.9).integer(); // 2
    //  // or
    //  let v2 = integer(2.9);
	//
    //  var v3 = var(-2.9).integer(); // -2
    //  // or
    //  var v4 = integer(-2.9);`
	//
    ND var  integer() const;

    // Floor decimals.
	// Convert decimal to nearest integer towards negative infinity.
	// return: An integer var
    // `let v1 = var(2.9).floor(); // 2
    //  // or
    //  let v2 = floor(2.9);
	//
    //  var v3 = var(-2.9).floor(); // -3
    //  // or
    //  var v4 = floor(-2.9);`
	//
    ND var  floor() const;

	/* For doc only. Actually implemented in var_base but documented here

	// Modulus function.
	// Identical to C++ % operator only for positive numbers and modulus
	// Negative denominators are considered as periodic with positive numbers
	// Result is between [0, modulus) if modulus is positive
	// Result is between (modulus, 0] if modulus is negative (symmetric)
	// throw: VarDivideByZero if modulus is zero.
	// Floating point works.
	// `let v1 = var(11).mod(5); // 1
	//  // or
	//  let v2 = mod(11, 5); // 1
	//  let v3 = mod(-11, 5); // 4
	//  let v4 = mod(11, -5); // -4
	//  let v5 = mod(-11, -5); // -1`
	//
	ND var  mod(in modulus) const;

	// Not documenting the overloaded versions
	//ND var  mod(double modulus) const;
	//ND var  mod(const int modulus) const;

	// Set floating point precision.
	// This is the number of post-decimal point digits to consider for floating point comparison and implicit conversion to strings.
	// The default precision is 4 which is 0.0001.
	// NUMBERS AND DIFFERENCES SMALLER THAN 0.0001 ARE TREATED AS ZERO UNLESS PRECISION IS INCREASED.
	// With the default precision, Exodus handles, without assistance, conversion to and from strings for positive and negative numbers between 0.0001 and 999'999'999'999.9999 (12 digits).
	// newprecision: New precision between -307 and 308 inclusive.
	// return: The new precision if successful or the old precision if not.
	// Not required if using common numbers or using the explicit rounding and formatting functions to convert numbers to strings.
	// Increasing the precision allows comparing and outputting smaller numbers but creates errors handling large numbers.
	// Setting precision inside a perform, execute or dictionary function lasts until termination of the function.
	// See cli/demo_precision for more info.
	// obj is var()
	//
	// `assert(0.000001_var == 0); /// NOTE WELL: Default precision 4.
	//  let new_precision1 = var::setprecision(6); // 6 // Increase the precision.
	//  // or
	//  let new_precision2 = setprecision(6);
	//  assert(0.000001_var != 0); /// NOTE: Precision 6.`
	//
       static int setprecision(int newprecision);

	// Get current floating point precision.
	// return: The current precision setting.
	// See setprecision() for more info.
	// obj is var()
	//
	// `let curr_precision1 = var::getprecision();
	//  // or
	//  let curr_precision2 = getprecision();`
	//
       static int getprecision();

    */  // For doc only.

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

	ND bool no_check_starts(SV substr) const;
	ND bool no_check_ends(SV substr) const;
	ND bool no_check_contains(SV substr) const;

	ND bool dbcursorexists(); //database, not terminal
	ND bool selectx(in fieldnames, in sortselectclause);

	// TODO check if can speed up by returning reference to converted self like MC

	// Internal primitive oconvs

	//// I/O Conversion Codes :
	///////////////////////////

	// oconv "D" Convert internal date to external format.
	// Date output: Convert internal date format to human readable date or calendar info in text format.
	// return: Human readable date or calendar info, or the original value unconverted if non-numeric.
	// flags: See examples below.
	// Any Dynamic array structure is preserved.
	// obj is vardate
	//
	// `let v1 = 19002;
	//  var v2;
	//  v2 =  v1.oconv( "D"   ) ; //  "09 JAN 2020"   // Default
	//
	//  v2 =  v1.oconv( "D/"  ) ; //  "01/09/2020"    // mm/dd/yyyy - American numeric
	//  v2 =  v1.oconv( "D-"  ) ; //  "01-09-2020"    // mm-dd-yyyy - American numeric
	//
	//  v2 =  v1.oconv( "D/E" ) ; //  "09/01/2020"    // dd/mm/yyyy - International numeric
	//  v2 =  v1.oconv( "D-E" ) ; //  "09-01-2020"    // dd-mm-yyyy - International numeric
	//
	//  v2 =  v1.oconv( "D2"  ) ; //  "09 JAN 20"     // 2 digit year
	//  v2 =  v1.oconv( "D0"  ) ; //  "09 JAN"        // No year
	//
	//  v2 =  v1.oconv( "DS"  ) ; //  "2020 JAN 09"   // yyyy mmm dd - ISO year first, alpha month
	//  v2 =  v1.oconv( "DS-" ) ; //  "2020-01-09"    // yyyy-mm-dd  - ISO year first, numeric month
	//
	//  v2 =  v1.oconv( "DZ"  ) ; //  " 9 JAN 2020"   // Leading 0 become spaces
	//  v2 =  v1.oconv( "DZZ" ) ; //  "9 JAN 2020"    // Leading 0 are suppressed
	//  v2 =  v1.oconv( "D!"  ) ; //  "09JAN2020"     // No separators
	//  v2 =  v1.oconv( "DS-!") ; //  "20200109"      // yyyymmdd packed
	//
	//  v2 =  v1.oconv( "DM"  ) ; //  "1"             // Month number
	//  v2 =  v1.oconv( "DMA" ) ; //  "JANUARY"       // Month name
	//  v2 =  v1.oconv( "DY"  ) ; //  "2020"          // Year number
	//  v2 =  v1.oconv( "DY2" ) ; //  "20"            // Year 2 digits
	//  v2 =  v1.oconv( "DD"  ) ; //  "9"             // Day number in month (1-31)
	//  v2 =  v1.oconv( "DW"  ) ; //  "4"             // Weekday number (1-7)
	//  v2 =  v1.oconv( "DWA" ) ; //  "THURSDAY"      // Weekday name
	//  v2 =  v1.oconv( "DQ"  ) ; //  "1"             // Quarter number
	//  v2 =  v1.oconv( "DJ"  ) ; //  "9"             // Day number in year
	//  v2 =  v1.oconv( "DL"  ) ; //  "31"            // Last day number of month (28-31)
	//
	//  // Dynamic array
	//  let v3 = "12345^12346]12347"_var;
	//  v2 = v3.oconv("D") ; //  "18 OCT 2001^19 OCT 2001]20 OCT 2001"_var
	//
	//  // or
	//  v2 =  oconv(v3, "D"   ) ;`
	//
	ND std::string oconv_D(const char* conversion) const;

	// iconv "D" Convert external date format to internal.
	// Date input: Convert human readable date to internal date format.
	// return: Internal date or "" if the input is an invalid date.
	// Internal date format is whole days since 1967-12-31 00:00:00 which is day 0.
	// Any Dynamic array structure is preserved.
	// obj is strvar
	//
	// `// International order "DE"
	//  var v2;
	//  v2 =             oconv(19005, "DE") ; //  "12 JAN 2020"
	//  v2 =    "12/1/2020"_var.iconv("DE") ; //  19005
	//  v2 =    "12 1 2020"_var.iconv("DE") ; //  19005
	//  v2 =    "12-1-2020"_var.iconv("DE") ; //  19005
	//  v2 =  "12 JAN 2020"_var.iconv("DE") ; //  19005
	//  v2 =  "jan 12 2020"_var.iconv("DE") ; //  19005
	//
	//  // American order "D"
	//  v2 =             oconv(19329, "D") ; //  "01 DEC 2020"
	//  v2 =    "12/1/2020"_var.iconv("D") ; //  19329
	//  v2 =   "DEC 1 2020"_var.iconv("D") ; //  19329
	//  v2 =   "1 dec 2020"_var.iconv("D") ; //  19329
	//
	//  // Reverse order
	//  v2 =   "2020/12/1"_var.iconv("DE") ; //  19329
	//  v2 =    "2020-12-1"_var.iconv("D") ; //  19329
	//  v2 =   "2020 1 dec"_var.iconv("D") ; //  19329
	//
	//  //Invalid date
	//  v2 =    "2/29/2021"_var.iconv("D") ; //  ""
	//  v2 =   "29/2/2021"_var.iconv("DE") ; //  ""
	//
	//  // or
	//  v2 = iconv("12/1/2020"_var, "DE") ; //  19005`
	//
	ND var  iconv_D(const char* conversion) const;

	// oconv "MT" Convert internal time to external format.
	// Time output: Convert internal time format to human readable time e.g. "10:30:59".
	// return: Human readable time or the original value unconverted if non-numeric.
	// Conversion code (e.g. "MTHS") is "MT" ^ flags ...
	// flags:
	// * H * Show AM/PM otherwise 24 hour clock is used.
	// * S * Output seconds
	// * 2 * Ignored (used in iconv)
	// * : * Any other flag is used as the separator char instead of ":"
	// Any Dynamic array structure is preserved.
	// obj is vartime
	//
	// `let v1  = 62000;
	//  var v2;
	//  v2 = v1.oconv("MT"  ); // "17:13"      // Default
	//  v2 = v1.oconv("MTH" ); // "05:13PM"    // 'H' flag for AM/PM
	//  v2 = v1.oconv("MTS" ); // "17:13:20"   // 'S' flag for seconds
	//  v2 = v1.oconv("MTHS"); // "05:13:20PM" // Both flags
	//
	//  let v3  = 0;
	//  v2 = v3.oconv("MT"  ); // "00:00"
	//  v2 = v3.oconv("MTH" ); // "12:00AM"
	//  v2 = v3.oconv("MTS" ); // "00:00:00"
	//  v2 = v3.oconv("MTHS"); // "12:00:00AM"
	//
	//  // Dynamic array
	//  let v4  = "61980^62040]62100"_var;
	//  v2 = v4.oconv("MT");    // "17:13^17:14]17:15"_var
	//
	//  // or
	//  v2 = oconv(v1, "MT");    // "17:13"`
	//
	ND std::string oconv_MT(const char* conversion) const;

	// iconv "MT" Convert external time format to internal.
	// Time input: Convert human readable time (e.g. "10:30:59") to internal time format.
	// return: Internal time or "" if the input is an invalid time.
	// Internal time format is whole seconds since midnight.
	// Accepts: Two or three groups of digits surrounded and separated by any non-digits char(s).
	// Any Dynamic array structure is preserved.
	// obj is strvar
	//
	// `var v2;
	//  v2 =       "17:13"_var.iconv( "MT" ) ; //  61980
	//  v2 =     "05:13PM"_var.iconv( "MT" ) ; //  61980
	//  v2 =    "17:13:20"_var.iconv( "MT" ) ; //  62000
	//  v2 =  "05:13:20PM"_var.iconv( "MT" ) ; //  62000
	//
	//  v2 =       "00:00"_var.iconv( "MT" ) ; //  0
	//  v2 =     "12:00AM"_var.iconv( "MT" ) ; //  0     // Midnight
	//  v2 =     "12:00PM"_var.iconv( "MT" ) ; //  43200 // Noon
	//  v2 =    "00:00:00"_var.iconv( "MT" ) ; //  0
	//  v2 =  "12:00:00AM"_var.iconv( "MT" ) ; //  0
	//
	//  // Dynamic array
	//  v2 = "17:13^05:13PM]17:13:20"_var.iconv("MT") ; //  "61980^61980]62000"_var
	//
	//  // or
	//  v2 = iconv("17:13", "MT") ; //  61980`
	//
	ND var  iconv_MT(bool strict) const;

	// oconv "MD" Convert internal numbers to external format.
	// Number output: Convert internal numbers to external text format after rounding and optional scaling.
	// return: A string or, if the value is not numeric, then no conversion is performed and the original value is returned.
	// conversion_code: (e.g. "MD20Z") is "MD" or "MC", 1st digit, 2nd digit, flags ...
	// * "MD" * Outputs like 123.45 (International)
	// * "MC" * Outputs like 123,45 (European)
	// 1st digit * Decimal places to display. Also decimal places to move if 2nd digit not present and no P flag present.
	// 2nd digit * Optional decimal places to move left if P flag not present.
	//
	// flags:
	// * P * Preserve decimal places. Same as 2nd digit = 0;
	// * Z * Zero flag - return "" if zero.
	// * X * No conversion - return as is.
	// * . * Separate thousands depending on MD or MC.
	// * , * Ditto
	// * - * Suffix negatives with "-" and positives with " " (space).
	// * < * Wrap negatives in "<" and ">" chars.
	// * C * Suffix negatives with "CR" and positives or zero with "DB".
	// * D * Suffix negatives with "DB" and positives or zero with "CR".
	//
	//  Any Dynamic array structure is preserved.
	// obj is varnum
	//
	// `var v1 = -1234.567;
	//  var v2;
	//  v2 =  v1.oconv( "MD20"   ) ; //   "-1234.57"
	//  v2 =  v1.oconv( "MD20,"  ) ; //  "-1,234.57"    // , flag
	//  v2 =  v1.oconv( "MC20,"  ) ; //  "-1.234,57"    // MC code
	//  v2 =  v1.oconv( "MD20,-" ) ; //   "1,234.57-"   // - flag
	//  v2 =  v1.oconv( "MD20,<" ) ; //  "<1,234.57>"   // < flag
	//  v2 =  v1.oconv( "MD20,C" ) ; //   "1,234.57CR"  // C flag
	//  v2 =  v1.oconv( "MD20,D" ) ; //   "1,234.57DB"  // D flag
	//
	//  // Dynamic array
	//  var v3 = "1.1^2.1]2.2"_var;
	//  v2 =  v3.oconv( "MD20"   ) ; //  "1.10^2.10]2.20"_var
	//
	//  // or
	//  v2 =  oconv(v1, "MD20"   ) ; //   "-1234.57"   `
	ND std::string oconv_MD(const char* conversion) const;

	//  v2 =  v1.oconv( "MD2"   ) ; //     "12.34"

	// oconv "L" "R" "C" Justify text and numbers.
	// Text justification: Left, right and center. Padding and truncating. See Procrustes.
	// e.g. "L#10", "R#10", "C#10"
	// Useful when outputting to terminal devices where spaces are used for alignment.
	// Dynamic array structure is preserved.
	// ASCII only.
	// obj is var
	//
	// `var v2;
	//  v2 =      "abcde"_var.oconv( "L#3" ) ; //  "abc"  // Truncating
	//  v2 =      "abcde"_var.oconv( "R#3" ) ; //  "cde"
	//  v2 =      "abcde"_var.oconv( "C#3" ) ; //  "abc"
	//
	//  v2 =      "ab"_var.oconv( "L#6" ) ; //  "ab␣␣␣␣"  // Padding
	//  v2 =      "ab"_var.oconv( "R#6" ) ; //  "␣␣␣␣ab"
	//  v2 =      "ab"_var.oconv( "C#6" ) ; //  "␣␣ab␣␣"
	//
	//  v2 =       var(42).oconv( "L(0)#5" ) ; //  "42000"  // Padding char (x)
	//  v2 =       var(42).oconv( "R(0)#5" ) ; //  "00042"
	//  v2 =       var(42).oconv( "C(0)#5" ) ; //  "04200"
	//  v2 =       var(42).oconv( "C(0)#5" ) ; //  "04200"
	//
	//  // Dynamic array
	//  v2 =       "f1^v1]v2"_var.oconv("L(_)#5") ; //  "f1___^v1___]v2___"_var
	//
	//  // Fail for non-ASCII (Should be 5)
	//  v2 =      "🐱"_var.oconv("L#5").textwidth() ; //  3
	//
	//  // or
	//  v2 =      oconv("abcd", "L#3" ) ; //  "abc" `
	//
	ND std::string oconv_LRC(in format) const;

	// oconv "T" Justify and fold text.
	// e.g. T#20
	// Useful when outputting to terminal devices where spaces are used for alignment.
	// Split text into multiple fixed length lines by inserting spaces and TM chars.
	// ASCII only.
	// obj is strvar
	//
	// `let v1 = "Have a nice day";
	//  let v2 = v1.oconv("T#10") ; //  "Have a␣␣␣␣|nice day␣␣"_var
	//  // or
	//  let v3  = oconv(v1, "T#10") ; //  "Have a␣␣␣␣|nice day␣␣"_var `
	//
	ND std::string oconv_T(in format) const;

	// oconv "MR" Replace characters.
	// e.g. MRU
	// `let v1 = "123/abC.";
	//  var v2;
	//  v2 = v1.oconv("MRL") ; //  "123/abc." // lcase
	//  v2 = v1.oconv("MRU") ; //  "123/ABC." // ucase
	//  v2 = v1.oconv("MRT") ; //  "123/Abc." // tcase
	//  v2 = v1.oconv("MRN") ; //  "123"      // Return only digits
	//  v2 = v1.oconv("MRA") ; //  "abC"      // Return only alphabetic
	//  v2 = v1.oconv("MRB") ; //  "123abC"   // Return only alphanumeric
	//  v2 = v1.oconv("MR/N") ; //  "/abC."   // Remove digits
	//  v2 = v1.oconv("MR/A") ; //  "123/."   // Remove alphabetic
	//  v2 = v1.oconv("MR/B") ; //  "/."      // Remove alphanumeric`
	//
	ND io   oconv_MR(const char* conversion);

	// oconv "HEX" Convert a string of chars to a string of pairs of hexadecimal digits.
	// strvar: A string. Numbers will be converted to strings for conversion. 1.2 -> "1.2" -> hex "312E32"
	// Dynamic array structure is not preserved. Field marks are converted to HEX as for all other bytes.
	// The size of the output is always precisely double that of the input.
	// This function is the exact inverse of iconv("HEX").
	// obj is strvar
	//
	// `var v2;
	//  v2 =      "ab01"_var.oconv( "HEX" ) ; //  "61" "62" "30" "31" 
	//  v2 =  "\xff\x00"_var.oconv( "HEX" ) ; //  "FF" "00"            // Any bytes are ok.
	//  v2 =         var(10).oconv( "HEX" ) ; //  "31" "30"            // Uses ASCII string equivalent of 10 i.e. "10".
	//  v2 =    "\u0393"_var.oconv( "HEX" ) ; //  "CE" "93"            // Greek capital Gamma in UTF8 bytes.
	//  v2 =      "a^]b"_var.oconv( "HEX" ) ; //  "61" "1E" "1D" "62"  // Field and value marks.
	//  // or
	//  v2 =       oconv("ab01"_var, "HEX") ; //  "61" "62" "30" "31"`
	//
	ND std::string oconv_HEX(const int ioratio) const;

	// iconv "HEX" Convert a string of pairs of hexadecimal digits to a string of chars.
	// strvar: Must be a string of only hex digits 0-9, a-f or A-F.
	// return: A string if all input was hex digits otherwise "".
	// Dynamic array structure is not preserved. Any field marks prevent conversion.
	// This function is the exact inverse of oconv("HEX").
	// After prefixing a "0" to an odd sized input, the size of the output is always precisely half that of the input.
	// obj is strvar
	//
	ND var  iconv_HEX(const int ioratio) const;

/* fake code to generate documentation

	// oconv "MX" Convert a number to hexadecimal string.
	// * "MX"   * Convert and trim leading zeros                      e.g. oconv(1025, "MX")  -> "401"
	// * "MXn"  * Pad with up to n leading zeros but do not truncate. e.g. oconv(1025, "MX8") -> "00000401"
	// * "MXnT" * Pad and truncate to n characters.                   e.g. oconv(1025, "MX2") -> "01"
	// "n":    Width. 0-9, A-G = 10 - 16.
	// varnum: A number or dynamic array of numbers. Floating point numbers are rounded to integers before conversion.
	// return: A string of hexadecimal digits or a dynamic array of the same. Elements that are not numeric are left untouched and unconverted.
	// Dynamic array structure is preserved.
	// Negative numbers are treated as unsigned 8 byte integers (uint64).
	// *  0 -> * "00"
	// *  1 -> * "01"
	// * 15 -> * "0F"
	// * -1 -> * "FFFF" "FFFF" "FFFF" "FFFF" (8 x "FF")
	// This function is a near inverse of iconv("MX").
	// obj is varnum
	//
	// `let v1 = "14.5]QQ]65535"_var.oconv("MX"); // "F]QQ]FFFF"_var
	//  // or
	//  let v2 = oconv("14.5]QQ]65535"_var, "MX");`
	//
	ND var oconv_MX() const;

	// iconv "MX" Convert a hexadecimal string to number.
	// strvar: A string or dynamic array of up to 16 hex digits: 0-9, a-f, A-F.
	// return: An integer or dynamic array of integers. Invalid elements are converted to "".
	// Dynamic array structure is preserved.
	// Hex strings are converted to unsigned 8 byte integers (uint64)
	// Leading zeros are ignored.
	// * "0"  -> * 0
	// * "00" -> * 0
	// * "1"  -> * 1
	//
	// Hex "FFFF" "FFFF" "FFFF" "FFFF" (8 x "FF") -> -1.
	// Hex "7FFF" "FFFF" "FFFF" "FFFF" -> The maximum positive integer: 9223372036854775805.
	// Hex "8000" "0000" "0000" "0000" -> The maximum negative integer: -9223372036854775808.
	// This function is the exact inverse of oconv("MX").
	// obj is strvar
	//
	// `let v1 = "F]QQ]FFFF"_var.iconv("MX"); // "15]]65535"_var
	//  // or
	//  let v2 = iconv("F]QQ]FFFF", "MX");`
	//
	ND var iconv_MX() const;

	// oconv "MB" Convert a number to a binary string of "1"s and "0"s,
	// varnum: If not numeric then no conversion is performed and the original value is returned.
	// obj is varnum
	//
	// `let v1 = var(255).oconv("MB"); // 1111'1111
	//  // or
	//  let v2 = oconv(255, "MB");`
	//
	ND var oconv_MB() const;
*/

	// oconv "TX" Convert a dynamic array to text.
	// Convert dynamic arrays to standard text format.
	// Useful for using text editors on dynamic arrays.
	// * FMs -> *   \n after escaping any embedded NL
	// * VMs -> *   literal "\" \n
	// * SMs -> *   literal "\\" \n
	// etc.
	// obj is strvar
	//
	// `// 1. Backslash in text remains backslash
	//  let v1 = var(_BS).oconv("TX");     // _BS
	//
	//  // 2. Literal "\n" -> literal "\\n" (Double escape any escaped NL chars)
	//  let v2 = var(_BS "n").oconv("TX"); // _BS _BS "n"
	//
	//  // 3. \n becomes literal "\n" (Single escape any NL chars)
	//  let v3 = var(_NL).oconv("TX");     // _BS "n"
	//
	//  // 4. FM -> \n
	//  let v4 = "f1^f2"_var.oconv("TX");  // "f1" _NL "f2"
	//
	//  // 5. VM -> "\" \n
	//  let v5 = "v1]v2"_var.oconv("TX");  // "v1" _BS _NL "v2"
	//
	//  // 6. SM -> "\\" \n
	//  let v6 = "s1}s2"_var.oconv("TX");  // "s1" _BS _BS _NL "s2"
	//
	//  // 7. TM -> "\\\" \n
	//  let v7 = "t1|t2"_var.oconv("TX");  // "t1" _BS _BS _BS _NL "t2"
	//
	//  // 8. ST -> "\\\\" \n
	//  let v8 = "st1~st2"_var.oconv("TX"); // "st1" _BS _BS _BS _BS _NL "st2"`
	//
	ND std::string oconv_TX(const char* conversion) const;

	// iconv "TX" Convert text to a dynamic array.
	// Convert standard text format to dynamic array.
	// Reverse of oconv("TX") above.
	// obj is strvar
	//
	ND var  iconv_TX(const char* conversion) const;

	/////////////////////
	// Stop documentation
	/// :
	/////////////////////

	// MD_ Decimal <- Decimal - Not implemented yet
	//  Any Dynamic array structure is preserved.
	ND var  iconv_MD(const char* conversion) const;

	ND std::fstream* osopenx(in osfilename, const bool utf8 = true, const bool or_throw = false) const;

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
	// No longer storing endpos so the user can AMEND the current field
	// and even perhaps decrement an iter after deleting a field.
//	mutable std::size_t endpos_ = std::string::npos;

 public:
	using value_type = var*;

	// Default constructor
	var_iter() = default;

	// Construct from var
	explicit var_iter(in v);

//	var_iter(var_iter&) = delete;
//	var_iter(var_iter&&) = delete;

	// Check iter != iter (i.e. iter != string::npos)
	   bool operator!=(const var_iter& vi);

	// Convert to var (copy)
	var  operator*() const;

//	// Convert to var reference
//	var&  operator*();

	// ++Iter prefix
	var_iter& operator++();

	// Iter++ postfix
	var_iter operator++(int);

	// --Iter prefix
	var_iter& operator--();

	// iter++ postfix
	var_iter operator--(int);

	// assign
	//void operator=(SV replacement);

};

///////////////////////////////
// dim_iter - iterate over vars
///////////////////////////////
//class PUBLIC dim_iter {
//private:
////	var* ptr_;
//	std::vector<var>::iterator ptr_;
//public:
//
//	using iterator_category = std::random_access_iterator_tag;
//	using value_type        = var;
//	using reference         = var&;
//	using pointer           = var*;
//	using difference_type   = unsigned long long;
//
////	dim_iter(var* ptr) : ptr_(ptr) {}
//	dim_iter(std::vector<var>::iterator iter) : ptr_(iter) {}
//
////	operator std::vector<var>::iterator() {
////		return std::vector<var>(*ptr_);
////	}
//
//	var& operator*() {
//		return *ptr_;
//	}
//
//	std::vector<var>::iterator operator->() {
//		return ptr_;
//	}
//
//	dim_iter& operator++() {
//		++ptr_;
//		return *this;
//	}
//
//	dim_iter operator++(int) {
//		dim_iter tmp(*this);
//		++ptr_;
//		return tmp;
//	}
//
//	dim_iter& operator--() {
//		--ptr_;
//		return *this;
//	}
//
//	dim_iter operator--(int) {
//		dim_iter tmp(*this);
//		--ptr_;
//		return tmp;
//	}
//
//	bool operator==(const dim_iter& rhs) const {
//		return ptr_ == rhs.ptr_;
//	}
//
//	bool operator!=(const dim_iter& rhs) const {
//		return !(*this == rhs);
//	}
//
//	var& operator[](int n) {
//		return ptr_[n];
//	}
//
//	dim_iter& operator+=(int n) {
//		ptr_ += n;
//		return *this;
//	}
//
//	dim_iter operator+(int n) const {
//		dim_iter tmp(*this);
//		tmp += n;
//		return tmp;
//	}
//
//	dim_iter& operator-=(int n) {
//		ptr_ -= n;
//		return *this;
//	}
//
//	dim_iter operator-(int n) const {
//		dim_iter tmp(*this);
//		tmp -= n;
//		return tmp;
//	}
//
//	long int operator-(const dim_iter& rhs) const {
//		return ptr_ - rhs.ptr_;
//	}
//
//	bool operator<(const dim_iter& rhs) const {
//		return ptr_ < rhs.ptr_;
//	}
//
//	bool operator>(const dim_iter& rhs) const {
//		return ptr_ > rhs.ptr_;
//	}
//
//	bool operator<=(const dim_iter& rhs) const {
//		return ptr_ <= rhs.ptr_;
//	}
//
//	bool operator>=(const dim_iter& rhs) const {
//		return ptr_ >= rhs.ptr_;
//	}
//
//};


class dim_const_iter; // Forward declaration for friendship

class dim_iter {
private:
    std::vector<var>::iterator ptr_;

    // Grant dim_const_iter access to ptr_
    friend class dim_const_iter;

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = var;
    using reference         = var&;
    using pointer           = var*;
    using difference_type   = std::ptrdiff_t;

    dim_iter() : ptr_() {}
    explicit dim_iter(std::vector<var>::iterator iter) : ptr_(iter) {}
    dim_iter(const dim_iter& other) : ptr_(other.ptr_) {}

    var& operator*() { return *ptr_; }
    const var& operator*() const { return *ptr_; }

    var* operator->() { return &(*ptr_); }
    const var* operator->() const { return &(*ptr_); }

    dim_iter& operator++() { ++ptr_; return *this; }
    dim_iter operator++(int) { dim_iter tmp(*this); ++ptr_; return tmp; }

    dim_iter& operator--() { --ptr_; return *this; }
    dim_iter operator--(int) { dim_iter tmp(*this); --ptr_; return tmp; }

    bool operator==(const dim_iter& rhs) const { return ptr_ == rhs.ptr_; }
    bool operator!=(const dim_iter& rhs) const { return !(*this == rhs); }

    var& operator[](difference_type n) { return ptr_[n]; }
    const var& operator[](difference_type n) const { return ptr_[n]; }

    dim_iter& operator+=(difference_type n) { ptr_ += n; return *this; }
    dim_iter operator+(difference_type n) const { dim_iter tmp(*this); tmp += n; return tmp; }

    dim_iter& operator-=(difference_type n) { ptr_ -= n; return *this; }
    dim_iter operator-(difference_type n) const { dim_iter tmp(*this); tmp -= n; return tmp; }

    difference_type operator-(const dim_iter& rhs) const { return ptr_ - rhs.ptr_; }

    bool operator<(const dim_iter& rhs) const { return ptr_ < rhs.ptr_; }
    bool operator>(const dim_iter& rhs) const { return ptr_ > rhs.ptr_; }
    bool operator<=(const dim_iter& rhs) const { return ptr_ <= rhs.ptr_; }
    bool operator>=(const dim_iter& rhs) const { return ptr_ >= rhs.ptr_; }
};

inline dim_iter operator+(std::ptrdiff_t n, const dim_iter& it) {
    return it + n;
}



class dim_const_iter {
private:
    std::vector<var>::const_iterator ptr_;

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = var;
    using reference         = const var&;
    using pointer           = const var*;
    using difference_type   = std::ptrdiff_t;

    dim_const_iter() : ptr_() {}
    explicit dim_const_iter(std::vector<var>::const_iterator iter) : ptr_(iter) {}
    dim_const_iter(const dim_const_iter& other) : ptr_(other.ptr_) {}
    // Conversion from dim_iter to dim_const_iter, now works with friendship
    explicit dim_const_iter(const dim_iter& other) : ptr_(other.ptr_) {}

    const var& operator*() const { return *ptr_; }
    const var* operator->() const { return &(*ptr_); }

    dim_const_iter& operator++() { ++ptr_; return *this; }
    dim_const_iter operator++(int) { dim_const_iter tmp(*this); ++ptr_; return tmp; }

    dim_const_iter& operator--() { --ptr_; return *this; }
    dim_const_iter operator--(int) { dim_const_iter tmp(*this); --ptr_; return tmp; }

    bool operator==(const dim_const_iter& rhs) const { return ptr_ == rhs.ptr_; }
    bool operator!=(const dim_const_iter& rhs) const { return !(*this == rhs); }

    const var& operator[](difference_type n) const { return ptr_[n]; }

    dim_const_iter& operator+=(difference_type n) { ptr_ += n; return *this; }
    dim_const_iter operator+(difference_type n) const { dim_const_iter tmp(*this); tmp += n; return tmp; }

    dim_const_iter& operator-=(difference_type n) { ptr_ -= n; return *this; }
    dim_const_iter operator-(difference_type n) const { dim_const_iter tmp(*this); tmp -= n; return tmp; }

    difference_type operator-(const dim_const_iter& rhs) const { return ptr_ - rhs.ptr_; }

    bool operator<(const dim_const_iter& rhs) const { return ptr_ < rhs.ptr_; }
    bool operator>(const dim_const_iter& rhs) const { return ptr_ > rhs.ptr_; }
    bool operator<=(const dim_const_iter& rhs) const { return ptr_ <= rhs.ptr_; }
    bool operator>=(const dim_const_iter& rhs) const { return ptr_ >= rhs.ptr_; }
};

inline dim_const_iter operator+(std::ptrdiff_t n, const dim_const_iter& it) {
    return it + n;
}



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
	// Allow easy extraction of fields, values and subvalues
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
	// Allow easy replacement of fields
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
		var_.updater(fn_, replacement);
	}

	void operator=(var_proxy1 rhs);
	void operator=(var_proxy2 rhs);
	void operator=(var_proxy3 rhs);

//	void operator=(var_proxy2 rhs) {
//		var_.updater(fn_, var(rhs));
//	}
//
//	void operator=(var_proxy3 rhs) {
//		var_.updater(fn_, var(rhs));
//	}

	// Operator bool
	//
	// Allow usage in if statements etc.
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

}; // var proxy1

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
		var_.updater(fn_, vn_, replacement);
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

}; // class var_proxy2

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
		var_.updater(fn_, vn_, sn_, replacement);
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

}; // class var_proxy3

ND inline var_proxy1 var::operator()(int fieldno) {return var_proxy1(*this, fieldno);}
ND inline var_proxy2 var::operator()(int fieldno, int valueno) {return var_proxy2(*this, fieldno, valueno);}
ND inline var_proxy3 var::operator()(int fieldno, int valueno, int subvalueno) {return var_proxy3(*this, fieldno, valueno, subvalueno);}

   inline void var_proxy1::operator=(var_proxy1 rhs) {var_.updater(fn_, var(rhs));}
   inline void var_proxy1::operator=(var_proxy2 rhs) {var_.updater(fn_, var(rhs));}
   inline void var_proxy1::operator=(var_proxy3 rhs) {var_.updater(fn_, var(rhs));}

   inline void var_proxy2::operator=(var_proxy1 rhs) {var_.updater(fn_, vn_, var(rhs));}
   inline void var_proxy2::operator=(var_proxy2 rhs) {var_.updater(fn_, vn_, var(rhs));}
   inline void var_proxy2::operator=(var_proxy3 rhs) {var_.updater(fn_, vn_, var(rhs));}

   inline void var_proxy3::operator=(var_proxy1 rhs) {var_.updater(fn_, vn_, sn_, var(rhs));}
   inline void var_proxy3::operator=(var_proxy2 rhs) {var_.updater(fn_, vn_, sn_, var(rhs));}
   inline void var_proxy3::operator=(var_proxy3 rhs) {var_.updater(fn_, vn_, sn_, var(rhs));}

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
	PUBLIC extern const var NL;

	PUBLIC extern const char* const _OS_NAME;
	PUBLIC extern const char* const _OS_VERSION;

	PUBLIC extern thread_local var BASEFMT;
	PUBLIC extern thread_local var DATEFMT;
	PUBLIC extern thread_local var TZ;

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
	CONSTINIT_VAR const var NL = NL_;

	// Basic time zone, date and number format
	thread_local var BASEFMT = "MD20,"; // base currency decimals and and MD for 1,234.56 format
	thread_local var DATEFMT = "DE";    // E for international numeric format on input
	thread_local var TZ      = "";      // .f(1) user tz offset from UTC in secs, .f(2) system tz offset from UTC in secs

#ifdef EXO_OS_NAME
	const char* const _OS_NAME = EXO_OS_NAME;
#endif

#ifdef EXO_OS_VERSION
	const char* const _OS_VERSION = EXO_OS_VERSION;
#endif

#endif // EXO_VAR_CPP

//
/////////////////////
// _var user literals
/////////////////////

// "abc^def"_var
ND PUBLIC var  operator""_var(const char* cstr, std::size_t size);

// 123456_var
ND PUBLIC var  operator""_var(unsigned long long int i);
//PUBLIC var  operator""_var(std::uint_t i);

// 123.456_var
ND PUBLIC var  operator""_var(long double d);

}  // namespace exo

// clang-format on

#endif //EXODUS_LIBEXODUS_EXODUS_VAR_H_
