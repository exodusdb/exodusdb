#ifndef EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_
#define EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_

#if EXO_MODULE
	import std;
#else
#	include <string>

// Using map for dict function cache instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// https://youtu.be/M2fKMP47slQ?t=476
#	include <map>
//#include <exodus/exocallable.h>
//#include "timeaccount.h"

#	include <exodus/var.h>
#endif

#include <exodus/vardefs.h>
#include <exodus/exoimpl.h>
#include <exodus/exoenv.h>
#include <exodus/exocallable.h>

namespace exo {

// mv.xyz is going to be used a lot by exodus programmers for exodus "global variables"
// eg mv.RECORD mv.DICT
//
// threadsafe! it is member data so it is global to the class/object and not global to the
// program
//
// it is a reference/pointer so that an external "subroutine" can be created which has
// identical exodus global variables to the "main program" as far as the exodus application
// programmer thinks
//
// being a reference ensures that exodus programs cannot exist without an mv
// however this restriction might be relaxed
//
// mv was initially a reference so that exodus application programmers could writew
// things like mv.ID (instead of the harder to understand, for an application programmer,
// mv->ID style) however now that a macro is used to generate mv.ID from just ID we could
// make mv to be a pointer and assign it as and when desired we would just need to change
//#define ID mv.ID
// to
//#define ID mv->ID
// so that ID RECORD etc. continue to appear to the application programmer to be "threadsafe
// global" variables

//class ExoProgram
class PUBLIC ExoProgram {

 private:

	// used by calculate to call dict libraries
	mutable std::string cached_dictid_;
	mutable var cached_dictrec_ = "";

	// A callable used in ExoProgram::calculate to call dict items in dict_xxxxxx.cpp
	mutable Callable* dict_callable_ = nullptr;

	// A cache of Callables dict functions
	std::map<std::string, Callable*> cached_dict_functions;

	// A callable used in ExoProgram::perform to call libraries WITH NO ARGUMENTS
	mutable Callable perform_callable_;

 public:

	// mv.XXXXXX is going to be used a lot by exodus programmers for exodus "global variables"
	// e.g. mv.RECORD mv.DICT etc.
	//
	// Threadsafe. Environment vars are member data so they are "global" to the
	// ExoProgram class/object and not global to the program.
	//
	// Not using thread_local for ExoProgram environment so they are safe even if you have
	// multiple ExodusProgram with different environments in the same thread.
	// However vardb.cpp uses a lot of threadlocal storage for database connectivity.
	//
	// External "subroutine" ExodusProgram objects in shared libraries can be created which has
	// identical exodus global variables to the "main program" as far as the exodus application
	// programmer can tell.
	//
	// mv being a reference ensures that exodus programs cannot exist without an mv
	//
	// Using a reference instead of a pointer allows debugging via mv.ID instead of mv->ID
	//
	// Macros are provided for all env vars like
	// #define ID mv.ID
	// to allow code to be written without the mv. prefix for readability
	// #define ID mv->ID //This form is not required because mv is reference.
	ExoEnv& mv;

 public:

	// Function arguments
	using in  = const var&; // same as CVR
	using io  =       var&; // same as VARREF
	using out =       var&; // same as VARREF

	// Constant var
	using let = const var; // same as CV

	// Not passing io/out parameters by pointer as per several style guides because
	// it is far too easy to type
	//
	//  outvar[4]
	//
	// thinking to get a substr when you meant
	//
	//  (*outvar)[4]
	//
	// and c++ happily compiles outvar[4] thinking that outvar is an array of pointers
	//
	// and gives random memory access as run time as you access the non-existent element [4].

#include <exodus/ioconv_custom.h>

	explicit ExoProgram(ExoEnv& inmv);
	//ExoProgram(ExoEnv& mv);
	//explicit ExoProgram(ExoEnv&& inmv) = delete;

	// No default constructor
	// data member mv is a reference and we only provide constructors for preexisting ExoEnv.
	//ExoProgram() = default;
	ExoProgram() = delete;

	// doing virtual isnt much use because external functions (which are based on
	// ExoProgram) need to have complete freedom of arguments to main(...) virtual var
	// main();

	// Is this required?
	virtual ~ExoProgram();

	///////////////////
	///// Select lists:
	///////////////////

	// obj is

	// All the following work on an environment variable CURSOR.
	bool select(in sortselectclause_or_filehandle = "");
	bool selectkeys(in keys);
ND	bool hasnext();
	bool readnext(out key);
	bool readnext(out key, out valueno);
	bool readnext(out record, out key, out valueno);
	void pushselect(out cursor);
	void popselect(in cursor);
	void clearselect();
	// THIS should be removed from exoprog and made a free function that doesnt use CURSOR
	bool deleterecord(in filename_or_handle_or_command, in key = "");
	bool savelist(SV listname);
	bool getlist(SV listname);
	bool deletelist(SV listname);
	[[deprecated ("exoprog:::formlist() Resolve by refactoring. Read keys directly and call selectkeys(keys)")]]
	bool formlist(SV filename_or_command, in keys = "", const int fieldno = 0);
	[[deprecated ("exoprog::makelist() Refactor makelist(\"\", keys) as selectkeys(keys) or use fixdeprecated")]]
	bool makelist(SV listname, in keys);

	//////////////////////
	///// Perform/Execute:
	//////////////////////

	// Check if a command's first word  is a performable/executable exodus program.
	var  libinfo(in command);

	// Run an exodus library's main function using a command like syntax similar to that of executable programs.
	// A "command line" is passed to the library in the usual COMMAND, SENTENCE and OPTIONS environment variables instead of function arguments.
	// The library's main function should have zero arguments. Performing a library function with main arguments results in them being unassigned and in some case core dump may occur.
	// The following environment variables are initialised on entry to the main function of the library. They are preserved untouched in the calling program.
	// SENTENCE, COMMAND, OPTIONS: Initialised from the argument "sentence".
	// RECUR0, RECUR1, RECUR2, RECUR3, RECUR4 to "".
	// ID, RECORD, MV, DICT initialised to "".
	// LEVEL is incremented by one.
	// All other environment variables are shared between the caller and callee. There is essentially only one environment in any one process or thread.
	// Any active select list is passed to the performed program and can be consumed by it. Conversely any active select list created by the library will be returned to the calling program.
	// sentence: The first word of this argument is used as the name of the library to be loaded and run. sentence is used to initialise the SENTENCE, COMMAND and OPTIONS environment variables.
	// Returns: Whatever var the library returns, or "" if it calls stop() or abort(()".
    // The return value can be ignored and discarded without any compiler warning.
	// Note that library functions may also be directly called using ordinary function syntax with multiple arguments if necessary. An "#include <libname.h>" line is required after the "programinit()" line. In this case, stop() and abort() in the called library terminate the calling program as well unless caught using try/catch syntax.
	var  perform(in sentence);

	// Run an exodus library's main function.
	// Identical to perform() but any currently active select list in the calling program is not accessible to the executed library and is preserved in the calling [program as is. Any select list created by the executed library is discarded when it terminates.
	var  execute(in sentence);

	// Close the current program and run an exodus library's main function.
	// Identical to perform().
	[[noreturn]]
	void chain(in libraryname);

	///////////////////////////
	///// Program termination :
	///////////////////////////

	// Return to parent exoprog
	// or quit to OS WITHOUT an error
	// bool to allow "or stop()"
	[[noreturn]]
	void stop(in message = "") const;

	// Return to parent exoprog
	// or quit to OS WITH an error 1
	// bool to allow "or abort()"
	[[noreturn]]
	void abort(in message = "") const;

	// Quit to OS WITH an error 2
	// bool to allow "or abortall()"
	[[noreturn]]
	void abortall(in message = "") const;

	// Quit to OS WITHOUT an error
	// bool to allow "or logoff()"
	//[[deprecated("Deprecated is a great way to highlight all uses of something!")]]
	[[noreturn]]
	void logoff(in message = "") const;

	///////////////////////////
	///// db file dictionaries:
	///////////////////////////

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
ND	var  calculate(in dictid);
ND	var  calculate(in dictid, in dictfile, in id, in record, in mv = 0);
ND	var  xlate(in filename, in key, in fieldno_or_name, const char* mode);

	/////////////////////
	///// i/o conversion:
	/////////////////////

	// iconv/oconv with access to specific exoprog conversions and also var's iconv/oconv patterns
	// Unlike var's iconv/oconv, exoprog's iconv/oconv have the ability to call custom functions like "[funname,args...]"
	//
	// [DATE]
	// [DATE,*]
	// [DATE,4]
	// [DATE,*4]
	// [DATE,DS-]
	//
	// [DATEPERIOD]
	// [DATEPERIOD," ^ adcompany.f(6) ^ "]
	// [DATEPERIOD," ^ doccompany.f(6) ^ "]
	// [DATEPERIOD," ^ firstmonth ^ "," ^ fin.maxperiod ^ "]
	// [DATEPERIOD,1,12]
	//
	// [DATETIME]
	// [DATETIME,4*]
	// [DATETIME,4*,DOS]
	// [DATETIME,4*,MTS]
	//
	// [TIME2]
	// [TIME2,48]
	// [TIME2,MT48]
	// [TIME2,MTS48]
ND	var  oconv(in input, in conversion);
ND	var  iconv(in input, in conversion);

	// Split combined amount and currency code string
	// e.g. "123.45XYZ" -> "123.45" and "XYZ"
	var  amountunit(in input0, out unitx);
ND	var  amountunit(in input0);

	/////////////////////////////
	///// Terminal i/o utilities:
	/////////////////////////////

	// Output a message and optionally request a response
	// options: R = Response requested. C upper case response.
	// If no terminal is available then the response will be empty.
	void note(in msg, in options = "") const;
	void note(in msg, in options, io response) const;

ND	var  decide(in question, in options = "") const;
	var  decide(in question, in options, out reply, const int defaultreply = 1) const;

ND	bool esctoexit() const;

	// TERMINAL specific terminal cursor control
ND	var  AT(const int code) const;
ND	var  AT(const int x, const int y) const;

ND	var  getcursor() const;
	void setcursor(in cursor) const;

	//////////////////////
	///// Array utilities:
	//////////////////////

	// Dynamic array fields become values and vice versa
	// Returns: The inverted dynamic array.
	// pad: If true then on return, all fields will have the same number of values with superfluous trailing VMs where necessary.
	//
	// `let v1 = "a]b]c^1]2]3"_var;
	//  let v2 = invertarray(v1); // "a]1^b]2^c]3"_var`
	//
ND	var  invertarray(in input, bool pad = false);

	// Sorts fields of multivalues of dynamic arrays in parallel
	// fns: VM separated list of field numbers to sort in parallel based on the first field number
	// order:
	// AL Ascending  - Left Justified  - Alphabetic
	// DL Descending - Left Justfiied  - Alphabetic
	// AR Ascending  - Right Justified - Numeric
	// DR Descending - Right Justified - Numeric
	void  sortarray(io array, in fns = "", in order = "");

	//////////////////////////
	///// Time/date utilities:
	//////////////////////////

	// Returns: Text of date and time in users time zone
	// e.g. 2MAR2025 11:52AM
	// Offset from UTC by SW seconds.
ND	var  timedate2();

	// Get local, server and utc date and time
	void  getdatetime(out localdate, out localtime, out sysdate, out systime, out utcdate, out utctime);

	// Get text of elapsed time since TIMESTAMP
	// TIMESTAMP is initialised with ostimestamp() at program/thread startup.
	// e.g. "< 1 ms"
ND	var  elapsedtimetext() const;

	// Get text of elapsed time since a given timestamp.
ND	var  elapsedtimetext(in timestamp_difference) const;

	// Get text of elapsed time between two timestamps
ND	var  elapsedtimetext(in timestamp1, in timestamp2) const;

	/////////////////////
	///// Record locking:
	/////////////////////

	// Does not actually return record
ND	bool lockrecord(in filename, io file, in keyx, in recordx, const int waitsecs = 0, const bool allowduplicate = false) const;
ND	bool lockrecord(in filename, io file, in keyx) const;
	bool unlockrecord(in filename, io file, in key) const;
	bool unlockrecord() const;

 private:

	////////////////////////
	///// ioconv date/time :
	////////////////////////

	// Formatting for dates that is sensitive to the American/International setting in DATEFORMAT.
	// Normally used for oconv() but can be used in reverse for iconv.
	// var: An internal date (a number).
	// Returns: A readable date in text format. e.g. "31 DEC 2020"
	// Alternatively one can use the ordinary date conversion patterns starting "D" e.g. "DE/" for international date with / like 31/12/2020.
	// `let v1 = iconv("JAN 9 2020", "D");
	//  assert(v1, "[DATE]")   == " 9/ 1/2020");  // Same as date conversion "D/Z"  assuming E from DATEFMT
	//  assert(v1, "[DATE,4]"  == " 9/ 1/2020");  // 4 is the default unless 2 is set in DATEFMT so may not not be needed.
	//  assert(v1, "[DATE,*4]" == "9/1/2020");    // Same as date conversion "D/ZZ" assuming E from DATEFMT
	//  assert(v1, "[DATE,*]"  == "9/1/2020");    // * means the same as date conversion "ZZ" (trim leading zeros and spaces)`
	var  exoprog_date(in type, in input0, in ndecs0, out output);

	// Formatting for numbers with currency code/unit suffix.
	// Normally used for oconv() but can be used in reverse for iconv.
	// var: A number with an optional currency code or unit suffix. e.g. "12345.67USD"
	// Returns: A formatted number with thousands separated conventionally e.g. "12.345.67USD".
	// iconv/oconv("[NUMBER]")      oconv leaves ndecimals untouched as in the input. iconv see below.
	// iconv/oconv("[NUMBER,2]")    Specified number of decimal places
	// iconv/oconv("[NUMBER,BASE]") Decimal places as per BASEFMT
	// iconv/oconv("[NUMBER,*]")    Leave decimal places untouched as in the input
	// iconv/oconv("[NUMBER,X]")    Leave decimal places untouched as in the input
	// iconv/oconv("[NUMBER,2Z]")   Z (suppress zero) combined with any other code for oconv results in empty output "" instead of "0.00" in case of zero input.
	//
	// Empty input "" gives empty output "".
	//
	// All leading, trailing and internal spaces are removed from the input.
	//
	// A trailing currency or unit code is ignored and returned on output.
	//
	// An exodus number is an optional leading + or - followed by one or more decimal digits 0-9 with a single optional decimal point placed anywhere.
	//
	// If the input is non-numeric then "" is returned and STATUS set to 2. In the case of oconv with multiple fields or values each field or value is processed separately but STATUS is set to 2 if any are non-numeric.
	//
	// iconv removes and oconv adds thousand separator chars. The thousands separator is  "," if BASEFMT starts with "MD" or "." if it starts with "MC".
	//
	// oconv:
	//
	// Add thousands separator chars and optionally standardise the number of decimal places.
	//
	// Multiple numbers in fields, values, subvalues etc. can be processed in one string.
	//
	// Any leading + character is preserved on output.
	//
	// Z suppresses zeros and returns empty string "" instead.
	//
	// Special format "[NUMBER,ndecs,move_ndecs]": move_ndecs causes decimal point to be shifted left if positive or right if negative.
	//
	// `var v1 = oconv("1234.5678USD", "[NUMBER,2]"); // "1,234.57USD"`
	//
	// iconv:
	//
	// Remove all thousands separator chars and optionally standardise the number of decimal places.
	//
	// If ndecs is not specified in the "[NUMBER]" pattern then ndecs is taken from the current RECORD using dictionary code NDECS if DICT is available otherwise it uses ndecs from BASEFMT.
	//
	// iconv only handles a single field/value.
	//
	// Optional prefix of "1/" or "/" causes the reciprocal of the number to be used. e.g. "1/100" or "/100" -> "0.01".
	//
	// `var v1 = iconv("1,234.5678USD", "[NUMBER]"); // "1234.57USD"`
	//
	var  exoprog_number(in type, in input0, in ndecs0, out output);

};

// clang-format off

class PUBLIC ExoStop     {public:explicit ExoStop    (in var1 = ""); var message;};
class PUBLIC ExoAbort    {public:explicit ExoAbort   (in var1 = ""); var message;};
class PUBLIC ExoAbortAll {public:explicit ExoAbortAll(in var1 = ""); var message;};
class PUBLIC ExoLogoff   {public:explicit ExoLogoff  (in var1 = ""); var message;};

// clang-format on

}  // namespace exo
#endif	// EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_
