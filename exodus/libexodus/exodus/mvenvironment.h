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

//keep in sync both 1) declaration in class and 2) contruction initialisation

	//per user
	var USERNAME;
	var PRIVILEGE;

	//per application
	var APPLICATION;

	//per host
	var STATION;

	//per execution
	var COMMAND;
	var OPTIONS;
	var EXECPATH;

	//per process
	var ROLLOUTFILE;
	var THREADNO;

	//per db access
	var DICT;
	var ID;
	var RECORD;
	var MV;
	var LISTACTIVE;
	var SESSION;
	var STATUS;
	var FILEERROR;
	var FILEERRORMODE;
	var FILES;

	//per configuration
	var DEFINITIONS;
	var SYSTEM;
	var SECURITY;

	//per request
	var SENTENCE;
	var PSEUDO;
	var DATA;
	var ANS;

	//temporary application globals
	var USER0;
	var USER1;
	//var USER2;//temp moved section
	var USER3;
	var USER4;

	//i18n/l10n - internationalisation/localisation
	var DATEFORMAT;
	var USER2;
	var SW;

	//encoding globals
	//MUST be the same length
	//TODO Should not be global otherwise cannot multithread MvEnvironment
	var LOWERCASE;
	var UPPERCASE;
	var INTERNALCHARS;
	var EXTERNALCHARS;

	var TCLSTACK;
	var INTCONST;
	var PRIORITYINT;

	//old scratch variables used for various buffering
	var AW;
	var EW;
	var XW;
	var VW;
	var PW;

	//pretty obsolete nowadays
	//environment variables may not be available until exported
	//do set -p to find out exported variables instead of all
	var CRTWIDE;
	var CRTHIGH;
	var LPTRWIDE;
	var LPTRHIGH;

	//define a type of object that holds many LabelledCommons
	//typedef std::map<const char*, LabelledCommon> LabelledCommons;
	//typedef std::map<std::string, boost::any> labelledcommons;
	LabelledCommon* labelledcommon[99];

};

}

#endif /*MVENVIRONMENT_H*/
