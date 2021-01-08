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

#ifndef MVENVIRONMENT_H
#define MVENVIRONMENT_H 1

#undef eq
#include <iostream>
#include <vector>
#include <unordered_map>
#define eq ==

#include <exodus/mv.h>
#include <exodus/mvfunctor.h>

// for labelled commons if not using array
//#include <map>
//#include <boost/any.hpp>

namespace exodus
{

// to avoid gcc 4 "warning: type attributes are honored only at type definition"
// dont declare DLL_PUBLIC on forward declarations
// class DLL_PUBLIC MvEnvironment;
class MvEnvironment;

/*
const static int NTHREADS = 1;

#ifdef EXO_MVENVIRONMENT_CPP
// plus 1 to allow for main thread to have its own
DLL_PUBLIC
std::vector<MvEnvironment*> global_environments(NTHREADS + 1);
#else
extern DLL_PUBLIC std::vector<MvEnvironment*> global_environments;
#endif
*/

class LabelledCommon
{
      //public:
	//virtual ~LabelledCommon() {}
};

class DLL_PUBLIC MvEnvironment
{
      public:
	virtual ~MvEnvironment();

	bool init(const int threadno);

	bool processno_islocked(int processno);

	// keep in sync both 1) declaration in class and 2) construction initialisation

	// per user
	var USERNAME = "";
	var PRIVILEGE = "";

	// per application
	var APPLICATION = "";

	// per host
	var STATION = "";

	// per execution
	var COMMAND = "";
	var OPTIONS = "";
	var EXECPATH = "";
	var CHAIN = "";

	// per process - set in init()
	var ROLLOUTFILE = "";
	var THREADNO = "";

	// per db access
	var DICT = "";
	var ID = "";
	var RECORD = "";
	var MV = "";
	var LISTACTIVE = "";
	var SESSION = "";
	var STATUS = "";
	var FILEERROR = "";
	var FILEERRORMODE = "";
	var FILES = "";

	// per configuration
	var DEFINITIONS = "definitions";
	var SYSTEM = "";
	var SECURITY = "";

	// per request
	var SENTENCE = "";
	var PSEUDO = "";
	var DATA = "";
	var ANS = "";

	// temporary application globals
	var USER0 = "";
	var USER1 = "";
	var USER2 = ""; // was base currency format
	var USER3 = "";
	var USER4 = "";

	// per execute level temporary application global
	var RECUR0 = "";
	var RECUR1 = "";
	var RECUR2 = "";
	var RECUR3 = "";
	var RECUR4 = "";

	// i18n/l10n - basic internationalisation/localisation
	var DATEFMT = "D/E";    // international date format
	var BASEFMT = "MD20P,"; // base currency format
	var SW = "";

	// encoding globals
	// MUST be the same length
	// TODO Should not be global otherwise cannot multithread MvEnvironment
	var LOWERCASE = LOWERCASE_;
	var UPPERCASE = UPPERCASE_;
	// var INTERNALCHARS = "\x1A\x1B\x1C\x1D\x1E\x1F";
	// var EXTERNALCHARS = _STM_ _TM_ _SM_ _VM_ _FM_ _RM_;

	var TCLSTACK = "";
	// INTCONST=keyboard constants 1:ESC 2:f5 7:f10
	var INTCONST = "\x1B" _FM_ "{f5?}" _FM_ _FM_ _FM_ _FM_ _FM_ "{f10?}";
	var PRIORITYINT = "";
	int COL1 = 0;
	int COL2 = 0;

	// old scratch variables used for various buffering
	var AW = "";
	var EW = "";
	var HW = "";
	var MW = "";
	var PW = "";
	// var SW;//moved section
	var VW = "";
	var XW = "";

	var RECCOUNT = "";

	// pretty obsolete nowadays
	// environment variables may not be available until exported
	// do set -p to find out exported variables instead of all
	var CRTWIDE = 80;
	var CRTHIGH = 25;
	var LPTRWIDE = 132;
	var LPTRHIGH = 66;

	// obsolete
	var ENVIRONKEYS = "";
	var ENVIRONSET = "";
	var DEFAULTSTOPS = "";
	var MOVEKEYS = "";
	var INDEXTIME = "";
	var LEVEL = "";
	var VOLUMES = "";

	// set in init()
	var PROCESSNO = "";

	var CURSOR = "1";

	// define a type of object that holds many LabelledCommons
	LabelledCommon* labelledcommon[99] = {0};

	// used to maintain locks per process eg on ~/tmp/exodus
	// init() opens it. destructor closes it
	int processnolockfd = 0;

	std::unordered_map<std::string, void*> dlopen_cache;

};

} // namespace exodus

#endif /*MVENVIRONMENT_H*/
