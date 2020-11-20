#ifndef MVPROGRAM_H
#define MVPROGRAM_H

#include <exodus/mvenvironment.h>
//#include <exodus/mvfunctor.h>

#include <unordered_map>

namespace exodus
{

class ExodusProgramBase
{

      public:
	using in = const var&;
	using io = var&;
	using out = var&;

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
	bool select(const var& sortselectclause DEFAULTNULL);
	void clearselect();
	bool hasnext();
	bool readnext(var& key);
	bool readnext(var& key, var& valueno);
	bool selectrecord(const var& sortselectclause DEFAULTNULL);
	bool readnextrecord(var& record, var& key);
	bool pushselect(const var& v1, var& v2, var& v3, var& v4);
	bool popselect(const var& v1, var& v2, var& v3, var& v4);

	bool savelist(const var& listname);
	bool getlist(const var& listname);
	bool formlist(const var& filename_or_command, const var& keys="", const var fieldno=0);
	bool makelist(const var& listname, const var& keys);
	bool deletelist(const var& listname);

	void note(const var& msg, const var& options = "") const;
	void note(const var& msg, const var& options, var& buffer, const var& params = "") const;
	void mssg(const var& msg, const var& options = "") const;
	void mssg(const var& msg, const var& options, var& buffer, const var& params = "") const;

	var authorised(const var& task0, var& msg, const var& defaultlock = "",
		       const var& username0 = "");
	var authorised(const var& task0);
	void readuserprivs() const;
	void writeuserprivs() const;

	var capitalise(const var& str0, const var& mode = var(), const var& wordseps = var()) const;

	var perform(const var& sentence);
	void chain(const var& libraryname);
	var execute(const var& sentence);

	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library?
	var calculate(const var& dictid);
	var calculate(const var& dictid, const var& dictfile, const var& id, const var& record,
		      const var& mv = 0);

	var xlate(const var& filename, const var& key, const var& fieldno_or_name, const var& mode);
	// moved to exodusprogrambase
	// var perform(const var& sentence);

	// ditto
	// given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library
	// called dict+DICT function dictid not const so we can mess with the library? var
	// calculate(const var& dictid); var calculate(const var& dictid, const var& dictfile, const
	// var& id, const var& record, const var& mv=0);

	var otherusers(const var& param);
	var otherdatasetusers(const var& param);

	// moved to mvprogram
	// var capitalise(const var& str0, const var& mode=var(), const var& wordseps=var()) const;
	////var capitalise(const var& str0, const var& mode="", const var& wordseps="") const;

	void debug() const;
	bool fsmsg(
	    const var& msg = "") const; // always returns false so can be used like return fsmsg();
	var sysvar(const var& var1, const var& var2, const var& mv3, const var& mv4);
	void setprivilege(const var& var1);

	// NB does not return record yet
	bool lockrecord(const var& filename, var& file, const var& keyx, const var& recordx,
			const int waitsecs = 0, const bool allowduplicate = false) const;
	// bool lockrecord(const var& xfilename, const var& xfile, const var& keyx, const var&
	// recordx, const var& waitsecs, const bool allowduplicate=false) const;
	bool lockrecord(const var& filename, var& file, const var& keyx) const;
	bool unlockrecord(const var& filename, var& file, const var& key) const;
	//unlock all
	bool unlockrecord() const;

	var decide(const var& question, const var& options) const;
	var decide(const var& question, const var& options, var& reply,
		   const int defaultreply = 0) const;

	void savescreen(var& origscrn, var& origattr) const;
	// void ostime(var& ostimenow) const;
	int keypressed(int delayusecs = 0) const;
	bool esctoexit() const;

	//	bool oswritex(const var& str, const var& filename) const;
	//	bool osbwritex(const var& str1, const var& filehandle, const var& filename, var&
	// offset) const; 	bool osbreadx(var& str1, const var& filehandle, const var& filename,
	// const int offset, const int length); 	bool osbreadx(var& str1, const var&
	// filehandle, const var& filename, var& offset, const int length);

	// moved to external function
	// bool authorised(const var& task, var& msg, const var& defaultlock="");
	// bool authorised(const var& task);
	// void readuserprivs();
	// void writeuserprivs();
	// void sysmsg(const var& msg);
	// var sendmail(const var& toaddress, const var& subject, const var& body0, const var&
	// attachfilename, const var& deletex, var& errormsg);

	var singular(const var& pluralnoun);
	void flushindex(const var& filename);
	var encrypt2(const var& encrypt0) const;
	var xmlquote(const var& str) const;
	var loginnet(const var& dataset, const var& username, var& cookie, var& msg);
	var AT(const int code) const;
	var AT(const int x, const int y) const;

	var handlefilename(const var& handle);
	var memspace(const var& requiredmemspace);

	// was MVDB
	var getuserdept(const var& usercode);

	var oconv(const var& input, const var& conversion);
	var iconv(const var& input, const var& conversion);

	var invertarray(const var& input, const var& force0=(0));

	var elapsedtimetext(const var& fromdate, const var& fromtime);
	var elapsedtimetext(const var& fromdate, const var& fromtime, var& uptodate, var& uptotime);

      private:
	// used by calculate to call dict libraries
	mutable ExodusFunctorBase* dict_exodusfunctorbase_;
	// TODO cache many not just one
	mutable var cache_dictid_;
	mutable var cache_dictrec_;
	std::unordered_map<std::string, ExodusFunctorBase*> dict_function_cache;

        //cache_dictid_ = "";
        //cache_perform_libid_ = "";
        //dict_exodusfunctorbase_ = NULL;

	// used by perform to call libraries WITH NO ARGUMENTS
	mutable ExodusFunctorBase perform_exodusfunctorbase_;
	// TODO cache many not just one
	mutable var cache_perform_libid_;
};

} // namespace exodus
#endif // MVPROGRAM_H
