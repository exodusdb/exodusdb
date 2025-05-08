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

//module #include <string>
//#include <thread>
#define EXOENV_CPP
#include <exodus/exoenv.h>

// avoid this unless absolutely necessary then possible to move this file out of exodus var library
//(stick to throwing VarError with a suitable error message)

namespace exo {

//int getprocessno(const char* filename, int* fd);
//bool processno_islocked2(int processno, int* fd);

//void releaseprocess(int* fd);
std::string mvgethostname();

// NB do not define default copy constructor and assignment in order to force
// derived classes to implement them since they are defined in the class header

// destructor
ExoEnv::~ExoEnv() {

	// std::wcout<<"ExoEnv: Closing Definitions ... "<<std::flush;
	if (this->DEFINITIONS.assigned() && this->DEFINITIONS) {
		this->DEFINITIONS.close();
	}
	// std::wcout<<"OK"<<std::endl;

	// std::wcout<<"ExoEnv: Disconnecting DB ... "<<std::flush;
	if (this->SESSION.assigned() && this->SESSION) {
		this->SESSION.close();
	}
	// std::wcout<<"OK"<<std::endl;

	// a file handle to make unique locks
	//if (processnolockfd != 0)
	//	releaseprocess(&processnolockfd);
}

// keep in sync both 1) declaration in class and 2) contruction initialisation
bool ExoEnv::init(const int threadno) {

	// std::wcout<<"ExoEnv::init("<<threadno<<")"<<std::endl;

	this->THREADNO = threadno;

	// pretty obsolete nowadays
	// environment variables may not be available until exported
	// do set -p to find out exported variables instead of all
	if (not this->CRTWIDE.osgetenv("COLUMNS") or not this->CRTWIDE)
		this->CRTWIDE = 80;
	if (not this->CRTHIGH.osgetenv("LINES") or not this->CRTHIGH)
		this->CRTHIGH = 25;

	//	this->THREADNO.outputl("PROCESS NO ===================================================== ");

	//this->STATION = var(mvgethostname()).field(".", 1);

	return true;
}

var ExoEnv::parse(in sentence) {

	// Parse words using spaces into an FM delimited string leaving quoted phrases intact.
	// Spaces are used to parse fields.
	// Spaces and single quotes are preserved inside double quotes.
	// Spaces are double quotes preserved inside single quotes.
	// Backslashes and any character following a backslash (particularly spaces, double and single quotes and backslashes) are treated as non-special characters.
	SENTENCE = sentence;

	COMMAND = SENTENCE.parse(' ');

	// Cut off OPTIONS from end of COMMAND if present
	// *** SIMILAR code in
	// 1. exofuncs.cpp exodus_main()
	// 2. exoprog.cpp  perform()
	// OPTIONS are in either (AbC) or {AbC} WITHOUT SPACES in the last field of COMMAND
	OPTIONS = COMMAND.field(FM, -1);
	if ((OPTIONS.starts("{") and OPTIONS.ends("}")) or (OPTIONS.starts("(") and OPTIONS.ends(")"))) {
		// Remove last field of COMMAND. TODO fpopper command or remover(-1)?
		COMMAND.cutter(-OPTIONS.len() - 1);
		// Remove first { or ( and last ) } chars of OPTIONS
		OPTIONS.cutter(1);
		OPTIONS.popper();
	} else {
		OPTIONS = "";
	}

	// Load the shared library file (always lowercase) TODO allow mixed case
	// creating a new object so that its member variables all start out unassigned.
	let libname = SENTENCE.field(" ", 1).lcase();

	return libname;
}

//bool ExoEnv::processno_islocked(int processno)
//{
//	return processno_islocked2(processno, &processnolockfd);
//}

}  // namespace exo
