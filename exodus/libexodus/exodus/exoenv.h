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

#ifndef EXOENV_H
#define EXOENV_H 1

//#undef eq
#include <iostream>

// Using map for dllib cache instead of unordered_map since it is faster
// up to about 400 elements according to https://youtu.be/M2fKMP47slQ?t=258
// and perhaps even more since it doesnt require hashing time.
// Perhaps switch to this https://youtu.be/M2fKMP47slQ?t=476
//#include <unordered_map>
#include <map>

#include <vector>
//#define eq ==

#include <exodus/var.h>
#include <exodus/dim.h>
#include <exodus/exocallable.h>

namespace exodus {

PUBLIC ND int getenvironmentn();
PUBLIC void setenvironmentn(const int environmentn);

PUBLIC ND int getenvironmentn(); 
PUBLIC ND var getprocessn();
PUBLIC ND var getexecpath();

// to avoid gcc 4 "warning: type attributes are honored only at type definition"
// dont declare PUBLIC on forward declarations
// class PUBLIC ExoEnv;
class ExoEnv;

/*
const static int NTHREADS = 1;

#ifdef EXOENV_CPP
// plus 1 to allow for main thread to have its own
PUBLIC
std::vector<ExoEnv*> global_environments(NTHREADS + 1);
#else
extern PUBLIC std::vector<ExoEnv*> global_environments;
#endif
*/

class LabelledCommon {
   public:
	virtual ~LabelledCommon(){};
};

class PUBLIC ExoEnv {
   public:
	virtual ~ExoEnv();

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

	// thread globals available to the application
	// remain the same across perform/execute levels
	// initialised to "" in thread init
	var USER0 = "";
	var USER1 = "";
	var USER2 = "";	 // was base currency format
	var USER3 = "";
	var USER4 = "";

	// thread globals available to the application
	// separate per execute/perform level
	// initialised to "" in execute/perform init
	var RECUR0 = "";
	var RECUR1 = "";
	var RECUR2 = "";
	var RECUR3 = "";
	var RECUR4 = "";

	// i18n/l10n - basic internationalisation/localisation
	var DATEFMT = "D/E";	 // international date format
	var BASEFMT = "MD20P,";	 // base currency format
	var SW = "";             // TZ offsets

	// Backward compatible - deprecated
	// Not used in exodus built-in functions like lcase/ucase
	// Should be the same length otherwise character loss
	var LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
	var UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	var TCLSTACK = "";

	// INTCONST=hot keys in some places
	// q = quit    (wasEsc)
	// x = execute (was F5)
	// m = menu    (was F10)
	var INTCONST = "q" _FM "x" _FM _FM _FM _FM _FM "m";
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

	var TERMINAL = "";

	// pretty obsolete nowadays
	// environment variables may not be available until exported
	// do set -p to find out exported variables instead of all
	var CRTWIDE = 80;
	var CRTHIGH = 25;
	var LPTRWIDE = 132;
	var LPTRHIGH = 66;

	// other
	var LEVEL = 1;
	var VOLUMES = "";

	// set in init()
	var THREADNO = "";

	var CURSOR = "1";

	var TIMESTAMP = "";

	// define a type of object that holds many LabelledCommons
	LabelledCommon* labelledcommon[99] = {0};

	// used to maintain locks per process eg on ~/tmp/exodus
	// init() opens it. destructor closes it
	int processnolockfd = 0;

	//std::unordered_map<std::string, void*> dlopen_cache;
	std::map<std::string, void*> dlopen_cache;
};

}  // namespace exodus

#endif //EXOENV_H
