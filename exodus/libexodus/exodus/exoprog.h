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

//class ExodusProgramBase
class PUBLIC ExodusProgramBase {

 private:

	// used by calculate to call dict libraries
	mutable std::string cached_dictid_;
	mutable var cached_dictrec_ = "";

	// A callable used in ExodusProgramBase::calculate to call dict items in dict_xxxxxx.cpp
	mutable Callable* dict_callable_ = nullptr;

	// A cache of Callables dict functions
	std::map<std::string, Callable*> cached_dict_functions;

	// A callable used in ExodusProgramBase::perform to call libraries WITH NO ARGUMENTS
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

	explicit ExodusProgramBase(ExoEnv& inmv);
	//ExodusProgramBase(ExoEnv& mv);
	//explicit ExodusProgramBase(ExoEnv&& inmv) = delete;

	// No default constructor
	// data member mv is a reference and we only provide constructors for preexisting ExoEnv.
	//ExodusProgramBase() = default;
	ExodusProgramBase() = delete;

	// doing virtual isnt much use because external functions (which are based on
	// ExodusProgramBase) need to have complete freedom of arguments to main(...) virtual var
	// main();

	// Is this required?
	virtual ~ExodusProgramBase();

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

	////////////////////
	///// User security:
	////////////////////

	// User authorisation checks
	var  authorised(in task0, io msg, in defaultlock = "", in username0 = "");
	var  authorised(in task0);

ND	var  otherusers(in param);
ND	var  otherdatasetusers(in param);

	//////////////////////
	///// Perform/Execute:
	//////////////////////

	// Check if a command's first word  is a performable/executable exodus program.
	var  libinfo(in command);

	// Run an exodus library's main function using a command like syntax similar to that of executable programs.
	// A "command line" is passed to the library in the usual COMMAND, SENTENCE and OPTIONS environment variables instead of function arguments.
	// The library's main function should have zero arguments. Performing a library function with main arguments results in them being unassigned and in some case core dump may occur.
	// The following environment variables are initialised on entry to the main function of the library. They are restored on return to the calling program.
	// SENTENCE, COMMAND, OPTIONS: Initialised from the argument "sentence".
	// RECUR0, RECUR1, RECUR2, RECUR3, RECUR4 to "".
	// ID, RECORD, MV, DICT initialised to "".
	// LEVEL is incremented by one.
	// Any active select list is passed to the performed program and can be consumed by it. Conversely any active select list created by the library will be returned to the calling program.
	// sentence: The first word of this argument is used as the name of the library to be loaded and run. sentence is used to initialise the SENTENCE, COMMAND and OPTIONS environment variables.
	// Returns: Whatever var the library returns, or "" if it calls stop() or abort(()".
    // The return value can be ignored and discarded without any compiler warning.
	// Note that library functions may also be directly called using ordinary function syntax with multiple arguments if necessary. An "#include <libname.h>" line is required after the "programinit()" line. In this case, stop() and abort() in the called library terminate the calling program as well unless caught using try/catch syntax.
	var  perform(in sentence);

	// Run an exodus library's main function.
	// Identical to perform() but any currently active select list in the calling program is not accessible to the executed library and is preserved in the callng [program as is. Any select list created by the executed library is discarded when it terminates.
	var  execute(in sentence);

	// Close the current program and run an exodus library's main function.
	// Identical to perform().
	[[noreturn]]
	void chain(in libraryname);

	//////////////////////////////////////
	///// dictionaries and i/o conversion:
	//////////////////////////////////////

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
ND	var  calculate(in dictid);
ND	var  calculate(in dictid, in dictfile, in id, in record, in mv = 0);
ND	var  xlate(in filename, in key, in fieldno_or_name, const char* mode);

	// ioconv with access to all exoprog functionality and base ioconv
	// Particularly the ability to call custom ioconv functions like "[xxxxxxxx]"
ND	var  oconv(in input, in conversion);
ND	var  iconv(in input, in conversion);

[[deprecated("Replace with tcase()")]]
ND	var  capitalise(in str0, in mode = var(), in wordseps = var()) const;

	// Split combined amount and currency code string
	// e.g. "123.45XYZ" -> "123.45" and "XYZ"
	var  amountunit(in input0, out unitx);
ND	var  amountunit(in input0);

	/////////////////////
	///// Record locking:
	/////////////////////

	// NB does not return record yet
ND	bool lockrecord(in filename, io file, in keyx, in recordx, const int waitsecs = 0, const bool allowduplicate = false) const;
ND	bool lockrecord(in filename, io file, in keyx) const;
	bool unlockrecord(in filename, io file, in key) const;
	bool unlockrecord() const;

//	void flushindex(in filename);

	/////////////////////////////
	///// Terminal i/o utilities:
	/////////////////////////////

	void note(in msg, in options = "") const;
	void note(in msg, in options, io buffer, in params = "") const;
	void mssg(in msg, in options = "") const;
	void mssg(in msg, in options, io buffer, in params = "") const;

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

	// FMs become VMs and vice versa
ND	var  invertarray(in input, in force0 = (0));

	// Sorts multiple fields of multivalues in parallel
	// fns: VM separated list of field numbers to sort in parallel based on the first field number
	// orderby:
	// AL Ascending Alphabetic
	// DL Descending Alphabetic
	// AR Ascending Numeric
	// DR Descending Numeric
	void  sortarray(io array, in fns = 0, in orderby0 = "");

	//////////////////////////
	///// Time/date utilities:
	//////////////////////////

	// Returns: Text of date and time in users time zone
	// e.g. 2MAR2025 11:52AM
	// Offset from UTC by SW seconds.
ND	var  timedate2();

	// Utility to return local, server and utc date and time
	void  getdatetime(out localdate, out localtime, out sysdate, out systime, out utcdate, out utctime);

	// Returns: Text of elapsed time since TIMESTAMP
	// TIMESTAMP is initialised with ostimestamp() at program/thread startup.
	// e.g. "< 1 ms"
ND	var  elapsedtimetext() const;

	// Returns: Text of elapsed time since a given timestamp.
ND	var  elapsedtimetext(in timestamp_difference) const;

	// Returns: Text of elapsed time between two timestamps
ND	var  elapsedtimetext(in timestamp1, in timestamp2) const;

	///////////////////////////
	///// Program flow control:
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

 private:

	// ioconv with a language specific month
	var  exoprog_date(in type, in input0, in ndecs0, out output);
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
