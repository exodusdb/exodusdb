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

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#ifdef _MSC_VER
#pragma warning (disable: 4530)
#endif

#define MV_NO_NARROW

#include <string>
#define EXO_MVENVIRONMENT_CPP
#include <exodus/mvenvironment.h>

//avoid this unless absolutely necessary then possible to move this file out of exodus var library
//(stick to throwing MVException with a suitable error message)
//#include <exodus/mvexceptions.h>

namespace exodus {

int getprocessno(const char* filename, int* fd);

//NB do not define default copy constructor and assignment in order to force
//derived classes to implement them since they are defined in the class header

//destructor
MvEnvironment::~MvEnvironment() {

	//std::wcout<<L"MvEnvironment: Closing Definitions ... "<<std::flush;
	if (this->DEFINITIONS.assigned() && this->DEFINITIONS) {
		this->DEFINITIONS.close();
	}
	//std::wcout<<L"OK"<<std::endl;

	//std::wcout<<L"MvEnvironment: Disconnecting DB ... "<<std::flush;
	if (this->SESSION.assigned() && this->SESSION) {
		this->SESSION.close();
	}
	//std::wcout<<L"OK"<<std::endl;

}

//keep in sync both 1) declaration in class and 2) contruction initialisation
bool MvEnvironment::init(const int threadno) {

	//std::wcout<<L"MvEnvironment::init("<<threadno<<L")"<<std::endl;

	//per user
	this->USERNAME = L"";
	this->PRIVILEGE = L"";

	//per application
	this->APPLICATION = L"";

	//per host
	this->STATION = L"";

	//per execution
	//this->COMMAND = L";//initialised in main
	//this->OPTIONS = L";//initialised in main
	//this->EXECPATH = L";//initialised in main

	//per process
	this->ROLLOUTFILE = L"~" ^ var(threadno) ^ L".$$$";
	this->THREADNO = threadno;

	//per record access
	this->DICT = L"";
	this->ID = L"";
	this->RECORD = L"";
	this->MV = L"";
	this->LISTACTIVE = 0;
	//this->SESSION = L"";//leave to be initialised in main() or on demand
	this->STATUS = L"";
	this->FILEERROR = L"";
	this->FILEERRORMODE = L"";
	this->FILES = L"";

	//per configuration
	this->DEFINITIONS = L"";
	this->SYSTEM = L"";
	this->SECURITY = L"";

	//per request
	this->SENTENCE = L"";
	this->PSEUDO = L"";
	this->DATA = L"";
	this->ANS = L"";

	//temporary application globals
	this->USER0 = L"";
	this->USER1 = L"";
	//var USER2 = L"";//temp moved section. can be recreated here when proper name created there
	this->USER3 = L"";
	this->USER4 = L"";

	//i18n/l10n - internationalisation/localisation
	this->DATEFORMAT = L"D/E";
	this->USER2 = L"MD20P,";//base currency format
	this->SW = L"";//timezone
	/* from initgeneral
	//SW<1> is the ADJUSTMENT to get display time from server time
	//SW<2> is the difference from gmt/utc to server time
	//SW<3> could be the adjustment to get dbtz from servertz
	*/

	//character encoding globals
	//MUST be the same length
	//TODO Should not be global otherwise cannot multithread MvEnvironment
	this->LOWERCASE = LOWERCASE_;
	this->UPPERCASE = UPPERCASE_;
	this->INTERNALCHARS = L"\x11\x12\x13\x14\x15\x16\x17";
	this->EXTERNALCHARS = _SSTM_ _STM_ _TM_ _SM_ _VM_ _FM_ _RM_;

	this->TCLSTACK = L"";
	this->INTCONST = L"";
	this->PRIORITYINT = L"";

	//old scratch variables used for various buffering
	this->AW = L"";
	this->EW = L"";
	this->HW = L"";
	this->MW = L"";
	this->PW = L"";
	//this->SW = L"";
	this->VW = L"";
	this->XW = L"";

	this->RECCOUNT = L"";

	//pretty obsolete nowadays
	//environment variables may not be available until exported
	//do set -p to find out exported variables instead of all
	this->CRTWIDE.osgetenv(L"COLUMNS");
	this->CRTHIGH.osgetenv(L"LINES");
	if (not this->CRTWIDE)
		this->CRTWIDE= 80;
	if (not this->CRTHIGH)
		this->CRTHIGH = 25;

	this->LPTRWIDE = 132;
	this->LPTRHIGH = 66;

        //obsolete
        this->ENVIRONKEYS=L"";
        this->ENVIRONSET=L"";
        this->DEFAULTSTOPS=L"";
        this->MOVEKEYS=L"";
        this->INDEXTIME=L"";
	this->LEVEL=L"";

	int fd;//memory leak unless closed in mvenvironment destructor;
	this->PROCESSNO=getprocessno("/tmp/exodus", &fd);

	return true;

}

/*bool MvEnvironment::lockrecord(const var& filename, const var& file,
		const var& keyx, const var& recordx, const var& waitsecs0, const bool allowduplicate) const {

	//linemark
	//common /shadow.mfs/ shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3

	var waitsecs;
	if (waitsecs0.unassigned())
		waitsecs = 0;
	else
		waitsecs = waitsecs0;

	//nb case where we do not wish to wai
	//wait
	// 0 fail immediately (eg looking for next batch/nextkey)
	// 9 wait for 9 seconds
	// - wait for a default number of seconds
	// * wait infinite number of seconds
	//if index(file,'message',1) else de bug
	lock: if (file.lock(keyx)) {
		return 1;
	} else {
		if (waitsecs) {
			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return 0;
	}

	//should not get here
	return 0;

	//evade warning: unused parameter
	if (filename || recordx) {
	}
	return 0;

}*/

}	//of namespace exodus
