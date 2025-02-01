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

	// work on CURSOR
	bool select(in sortselectclause_or_filehandle DEFAULT_EMPTY);
	ND bool hasnext();
	bool readnext(io key);
	bool readnext(io key, io valueno);
	bool readnext(io record, io key, io valueno);
	bool pushselect(in v1, io v2, io v3, io v4);
	bool popselect(in v1, io v2, io v3, io v4);
	void clearselect();
	// THIS should be removed from exoprog and made a free function that doesnt use CURSOR
	bool deleterecord(in filename_or_handle_or_command, in key DEFAULT_EMPTY);

	bool savelist(in listname);
	bool getlist(in listname);
	bool formlist(in filename_or_command, in keys = "", const var fieldno = 0);
	bool makelist(in listname, in keys);
	bool deletelist(in listname);

	void note(in msg, in options = "") const;
	void note(in msg, in options, io buffer, in params = "") const;
	void mssg(in msg, in options = "") const;
	void mssg(in msg, in options, io buffer, in params = "") const;

	var authorised(in task0, io msg, in defaultlock = "", in username0 = "");
	var authorised(in task0);
	void readuserprivs() const;
	void writeuserprivs() const;

	ND var capitalise(in str0, in mode = var(), in wordseps = var()) const;

	var libinfo(in command);
	var perform(in sentence);
	[[noreturn]] void chain(in libraryname);
	var execute(in sentence);

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
	ND var calculate(in dictid);
	ND var calculate(in dictid, in dictfile, in id, in record, in mv = 0);
	ND var xlate(in filename, in key, in fieldno_or_name, const char* mode);

	ND var otherusers(in param);
	ND var otherdatasetusers(in param);

	//bool fsmsg(in msg = "") const;	 // always returns false so can be used like return fsmsg();
	ND var sysvar(in var1, in var2, in mv3, in mv4);
	void setprivilege(in var1);

	// NB does not return record yet
	ND bool lockrecord(in filename, io file, in keyx, in recordx, const int waitsecs = 0, const bool allowduplicate = false) const;
	ND bool lockrecord(in filename, io file, in keyx) const;
	bool unlockrecord(in filename, io file, in key) const;
	bool unlockrecord() const;

	ND var decide(in question, in options = "") const;
	var decide(in question, in options, io reply, const int defaultreply = 1) const;

	void savescreen(io origscrn, io origattr) const;
	var keypressed(int milliseconds = 0) const;
	ND bool esctoexit() const;

	void flushindex(in filename);
	ND var encrypt2(in encrypt0) const;
	ND var xmlquote(in str) const;
	ND bool loginnet(in dataset, in username, io cookie, io msg);

	// TERMINAL specific terminal cursor control
	ND var AT(const int code) const;
	ND var AT(const int x, const int y) const;

	ND var getcursor() const;
	void setcursor(in cursor) const;

	ND var getprompt() const;
	void setprompt(in prompt) const;

	ND var handlefilename(in handle);

	ND var getuserdept(in usercode);

	// ioconv with access to all exoprog functionality and base ioconv
	// Particularly the ability to call custom ioconv funcs like "[xxxxxxxx]"
	ND var oconv(in input, in conversion);
	ND var iconv(in input, in conversion);

	ND var invertarray(in input, in force0 = (0));
	void sortarray(io array, in fns = 0, in orderby0 = "");

	ND var timedate2() {return timedate2(var(), var());}
	ND var timedate2(in localdate0, in localtime0, in glang = "");
	void getdatetime(out localdate, out localtime, out sysdate, out systime, out utcdate, out utctime);

	ND var elapsedtimetext() const; // Since TIMESTAMP
	ND var elapsedtimetext(in timestamp_difference) const;
	ND var elapsedtimetext(in timestamp1, in timestamp2) const;

	ND var amountunit(in input0);
	var amountunit(in input0, out unitx);

	// Program flow control
	///////////////////////

	// Return to parent exoprog
	// or quit to OS WITHOUT an error
	// bool to allow "or stop()"
	[[noreturn]] bool stop(in description DEFAULT_EMPTY) const;

	// Return to parent exoprog
	// or quit to OS WITH an error 1
	// bool to allow "or abort()"
	[[noreturn]] bool abort(in description DEFAULT_EMPTY) const;

	// Quit to OS WITH an error 2
	// bool to allow "or abortall()"
	[[noreturn]] bool abortall(in description DEFAULT_EMPTY) const;

	// Quit to OS WITHOUT an error
	// bool to allow "or logoff()"
	//[[deprecated("Deprecated is a great way to highlight all uses of something!")]]
	[[noreturn]] bool logoff(in description DEFAULT_EMPTY) const;

 private:

	// ioconv with a language specific month
	var exoprog_date(in type, in input0, in ndecs0, io output);
	var exoprog_number(in type, in input0, in ndecs0, io output);

};

// clang-format off

class PUBLIC MVStop     {public:explicit MVStop    (in var1 DEFAULT_EMPTY); var description;};
class PUBLIC MVAbort    {public:explicit MVAbort   (in var1 DEFAULT_EMPTY); var description;};
class PUBLIC MVAbortAll {public:explicit MVAbortAll(in var1 DEFAULT_EMPTY); var description;};
class PUBLIC MVLogoff   {public:explicit MVLogoff  (in var1 DEFAULT_EMPTY); var description;};

// clang-format on

}  // namespace exo
#endif	// EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_
