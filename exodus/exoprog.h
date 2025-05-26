#ifndef LIBEXODUS_EXOPROG_H_
#define LIBEXODUS_EXOPROG_H_

#if EXO_MODULE
	import std;
#else
#	include <string>
#	include <future>
#	include <generator>
#	include <coroutine>
// Using map for dict function cache instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// https://youtu.be/M2fKMP47slQ?t=476
#	include <map>

#	include <exodus/var.h>
#endif

#include <exodus/vardefs.h>
#include <exodus/exoimpl.h>
#include <exodus/exoenv.h>
#include <exodus/exocallable.h>
//#include <exodus/thread_pool.h>
//#include <exodus/threadsafequeue.h>
//#include <exodus/job.h>
#include <exodus/job_manager.h>
#include <exodus/fiber_manager.h>

namespace exo {

// Template to reset a range of objects to their default-constructed state
template<typename T>
void reset_range(T& first, T& last);

using function = var;
using subroutine = void;
using func = var;
using subr = void;

//using Jobs = std::vector<Job>;
//using Queue = ThreadSafeQueue<var>;
//using ResultQueue = ThreadSafeQueue<ExoEnv>;

//class ExoProgram
class PUBLIC ExoProgram : public FiberManager, public JobManager {

 private:

	// used by calculate to call dict libraries
	mutable std::string cached_dictid_;
	mutable var cached_dictrec_ = "";

	// A callable used in ExoProgram::calculate to call dict items in dict_xxxxxx.cpp
	mutable Callable* dict_callable_ = nullptr;

	// A cache of Callables dict functions
	std::map<std::string, Callable*> cached_dict_functions;

    // Shared state
//    std::shared_ptr<ThreadSafeQueue<var>> input_queue;
//    std::shared_ptr<ThreadSafeQueue<var>> output_queue;

 public:

//	std::shared_ptr<ResultQueue> result_queue_ = std::make_shared<ResultQueue>();

	// ev.XXXXXX is going to be used a lot by exodus programmers for exodus "global variables"
	// e.g. ev.RECORD ev.DICT etc.
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
	// ev being a reference ensures that exodus programs cannot exist without an ev
	//
	// Using a reference instead of a pointer allows debugging via ev.ID instead of ev->ID
	//
	// Macros are provided for all env vars like
	// #define ID ev.ID
	// to allow code to be written without the ev. prefix for readability
	// #define ID ev->ID //This form is not required because ev is reference.
	ExoEnv& ev;

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
	//ExoProgram(ExoEnv& ev);
	//explicit ExoProgram(ExoEnv&& inmv) = delete;

	// No default constructor
	// data member ev is a reference and we only provide constructors for preexisting ExoEnv.
	//ExoProgram() = default;
	ExoProgram() = delete;

	// Virtual main() isnt much use because .so external functions (which are based on
	// ExoProgram) need to have complete freedom of arguments to main(...) virtual var
	// main();

	// We MIGHT be able to get away with no virtual destructor IIF
	// we ONLY use the factory function to create and delete derived xxx_ExoProgram objects.
	// Without virtual, xxx_ExoProgram dtor is not present in .so files
	// which could be a problem if not using factory function to create .so. objects.
	virtual ~ExoProgram();

    int run_main(var (ExoProgram::*main_func)(), int argc, const char* argv[], int threadno = 0);

	///////////////////
	///// Select lists:
	///////////////////

	// obj is

	// Create an active select list using a natural language sort/select command.
	// This and all the following exoprog member functions work on an environment variable CURSOR.
	// Identical functions are available directly on plain var objects but vars have less functionality regarding dictionaries and environment variables which are built-in to exoprog.
	// return: True if an active select list was created, false otherwise.
	// In the following examples, various environment variables like RECORD, ID and MV are used instead of declaring and using named vars. In actual code, either may be freely used.
	//
	// `select("xo_clients by name by type with type 'A' 'B' and with balance between 0 and 2000");
	//  if (readnext(ID)) ... ok`
	//
	bool select(in sortselectclause_or_filehandle = "");
	// TODO make ND?

	// Create an active select list from a dynamic array of keys.
	//
	// `selectkeys("SB001^JB001^JB002"_var);
	//  if (readnext(ID)) ... ok // ID -> "SB001"`
	//
	bool selectkeys(in keys);

	// Check if a select list is active.
	//
	// `if (hasnext()) ... ok`
	//
ND	bool hasnext();

	// Get the next key from an active select list.
	// key[out]: A string. Typically the key of a db file record.
	// return: True if an active select list was available and the next key in the list was obtained.
	//
	// `selectkeys("SB001^JB001^JB002"_var);
	//  if (readnext(ID)) ... ok // ID -> "SB001"`
	//
	bool readnext(out key);

	// Get the next key and value number pair from an active select list.
	// key[out]: A string. Typically the key of a db file record.
	// valueno[out]: Is only available in select lists that have been created by sort/select commands that refer to multi-valued db dictionary fields where db records have multiple values for a specific field. In this case, a record key will appear multiple times in the select list since each multivalue is exploded for the purpose of sorting and selecting. This can be viewed as a process of "normalising" multivalues so they appear as multiple records instead of being held in a single record.
	// return: True if an active select list was available and the next key in the list was obtained.
	//
	// `selectkeys("SB001]2^SB001]1^JB001]2"_var);
	//  if (readnext(ID, MV)) ... ok // ID -> "SB001" // MV -> 2`
	//
	bool readnext(out key, out valueno);

	// Get the next record, key and value no from an active select list.
	// record[out]: Is only available in select lists that have been created with the final (R) option. Otherwise the record will be returned as an empty string and must be obtained using a db read() function.
	// key[out]: A string. Typically the key of a db file record.
	// valueno[out]: Is only available in select lists that have been created by sort/select commands that refer to multi-valued db dictionary fields where db records have multiple values for a specific field.
	// return: True if an active select list was available and the next key in the list was obtained.
	//
	// `select("xo_clients by name (R)");
	//  if (readnext(RECORD, ID, MV)) ... ok;
	//  assert(not RECORD.empty());`
	//
	bool readnext(out record, out key, out valueno);

	// Get a reference to the currently active select list and suspend it.
	// Allow another select list to be activated and used temporarily before the original select list is reactivated.
	// Multiple levels of pushselect/popselect can be used.
	// cursor[out]: A var that can be passed later on to the popselect() function to reactivate the saved list.
	//
	// `select("xo_clients by name");
	//  var saved_xo_clients_cursor;
	//  pushselect(saved_xo_clients_cursor);
	//  //
	//  // ... work with another select list ...
	//  //
	//  popselect(saved_xo_clients_cursor); // Reactivate the original select list.`
	//
	void pushselect(out cursor);
	// TODO what happens if no list is active?

	// Re-activate a select list using a reference provided by pushselect().
	// cursor: A var created by the pushselect() function.
	// See pushselect() for more info.
	void popselect(in cursor);
	// TODO what happens if cursor is junk?

	// Deactivate an active select list.
	// If no select list is active then nothing is done.
	//
	// `clearselect();`
	//
	void clearselect();

	// Delete multiple DB records using an active select list.
	// return: False if any records could not be deleted.
	// Contrast this function with the two argument "deleterecord(file, key)" function that deletes a single record.
	//
	// `if (select("xo_clients with type 'Q' and with balance between 0 and 100")) {
	//    if (deleterecord("xo_clients")) ...
	//  }`
	//
	bool deleterecord(in filename);

	// Delete a single DB record.
	//
	// `let file = "xo_clients", key = "QQ001";
	//  write("" on file, key);
	//  if (not deleterecord(file, key)) ...
	//  // or
	//  write("" on file, key);
	//  if (not file.deleterecord(key)) ...`
	//
	ND bool deleterecord(in dbfile, in key);

	// Save the current active select list under a given name.
	// Select lists are saved in the DB file "lists". They are accessible to any Exodus process and remain until specifically deleted.
	// After saving, the list is no longer active and hasnext() will return false.
	// return: True if an active select list was saved, false if there was no active select list.
	//
	// `selectkeys("SB001^SB002"_var);
	//  if (not savelist("my_list")) ...`
	//
	bool savelist(SV listname);
	// TODO make it ND? void?

	// Create an active select list using a saved select list of a given name.
	// A saved list is obtained from the "lists" file and activated. The list remains in the lists file for multiple use.
	// return: True if an active select list was successfully reactivated, otherwise false.
	//
	// `if (not getlist("my_list")) ...`
	//
	bool getlist(SV listname);
	// TODO make it ND

	// Remove a saved select list by name.
	// A saved list is deleted from the "lists" file.
	//
	// `if (not deletelist("my_list")) ...`
	//
	bool deletelist(SV listname);
	// TODO make it ND

	[[deprecated ("exoprog:::formlist() Resolve by refactoring. Read keys directly and call selectkeys(keys)")]]
	bool formlist(SV filename_or_command, in keys = "", const int fieldno = 0);
	[[deprecated ("exoprog::makelist() Refactor makelist(\"\", keys) as selectkeys(keys) or use fixdeprecated")]]
	bool makelist(SV listname, in keys);

	//////////////////////////
	///// Perform/Execute/Run:
	//////////////////////////

	// Run an Exodus program/library.
	// Creates a new instance of an Exodus program library function object and calls its main() function using a command-like syntax, similar to that of running an OS executable program, and passes its arguments through the COMMAND and OPTIONS variables.
	// A performed program/library's main function should have no arguments otherwise they appear unassigned and a segfault or core dump may occur.
	// The Exodus program class member variables of a performed or executed program/library are all, as might be expected, initially unassigned unless specifically initialised inline. Note that this is not the same as calling a program library function (using function call syntax and round brackets funcx()), where the program/library/function's member variables are initially unassigned but retain their state between calls.
	// command_line: Used to initialise the COMMAND, SENTENCE and OPTIONS environment variables of the performed exodus program/library. Analogous to passing function arguments. The first word of command_line is used as the name of the program/library to be loaded and run.
	// return: Whatever the program returns from main() or passes as an argument to stop(). If the program terminates abnormally then it will return "" and lasterror() will contain some error message. The return value may be ignored so there is no need so wrap perform statements in if clauses to avoid compiler warnings.
	// throw: All the various runtime errors based on VarError e.g. VarUnassigned.
	// environment: The following environment variables are initialised on entry to the main function of the program/library and are preserved untouched (actually restored) in the calling program.
	// * COMMAND, OPTIONS, SENTENCE.
	// * RECUR0, RECUR1, RECUR2, RECUR3, RECUR4 to "".
	// * ID, RECORD, MV, DICT initialised to "".
	// * LEVEL is incremented by one.
	// * All other environment variables are shared between the caller and callee. There is essentially only one environment in any one process or thread.
	// * CURSOR Any active select list in CURSOR is passed to the performed program/library and can be consumed by it. Conversely any active select list created by the performed program/library will be returned to the calling program. In other words, both the performing and the performed programs/libraries share a single active select list environment. This is different from execute() where the executed program/library gets its own private active select list, initially inactive.
	// Exodus program/library/functions may also be called directly using conventional function calling syntax. To call an exodus program/library called progname using either the syntax "call progname(args...);" or "var v1 = progname(args...);" you must "#include <progname.h>" after the "programinit()" or "libraryinit()" lines in your program/library. See library.h for more info.
	var  perform(in command_line);

	// Run an exodus program/library.
	// Identical to perform() but any currently active select list in the calling program/library is not accessible to the executed program/library and is preserved in the calling [program as is. Any select list created by the executed library is discarded when it terminates.
	var  execute(in command_line);

	// Close the current program and perform another one.
	// Similar to perform() except that the current program closes first and all environment variables carry forward unchanged.
	[[noreturn]]
	void chain(in command_line);

	// Check if a lib exists.
	// Can be checked before perform/execute to avoid errors.
	// Currently it does not check if the library is actually loadable.
	// return: osfile info.
	var  libinfo(in libname);

	///////////////////////////
	///// Program termination :
	///////////////////////////

	// Stop the current exodus program/library normally.
	// Either return to the performing or executing parent exodus program/library, or exit to the OS if none.
	// result: Optional. It will be used as the return value of a parent program's perform() or execute() function, or if none, and therefore returning to the OS, it will be output to stdout if non-numeric or, if numeric, used as the exit status.
	[[noreturn]]
	void stop(in result = "") const;

	// Abort the current exodus program/library.
	// Similar to stop but if exiting to the OS then the default exit status is 1.
	// message: Optional. If exiting to the OS then it will be output to stderr or, if numeric, used as the exit status.
	[[noreturn]]
	void abort(in message = "") const;

	// Abort the current exodus program/library.
	// Similar to abort but if exiting to the OS then the default exit status is 2.
	[[noreturn]]
	void abortall(in message = "") const;

//	// Stop the current exodus program/library normally.
//	//[[deprecated("Deprecated is a great way to highlight all uses of something!")]]
//	[[noreturn]]
//	void logoff(in message = "") const;

	///////////////////////////
	///// db file dictionaries:
	///////////////////////////

	// Get DB record field values given field name only.
	// Use a dictionary file that contains info sufficient to either extract or calculate the required value.
	// 1. Read fieldinfo from the current DICT file using readc() for caching.
	// 2. Use the fieldinfo to:
	//   * Either extract a specific field number from the current RECORD/ID/MV environment variables.
	//   * Or call a dictionary function library to calculate the result.
ND	var  calculate(in dictid);

	// Get DB record field values given name and data.
	// Same as the one argument version of calculate() but RECORD/ID/MV are provided as arguments instead of being hard coded.
ND	var  calculate(in dictid, in dictfile, in id, in record, in ev = 0);

	// Read DB field using field number or name.
	// 1. Read a record from a given file and key using readc() for caching.
	// 2. Extracts the given field number or calls calculate(dictid...) if non-numeric.
	// filename: The file to read.
	// key: The key of the record to read.
	//
	// fieldno_or_name: The field  to return.
	// * nn * Field number nn.
	// * 0  * The record key (ID).
	// * "" * The whole record (RECORD)
	// * dictid * The function calculate(dictid) will be called.
	//
	// mode: If the record does not exist.
	// * "X" * Return ""
	// * "C" * Return the key.
	//
	// MV: Environment variable. Will be used to select a particular value if not zero, or all values if zero.
ND	var  xlate(in filename, in key, in fieldno_or_name, const char* mode);

	////////////////////
	///// io conversion:
	////////////////////

	// iconv/oconv "[...]"
	// ExoProgram's iconv/oconv functions have access to ExoProgram's environment variables like BASEFMT, DATEFMT and TZ and have the ability to call custom functions like "[funname,args...]"
	//
	// [NUMBER]  // built-in. See doc below.
	// [DATE]    // built-in. See doc below.
	// [DATEPERIOD]  e.g. [DATEPERIOD,1] [DATEPERIOD,1,12]
	// [DATETIME]    e.g. [DATETIME,4*,DOS] [DATETIME,4*,MTS] [DATETIME,4*]
	// [TIME2]       e.g. [TIME2,MT] [TIME2,MTS] [TIME2,MTS48]
	//
ND	var  oconv(in input, in conversion);
ND	var  iconv(in input, in conversion);

 private:

	////////////////////////
	///// ioconv date/time :
	////////////////////////

	// iconv/oconv "[DATE]"
	// Use iconv/oconv code "[DATE,args]" when you want date conversion to depend on the environment variable DATEFMT, particularly its American/International setting. Otherwise use ordinary "D" conversion codes directly for slightly greater performance.
	//
	// var: [oconv] An internal date (a number).
	// return: [oconv] A readable date in text format depending on "[DATE,args]" e.g. "31 DEC 2020" "31/12/2020" "12/31/2020"
	// var: [iconv] A date in text format as above.
	// return: [iconv] An internal date (a number) or "" if the input could not be understood as a valid date.
	// args: If args is empty then DATEFMT is used as the conversion code. If args starts with "D" then args is used as the conversion codes but any E option in DATEFMT is appended. If args does not start with "D" then args are appended to DATEFMT, a "Z" option is appended, and the result used as the conversion code. A "*" option is equivalent to a second "Z" option.
	// If you are calling iconv/oconv in code and DATEFMT is adequate for your needs then pass it directly as a function argument e.g. 'var v1 = iconv|oconv(v2, DATEFORMAT);' instead of indirectly like 'var v1 = iconv|oconv(v2, "[DATE]");'.
	//
	// `DATEFMT = "D/E";
	//  let v1 = iconv("JAN 9 2025", "D");
	//  assert(oconv(v1, "[DATE]"   ) == " 9/ 1/2025");  // "D/EZ" or "[DATE,D]" equivalent assuming D/E in DATEFMT (replace leading zeros with spaces)
	//  assert(oconv(v1, "[DATE,4]" ) == " 9/ 1/2025");  // "D4Z"  equivalent assuming D/E in DATEFMT (replace leading zeros with spaces)
	//  assert(oconv(v1, "[DATE,*4]") == "9/1/2025");    // "D4ZZ" equivalent assuming D/E in DATEFMT (trim leading zeros and spaces)
	//  assert(oconv(v1, "[DATE,*]" ) == "9/1/2025");    // "DZZ"  equivalent assuming D/E in DATEFMT (trim leading zeros and spaces)`
	//
	var  exoprog_date(in type, in input0, in ndecs0, out output);

	// iconv/oconv "[NUMBER]"
	// Use iconv/oconv "[NUMBER,args]" either when your numbers have currency or unit code suffixes or when you want number conversion to depend on the environment variable BASEFMT to determine thousands separator and decimal point. Otherwise use ordinary "MD" conversion codes directly for slightly greater performance.
	// Formatting for numbers with optional currency code/unit suffix and is sensitive to the International or European setting in BASEFMT regarding use of commas or dots for thousands separators and decimal points.
	// Primarily used for oconv() but can be used in reverse for iconv.
	// var: A number with an optional currency code or unit suffix. e.g. "12345.67USD"
	// return: A formatted number with thousands separated conventionally e.g. "12.345.67USD".
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
	// `var v1 = oconv("1234.5USD", "[NUMBER,2]"); // "1,234.50USD" // Comma added and decimal places corrected.`
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
	// `var v1 = iconv("1,234.5678USD", "[NUMBER]"); // "1234.57USD" // Comma removed`
	//
	var  exoprog_number(in type, in input0, in ndecs0, out output);

public:

	// Parse amount+currency code string.
	// Split amount+currency code/unit string into number and currency code/unit.
	// var: "123.45USD"
	// return: e.g. "123.45"
	// unitx[out]: e.g. "USD"
	var  amountunit(in input0, out unitx);
ND	var  amountunit(in input0);

	//////////////////////////
	///// Time/date utilities:
	//////////////////////////

	// Get text of date and time.
	// In users time zone
	// e.g. "2MAR2025 11:52AM"
	// Offset from UTC by TZ seconds.
ND	var  timedate2();

	// Get current user, server and UTC dates and times.
	// User date and time is determined by adding the environment variable TZ.f(1)'s TZ offset (in seconds) to UTC date/time obtained from the operating system.
	// "system" date and time is normally the same as UTC date/time and is determined by adding the environment variable TZ.f(2)'s TZ offset (in seconds) to UTC date/time obtained from the operating system.
	//
	void  getdatetime(out user_date, out user_time, out system_date, out system_time, out UTC_date, out UTC_time);

	// Get text of elapsed time.
	// Since environment variable TIMESTAMP.
	// TIMESTAMP is initialised with ostimestamp() at program/thread startup.
	// TIMESTAMP can be updated using ostimestamp() as and when desired.
	//
	// `var v1 = elapsedtimetext(); // e.g. "< 1ms"`
	//
ND	var  elapsedtimetext() const;

	// Get text of elapsed time.
	// Between two given timestamps
	// Warning: Use ostimestamp() not ostime(). The first is in days and the second is in seconds.
	//
	// `let v1 = elapsedtimetext(0, 0.55);  // "13 hours, 12 mins"
	//  let v2 = elapsedtimetext(0, 0.001); // "1 min, 26 secs"`
	//
ND	var  elapsedtimetext(in timestamp1, in timestamp2) const;

	/////////////////////////////
	///// Terminal i/o utilities:
	/////////////////////////////

	// Output a message to stdout and optionally request input.
	// If input is requested but stdin is not a terminal then set the response to "" and continue.
	// options: R = Response requested. C upper case response.
	//
	// `var response;
	//  // call note("Enter something", "RC", response);`
	//
	void note(in msg, in options, io response) const;

	// Output a message to stdout.
	//
	// `call note("Hello world.");`
	//
	void note(in msg) const;

	// Input user selection from a list of options.
	// If stdin is not a terminal, set the response to "" and continue.
	// return: The chosen option (value not number) or "" if the user cancelled.
ND	var  decide(in question, in options = "") const;

	// Input user selection from a list of options.
	// If stdin is not a terminal set the response to the default value, or "" if none, and continue.
	// defaultreply: A default option if the user presses Enter.
	// reply[out]: The option number that the user chose or "" if they cancelled.
	// return: The chosen option (value not number) or "" if the user cancelled.
	var  decide(in question, in options, out reply, const int defaultreply = 1) const;

	// Check for user pause or cancel.
	// If stdin is a terminal, check if a key has been pressed and, if so, pause execution and ask the user to confirm if they want to escape/cancel or resume processing.
	// return:
	// * True  * A key has been pressed and the user chose not to resume.
	// * False * stdif is not a terminal, no key has been pressed, or the user chose to resume.
ND	bool esctoexit() const;

	// Get a string to control terminal operation.
	// return: A string to be output to the terminal in order to accomplish the desired operation.
	// The terminal protocol is xterminal.
	// code:
	// *   n * Position the cursor at column number n (1 based).
	// *   0 * Position the cursor at the first column (1).
	// *  -1 * Clear the screen and home the cursor.
	// *  -2 * Position the cursor at the top left home (x, y = 1, 1).
	// *  -3 * Clear from the cursor at the end of screen.
	// *  -4 * Clear from cursor to end of line.
	// * -40 * Position the cursor at columnno 0 and clear to end of line.
ND	var  AT(const int code) const;

	// Get a terminal cursor positioning string.
	// return: A string to be output to the terminal to position the cursor at the desired screen x and y position.
	// The terminal protocol is xterminal.
ND	var  AT(const int x, const int y) const;

	// Get the position of the terminal cursor.
	// cursor[out]: If stdin is a terminal, an FM delimited string containing the x and y coordinates of the current terminal cursor.
	// x and y are 1 based, not 0 based.
	// If stdin is not a terminatl then an empty string "" is returned.
	// The cursor additionally contains a third field which contains the delay in ms from the terminal.
	// The FM delimited string returned can be later passed to setcursor() to reposition the cursor back to its original position or it can be parsed and used accordingly.
	// delayms: Default 3000ms. The maximum time to wait for terminal response.
	// max_errors: Default is 0. If not zero, reset the number of times to error before automatically disabling getcursor(). max_errors is initialised to 3. If negative then max_errors has the the effect of disabling all future calls to getcursor().
	// In case the terminal fails to respond correctly within the required timeout, or is currently disabled due to too many failures, or has been specifically disabled then the returned "cursor" var contains a 4th field:
	// * TIMEOUT          * The terminal failed to respond within the timeout.
	// * READ_ERROR       * Failed to read terminal response.
	// * INVALID_RESPONSE * Terminal response invalid.
	// * SETUP_ERROR      * Terminal setup failed.
	// * DISABLED         * Terminal is disabled due to more errors than the maximum currently set.
	//
	// `var cursor;
	//  if (isterminal() and not getcursor(cursor)) ... // cursor becomes something like "1^20^0.012345"_var`
	//
ND	bool getcursor(out cursor, int delayms = 3000, int max_errors = 0) const;

	// Get the position of the terminal cursor.
	// For more info see the main getcursor() function above.
	//
	// `let cursor = getcursor(); // If isterminal() then cursor becomes something like "0^20^0.012345"_var`
	//
ND	var  getcursor() const;

	// If stdin is a terminal, position the cursor at x and y as per the given coordinates.
	// cursor_coordinates: An FM delimited string containing the x and y coordinates of the terminal cursor as can be obtained by getcursor().
	// `if (isterminal()) {
	//      let cursor = getcursor(); // Save the current cursor position.
	//      TRACE(cursor)             // Show the saved cursor position.
	//      printx(AT(0,0));          // Position the cursor at 0,0.
	//      setcursor(cursor);        // Restore its position
	//  }`
	void setcursor(in cursor_coordinates) const;

	//////////////////////
	///// Array utilities:
	//////////////////////

	// Dynamic array fields become values and vice versa
	// return: The inverted dynamic array.
	// pad: If true then on return, all fields will have the same number of values with superfluous trailing VMs where necessary.
	//
	// `let v1 = "a]b]c^1]2]3"_var;
	//  let v2 = invertarray(v1); // "a]1^b]2^c]3"_var`
	//
ND	var  invertarray(in input, bool pad = false);

	// Sort parallel fields of multivalues of dynamic arrays
	// fns: VM separated list of field numbers to sort in parallel based on the first field number
	// order:
	// * AL * Ascending  - Left Justified (Alphabetic)
	// * DL * Descending - Left Justified (Alphabetic)
	// * AR * Ascending  - Right Justified (Numeric/Natural)
	// * DR * Descending - Right Justified (Numeric/Natural)
	//
	// `var v1 = "f1^10]20]2]1^ww]xx]yy]zz^f3^f4"_var;  // fields 2 and 3 are parallel multivalues and currently unordered.
	//  sortarray(v1, "2]3"_var, "AR"); // v1 -> "f1^1]2]10]20^zz]yy]ww]xx^f3^f4"_var`
	//
	void  sortarray(io array, in fns = "", in order = "");

	/////////////////////
	///// Record locking:
	/////////////////////

ND	bool lockrecord(in filename, io file, in keyx, in unused, const int waitsecs = 0, const bool allowduplicate = false) const;
ND	bool lockrecord(in filename, io file, in keyx) const;
	bool unlockrecord(in filename, io file, in key) const;
	bool unlockrecord() const;

	var& ID        {ev.ID};
	var& RECORD    {ev.RECORD};
	var& FILE      {ev.FILE};
	var& DICT      {ev.DICT};
	var& ANS       {ev.ANS};
	var& MV        {ev.MV};
	var& PSEUDO    {ev.PSEUDO};
	var& DATA      {ev.DATA};
//	var& LISTACTIVE {ev.LISTACTIVE};

	var& USERNAME  {ev.USERNAME};
	var& APPLICATION {ev.APPLICATION};
	var& SENTENCE  {ev.SENTENCE};
	var& CHAIN     {ev.CHAIN};

	var& USER0     {ev.USER0};
	var& USER1     {ev.USER1};
	var& USER2     {ev.USER2};
	var& USER3     {ev.USER3};
	var& USER4     {ev.USER4};

	var& RECUR0    {ev.RECUR0};
	var& RECUR1    {ev.RECUR1};
	var& RECUR2    {ev.RECUR2};
	var& RECUR3    {ev.RECUR3};
	var& RECUR4    {ev.RECUR4};

	const var& EXECPATH  {ev.EXECPATH};
	var& COMMAND   {ev.COMMAND};
	var& OPTIONS   {ev.OPTIONS};

	var& DEFINITIONS   {ev.DEFINITIONS};
	var& SECURITY  {ev.SECURITY};
	var& SYSTEM    {ev.SYSTEM};
	var& SESSION   {ev.SESSION};
	var& THREADNO  {ev.THREADNO};

	var& STATION   {ev.STATION};
	//var& DATEFM  {ev.DATEFMT};
	//var& BASEFMT {ev.BASEFMT};
	var& PRIVILEGE {ev.PRIVILEGE};
	var& FILES     {ev.FILES};
	var& TCLSTACK  {ev.TCLSTACK};
	var& INTCONST  {ev.INTCONST};
	var& STATUS    {ev.STATUS};
	int& COL1      {ev.COL1};
	var& COL2      {ev.COL2};
	var& PRIORITYINT   {ev.PRIORITYINT};

	var& FILEERRORMODE {ev.FILEERRORMODE};
	var& FILEERROR     {ev.FILEERROR};

	var& RECCOUNT      {ev.RECCOUNT};

	var& AW            {ev.AW};
	var& EW            {ev.EW};
	var& HW            {ev.HW};
	var& MW            {ev.MW};
	var& PW            {ev.PW};
	//var& SW          {ev.SW};
	var& VW            {ev.VW};
	var& XW            {ev.XW};

	var& CRTHIGH       {ev.CRTHIGH};
	var& CRTWIDE       {ev.CRTWIDE};
	var& LPTRHIGH      {ev.LPTRHIGH};
	var& LPTRWIDE      {ev.LPTRWIDE};

	const var& TERMINAL      {ev.TERMINAL};
	var& LEVEL         {ev.LEVEL};

//	var& THREADNO      {ev.THREADNO};
	var& CURSOR        {ev.CURSOR};
	var& TIMESTAMP     {ev.TIMESTAMP};

	var& VOLUMES       {ev.VOLUMES};

};

// clang-format off

class PUBLIC ExoExit                      {public:explicit ExoExit    (in var1 = ""); var message;};
//class PUBLIC ExoStop     : public ExoExit {using ExoExit::ExoExit}; //linker errors in application code
class PUBLIC ExoStop     : public ExoExit {public:explicit ExoStop    (in var1 = ""); var message;};
class PUBLIC ExoAbort    : public ExoExit {public:explicit ExoAbort   (in var1 = ""); var message;};
class PUBLIC ExoAbortAll : public ExoExit {public:explicit ExoAbortAll(in var1 = ""); var message;};

// clang-format on

}  // namespace exo
#endif	// LIBEXODUS_EXOPROG_H_
