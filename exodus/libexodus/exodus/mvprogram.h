#ifndef MVPROGRAM_H
#define MVPROGRAM_H

#include <unordered_map>

#include <exodus/mvenvironment.h>
//#include <exodus/mvfunctor.h>

namespace exodus {

//class ExodusProgramBase
class PUBLIC ExodusProgramBase {

   public:

	using in  = const var&; // "CVR"    means "Const Var Reference"
	using io  =       var&; // "VARREF" means "Var Reference"
	using out =       var&; // "VARREF" means "Var Reference"

	using let =       var ; // "let" behaves like javascript "let" ie. it is scoped ... but so is exodus var
	using con = const var ; // "CV"     means "Constant Var" e.g. con x = "";

#include <exodus/ioconv_custom.h>

	ExodusProgramBase(MvEnvironment& inmv);
	ExodusProgramBase(MvEnvironment&& inmv) = delete;

	// doing virtual isnt much use because external functions (which are based on
	// ExodusProgramBase) need to have complete freedom of arguments to main(...) virtual var
	// main();

	virtual ~ExodusProgramBase();

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
	MvEnvironment& mv;

	// work on CURSOR
	bool select(CVR sortselectclause_or_filehandle DEFAULT_STRING);
	ND bool hasnext();
	bool readnext(VARREF key);
	bool readnext(VARREF key, VARREF valueno);
	bool readnext(VARREF record, VARREF key, VARREF valueno);
	bool pushselect(CVR v1, VARREF v2, VARREF v3, VARREF v4);
	bool popselect(CVR v1, VARREF v2, VARREF v3, VARREF v4);
	void clearselect();
	bool deleterecord(CVR filename_or_handle_or_command, CVR key DEFAULT_STRING);

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
	bool writeuserprivs() const;

	ND var capitalise(CVR str0, CVR mode = var(), CVR wordseps = var()) const;

	var libinfo(CVR command);
	var perform(CVR sentence);
	void chain(CVR libraryname);
	var execute(CVR sentence);

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
	ND var calculate(CVR dictid);
	ND var calculate(CVR dictid, CVR dictfile, CVR id, CVR record, CVR mv = 0);

	ND var xlate(CVR filename, CVR key, CVR fieldno_or_name, const char* mode);
	// moved to exodusprogrambase
	// var perform(CVR sentence);

	// ditto
	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library? var
	// calculate(CVR dictid); var calculate(CVR dictid, CVR dictfile, const
	// VARREF id, CVR record, CVR mv=0);

	ND var otherusers(CVR param);
	ND var otherdatasetusers(CVR param);

	// moved to mvprogram
	// var capitalise(CVR str0, CVR mode=var(), CVR wordseps=var()) const;
	////var capitalise(CVR str0, CVR mode="", CVR wordseps="") const;

	void debug() const;
	bool fsmsg(CVR msg = "") const;	 // always returns false so can be used like return fsmsg();
	ND var sysvar(CVR var1, CVR var2, CVR mv3, CVR mv4);
	void setprivilege(CVR var1);

	// NB does not return record yet
	ND bool lockrecord(CVR filename, VARREF file, CVR keyx, CVR recordx, const int waitsecs = 0, const bool allowduplicate = false) const;
	// bool lockrecord(CVR xfilename, CVR xfile, CVR keyx, CVR
	// recordx, CVR waitsecs, const bool allowduplicate=false) const;
	ND bool lockrecord(CVR filename, VARREF file, CVR keyx) const;
	bool unlockrecord(CVR filename, VARREF file, CVR key) const;
	//unlock all
	bool unlockrecord() const;

	ND var decide(CVR question, CVR options) const;
	var decide(CVR question, CVR options, VARREF reply, const int defaultreply = 1) const;

	void savescreen(VARREF origscrn, VARREF origattr) const;
	// void ostime(VARREF ostimenow) const;
	var keypressed(int milliseconds = 0) const;
	ND bool esctoexit() const;

	//	bool oswritex(CVR str, CVR filename) const;
	//	bool osbwritex(CVR str1, CVR filehandle, CVR filename, VARREF
	// offset) const; 	bool osbreadx(VARREF str1, CVR filehandle, CVR filename,
	// const int offset, const int length); 	bool osbreadx(VARREF str1, CVR
	// filehandle, CVR filename, VARREF offset, const int length);

	// moved to external function
	// bool authorised(CVR task, VARREF msg, CVR defaultlock="");
	// bool authorised(CVR task);
	// void readuserprivs();
	// void writeuserprivs();
	// void sysmsg(CVR msg);
	// var sendmail(CVR toaddress, CVR subject, CVR body0, CVR
	// attachfilename, CVR deletex, VARREF errormsg);

	ND var singular(CVR pluralnoun);
	void flushindex(CVR filename);
	ND var encrypt2(CVR encrypt0) const;
	ND var xmlquote(CVR str) const;
	ND bool loginnet(CVR dataset, CVR username, VARREF cookie, VARREF msg);

	ND var AT(const int code) const;
	ND var AT(const int x, const int y) const;

	ND var getcursor();
	void setcursor(CVR cursor) const;

	ND var getprompt() const;
	void setprompt(CVR prompt) const;

	ND var handlefilename(CVR handle);
	//ND var memspace(CVR requiredmemspace);

	// was MVDB
	ND var getuserdept(CVR usercode);

	ND var oconv(CVR input, CVR conversion);
	ND var iconv(CVR input, CVR conversion);

	ND var invertarray(CVR input, CVR force0 = (0));
	void sortarray(VARREF array, CVR fns = 0, CVR orderby0 = "");

	ND var elapsedtimetext(CVR fromdate, CVR fromtime);
	ND var elapsedtimetext(CVR fromdate, CVR fromtime, VARREF uptodate, VARREF uptotime);

	ND var amountunit(in input0);
	var amountunit(in input0, out unitx);

   private:
	var number(CVR type, CVR input0, CVR ndecs0, VARREF output);
	// used by calculate to call dict libraries
	mutable ExodusFunctorBase* dict_exodusfunctorbase_;
	// TODO cache many not just one
	mutable var cache_dictid_;
	mutable var cache_dictrec_;
	std::unordered_map<std::string, ExodusFunctorBase*> dict_function_cache;

	//cache_dictid_ = "";
	//cache_perform_libid_ = "";
	//dict_exodusfunctorbase_ = nullptr;

	// used by perform to call libraries WITH NO ARGUMENTS
	mutable ExodusFunctorBase perform_exodusfunctorbase_;
	// TODO cache many not just one
	mutable var cache_perform_libid_;
};

}  // namespace exodus
#endif	// MVPROGRAM_H
