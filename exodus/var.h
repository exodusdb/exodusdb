#ifndef LIBEXODUS_VAR_H_
#define LIBEXODUS_VAR_H_
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
//#include <exodus/vartyp.h>

namespace exo {

	class var;
	class rex;
	class dim;
	class var_iter;
	class var_proxy1;
	class var_proxy2;
	class var_proxy3;

	using VAR    =       var;
	using VARREF =       var&;
	using CVR    = const var&;
	using TVR    =       var&&;

	using in     = const var&;
	using out    =       var&;
	using io     =       var&;
}

// var_base provides the basic var-like functionality for var
#include <exodus/varb.h> // var_base
#include <exodus/vars.h> // var_stg
#include <exodus/varo.h> // var_os

namespace exo {

// class var
class PUBLIC var : public var_os {

	// Using applies to all member functions everywhere
	using CVR    = const var&;

	using in     = const var&;
	using out    =       var&;
	using io     =       var&;

	friend class dim;
	friend class rex;

public:

	// Define all ctor/assign/conversions inline to ensure optimisation can remove redundant assembler.
	// This is important for value types like var.

	// Inherit all constructors from var_os > var_stg > var_base
	using var_os::var_os;

	// Copy ctor
	var(const var& other) = default;

	// Move ctor
	var(var&& other) noexcept = default;

	// ASSIGNMENT

	// Inherit all assignment operators to convert all types DIRECTLY
	// otherwise we get nonsense like using our implicit copy/move ctors from var AFTER using var_base ctors from various types
    using var_os::operator=;

	// Return void to prevent accidental use in conditionals (e.g., if (v = "x"))
	// and to discourage assignment chaining, enhancing code clarity.

	// Copy assignment
	void /*var&*/ operator=(const var& other) {  // Suppress implicit operators
		var_os::operator=(other);  // Delegate to base
		return /**this*/;
	}

	// Move assignment
	void /*var&*/ operator=(var&& other) & noexcept { var_os::operator=(std::move(other)); }

	// Initializer list
	void /*var&*/ operator=(std::initializer_list<var> list) & {
		var_os::operator=(var(list));  // Reuse constructor, assign via var_os
	}

	// Implicit conversions to var

    using var_os::operator var;

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
	// Pseudonyms:
	// ┌──────────────────────┬───────────────┬──────────────────┐
	// │ Usage                │ Non-Constant  │ Constant         │
	// ├──────────────────────┼───────────────┼──────────────────┤
	// │ Variables:           │    var → var  │ let → const var  │
	// ├──────────────────────┼───────────────┼──────────────────┤
	// │ Function parameters: │ out/io → var& │ in  → const var& │
	// └──────────────────────┴───────────────┴──────────────────┘
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
	// * ~     * STM  Subtext mark
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
			var_str += std::string_view(var(item));
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
	ND var  format(in fmt_str, Args&&... args) const {
		THISIS("var  var::format(SV fmt_str, Args&&... args) const")
		assertString(function_sig);
		// *this becomes the first format argument and any additional arguments become additionl format arguments
		return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(*this, args...) );
	}

#endif //EXO_FORMAT

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
	// * False * Transaction was in error or DB refused. See lasterror() for info.
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
	// * Source filename does not exist.
	// * New filename is invalid.
	// * New filename already exists.
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
	// * File name does not exist.
	//
	// `let dbconn = "exodus", filename = "xo_gendoc_temp2";
	//  if (not dbconn.clearfile(filename)) ...
	//  // or
	//  if (not clearfile(filename)) ...`
	//
	ND bool clearfile(in filename) const;

	// Delete a DB file.
	// return: True or False. See lasterror() for errors.
	// * File name does not exist.
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

	/* For doc only. Actually implemented in var_base but documented here

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
	// ND var  mod(double modulus) const;
	// ND var  mod(const int modulus) const;

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
	//  // 8. STM -> "\\\\" \n
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

}; // class var_iter

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

}; // class var_proxy1

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
	PUBLIC extern const var STM;

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
	CONSTINIT_VAR const var STM = STM_;

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

///////////////////////////////
// var_stg template definitions
///////////////////////////////

// Replace

ND var  var_stg::replace(const rex& regex, ReplacementFunction auto repl_func)
                                                           && {replacer(regex,repl_func);    return move();}

   IO   var_stg::replacer(const rex& regex, ReplacementFunction auto repl_func) REF {*this = replace(regex, repl_func);}

ND var  var_stg::replace(const rex& regex, ReplacementFunction auto repl_func) const {

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

// Unpack

template <size_t N>
auto var_stg::unpack/*<N>*/(SV delim /*= _FM*/) const -> std::array<var, N> {
    THISIS("auto var::unpack<N>(SV delim /*= _FM*/) const")
    assertString(function_sig);

	// std::vector<var> vv1 = this->split(delim);
	// Utility somewhere in the forest
	/*nondoc*/ auto basic_split(const var& v1, SV delim) -> std::vector<var>;
	/*nondoc*/ auto vv1 = basic_split(var_str, delim);
	return [&vv1]<size_t... Is>(std::index_sequence<Is...>) {
	return std::array<var, N>{
		(Is < vv1.size() ? std::move(vv1[Is]) : std::move(var()))...
	};
	}(std::make_index_sequence<N>{});
}

// Append

ND var  var_stg::append(const auto&... appendable) const& {var nrvo = this->clone(); (nrvo ^= ... ^= appendable); return nrvo;}
ND var  var_stg::append(const auto&... appendable)   && {((*this) ^= ... ^= appendable);      return move();}

// Implicit conversion of var_base to var
//var_base::operator var()       { return *reinterpret_cast<      var*>(this); }
//var_base::operator var() const { return *reinterpret_cast<const var*>(this); }
	constexpr var_base::operator       var()  const    { return *reinterpret_cast<const var*>(this); }
//	constexpr var_base::operator const var()  const    { return *reinterpret_cast<const var*>(this); }
	constexpr var_base::operator const var&() const &  { return *reinterpret_cast<const var*>(this); }
	constexpr var_base::operator       var&()       &  { return *reinterpret_cast<var*>(this); }
	constexpr var_base::operator       var()        && { return *reinterpret_cast<const var*>(this); }

//	constexpr var_os::operator       var()  const    { return *reinterpret_cast<const var*>(this); }
//	constexpr var_os::operator const var()  const    { return *reinterpret_cast<const var*>(this); }
//	constexpr var_os::operator const var&() const &  { return *reinterpret_cast<const var*>(this); }
//	constexpr var_os::operator       var&()       &  { return *reinterpret_cast<var*>(this); }
//	constexpr var_os::operator       var()        && { return *reinterpret_cast<const var*>(this); }

//	constexpr exo::var_os::operator exo::var&() & {return *reinterpret_cast<exo::var*>(this);};

}  // namespace exo

// clang-format on

#endif //LIBEXODUS_VAR_H_
