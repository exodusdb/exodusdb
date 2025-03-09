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

#if EXO_MODULE
	import std;
#else
#	include <string>
#	include <map>
#endif

#if EXO_MODULE
	import var;
#else
#	include <exodus/exoimpl.h>
#	include <exodus/exocallable.h>
#endif
#include <exodus/vardefs.h>

namespace exo {

using let = const var;

ND PUBLIC int  getenvironmentn();
   PUBLIC void setenvironmentn(const int environmentn);

ND PUBLIC int  getenvironmentn();
ND PUBLIC var  getprocessn();
ND PUBLIC var  getexecpath();
ND PUBLIC std::string gethostname();

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

// Tip about os shell environment variables
// OS shell  environment variables are not available to child processes until exported
// "set -p" to find out exported variables instead of all
// "env" - run a program in a modified environment
// See man bash for more info

// Abstract base class
class ExoCommon {
 public:
	// Virtual because derived class's members should be destructed
	virtual ~ExoCommon(){}
};

class PUBLIC ExoEnv final {
 public:
	virtual ~ExoEnv();

	bool init(const int threadno);

	bool processno_islocked(int processno);

	// Keep in sync in exoenv.h and exomacros.h

	// Per thread/exoprogram environment
	// All performed/executed/xlated/called exoprograms (libraries) share the same exodus environment
	var THREADNO  = "";
	let TIMESTAMP = var().ostimestamp();
	// Note that osgetenv is member function not a exofuncs free function
	let TERMINAL  = var().isterminal() ? osgetenv("TERM") : "";
	let EXECPATH  = getexecpath();
	// Updated to requestor's ip address in service listen
	var STATION   = var(gethostname()).field(".", 1);

	// Terminal style text formatting
	var CRTWIDE = 80;
	var CRTHIGH = 25;
	var LPTRWIDE = 132;
	var LPTRHIGH = 66;

	// Application flag indicating obsolete code
	var VOLUMES = "";

	var USERNAME = "";
	var PRIVILEGE = "";
	var APPLICATION = "";
	var DEFINITIONS = "definitions";
	var SYSTEM = "";
	var SECURITY = "";

	// Exoprogram globals available for any application defined use.
	// Common to all perform/execute levels. CF RECUR0-4
	// Initialised to "" in thread init
	var USER0 = "";
	var USER1 = "";
	var USER2 = "";	 // was base currency format
	var USER3 = "";
	var USER4 = "";

	// i18n/l10n - basic internationalisation/localisation
//	var DATEFMT = "D/E";	 // 31/12/2020 International date format
	var DATEFMT = "DE";	     // 31 DEC 2020 alpha month output, International date format input
//	var DATEFMT = "D";	     // 31 DEC 2020 alpha month output, American date format input
	//var BASEFMT = "MD20P,";	 // base currency format
	var BASEFMT = "MD20,";	 // base currency format
	var SW = "";             // User TZ offset from UTC in seconds

	// Terminal key definitions
	// INTCONST=hot keys in some places
	// q = quit    (wasEsc)
	// x = execute (was F5)
	// m = menu    (was F10)
	var INTCONST = "q" _FM "x" _FM _FM _FM _FM _FM "m";
	var PRIORITYINT = "";

	// Old scratch variables used for various buffering
	// Common to all perform/execute levels
	var AW = "";
	var EW = "";
	var HW = "";
	var MW = "";
	var PW = "";
	// var SW;//moved section
	var VW = "";
	var XW = "";

	// Per command line, perform or execute
	var SENTENCE = "";
	var COMMAND = "";
	var OPTIONS = "";
	var CHAIN = "";
	var LEVEL = 1;
	var TCLSTACK = "";
	// Initialised to "" in execute/perform init
	var RECUR0 = "";
	var RECUR1 = "";
	var RECUR2 = "";
	var RECUR3 = "";
	var RECUR4 = "";

	// Per select
	var FILE = "";
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
	var RECCOUNT = "";
	var CURSOR = "1";

	// General inter-function variables
	var PSEUDO = "";// Often holds RECORD-like request data
	var DATA = "";// Was queue for input()
	var ANS = "";// result from perform/execute/xlate

	// Per string access
	int COL1 = 0;
	var COL2 = 0;

	// define a type of object that holds many ExoCommons
	ExoCommon* namedcommon[99] = {nullptr};

	// Evade a warning from clang 18 when building with modules
	// about some unused "less" function in the std::map<std::string, void*> that we use
	// root cause in std.cppm?
//neo@hp7:~/exodus$ [255/450] Building CXX object exodus/libexodus/exodus/CMakeFiles/exodus.dir/exoenv.cpp.o
//In module 'std' imported from /root/exodus/exodus/libexodus/exodus/../exodus/exoenv.h:27:
///usr/bin/../lib/gcc/x86_64-linux-gnu/99/../../../../include/c++/99/bits/stl_function.h:403:26: warning: 'binary_function<std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char>, bool>' is deprecated [-Wdeprecated-declarations]
//  403 |     struct less : public binary_function<_Tp, _Tp, bool>
//      |                          ^
#pragma clang diagnostic push
// Evade clang -Wdeprecated-declarations warning when using dlopen_cache
// Some "using" statement must be missing from std.cppm
#if defined(EXO_MODULE) && __clang_major__ >= 18
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

	// A cache of handles to dynamically loaded shared libraries
	std::map<std::string, void*> dlopen_cache;
//	std::map<std::string, void*, std::less<std::string<char>>> dlopen_cache;
//	std::map<std::string, void*, std::less<char>> dlopen_cache;

#pragma clang diagnostic pop

 private:

var osgetenv(SV envcode) {
	var envvalue;
	if (envvalue.osgetenv(envcode))
		return envvalue;
	return "";
}

}; // class ExoEnv

}  // namespace exo

#endif //EXOENV_H
