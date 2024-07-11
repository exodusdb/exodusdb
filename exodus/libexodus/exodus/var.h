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

// Info used in cli/compile
//
#define EXODUS_RELEASE "24.07"
#define EXODUS_PATCH "24.07.0"

#include <iostream>
#include <string>
#include <string_view>
#include <array> // used in operator<<

// var_base provides the basic var-like functionality for var
//
#include <exodus/varb.h>

// Use ASCII 0x1A-0x1F for PickOS separator chars instead
// of PickOS 0xFA-0xFF which are illegal utf-8 bytes

// Also defined in pgexodus in extract.c etc.

// The var versions of the following (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// Leading _ char* versions of classic pick delimiters
// Using macros to allow use of space as compile time concatenation operator
// e.g. _FM _VM will compile directly to "\x1F\x1E"

#define _RM "\x1F"  // Record Mark
#define _FM "\x1E"  // Field Mark
#define _VM "\x1D"  // Value Mark
#define _SM "\x1C"  // Subvalue Mark
#define _TM "\x1B"  // Text Mark
#define _ST "\x1A"  // Subtext Mark

#define _BS "\\"
#define _DQ "\""
#define _SQ "\'"

// trailing _ char versions of classic pick delimiters
//

CONSTINIT_OR_CONSTEXPR auto RM_ = '\x1F';     // Record Mark
CONSTINIT_OR_CONSTEXPR auto FM_ = '\x1E';     // Field Mark
CONSTINIT_OR_CONSTEXPR auto VM_ = '\x1D';     // Value Mark
CONSTINIT_OR_CONSTEXPR auto SM_ = '\x1C';     // Subvalue Mark
CONSTINIT_OR_CONSTEXPR auto TM_ = '\x1B';     // Text Mark
CONSTINIT_OR_CONSTEXPR auto ST_ = '\x1A';     // Subtext Mark

CONSTINIT_OR_CONSTEXPR auto BS_ = '\\';
CONSTINIT_OR_CONSTEXPR auto DQ_ = '\"';
CONSTINIT_OR_CONSTEXPR auto SQ_ = '\'';

// Visible versions of FM, VM etc.
//
// printx() converts FM etc to these characters. user literal conversion _var also has them but hard coded in fmiconverter()
//#define _VISIBLE_FMS "_^]\[Z"  //PickOS standard. Backslash not good since it is often used for escaping chars. Z is normal letter.
//#define _VISIBLE_FMS "<[{}]>" //logical but hard to read direction of brackets quickly
//#define _VISIBLE_FMS "_^]}`~" //all uncommon in natural language. first 3 _^] are identical to pickos
//#define _VISIBLE_FMS "_^]}|~"   //all uncommon in natural language. first 3 _^] are identical to pickos
#define _VISIBLE_FMS "`^]}|~"   //all uncommon in natural language. ^] are identical to pickos. Using ` for RM since _ common in IT
CONSTINIT_OR_CONSTEXPR auto VISIBLE_RM_ = '`';
CONSTINIT_OR_CONSTEXPR auto VISIBLE_FM_ = '^';
CONSTINIT_OR_CONSTEXPR auto VISIBLE_VM_ = ']';
CONSTINIT_OR_CONSTEXPR auto VISIBLE_SM_ = '}';
CONSTINIT_OR_CONSTEXPR auto VISIBLE_TM_ = '|';
CONSTINIT_OR_CONSTEXPR auto VISIBLE_ST_ = '~';
#define _ALL_FMS _RM _FM _VM _SM _TM _ST

// Useful TRACE() function for debugging
//
#define TRACE(EXPRESSION) \
	var(EXPRESSION).convert(_ALL_FMS, _VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "=");
#define TRACE2(EXPRESSION) \
	std::cerr << (EXPRESSION) << std::endl;

// Readability for defaults
//
#define DEFAULT_UNASSIGNED = var()
#define DEFAULT_EMPTY = ""
#define DEFAULT_DOT = "."
#define DEFAULT_SPACE = " "
#define DEFAULT_VM = VM_
#define DEFAULT_NULL = nullptr

// Support var::format and var::println
//
#if __GNUC__  > 11 || __clang_major__ > 1
#	define EXO_FORMAT
#	ifdef EXO_FORMAT
#		pragma GCC diagnostic ignored "-Winline"
#		pragma clang diagnostic ignored "-Wswitch-default" //18 24.04
#		pragma clang diagnostic ignored "-Wunsafe-buffer-usage" //18 24.04
#		pragma clang diagnostic ignored "-Wreserved-id-macro" //18 20.04
#		pragma clang diagnostic ignored "-Wduplicate-enum" //18 20.04
#		include <fmt/format.h>
#	endif
#endif

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

// Note:
// Inside class var_base
// using VAR    =       var_base;
// using VARREF =       var_base&;
// using CVR    = const var_base&;
// using TVR    =       var_base&&;

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                               VAR
///////////////////////////////////////////////////////////////////////////////////////////////////

// class var
// using CRTP to capture a customised base class that knows what a var is
class PUBLIC var : public var_mid<var> {

//// class var
//// using CRTP to capture a customised base class that knows what a var is
//class PUBLIC var : public var_base<var> {

	using VARREF = var&;
	using CVR    = const var&;

	friend class dim;
	friend class rex;

	friend var operator""_var(const char* cstr, std::size_t size);

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
	CONSTEXPR
	var(std::initializer_list<T> list)
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
	PUBLIC friend std::ostream& operator<<(std::ostream& ostream1, var outvar) {

		outvar.assertString(__PRETTY_FUNCTION__);

		constexpr std::array VISIBLE_FMS_EXCEPT_ESC {VISIBLE_ST_, TM_, VISIBLE_SM_, VISIBLE_VM_, VISIBLE_FM_, VISIBLE_RM_};

		// Replace various unprintable field marks with unusual ASCII characters
		// Leave ESC as \x1B because it is used to control ANSI terminal control sequences
		// std::string str = "\x1A\x1B\x1C\x1D\x1E\x1F";
		// |\x1B}]^~  or in high to low ~^]}\x1B|     or in TRACE() ... ~^]}_|
		for (auto& charx : outvar.var_str) {
			if (charx <= 0x1F && charx >= 0x1A) {
				UNLIKELY
				//charx = "|\x1B}]^~"[charx - 0x1A];
				charx = VISIBLE_FMS_EXCEPT_ESC[charx - 0x1A];
			}
		}

		// use toString() to avoid creating a constructor which logs here recursively
		// should this use a ut16/32 -> UTF8 code facet? or convert to UTF8 and output to ostream?
		ostream1 << outvar.var_str;
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
	ND var operator()(int fieldno, int valueno = 0, int subvalueno = 0) const {return this->f(fieldno, valueno, subvalueno);}
	//ND var operator()(int fieldno, int valueno = 0, int subvalueno = 0) &&      {return a(fieldno, valueno, subvalueno);}

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

	var operator++(int) &;
	var operator--(int) &;

	VARREF operator++() &;
	VARREF operator--() &;

	// OUTPUT
	/////////

	// To stdout/cout buffered
	CVR output() const;      // stdout no new line, buffered
	CVR outputl() const;     // stdout starts a new line, flushed
	CVR outputt() const;     // stdout adds a tab, buffered

	// As above but with a prefix
	CVR output(CVR var1) const;  // stdout with a prefix, no new line, buffered
	CVR outputl(CVR var1) const; // stdout with a prefix, starts a new line, flushed
	CVR outputt(CVR var1) const; // stdout with a prefix, adds a tab, buffered

	// To stdlog/clog buffered
	CVR logput() const;  // stdlog no new line, buffered
	CVR logputl() const; // stdlog starts a new line, flushed

	// As above but with a prefix
	CVR logput(CVR var1) const;  // stdlog with a prefix, no new line, buffered
	CVR logputl(CVR var1) const; // stdlog with a prefix, starts a new line, flushed

	// To stderr/cerr usually unbuffered
	CVR errput() const;  // stderr no new line, flushed
	CVR errputl() const; // stderr starts a new line, flushed

	// As above but with a prefix
	CVR errput(CVR var1) const;  // stderr with a prefix, no new line, flushed
	CVR errputl(CVR var1) const; // stderr with a prefix, starts a new line, flushed

	// Output to a given stream
	CVR put(std::ostream& ostream1) const;

	// STANDARD INPUT
	/////////////////

	VARREF input();
	VARREF input(CVR prompt);
	VARREF inputn(const int nchars);

	ND bool isterminal() const;
	ND bool hasinput(int milliseconds = 0) const;
	ND bool eof() const;
	bool echo(const int on_off) const;

	void breakon() const;
	void breakoff() const;

	// MATH/BOOLEAN
	///////////////

	ND var abs() const;
// Moved to var_base
//	ND var mod(CVR divisor) const;
//	ND var mod(double divisor) const;
//	ND var mod(const int divisor) const;
	ND var pwr(CVR exponent) const;
	ND var rnd() const;
	void initrnd() const;
	ND var exp() const;
	ND var sqrt() const;
	ND var sin() const;
	ND var cos() const;
	ND var tan() const;
	ND var atan() const;
	ND var loge() const;
	//ND var int() const;//reserved word
	ND var integer() const;//returns a var in case you need a var and not an int  which the c++ built-in int(varx) produces
	ND var floor() const;
	ND var round(const int ndecimals = 0) const;

	// LOCALE
	/////////

	bool setxlocale() const;
	ND VARREF getxlocale();

	// STRING CREATION
	//////////////////

	// var chr() const;
	// version 1 chr - only char 0 - 255 returned in a single byte
	// bytes 128-255 are not valid utf-8 so cannot be written to database/postgres
	ND var chr(const int num) const;  // ASCII

	// version 2 textchr - returns utf8 byte sequences for all unicode code points
	// not unsigned int so to get utf codepoints > 2^63 must provide negative ints
	// not providing implicit constructor from var to unsigned int due to getting ambigious conversions
	// since int and unsigned int are parallel priority in c++ implicit conversions
	ND var textchr(const int num) const;  // UTF8

	ND var str(const int num) const;
	ND var space() const;

	ND var numberinwords(CVR languagename_or_locale_id DEFAULT_EMPTY);

	////////////
	// STRING // All utf8 unless mentioned
	////////////

	// STRING INFO
	//////////////

	ND var seq() const;     // byte

	ND var textseq() const;

	ND var len() const;     // bytes

	// number of output columns. Allows multi column unicode and reduces combining characters etc. like e followed by grave accent
	// Possibly does not properly calculate combining sequences of graphemes e.g. face followed by colour
	ND var textwidth() const;

	// STRING SCANNING
	//////////////////

	ND var textlen() const;
	ND var fcount(SV str) const;
	ND var count(SV str) const;
	ND var match(SV regex, SV regex_options DEFAULT_EMPTY) const;

	//                                  Javascript   PHP             Python       Go          Rust          C++
	ND bool starts(SV str) const;    // startsWith() str_starts_with startswith() HasPrefix() starts_with() starts_with
	ND bool ends(SV str) const;      // endsWith     str_ends_with   endswith     HasSuffix() ends_with()   ends_with
	ND bool contains(SV str) const;  // includes()   str_contains    contains()   Contains()  contains()    contains

	//https://en.wikipedia.org/wiki/Comparison_of_programming_languages_(string_functions)#Find
	//ND var index(SV str) const;
	ND var index(SV str, const int startchar1 = 1) const;   // return byte no if found or 0 if not. startchar1 is byte no to start at.
	ND var indexn(SV str, const int occurrence) const;      // ditto. occurrence 1 = find first occurrence
	ND var indexr(SV str, const int startchar1 = -1) const; // ditto. reverse search. startchar1 -1 starts from the last byte

	ND var search(SV regex, VARREF startchar1, SV regex_options DEFAULT_EMPTY) const; // returns match with groups and startchar1 one after matched

//	//static member for speed on std strings because of underlying boost implementation
//	static int localeAwareCompare(const std::string& str1, const std::string& str2);
//	//int localeAwareCompare(const std::string& str2) const;

	// STRING CONVERSION - Non-mutating
	////////////////////

	ND var ucase() const&;
	ND var lcase() const&;
	ND var tcase() const&;
	ND var fcase() const&;
	ND var normalize() const&;
	ND var invert() const&;

	ND var lower() const&;
	ND var raise() const&;
	ND var crop() const&;

	ND var quote() const&;
	ND var squote() const&;
	ND var unquote() const&;

	ND var trim(SV trimchars  DEFAULT_SPACE) const&;     // byte trimchars
	ND var trimfirst(SV trimchars DEFAULT_SPACE) const&; // byte trimchars
	ND var trimlast(SV trimchars DEFAULT_SPACE) const&;  // byte trimchars
	ND var trimboth(SV trimchars DEFAULT_SPACE) const&;  // byte trimchars

	ND var first() const&; // max 1 byte
	ND var last() const&;  // max 1 byte
	ND var first(const size_t length) const&; // byte length
	ND var last(const size_t length) const&;  // byte length
	ND var cut(const int length) const&;      // byte length
	ND var paste(const int pos1, const int length, SV insertstr) const&; // byte pos1, length
	ND var paste(const int pos1, SV insertstr) const&; // byte pos1
	ND var prefix(SV insertstr) const&;
	//ND var append(SV appendstr) const&;
	ND var pop() const&;                      // byte removed

	// var.fieldstore(separator,fieldno,nfields,replacement)
	ND var fieldstore(SV separator, const int fieldno, const int nfields, CVR replacement) const&;

	//ND var substr(const int pos1, const int length) const&; // byte pos1, length
	ND var substr(const int startindex1, const int length) const&;
	//ND var substr(const int pos1) const&;                   // byte pos1
	ND var substr(const int startindex1) const&;

	ND var b(const int pos1, const int length) const&; // byte pos1, length
	ND var b(const int pos1) const&; // byte pos1

	ND var convert(SV fromchars, SV tochars) const&;        // byte fromchars, tochars
	ND var textconvert(SV fromchars, SV tochars) const&;

	ND var replace(SV fromstr, SV tostr) const&;
	ND var replace(const rex& regex, SV tostr) const&;
	//ND var regex_replace(SV regex, SV replacement, SV regex_options DEFAULT_EMPTY) const&;

	ND var unique() const&;
	ND var sort(SV sepchar = _FM) const&;
	ND var reverse(SV sepchar = _FM) const&;
	ND var shuffle(SV sepchar = _FM) const&;
	ND var parse(char sepchar = ' ') const&;

	// SAME ON TEMPORARIES - MUTATE FOR SPEED
	/////////////////////////////////////////

	// utf8/byte as for accessors

	ND VARREF ucase() &&;
	ND VARREF lcase() &&;
	ND VARREF tcase() &&;
	ND VARREF fcase() &&;
	ND VARREF normalize() &&;
	ND VARREF invert() &&;

	ND VARREF lower() &&;
	ND VARREF raise() &&;
	ND VARREF crop() &&;

	ND VARREF quote() &&;
	ND VARREF squote() &&;
	ND VARREF unquote() &&;

	ND VARREF trim(SV trimchars DEFAULT_SPACE) &&;
	ND VARREF trimfirst(SV trimchars DEFAULT_SPACE) &&;
	ND VARREF trimlast(SV trimchars DEFAULT_SPACE) &&;
	ND VARREF trimboth(SV trimchars DEFAULT_SPACE) &&;

	ND VARREF first() &&;
	ND VARREF last() &&;
	ND VARREF first(const size_t length) &&;
	ND VARREF last(const size_t length) &&;
	ND VARREF cut(const int length) &&;
	ND VARREF paste(const int pos1, const int length, SV insertstr) &&;
	ND VARREF paste(const int pos1, SV insertstr) &&;
	ND VARREF prefix(SV insertstr) &&;
	//ND VARREF append(SV appendstr) &&;
	ND VARREF pop() &&;

	ND VARREF fieldstore(SV sepchar, const int fieldno, const int nfields, CVR replacement) &&;
	ND VARREF substr(const int pos1, const int length) &&;
	ND VARREF substr(const int pos1) &&;

	ND VARREF convert(SV fromchars, SV tochars) &&;
	ND VARREF textconvert(SV fromchars, SV tochars) &&;
	ND VARREF replace(const rex& regex, SV tostr) &&;
	ND VARREF replace(SV fromstr, SV tostr) &&;
	//ND VARREF regex_replace(SV regex, SV replacement, SV regex_options DEFAULT_EMPTY) &&;

	ND VARREF unique() &&;
	ND VARREF sort(SV sepchar = _FM) &&;
	ND VARREF reverse(SV sepchar = _FM) &&;
	ND VARREF shuffle(SV sepchar = _FM) &&;
	ND VARREF parse(char sepchar = ' ') &&;

	// STRING MUTATORS
	//////////////////

	// utf8/byte as for accessors

	VARREF ucaser();
	VARREF lcaser();
	VARREF tcaser();
	VARREF fcaser();
	VARREF normalizer();
	VARREF inverter();

	VARREF quoter();
	VARREF squoter();
	VARREF unquoter();

	VARREF lowerer();
	VARREF raiser();
	VARREF cropper();

	VARREF trimmer(SV trimchars DEFAULT_SPACE);
	VARREF trimmerfirst(SV trimchars DEFAULT_SPACE);
	VARREF trimmerlast(SV trimchars DEFAULT_SPACE);
	VARREF trimmerboth(SV trimchars DEFAULT_SPACE);

	VARREF firster();
	VARREF laster();
	VARREF firster(const size_t length);
	VARREF laster(const size_t length);
	VARREF cutter(const int length);
	VARREF paster(const int pos1, const int length, SV insertstr);
	VARREF paster(const int pos1, SV insertstr);
	VARREF prefixer(SV insertstr);
	//VARREF appender(SV appendstr);
	VARREF popper();

	VARREF fieldstorer(SV sepchar, const int fieldno, const int nfields, CVR replacement);
	VARREF substrer(const int pos1, const int length);
	//VARREF substrer(const int pos1);
	// TODO look at using erase to speed up
	//VARREF substrer(const int startindex1) {this->toString();return this->substrer(startindex1, static_cast<int>(var_str.size()));}
	//VARREF substrer(const int startindex1) {return this->substrer(startindex1, static_cast<int>(var_str.size()));}
	VARREF substrer(const int startindex1) {this->assertString(__PRETTY_FUNCTION__);return this->substrer(startindex1, static_cast<int>(var_str.size()));}

	VARREF converter(SV fromchars, SV tochars);
	VARREF textconverter(SV fromchars, SV tochars);
	VARREF replacer(const rex& regex, SV tostr);
	VARREF replacer(SV fromstr, SV tostr);
	//VARREF regex_replacer(SV regex, SV replacement, SV regex_options DEFAULT_EMPTY);

	VARREF uniquer();
	VARREF sorter(SV sepchar = _FM);
	VARREF reverser(SV sepchar = _FM);
	VARREF shuffler(SV sepchar = _FM);
	VARREF parser(char sepchar = ' ');

	// OTHER STRING ACCESS
	//////////////////////

	ND var hash(const uint64_t modulus = 0) const;

	ND dim split(SV sepchar = _FM) const;

	// v3 - returns bytes from some byte number upto the first of a given list of bytes
	// this is something like std::string::find_first_of but doesnt return the delimiter found
	var substr(const int pos1, CVR delimiterchars, int& endindex) const;
	var b(const int pos1, CVR delimiterchars, int& endindex) const {return substr(pos1, delimiterchars, endindex);}

	// v4 - like v3. was named "remove" in pick. notably used in nlist to print parallel columns
	// of mixed combinations of multivalues/subvalues and text marks
	// correctly lined up mv to mv, sv to sv, tm to tm even when particular columns were missing
	// some vm/sm/tm
	// it is like substr(pos1,delimiterbytes,endindex) except that the delimiter bytes are
	// hard coded as the usual RM/FM/VM/SM/TM/ST
	// except that it updates the startstopindex to point one after found delimiter byte and
	// returns the delimiter no (1-6)
	// if no delimiter byte is found then it returns bytes up to the end of the string, sets
	// startstopindex to after tne end of the string and returns delimiter no 0 NOTE that it
	// does NOT remove anything from the source string var remove(VARREF pos1, VARREF
	// delimiterno) const;
	var substr2(VARREF startstopindex, VARREF delimiterno) const;
	var b2(VARREF startstopindex, VARREF delimiterno) const {return substr2(startstopindex, delimiterno);}

	ND var field(SV strx, const int fieldnx = 1, const int nfieldsx = 1) const;

	// field2 is a version that treats fieldn -1 as the last field, -2 the penultimate field etc. -
	// TODO Should probably make field() do this (since -1 is basically an erroneous call) and remove field2
	ND var field2(SV separator, const int fieldno, const int nfields = 1) const {
		if (fieldno >= 0) LIKELY
			return field(separator, fieldno, nfields);
		return field(separator, this->count(separator) + 1 + fieldno + 1, nfields);
	}

	// I/O CONVERSION
	/////////////////

	ND var oconv(const char* convstr) const;
	ND var iconv(const char* convstr) const;

#ifdef EXO_FORMAT

#if __GNUC__ >= 7 || __clang_major__ > 15
	// Works at compile time (only? or if possible)
#	define EXO_FORMAT_STRING_TYPE1 fmt::format_string<var, Args...>
#	define EXO_FORMAT_STRING_TYPE2 fmt::format_string<Args...>
#else
	// Always run time
#	define EXO_FORMAT_STRING_TYPE1 SV
#	define EXO_FORMAT_STRING_TYPE2 SV
#endif

template<class... Args>
	ND CONSTEXPR
	var format(EXO_FORMAT_STRING_TYPE1&& fmt_str, Args&&... args) const {

//error: call to consteval function 'fmt::basic_format_string<char, exo::var &>
//::basic_format_string<fmt::basic_format_string<char, const exo::var &>, 0>' is not a constant expression
// 2033 |                         return fmt::format(fmt_str, *this, args... );
//      |                                            ^

#if __clang_major__ == 0
#if __cpp_if_consteval >= 202106L
		if consteval {
#else
		if (std::is_constant_evaluated()) {
#endif
// OK in 2404 g++ but not OK in 2404 clang
// clang on 22.04 cannot accept compile time format string even if a cstr
///root/exodus/test/src/test_format.cpp:14:18: error: call to consteval function 'fmt::basic_format_string<char, exo::var>::basic_format_string<ch
//ar[7], 0>' is not a constant expression
//        assert(x.format("{:.2f}").outputl() == "12.35");

			return fmt::format(std::forward<EXO_FORMAT_STRING_TYPE1>(fmt_str), *this, std::forward<Args>(args)... );
		} else
#endif

		{
			return fmt::vformat(fmt_str, fmt::make_format_args(*this, std::forward<Args>(args)...) );
		}
	}

	template<class... Args>
	ND var vformat(SV fmt_str, Args&&... args) const {
		return fmt::vformat(fmt_str, fmt::make_format_args(*this, args...) );
	}

#endif //EXO_FORMAT

	ND var from_codepage(const char* codepage) const;
	ND var to_codepage(const char* codepage) const;

	// CLASSIC MV STRING FUNCTIONS
	//////////////////////////////

	// this function hardly occurs anywhere in exodus code and should probably be renamed to
	// something better it was called replace() in Pick Basic but we are now using "replace()" to
	// change substrings using regex (similar to the old Pick Basic replace function) its mutator function
	// is .r()
//	ND var pickreplace(const int fieldno, const int valueno, const int subvalueno, CVR replacement) const;
//	ND var pickreplace(const int fieldno, const int valueno, CVR replacement) const;
//	ND var pickreplace(const int fieldno, CVR replacement) const;
	ND var pickreplace(const int fieldno, const int valueno, const int subvalueno, CVR replacement) const {return var(*this).r(fieldno, valueno, subvalueno, replacement);}
	ND var pickreplace(const int fieldno, const int valueno, CVR replacement) const {return var(*this).r(fieldno, valueno, 0, replacement);}
	ND var pickreplace(const int fieldno, CVR replacement) const {return var(*this).r(fieldno, 0, 0, replacement);}

	// cf mutator inserter()
	ND var insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) const& {return var(*this).inserter(fieldno, valueno, subvalueno, insertion);}
	ND var insert(const int fieldno, const int valueno, CVR insertion) const& {return this->insert(fieldno, valueno, 0, insertion);}
	ND var insert(const int fieldno, CVR insertion) const& {return this->insert(fieldno, 0, 0, insertion);}

	/// remove() was delete() in Pick Basic
	// var erase(const int fieldno, const int valueno=0, const int subvalueno=0) const;
	//ND var remove(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	ND var remove(const int fieldno, const int valueno = 0, const int subvalueno = 0) const {return var(*this).remover(fieldno, valueno, subvalueno);}

	//.f(...) stands for .attribute(...) or extract(...)
	// Pick Basic
	// xxx=yyy<10>";
	// becomes c++
	// xxx=yyy.f(10);
	ND var f(const int fieldno, const int valueno = 0, const int subvalueno = 0) const;
	ND var extract(const int fieldno, const int valueno = 0, const int subvalueno = 0) const {return this->f(fieldno, valueno, subvalueno);}

	// SAME AS ABOVE ON TEMPORARIES
	///////////////////////////////

	ND VARREF insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) && {return this->inserter(fieldno, valueno, subvalueno, insertion);}
	ND VARREF insert(const int fieldno, const int valueno, CVR insertion) && {return this->inserter(fieldno, valueno, 0, insertion);}
	ND VARREF insert(const int fieldno, CVR insertion) && {return this->inserter(fieldno, 0, 0, insertion);}

	// MV STRING FILTERS
	////////////////////

	ND var sum() const;
	ND var sumall() const;
	ND var sum(SV sepchar) const;

	// binary ops + - * / : on mv strings 10]20]30
	// e.g. var("10]20]30").mv("+","2]3]4")
	// result is "12]23]34"
	ND var mv(const char* opcode, CVR var2) const;

	// MV STRING MUTATORS
	/////////////////////

	// mutable versions update and return source
	// r stands for "replacer" abbreviated due to high incidience in code
	// Pick Basic
	// xyz<10>="abc";
	// becomes c++
	//  xyz.r(10,"abc");

	// r() is short for replacer() since it is probably the most common var function after a()
	VARREF r(const int fieldno, const int valueno, const int subvalueno, CVR replacement);
	//VARREF r(const int fieldno, const int valueno, CVR replacement);
	//VARREF r(const int fieldno, CVR replacement);
	VARREF r(const int fieldno, const int valueno, CVR replacement) {return r(fieldno, valueno, 0, replacement);}
	VARREF r(const int fieldno, CVR replacement) {	return r(fieldno, 0, 0, replacement);}

	VARREF inserter(const int fieldno, const int valueno, const int subvalueno, CVR insertion);
	VARREF inserter(const int fieldno, const int valueno, CVR insertion) {return this->inserter(fieldno, valueno, 0, insertion);}
	VARREF inserter(const int fieldno, CVR insertion) {return this->inserter(fieldno, 0, 0, insertion);}

	// VARREF eraser(const int fieldno, const int valueno=0, const int subvalueno=0);
	VARREF remover(const int fieldno, const int valueno = 0, const int subvalueno = 0);
	//-er version could be extract and erase in one go
	// VARREF extracter(int fieldno,int valueno=0,int subvalueno=0) const;

	// MV STRING LOCATORS
	/////////////////////

	// should these be like extract, replace, insert, delete
	// locate(fieldno, valueno, subvalueno,target,setting,by DEFAULT_EMPTY)
	ND bool locate(CVR target) const;
	ND bool locate(CVR target, VARREF setting) const;
	ND bool locate(CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	ND bool locateusing(const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;
	ND bool locateusing(const char* usingchar, CVR target) const;

	// locateby without fieldno or valueno arguments uses character VM
	ND bool locateby(const char* ordercode, CVR target, VARREF setting) const;

	// locateby with fieldno=0 uses character FM
	ND bool locateby(const char* ordercode, CVR target, VARREF setting, const int fieldno, const int valueno = 0) const;

	// locatebyusing
	ND bool locatebyusing(const char* ordercode, const char* usingchar, CVR target, VARREF setting, const int fieldno = 0, const int valueno = 0, const int subvalueno = 0) const;

	// DATABASE ACCESS
	//////////////////

	ND bool connect(CVR conninfo DEFAULT_EMPTY);
	void disconnect();
	void disconnectall();

	ND bool attach(CVR filenames);
	void detach(CVR filenames);

	// var() is a db connection or default connection
	ND bool begintrans() const;
	ND bool rollbacktrans() const;
	ND bool committrans() const;
	ND bool statustrans() const;
	   void cleardbcache() const;

	ND bool sqlexec(CVR sqlcmd) const;
	ND bool sqlexec(CVR sqlcmd, VARREF response) const;

	ND var  lasterror() const;
	   var  loglasterror(CVR source DEFAULT_EMPTY) const;

	// DATABASE MANAGEMENT
	//////////////////////

	ND bool dbcreate(CVR dbname) const;
	ND var  dblist() const;
	ND bool dbcopy(CVR from_dbname, CVR to_dbname) const;
	ND bool dbdelete(CVR dbname) const;

	ND bool createfile(CVR filename) const;
	ND bool renamefile(CVR filename, CVR newfilename) const;
	ND bool deletefile(CVR filename) const;
	ND bool clearfile(CVR filename) const;
	ND var  listfiles() const;

	ND bool createindex(CVR fieldname, CVR dictfile DEFAULT_EMPTY) const;
	ND bool deleteindex(CVR fieldname) const;
	ND var  listindex(CVR filename DEFAULT_EMPTY, CVR fieldname DEFAULT_EMPTY) const;

	// bool selftest() const;
	ND var  version() const;

	ND var  reccount(CVR filename DEFAULT_EMPTY) const;
	   var  flushindex(CVR filename DEFAULT_EMPTY) const;

	ND bool open(CVR dbfilename, CVR connection DEFAULT_EMPTY);
	   void close();

	// 1=ok, 0=failed, ""=already locked
	ND var  lock(CVR key) const;
	// void unlock(CVR key) const;
	// void unlockall() const;
	   bool unlock(CVR key) const;
	   bool unlockall() const;

	// db file i/o
	ND bool read(CVR filehandle, CVR key);
	   bool write(CVR filehandle, CVR key) const;
	   bool deleterecord(CVR key) const;
	ND bool updaterecord(CVR filehandle, CVR key) const;
	ND bool insertrecord(CVR filehandle, CVR key) const;

	// specific db field i/o
	ND bool readf(CVR filehandle, CVR key, const int fieldno);
	   bool writef(CVR filehandle, CVR key, const int fieldno) const;

	// cached db file i/o
	ND bool readc(CVR filehandle, CVR key);
	   bool writec(CVR filehandle, CVR key) const;
	   bool deletec(CVR key) const;

	// ExoEnv function now to allow access to RECORD ID DICT etc. and call external
	// functions
	// var calculate() const;

	ND var  xlate(CVR filename, CVR fieldno, const char* mode) const;

	// DATABASE SORT/SELECT
	///////////////////////

	ND bool select(CVR sortselectclause DEFAULT_EMPTY);
	   void clearselect();

	//ND bool hasnext() const;
	ND bool hasnext();
	ND bool readnext(VARREF key);
	ND bool readnext(VARREF key, VARREF valueno);
	ND bool readnext(VARREF record, VARREF key, VARREF valueno);

	   bool savelist(CVR listname);
	ND bool getlist(CVR listname);
	ND bool makelist(CVR listname, CVR keys);
	   bool deletelist(CVR listname) const;
	ND bool formlist(CVR keys, CVR fieldno = 0);

	//bool saveselect(CVR filename);

	// OS TIME/DATE
	///////////////

	ND var  date() const;//int days since pick epoch 1967-12-31
	ND var  time() const;//int seconds since last midnight
	ND var  ostime() const;
	ND var  timestamp() const; // floating point fractional days since pick epoch 1967-12-31 00:00:00
	ND var  timestamp(CVR ostime) const; // construct a timestamp from a date and time

	   void ossleep(const int milliseconds) const;
	   var  oswait(const int milliseconds, SV directory) const;

	// OS FILE SYSTEM
	/////////////////

	ND bool osopen(CVR filename, const char* locale DEFAULT_EMPTY) const;
	ND bool osbread(CVR osfilevar, VARREF offset, const int length);
	ND bool osbwrite(CVR osfilevar, VARREF offset) const;
	   void osclose() const;

	ND bool osread(const char* osfilename, const char* codepage DEFAULT_EMPTY);
	ND bool oswrite(CVR osfilename, const char* codepage DEFAULT_EMPTY) const;
	ND bool osremove() const;
	ND bool osrename(CVR new_dirpath_or_filepath) const;
	ND bool oscopy(CVR to_osfilename) const;
	ND bool osmove(CVR to_osfilename) const;

	ND var  oslist(SV globpattern DEFAULT_EMPTY, const int mode = 0) const;
	ND var  oslistf(SV globpattern DEFAULT_EMPTY) const;
	ND var  oslistd(SV globpattern DEFAULT_EMPTY) const;
	ND var  osinfo(const int mode) const;
	ND var  osfile() const;
	ND var  osdir() const;
	ND var  osinfo() const;
	ND bool osmkdir() const;
	ND bool osrmdir(bool evenifnotempty = false) const;

	// TODO check for threadsafe
	ND var ospid() const;
	ND var ostid() const;
	ND var oscwd() const;
	ND bool oscwd(CVR newpath) const;
	void osflush() const;

	// OS SHELL/ENVIRONMENT
	///////////////////////

	ND bool osshell() const;
	ND bool osshellread(CVR oscmd);
	ND bool osshellwrite(CVR oscmd) const;
	ND var  ostempdirpath() const;
	ND var  ostempfilename() const;

	ND bool osgetenv(const char* code);
	   void ossetenv(const char* code) const;

	friend class var_iter;

	//BEGIN/END - free functions to create iterators over a var
	PUBLIC friend var_iter begin(CVR v);
	PUBLIC friend var_iter end(CVR v);

	///////////////////////////
	// PRIVATE MEMBER FUNCTIONS
	///////////////////////////

 private:

	ND bool cursorexists(); //database, not terminal
	ND bool selectx(CVR fieldnames, CVR sortselectclause);

	   var  setlasterror(CVR msg) const;

	// TODO check if can speed up by returning reference to converted self like MC

	// Faster primitive oconv
	// L/R/C: Text -> left/right/center padded and truncated
	ND std::string oconv_LRC(CVR format) const;
	// T: Text -> justified and folded
	ND std::string oconv_T(CVR format) const;
	// D: Int -> Date
	ND std::string oconv_D(const char* conversion) const;
	// MT: Int -> Time
	ND std::string oconv_MT(const char* conversion) const;
	// MD: Decimal -> Decimal
	ND std::string oconv_MD(const char* conversion) const;
	// MR: Character replacement
	ND VARREF oconv_MR(const char* conversion);
	// HEX: Chars -> Hex
	ND std::string oconv_HEX(const int ioratio) const;
	// TX: Record (FM) -> text (\n) and \ line endings
	ND std::string oconv_TX(const char* conversion) const;

	// Faster primitive iconv
	// D: Int <- Date
	ND var iconv_D(const char* conversion) const;
	// MT: Int <- Time
	ND var iconv_MT() const;
	// MD_ Decimal <- Decimal
	ND var iconv_MD(const char* conversion) const;
	// Chars <- Hex
	ND var iconv_HEX(const int ioratio) const;
	// TX: Record (FM) <- text (\n) and \ line endings
	ND var iconv_TX(const char* conversion) const;

	ND std::fstream* osopenx(CVR osfilename, const char* locale) const;

	bool THIS_IS_OSFILE() const { return ((var_typ & VARTYP_OSFILE) != VARTYP_UNA); }

	// Convert _VISIBLE_FMS to _ALL_FMS
	// In header to perhaps aid runtime string literal conversion for operator""_var
	// since currently it cannot be constexpr due to var containing a std::string
	VARREF fmiconverter() {
		for (char& c : this->var_str) {
			switch (c) {
				// Most common first to perhaps aid optimisation
				case VISIBLE_FM_: c = FM_; break;
				case VISIBLE_VM_: c = VM_; break;
				case VISIBLE_SM_: c = SM_; break;
				case VISIBLE_TM_: c = TM_; break;
				case VISIBLE_ST_: c = ST_; break;
				case VISIBLE_RM_: c = RM_; break;
				// All other chars left unconverted
				default:;
			}
		}
		return *this;
	}

	// Convert _ALL_FMS to _VISIBLE_FMS
	VARREF fmoconverter() {
		for (char& c : this->var_str) {
			if (c > RM_ || c > RM_) {
				switch (c) {
					// In order to perhaps aid optimisation
					case RM_: c = VISIBLE_RM_; break;
					case FM_: c = VISIBLE_FM_; break;
					case VM_: c = VISIBLE_VM_; break;
					case SM_: c = VISIBLE_SM_; break;
					case TM_: c = VISIBLE_TM_; break;
					case ST_: c = VISIBLE_ST_; break;
					// All other chars left unconverted;
					default:;
				}
			}
		}
		return *this;
	}

};  // class "var"

// NB we should probably NEVER add operator^(VARREF var1, bool)
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

/////////////////////////////////
// var_iter - iterate over fields
/////////////////////////////////
class PUBLIC var_iter {

	const var* pvar_;
	mutable std::string::size_type startpos_ = 0;
	mutable std::string::size_type endpos_ = std::string::npos;

 public:
	// Default constructor
	var_iter() = default;

	// Construct from var
	var_iter(CVR v);

	// Check iter != iter (i.e. iter != string::npos)
	bool operator!=(const var_iter& vi);

	// Convert to var
	var operator*() const;

	// Iter++
	var_iter operator++();

	// Iter-- prefix
	var_iter operator--();

	// --iter postfix
	var_iter operator--(int);

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
	void operator=(CVR replacement) {
		var_.r(fn_, replacement);
	}

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

	void operator=(CVR replacement) {
		var_.r(fn_, vn_, replacement);
	}

	explicit operator bool() const {
		return var_.f(fn_, vn_);
	}

	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND var operator[](const int pos1) const {
		return this->at(pos1);
	}

	ND var at(const int pos1) const;

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

	void operator=(CVR replacement) {
		var_.r(fn_, vn_, sn_, replacement);
	}

	explicit operator bool() const {
		return var_.f(fn_, vn_, sn_);
	}

	// DONT change deprecation wordng without also changing it in cli/fixdeprecated
	[[deprecated ("EXODUS: Replace single character accessors like xxx[n] with xxx.at(n)")]]
	ND var operator[](const int pos1) const {
		return this->at(pos1);
	}

	ND var at(const int pos1) const;

};

ND inline var_proxy1 var::operator()(int fieldno) {return var_proxy1(*this, fieldno);}
ND inline var_proxy2 var::operator()(int fieldno, int valueno) {return var_proxy2(*this, fieldno, valueno);}
ND inline var_proxy3 var::operator()(int fieldno, int valueno, int subvalueno) {return var_proxy3(*this, fieldno, valueno, subvalueno);}

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

#endif

/////////////////////////////////////
// A global flag used in mvdbpostgres
/////////////////////////////////////
[[maybe_unused]] static inline int DBTRACE = var().osgetenv("EXO_DBTRACE");

//
/////////////////////
// _var user literals
/////////////////////

// "abc^def"_var
PUBLIC var operator""_var(const char* cstr, std::size_t size);

// 123456_var
PUBLIC var operator""_var(unsigned long long int i);

// 123.456_var
PUBLIC var operator""_var(long double d);

}  // namespace exo

#endif //EXODUS_LIBEXODUS_EXODUS_VAR_H_
