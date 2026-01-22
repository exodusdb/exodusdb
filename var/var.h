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

// gendoc: var - General

// clang-format off

#if EXO_MODULE > 1
	import std;
#else
#	include <fstream>
#	include <string>
#	include <string_view>
#	include <vector>
#	include <array>
#endif

// Purely #define so can done before any declarations
// Always need to be run because modules dont export #define
#include <var/vardefs.h>

#include <var/format.h>

namespace exo {

	class var;
	class rex;
	class dim;
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

#include <var/vard.h> // var_db

namespace exo {

// class var
// Inheritance chain: var > var_db > var_os > var_stg > var_base
class PUBLIC var : public var_db {

	// "using" applies to all member functions except ctor/dtor/operator
	using CVR    = const var&;

	using in     = const var&;
	using out    =       var&;
	using io     =       var&;

	friend class dim;
	friend class rex;

public:

	// Note:  Define all ctor/assign/conversions inline to ensure optimisation can remove redundant assembler.
	// This is important for value types like var.

	// Inherit all constructors from var_db > var_os > var_stg > var_base
	using var_db::var_db;

	// Copy ctor
	var(const var& other) = default;

	// Move ctor
	var(var&& other) noexcept = default;

	// ASSIGNMENT

	// Inherit all assignment operators to convert all types DIRECTLY
	// otherwise we get nonsense like using our implicit copy/move ctors from var AFTER using var_base ctors from various types
    using var_db::operator=;

	// Return void to prevent accidental use in conditionals (e.g., if (v = "x"))
	// and to discourage assignment chaining, enhancing code clarity.

	// Copy assignment
	void /*var&*/ operator=(const var& other) {  // Suppress implicit operators
		var_db::operator=(other);  // Delegate to base
		return /**this*/;
	}

	// Move assignment
	void /*var&*/ operator=(var&& other) & noexcept { var_db::operator=(std::move(other)); }

	// Initializer list
	void /*var&*/ operator=(std::initializer_list<var> list) & {
		var_db::operator=(var(list));  // Reuse constructor, assign via var_db
	}

	// Implicit conversions to var

    using var_db::operator var;

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

	*/

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
			var_str += std::string_view(var_base(item));
			var_str.push_back(FM_);
		}
		if (!var_str.empty())
			var_str.pop_back();
	}

	/* fake for gendoc

	// Dynamic array *field* update and append:
	// See also inserter() and remover().
	//
	// `var v1 = "aa^bb"_var;
	//  v1(4) = 44; // v1 -> "aa^bb^^44"_var
	//  // Field number -1 causes appending a field when updating.
	//  v1(-1) = "55"; // v1 -> "aa^bb^^44^55"_var`
	//
	// It is recommended to use "v1.f(fieldno)" syntax using a ".f(" prefix to access (extract) fields in expressions instead of plain "v1(fieldno)". The syntax var.f(...) will always compile whereas var(...) does not compile in all contexts. It will compile only if being called on a constant var or in a location which requires an implicit conversion to var. This is due to C++ not making a clear distinction between usage on the left and right side of assignment operator =. Furthermore using plain round brackets without the leading .f can be confused with function call syntax.
	//
	// `var v1 = "aa^bb^cc"_var;
	//  var v2 = v1.f(2); // "bb" /// .f() style access. Recommended.
	//  var v3 =   v1(2); // "bb" ///   () style access. Not recommended.`
	//
	ND var  operator()(int fieldno) const;

	// Dynamic array *value* update and append
	// See also inserter() and remover().
	//
	// `var v1 = "aa^b1]b2^cc"_var;
	//  v1(2, 4) = "44"; // v1 -> "aa^b1]b2]]44^cc"_var
	//  // value number -1 causes appending a value when updating.
	//  v1(2, -1) = 55; // v1 -> "aa^b1]b2]]44]55^cc"_var`
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
	// Equivalent to var v1 = v2.extract(1,2,3);
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

	// Dynamic array *subvalue* update and append
	// See also inserter() and remover().
	//
	// `var v1 = "aa^bb^cc"_var;
	//  v1(2, 2, 2) = "22"; // v1 -> "aa^bb]}22^cc"_var
	//  // subvalue number -1 causes appending a subvalue when updating.
	//  v1(2, 2, -1) = 33; // v1 -> "aa^bb]}22}33^cc"_var`
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

	/*nodoc*/ ND var_proxy1 operator()(int fieldno);
	/*nodoc*/ ND var_proxy2 operator()(int fieldno, int valueno);
	/*nodoc*/ ND var_proxy3 operator()(int fieldno, int valueno, int subvalueno);

	///// I/O CONVERSION:
	////////////////////

	// obj is var

	// Convert internal format data to output format for display.
	// convstr: A conversion code or pattern. See ICONV/OCONV PATTERNS
	// return: The data in external display format or, if the data is invalid and cannot converted, most conversions return the original data UNCONVERTED.
	// throw: VarNotImplemented if convstr is invalid
	//
	// `let v1 = var(30123).oconv("D/E"); // "21/06/2050"
	//  // or
	//  let v2 = oconv(30123, "D/E");`
	//
	ND var  oconv(const char* convstr) const;

	// Convert output display format data to internal format.
	// convstr: A conversion code or pattern. See ICONV/OCONV PATTERNS
	// return: The data in internal format or, if the data is invalid and cannot be converted, most conversions return the EMPTY STRING ""
	// throw: VarNotImplemented if convstr is invalid
	//
	// `let v1 = "21 JUN 2050"_var.iconv("D/E"); // 30123
	//  // or
	//  let v2 = iconv("21 JUN 2050", "D/E");`
	//
	ND var  iconv(const char* convstr) const;

#if EXO_FORMAT

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
//		return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(*this, args...) );
		return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(static_cast<const var_base&>(*this), cast_var_to_var_base(std::forward<Args>(args))...) );
//		return fmt::vformat(std::string_view(fmt_str), fmt::make_format_args(cast_var_to_var_base(*this), cast_var_to_var_base(std::forward<Args>(args))...) );
	}

#endif //EXO_FORMAT

	// return: The last OS or DB error message.
	// obj is var()
	//
	// `var v1 = var::lasterror();
	//  // or
	//  var v2 = lasterror();`
	//
	ND static var  lasterror();

	// Set the lasterror() message.
	   static void setlasterror(in msg);

	// Log the last OS or DB error message.
	// Output: To stdlog
	// Prefix the output with source if provided.
	// obj is var()
	//
	// `var::loglasterror("main:");
	//  // or
	//  loglasterror("main:");`
	//
	   static void loglasterror(in source = "");

	///////////////////////////
	// PRIVATE MEMBER FUNCTIONS
	///////////////////////////

 private:

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
	//  let v1 = var(_BSL).oconv("TX");     // _BSL
	//
	//  // 2. Literal "\n" -> literal "\\n" (Double escape any escaped NL chars)
	//  let v2 = var(_BSL "n").oconv("TX"); // _BSL _BSL "n"
	//
	//  // 3. \n becomes literal "\n" (Single escape any NL chars)
	//  let v3 = var(_NL).oconv("TX");     // _BSL "n"
	//
	//  // 4. FM -> \n
	//  let v4 = "f1^f2"_var.oconv("TX");  // "f1" _NL "f2"
	//
	//  // 5. VM -> "\" \n
	//  let v5 = "v1]v2"_var.oconv("TX");  // "v1" _BSL _NL "v2"
	//
	//  // 6. SM -> "\\" \n
	//  let v6 = "s1}s2"_var.oconv("TX");  // "s1" _BSL _BSL _NL "s2"
	//
	//  // 7. TM -> "\\\" \n
	//  let v7 = "t1|t2"_var.oconv("TX");  // "t1" _BSL _BSL _BSL _NL "t2"
	//
	//  // 8. STM -> "\\\\" \n
	//  let v8 = "st1~st2"_var.oconv("TX"); // "st1" _BSL _BSL _BSL _BSL _NL "st2"`
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

ND var  var_stg::replace(const rex& regex, ReplacementFunction auto repl_func) const & {

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

template <std::size_t N>
auto var_stg::unpack/*<N>*/(SV delim /*= _FM*/) const -> std::array<var, N> {
    THISIS("auto var::unpack<N>(SV delim /*= _FM*/) const")
    assertString(function_sig);

	// std::vector<var> vv1 = this->split(delim);
	// Utility somewhere in the forest
	/*nondoc*/ auto basic_split(const var& v1, SV delim) -> std::vector<var>;
	/*nondoc*/ auto vv1 = basic_split(var_str, delim);
	return [&vv1]<std::size_t... Is>(std::index_sequence<Is...>) {
	return std::array<var, N>{
		(Is < vv1.size() ? std::move(vv1[Is]) : std::move(var()))...
	};
	}(std::make_index_sequence<N>{});
}

// Append

ND var  var_stg::append(const auto&... appendable) const& {var nrvo = this->clone(); (nrvo ^= ... ^= appendable); return nrvo;}
ND var  var_stg::append(const auto&... appendable)   && {((*this) ^= ... ^= appendable);      return move();}

#ifdef EXO_VAR_CPP

// Implicit conversion of var_base to var
//var_base::operator var()       { return *reinterpret_cast<      var*>(this); }
//var_base::operator var() const { return *reinterpret_cast<const var*>(this); }
	CONSTEXPR var_base::operator       var()  const &  { return *reinterpret_cast<const var*>(this); }
//	CONSTEXPR var_base::operator const var()  const    { return *reinterpret_cast<const var*>(this); }
	CONSTEXPR var_base::operator const var&() const &  { return *reinterpret_cast<const var*>(this); }
	CONSTEXPR var_base::operator       var&()       &  { return *reinterpret_cast<var*>(this); }
	CONSTEXPR var_base::operator       var()        && { return *reinterpret_cast<const var*>(this); }

//	CONSTEXPR var_db::operator       var()  const    { return *reinterpret_cast<const var*>(this); }
//	CONSTEXPR var_db::operator const var()  const    { return *reinterpret_cast<const var*>(this); }
//	CONSTEXPR var_db::operator const var&() const &  { return *reinterpret_cast<const var*>(this); }
//	CONSTEXPR var_db::operator       var&()       &  { return *reinterpret_cast<var*>(this); }
//	CONSTEXPR var_db::operator       var()        && { return *reinterpret_cast<const var*>(this); }

//	CONSTEXPR exo::var_db::operator exo::var&() & {return *reinterpret_cast<exo::var*>(this);};

#endif

}  // namespace exo

// clang-format on

#endif //LIBEXODUS_VAR_H_
