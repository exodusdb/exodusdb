#ifndef MVPROGRAM_H
#define MVPROGRAM_H

#include <exodus/mvenvironment.h>
//#include <exodus/mvfunctor.h>

#include <unordered_map>

namespace exodus {

class ExodusProgramBase {

   public:
	using in = const var&; // CVR;
	using io = var&;       // VARREF;
	using out = var&;      // VARREF;
	using let = const var;

#include <exodus/ioconv_custom.h>

	DLL_PUBLIC
	ExodusProgramBase(MvEnvironment& inmv);
	DLL_PUBLIC
	ExodusProgramBase(MvEnvironment&& inmv) = delete;

	// doing virtual isnt much use because external functions (which are based on
	// ExodusProgramBase) need to have complete freedom of arguments to main(...) virtual var
	// main();

	DLL_PUBLIC
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
	bool hasnext();
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
	void writeuserprivs() const;

	var capitalise(CVR str0, CVR mode = var(), CVR wordseps = var()) const;

	var libinfo(CVR command);
	var perform(CVR sentence);
	void chain(CVR libraryname);
	var execute(CVR sentence);

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
	var calculate(CVR dictid);
	var calculate(CVR dictid, CVR dictfile, CVR id, CVR record, CVR mv = 0);

	var xlate(CVR filename, CVR key, CVR fieldno_or_name, CVR mode);
	// moved to exodusprogrambase
	// var perform(CVR sentence);

	// ditto
	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library? var
	// calculate(CVR dictid); var calculate(CVR dictid, CVR dictfile, const
	// VARREF id, CVR record, CVR mv=0);

	var otherusers(CVR param);
	var otherdatasetusers(CVR param);

	// moved to mvprogram
	// var capitalise(CVR str0, CVR mode=var(), CVR wordseps=var()) const;
	////var capitalise(CVR str0, CVR mode="", CVR wordseps="") const;

	void debug() const;
	bool fsmsg(
		CVR msg = "") const;	 // always returns false so can be used like return fsmsg();
	var sysvar(CVR var1, CVR var2, CVR mv3, CVR mv4);
	void setprivilege(CVR var1);

	// NB does not return record yet
	bool lockrecord(CVR filename, VARREF file, CVR keyx, CVR recordx, const int waitsecs = 0, const bool allowduplicate = false) const;
	// bool lockrecord(CVR xfilename, CVR xfile, CVR keyx, CVR
	// recordx, CVR waitsecs, const bool allowduplicate=false) const;
	bool lockrecord(CVR filename, VARREF file, CVR keyx) const;
	bool unlockrecord(CVR filename, VARREF file, CVR key) const;
	//unlock all
	bool unlockrecord() const;

	var decide(CVR question, CVR options) const;
	var decide(CVR question, CVR options, VARREF reply, const int defaultreply = 1) const;

	void savescreen(VARREF origscrn, VARREF origattr) const;
	// void ostime(VARREF ostimenow) const;
	var keypressed(int milliseconds = 0) const;
	bool esctoexit() const;

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

	var singular(CVR pluralnoun);
	void flushindex(CVR filename);
	var encrypt2(CVR encrypt0) const;
	var xmlquote(CVR str) const;
	var loginnet(CVR dataset, CVR username, VARREF cookie, VARREF msg);
	var AT(const int code) const;
	var AT(const int x, const int y) const;

	var handlefilename(CVR handle);
	var memspace(CVR requiredmemspace);

	// was MVDB
	var getuserdept(CVR usercode);

	var oconv(CVR input, CVR conversion);
	var iconv(CVR input, CVR conversion);

	var invertarray(CVR input, CVR force0 = (0));
	void sortarray(VARREF array, CVR fns = 0, CVR orderby0 = "");

	var elapsedtimetext(CVR fromdate, CVR fromtime);
	var elapsedtimetext(CVR fromdate, CVR fromtime, VARREF uptodate, VARREF uptotime);

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
