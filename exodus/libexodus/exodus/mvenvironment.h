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
#define eq ==

#include <exodus/mv.h>
#include <exodus/mvfunctor.h>

//for labelled commons if not using array
//#include <map>
//#include <boost/any.hpp>

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

class LabelledCommon{
	public:
	virtual ~LabelledCommon(){}
};

class DLL_PUBLIC MvEnvironment
{
public:

	virtual ~MvEnvironment();

	bool init(const int threadno);

	bool processno_islocked(int processno);

//keep in sync both 1) declaration in class and 2) construction initialisation

	//per user
	var USERNAME = L"";
	var PRIVILEGE = L"";

	//per application
	var APPLICATION = L"";

	//per host
	var STATION = L"";

	//per execution
	var COMMAND = L"";
	var OPTIONS = L"";
	var EXECPATH = L"";
	var CHAIN = L"";

	//per process - set in init()
	var ROLLOUTFILE = L"";
	var THREADNO = L"";

	//per db access
	var DICT = L"";
	var ID = L"";
	var RECORD = L"";
	var MV = L"";
	var LISTACTIVE = L"";
	var SESSION = L"";
	var STATUS = L"";
	var FILEERROR = L"";
	var FILEERRORMODE = L"";
	var FILES = L"";

	//per configuration
	var DEFINITIONS = L"definitions";
	var SYSTEM = L"";
	var SECURITY = L"";

	//per request
	var SENTENCE = L"";
	var PSEUDO = L"";
	var DATA = L"";
	var ANS = L"";

	//temporary application globals
	var USER0 = L"";
	var USER1 = L"";
	var USER2 = L"";//was base currency format
	var USER3 = L"";
	var USER4 = L"";

	//per execute level temporary application global
	var RECUR0 = L"";
	var RECUR1 = L"";
	var RECUR2 = L"";
	var RECUR3 = L"";
	var RECUR4 = L"";

	//i18n/l10n - basic internationalisation/localisation
	var DATEFMT = L"D/E";//international date format
	var BASEFMT = L"MD20P,";//base currency format
	var SW = L"";

	//encoding globals
	//MUST be the same length
	//TODO Should not be global otherwise cannot multithread MvEnvironment
	var LOWERCASE = LOWERCASE_;
	var UPPERCASE = UPPERCASE_;
	var INTERNALCHARS = L"\x11\x12\x13\x14\x15\x16\x17";
	var EXTERNALCHARS = _SSTM_ _STM_ _TM_ _SM_ _VM_ _FM_ _RM_;

	var TCLSTACK = L"";
	//INTCONST=keyboard constants 1:esc 2:f5 7:f10
	var INTCONST = L"\x1b" _FM_ L"{f5?}" _FM_ _FM_ _FM_ _FM_ _FM_ "{f10?}";
	var PRIORITYINT = L"";
	var COL1;
	var COL2;

	//old scratch variables used for various buffering
	var AW = L"";
	var EW = L"";
	var HW = L"";
	var MW = L"";
	var PW = L"";
	//var SW;//moved section
	var VW = L"";
	var XW = L"";

	var RECCOUNT = L"";

	//pretty obsolete nowadays
	//environment variables may not be available until exported
	//do set -p to find out exported variables instead of all
	var CRTWIDE = 80;
	var CRTHIGH = 25;
	var LPTRWIDE = 132;
	var LPTRHIGH = 66;

	//obsolete
	var ENVIRONKEYS = L"";
	var ENVIRONSET = L"";
	var DEFAULTSTOPS = L"";
	var MOVEKEYS = L"";
	var INDEXTIME = L"";
	var LEVEL = L"";
	var VOLUMES = L"";

	//set in init()
	var PROCESSNO = L"";

	var CURSOR = L"1";

	//define a type of object that holds many LabelledCommons
	//typedef std::map<const char*, LabelledCommon> LabelledCommons;
	//typedef std::map<std::string, boost::any> labelledcommons;
	LabelledCommon* labelledcommon[99]={0};

	//used to maintain locks per process eg on ~/tmp/exodus
	//init() opens it. destructor closes it
        int processnolockfd=0;
};

}

#endif /*MVENVIRONMENT_H*/
