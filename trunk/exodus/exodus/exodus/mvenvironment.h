/*
Copyright (c) 2009 Stephen John Bush

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

#ifndef MVENVIRONMENT_H
#define MVENVIRONMENT_H 1

#undef eq
#include <iostream>
#include <vector>
#define eq ==

#include <exodus/mv.h>
#include <exodus/mvfunctor.h>

namespace exodus
{

const static int NTHREADS=1;

//to avoid gcc 4 "warning: type attributes are honored only at type definition"
//dont declare DLL_PUBLIC on forward declarations
//class DLL_PUBLIC MvEnvironment;
class MvEnvironment;

#ifdef EXO_MVENVIRONMENT_CPP
	//plus 1 to allow for main thread to have its own
	DLL_PUBLIC
	std::vector<MvEnvironment*> global_environments(NTHREADS+1);
#else
	extern
	DLL_PUBLIC
	std::vector<MvEnvironment*> global_environments;
#endif

class DLL_PUBLIC MvEnvironment
{
public:

	virtual ~MvEnvironment();
    
	bool init(const int threadno);
	
	var ID;
	var RECORD;
	var DICT;
	var MV;
	var ANS;
	var LISTACTIVE;

	var USERNAME;
	var ACCOUNT;
	var SENTENCE;

	var DATA;
	var PSEUDO;

	var UPPERCASE;
	var LOWERCASE;
	
	var CRTHIGH;
	var CRTWIDE;
	
	var COL1;
	var COL2;
	var STATION;
	var STATUS;
	
	//seems to fail initialisation in msvc2005 in release mode only ?!

	var USER0;
	var USER1;
	var USER2;
	var USER3;
	var USER4;

	var EXECPATH;
	var COMMAND;
	var OPTIONS;

	var AW;
	var ENVIRONSET;
	var EW;
	var FILEERROR;
	var FILEERRORMODE;
	var FLUSHNEEDED;
	var HW;
	var INTCONST;
	var PRIVILEGE;
	var PRIORITYINT;
	var ROLLOUTFILE;
	
	var TCLSTACK;
	var VOLUMES;
	var FILES;
	
	var DEFINITIONS;
	var SECURITY;
	var SESSION;
	var SYSTEM;
	var THREADNO;

	//given dictid reads dictrec from DICT file and extracts from RECORD/ID or calls library called dict+DICT function dictid
	//not const so we can mess with the library?
	var calculate(const var& dictid);

	var otherusers(const var& param);
	var otherdatasetusers(const var& param);
	
	//define a type of object that holds many LabelledCommons
	//typedef std::map<const char*, LabelledCommon> LabelledCommons;
	
	//NB does not return record yet
	bool lockrecord(const var& filename, const var& file, const var& keyx, const var& recordx, const int waitsecs=0) const;
	bool lockrecord(const var& filename, const var& file, const var& keyx) const;
	
	var capitalise(const var& str0, const var& mode=L"", const var& wordseps=L"") const;
	
	void mssg(const var& msg) const;
	void mssg(const var& msg, const var& options) const;
	void mssg(const var& msg, const var& options, var& buffer, const var& params) const;
	void msg2(const var& msg, const var& options, var& buffer, const var& params) const;
	
	void note(const var& msg) const;
	void note(const var& msg, const var& options) const;
	void note(const var& msg, const var& options, var& buffer, const var& params) const;
	void note2(const var& msg, const var& options, var& buffer, const var& params) const;
	
	void msgbase(const var& msg, const var& options=L"", const var& response=L"", const var& params=L"") const;
	
	var handlefilename(const var& handle) const;
	void debug() const;
	void fsmsg() const;
	var sysvar(const var& mv1,const var& mv2,const var& mv3,const var& mv4);
    void setprivilege(const var& mv1);
    bool openfile(const var& filename, var& file) const;
    bool openfile2(const var& filename, var& file, const var& similarfilename, const var& autocreate=L"") const;
	
	bool lockrecord(const var& xfilename, const var& xfile, const var& keyx, const var& recordx, const var& waitsecs) const;
	bool unlockrecord(const var& filename, const var& file, const var& key) const;
	
	var decide(const var& question, const var& options) const;
	var decide(const var& question, const var& options, var& reply) const;
	var decide(const var& question, const var& options, var& reply, var& buffer) const;
	
	void savescreen(var& origscrn, var& origattr) const;
	//void ostime(var& ostimenow) const;
	int keypressed(int delayusecs=0) const;
	bool esctoexit() const;
	
	void oswritex(const var& str, const var& filename) const;
	void osbwritex(const var& str1, const var& filehandle, const var& filename, const int offset) const;
	void osbreadx(var& str1, const var& filehandle, const var& filename, const int startoffset, const int length);
	
	bool authorised(const var& task, var& msg, const var& defaultlock=L"");
	bool authorised(const var& task);
	void readuserprivs();
	void writeuserprivs();
	void logger(const var& programname, const var& logtext);
	var singular(const var& pluralnoun);
	void flushindex(const var& filename);
	void sysmsg(const var& msg);
	var sendmail(const var& toaddress, const var& subject, const var& body0, const var& attachfilename, const var& deletex, var& errormsg);
	var encrypt2(const var& encrypt0) const;
	var xmlquote(const var& str) const;
	var loginnet(const var& dataset, const var& username, var& cookie, var& msg);
	var at(const int code) const;
	var at(const int x, const int y) const;
	
    //was MVDB
	var getuserdept(const var& usercode);

private:

	//used by calculate to call dict libraries
	mutable ExodusFunctorBase exodusfunctorbase_;
	//TODO cache many not just one
	mutable var cache_dictid_;
	mutable var cache_dictrec_;

};

}

#endif /*MVENVIRONMENT_H*/
