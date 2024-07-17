#ifndef EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_
#define EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_

import std;

//module #include <string>

// Using map for dict function cache instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// Perhaps switch to this https://youtu.be/M2fKMP47slQ?t=476
//#include <unordered_map>
//module #include <map>

#include <exodus/exoenv.h>
//#include <exodus/exocallable.h>
//#include "timeaccount.h"

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
	bool select(CVR sortselectclause_or_filehandle DEFAULT_EMPTY);
	ND bool hasnext();
	bool readnext(VARREF key);
	bool readnext(VARREF key, VARREF valueno);
	bool readnext(VARREF record, VARREF key, VARREF valueno);
	bool pushselect(CVR v1, VARREF v2, VARREF v3, VARREF v4);
	bool popselect(CVR v1, VARREF v2, VARREF v3, VARREF v4);
	void clearselect();
	bool deleterecord(CVR filename_or_handle_or_command, CVR key DEFAULT_EMPTY);

	bool savelist(CVR listname);
	bool getlist(CVR listname);
	bool formlist(CVR filename_or_command, CVR keys = "", const var fieldno = 0);
	bool makelist(CVR listname, CVR keys);
	bool deletelist(CVR listname);

	void note(CVR msg, CVR options = "") const;
	void note(CVR msg, CVR options, VARREF buffer, CVR params = "") const;
	void mssg(CVR msg, CVR options = "") const;
	void mssg(CVR msg, CVR options, VARREF buffer, CVR params = "") const;

	var authorised(CVR task0, VARREF msg, CVR defaultlock = "", CVR username0 = "");
	var authorised(CVR task0);
	void readuserprivs() const;
	void writeuserprivs() const;

	ND var capitalise(CVR str0, CVR mode = var(), CVR wordseps = var()) const;

	var libinfo(CVR command);
	var perform(CVR sentence);
	[[noreturn]] void chain(CVR libraryname);
	var execute(CVR sentence);

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
	ND var calculate(CVR dictid);
	ND var calculate(CVR dictid, CVR dictfile, CVR id, CVR record, CVR mv = 0);
	ND var xlate(CVR filename, CVR key, CVR fieldno_or_name, const char* mode);

	ND var otherusers(CVR param);
	ND var otherdatasetusers(CVR param);

	//bool fsmsg(CVR msg = "") const;	 // always returns false so can be used like return fsmsg();
	ND var sysvar(CVR var1, CVR var2, CVR mv3, CVR mv4);
	void setprivilege(CVR var1);

	// NB does not return record yet
	ND bool lockrecord(CVR filename, VARREF file, CVR keyx, CVR recordx, const int waitsecs = 0, const bool allowduplicate = false) const;
	ND bool lockrecord(CVR filename, VARREF file, CVR keyx) const;
	bool unlockrecord(CVR filename, VARREF file, CVR key) const;
	bool unlockrecord() const;

	ND var decide(CVR question, CVR options = "") const;
	var decide(CVR question, CVR options, VARREF reply, const int defaultreply = 1) const;

	void savescreen(VARREF origscrn, VARREF origattr) const;
	var keypressed(int milliseconds = 0) const;
	ND bool esctoexit() const;

	void flushindex(CVR filename);
	ND var encrypt2(CVR encrypt0) const;
	ND var xmlquote(CVR str) const;
	ND bool loginnet(CVR dataset, CVR username, VARREF cookie, VARREF msg);

	// TERMINAL specific terminal cursor control
	ND var AT(const int code) const;
	ND var AT(const int x, const int y) const;

	ND var getcursor() const;
	void setcursor(CVR cursor) const;

	ND var getprompt() const;
	void setprompt(CVR prompt) const;

	ND var handlefilename(CVR handle);

	ND var getuserdept(CVR usercode);

	// ioconv with access to all exoprog functionality and base ioconv
	// Particularly the ability to call custom ioconv funcs like "[xxxxxxxx]"
	ND var oconv(CVR input, CVR conversion);
	ND var iconv(CVR input, CVR conversion);

	ND var invertarray(CVR input, CVR force0 = (0));
	void sortarray(VARREF array, CVR fns = 0, CVR orderby0 = "");

	ND var timedate2() {return timedate2(var(), var());}
	ND var timedate2(in localdate0, in localtime0, in glang = "");
	void getdatetime(out localdate, out localtime, out sysdate, out systime, out utcdate, out utctime);

	ND var elapsedtimetext() const; // Since TIMESTAMP
	ND var elapsedtimetext(CVR timestamp_difference) const;
	ND var elapsedtimetext(CVR timestamp1, CVR timestamp2) const;

	ND var amountunit(in input0);
	var amountunit(in input0, out unitx);

	// Program flow control
	///////////////////////

	// Return to parent exoprog
	// or quit to OS WITHOUT an error
	// bool to allow "or stop()"
	[[noreturn]] bool stop(CVR description DEFAULT_EMPTY) const;

	// Return to parent exoprog
	// or quit to OS WITH an error 1
	// bool to allow "or abort()"
	[[noreturn]] bool abort(CVR description DEFAULT_EMPTY) const;

	// Quit to OS WITH an error 2
	// bool to allow "or abortall()"
	[[noreturn]] bool abortall(CVR description DEFAULT_EMPTY) const;

	// Quit to OS WITHOUT an error
	// bool to allow "or logoff()"
	//[[deprecated("Deprecated is a great way to highlight all uses of something!")]]
	[[noreturn]] bool logoff(CVR description DEFAULT_EMPTY) const;

 private:

	// ioconv with a language specific month
	var exoprog_date(CVR type, CVR input0, CVR ndecs0, VARREF output);
	var exoprog_number(CVR type, CVR input0, CVR ndecs0, VARREF output);

};

// clang-format off

class PUBLIC MVStop     {public:explicit MVStop    (CVR var1 DEFAULT_EMPTY); var description;};
class PUBLIC MVAbort    {public:explicit MVAbort   (CVR var1 DEFAULT_EMPTY); var description;};
class PUBLIC MVAbortAll {public:explicit MVAbortAll(CVR var1 DEFAULT_EMPTY); var description;};
class PUBLIC MVLogoff   {public:explicit MVLogoff  (CVR var1 DEFAULT_EMPTY); var description;};

// clang-format on

}  // namespace exo
#endif	// EXODUS_LIBEXODUS_EXODUS_EXOPROG_H_
