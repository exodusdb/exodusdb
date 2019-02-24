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
bool processno_islocked2(int processno, int* fd);

void releaseprocess(int* fd);
std::string mvgethostname();

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

	//a file handle to make unique locks
	if (processnolockfd!=0)
		releaseprocess(&processnolockfd);
}

//keep in sync both 1) declaration in class and 2) contruction initialisation
bool MvEnvironment::init(const int threadno) {

	//std::wcout<<L"MvEnvironment::init("<<threadno<<L")"<<std::endl;

	//per process
	this->ROLLOUTFILE = L"~" ^ var(threadno) ^ L".$$$";
	this->THREADNO = threadno;

	//pretty obsolete nowadays
	//environment variables may not be available until exported
	//do set -p to find out exported variables instead of all
	this->CRTWIDE.osgetenv(L"COLUMNS");
	this->CRTHIGH.osgetenv(L"LINES");
	if (not this->CRTWIDE)
		this->CRTWIDE= 80;
	if (not this->CRTHIGH)
		this->CRTHIGH = 25;

	this->PROCESSNO=getprocessno("/tmp/exodus", &processnolockfd);

	this->STATION=var(mvgethostname()).field(L".",1);

	return true;

}

bool MvEnvironment::processno_islocked(int processno) {
	return processno_islocked2(processno, &processnolockfd);
}

}	//of namespace exodus
