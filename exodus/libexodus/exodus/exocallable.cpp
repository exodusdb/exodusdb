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

// exocallable provides simple function-like syntax xyz() to call main functions in objects
// stored in external shared objects libraries
//
// strategy is to open a pointer to the main function in an object in the library
//
// initialisation
// 1. open a pointer to a library (plibrary_)
// 2. open a pointer to a global function in that library (pfunction_)
// 3. call the global function from 2. to retrieve a new object and pointer
//   to its main function pobject_ and pmemberfunction_)

// usage
// 4. call the main function on that object to execute the code

// NOTE that the code can also stop at step 2 to call global functions directly BUT
// although calling global functions gives them mv (via argument) they cant call
// external subroutines since external subroutines are implemented in exodus
// as member functions which require special coding to be called from global functions

//#define TRACING 0
#define TRACING 1

#include <iostream>

/////////////////////////////////////////////
// WINDOWS
/////////////////////////////////////////////

// portable shared/dynamic library macros
// makes linux dlopen, dlsym, dlclose syntax work on windows
// http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=746&lngWId=3
#if defined(_WIN32) || defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)

// windows.h has function defined so save function definition, undefine it for restoring
#define _save_function_ function
#undef function

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

//restore function definition
#define function _save_function_

using library_t = HINSTANCE;
//# define dlopen(arg1,arg2) LoadLibrary(arg1)
#define dlopen(arg1, arg2) LoadLibraryA(arg1)
#define dlsym(arg1, arg2) GetProcAddress(arg1, arg2)
// note: MS FreeLibrary returns non-zero for success and 0 for failure
#define dlclose(arg1) FreeLibrary(arg1)
#define EXODUSLIBPREFIX
#define EXODUSLIBEXT ".dll"

/////////////////////////////////////////////
// LINUX
/////////////////////////////////////////////

#else
#define DLERROR
#include <dlfcn.h>
using library_t = void*;
#define EXODUSLIBPREFIX "~/lib/lib"
//# define EXODUSLIBPREFIX "./lib"+
#define EXODUSLIBEXT ".so"

#endif

// needed for strcmp
//#include <string.h>

// needed for getenv
#include <stdlib.h>

/* could be used to force lower case library file name
#include <ctype.h>
char* stolower(char* s)
{
	char* p = s;
	while (*p = tolower(*p))
		p++;
	return s;
}
*/

// probably no need for LOCKDLCACHE since we have a separate cache per exoenv
#include <mutex>
static std::mutex global_mutex_lockdlcache;
//#define LOCKDLCACHE std::lock_guard<std::mutex> guard(global_mutex_lockdlcache);
#define LOCKDLCACHE std::lock_guard guard(global_mutex_lockdlcache);

#include <exodus/varimpl.h>
#include <exodus/exoenv.h>
#include <exodus/exocallable.h>
namespace exodus {

// using namespace exodus;

// default constructor.  probably followed by .init(libname,funcname,mv)
CallableBase::CallableBase()
	// TODO optimise by only initialise one and detect usage on that only
	: mv_(nullptr), libraryname_(""), functionname_(""), plibrary_(nullptr), pfunction_(nullptr), pobject_(nullptr), pmemberfunction_(nullptr) {
	pobject_ = 0;
}

// constructor to provide everything immediately
CallableBase::CallableBase(const std::string libname, const std::string funcname, ExoEnv& mv)
	: mv_(&mv), libraryname_(libname), functionname_(funcname), plibrary_(nullptr), pfunction_(nullptr), pobject_(nullptr), pmemberfunction_(nullptr) {
	pobject_ = 0;
}

// constructor to provide library name and function immediately
CallableBase::CallableBase(const std::string libname, const std::string funcname)
	: mv_(nullptr), libraryname_(libname), functionname_(funcname), plibrary_(nullptr), pfunction_(nullptr), pobject_(nullptr), pmemberfunction_(nullptr) {
	pobject_ = 0;
}

// constructor to provide environment immediately. probably followed by .init(libname,funcname)
CallableBase::CallableBase(ExoEnv& mv)
	: mv_(&mv), libraryname_(""), functionname_(""), plibrary_(nullptr), pfunction_(nullptr), pobject_(nullptr), pmemberfunction_(nullptr) {
	pobject_ = 0;
}

CallableBase::~CallableBase() {
	// will delete any shared object first
	closelib();
}

// atm designed to be called once only the first time an external function is called
bool CallableBase::init(const char* newlibraryname, const char* newfunctionname, ExoEnv& mv) {
#if TRACING >= 4
	std::cout << "exocallable:init(lib,func)  " << libraryname_ << ", " << functionname_ << std::endl;
#endif
	mv_ = &mv;
	checkload(newlibraryname, newfunctionname);

	// call a function in the library to create one of its "exodus program" objects
	// nb we MUST call the same library to delete it
	// so that the same memory management routine is called to create and delete it.
	// pfunction_ return a pobject_ if pobject_ is passed in nullptr (using mv as an init argument)
	// or deletes a pobject_ if not

	// generate an error here to debug
	//	pobject_->main();

	pfunction_(pobject_, *mv_, pmemberfunction_);
	// pobject_->main();
	//((*pobject_).*(pmemberfunction_))();
	// CALLMEMBERFUNCTION(*pobject_,pmemberfunctibon_)();
	if (pobject_ == nullptr || pmemberfunction_ == nullptr)
		return false;

#if TRACING >= 3
	std::cout << "exocallable:init(lib,func)OK" << libraryname_ << ", " << functionname_ << " " << pobject_ << "," << pmemberfunction_ << std::endl;
#endif
	return true;
}

// called from every library .h file if _pmemberfunction is nullptr
// atm designed to be called once only the first time an external function is called
// assuming library and function names and mv are already set
bool CallableBase::init() {
#if TRACING >= 4
	std::cout << "exocallable:init()          " << libraryname_ << ", " << functionname_ << std::endl;
#endif
	checkload(libraryname_, functionname_);

	// call a function in the library to create one of its "exodus program" objects
	// nb we MUST call the same library to delete it
	// so that the same memory management routine is called to create and delete it.
	// pfunction_ return a pobject_ if pobject_ is passed in nullptr (using mv as an init argument)
	// or deletes a pobject_ if not

	// generate an error here to debug
	//	pobject_->main();

	pfunction_(pobject_, *mv_, pmemberfunction_);
	// pobject_->main();
	//((*pobject_).*(pmemberfunction_))();
	// CALLMEMBERFUNCTION(*pobject_,pmemberfunctibon_)();
	if (pobject_ == nullptr || pmemberfunction_ == nullptr) {
#if TRACING >= 1
		std::cout << "exocallable:init() NOK " << libraryname_ << ", " << functionname_ << std::endl;
#endif
		return false;
	}
#if TRACING >= 3
	std::cout << "exocallable:init()          " << libraryname_ << ", " << functionname_ << " " << pobject_ << "," << pmemberfunction_ << std::endl;
#endif
	return true;
}

// called by dict/perform/execute ... can be called repeatably since buffers lib and func
// perform/execute call with forcenew=true to ensure global variables are unassigned initially
bool CallableBase::initsmf(const char* newlibraryname, const char* newfunctionname, const bool forcenew) {

#if TRACING >= 4
	std::cout << "exocallable:initsmf: === in === " << newlibraryname << std::endl;
#endif

	if (newlibraryname != libraryname_ && !openlib(newlibraryname)) {
		return false;
	}

	// make sure we have the right program object creation/deletion function
	// forcenew is used in perform/execute to ensure that all global variables
	// are initialised ie performing the same command twice
	// starts from scratch each time
	if (forcenew || newfunctionname != functionname_) {
		if (!openfunc(newfunctionname)) {
			return false;
		}
		// call a function in the library to create one of its "exodus program" objects
		// nb we MUST call the same library to delete it
		// so that the same memory management routine is called to create and delete it.
		// pfunction_ return a pobject_ if pobject_ is passed in nullptr (using mv as an init
		// argument)
		// or deletes a pobject_ if not

		// generate an error here to debug
		//	pobject_->main();

		// create a shared object and get pointer to its main member function
		pfunction_(pobject_, *mv_, pmemberfunction_);
		// pobject_->main();
		//((*pobject_).*(pmemberfunction_))();
		// CALLMEMBERFUNCTION(*pobject_,pmemberfunctibon_)();
		if (pobject_ == nullptr || pmemberfunction_ == nullptr) {
			return false;

#if TRACING >= 1
			std::cout << "exocallable:initsmf: ko: no pobject_" << libraryname_ << " "
					  << functionname_ << std::endl;
#endif
		}
	}

#if TRACING >= 3
	std::cout << "exocallable:initsmf()       " << libraryname_ << ", " << functionname_ << " " << pobject_ << "," << pmemberfunction_ << std::endl;
#endif

	return true;
}

// this version just gets a pointer to an external shared global function
// not used atm?
bool CallableBase::initsgf(const char* newlibraryname, const char* newfunctionname) {
	if (newlibraryname != libraryname_ and !openlib(newlibraryname)) {
		return false;
	}

	// make sure we have the right program object creation/deletion function
	if (newfunctionname != functionname_ and !openfunc(newfunctionname)) {
		return false;
	}

	return true;
}

// assign (case sensitive)
// pickos xxx="functionname"; call @xxx
void CallableBase::operator=(const char* newlibraryname) {
	if (newlibraryname != libraryname_) {
		closelib();
		libraryname_ = newlibraryname;
	}
	return;
}

bool CallableBase::checkload(std::string newlibraryname, std::string newfunctionname) {
#if TRACING >= 4
	std::cout << "exocallable:checkload: in>" << newlibraryname << " " << newfunctionname
			  << std::endl;
#endif

	// find the library or fail
	if (not openlib(newlibraryname)) {

#if TRACING >= 1
		std::cout << "exocallable:checkload: ko:" << newlibraryname << std::endl;
#endif

		throw VarError("Unable to load " ^ var(libraryfilename_));
		return false;
	}

	// find the function or fail
	if (not openfunc(newfunctionname)) {
#if TRACING >= 1
		std::cout << "exocallable:checkload: ko:" << libraryname_ << " " << newfunctionname
				  << std::endl;
#endif

		throw VarError("Unable to find function " ^ var(newfunctionname) ^ " in " ^
					  var(libraryfilename_));
		return false;
	}

#if TRACING >= 4
	std::cout << "exocallable:checkload: ok<" << libraryname_ << std::endl;
#endif

	return true;
}

std::string CallableBase::libfilename(std::string libraryname) const {

	//look for lib file in ~/lib/libXXXXXX.so
	std::string libfilename = EXODUSLIBPREFIX + libraryname + EXODUSLIBEXT;
	if (libfilename[0] == '~') {
		// env string is copied into string so following getenv usage is safe
		var exo_HOME;
		if (not exo_HOME.osgetenv("EXO_HOME"))
			exo_HOME.osgetenv("HOME");
		libfilename.replace(0, 1, exo_HOME.toString());
	}

	return libfilename;
}

bool CallableBase::openlib(std::string newlibraryname) {

	//uses a cache in mv_->dlopen_cache

#if TRACING >= 4
	std::cout << "exocallable:openlib: >>> in >>> " << newlibraryname << std::endl;
#endif

	// open the library or return 0
	// dlopen arg2 is ignored by macro on windows

	closelib();

#ifdef DLERROR
	dlerror();
#endif

	{
		//no need for this if dlcache is per mv and is therefore effectively threadlocal?
		LOCKDLCACHE

		//look for library in cache
		auto cacheentry = mv_->dlopen_cache.find(newlibraryname);
		if (cacheentry != mv_->dlopen_cache.end()) {
			//std::cout << "using dlopen cache for " << newlibraryname << std::endl;
			plibrary_ = mv_->dlopen_cache.at(newlibraryname);
			libraryname_ = newlibraryname;
			return true;
		}
	}

	//look for lib file in ~/lib/libXXXXXX.so
//	libraryfilename_ = EXODUSLIBPREFIX + newlibraryname + EXODUSLIBEXT;
//	if (libraryfilename_[0] == '~') {
//		// env string is copied into string so following getenv usage is safe
//		var exo_HOME;
//		if (not exo_HOME.osgetenv("EXO_HOME"))
//			exo_HOME.osgetenv("HOME");
//		libraryfilename_.replace(0, 1, exo_HOME.toString());
//		//var(libraryfilename_).logputl();
//	}
	libraryfilename_ = libfilename(newlibraryname);

//	FILE* file = fopen(libraryfilename_.c_str(), "r");
//	if (file)
//		fclose(file);
//	else
//		libraryfilename_ = "lib" + newlibraryname + EXODUSLIBEXT;

	// var(libraryfilename_).logputl();

		// RTLD_NOW
	// All necessary relocations shall be performed when the object is first loaded.
	// This may waste some processing if relocations are performed for functions that are never
	// referenced. This behavior may be useful for applications that need to know as soon as an
	// object is loaded that all symbols referenced during execution are available.
	// RTLD_LAZY|RTLD_LOCAL may be a better option
	plibrary_ = (void*)dlopen(libraryfilename_.c_str(), RTLD_NOW);

#ifdef DLERROR
	///root/lib/libdict_invoices.so: undefined symbol: _ZN6exodus14ostempfilenameEv
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).errputl();
#endif

	// Try without path in case the library is system installed e.g. in /usr/local/lib
	if (plibrary_ == nullptr) {
		auto pos0 = libraryfilename_.rfind(OSSLASH_);
		if (pos0 != std::string::npos && pos0 != libraryfilename_.size() - 1) {
			auto purelibraryfilename = libraryfilename_.substr(pos0 + 1);
			//TRACE(purelibraryfilename)
			plibrary_ = (void*)dlopen(purelibraryfilename.c_str(), RTLD_NOW);
		}
	}

	if (plibrary_ == nullptr) {
		//#if TRACING >= 1
		//		std::cerr << "exocallable:openlib: <<< ko <<< " << libraryfilename_ << std::endl;
		//#endif
		// std::cerr<<libraryfilename_<<" cannot be found or cannot be opened"<<std::endl;
		var libraryfilename = libraryfilename_;
		if (libraryfilename.osfile())
			throw VarError(libraryfilename ^ " Cannot be linked/wrong version. Run with LD_DEBUG=libs for more info. Look for 'fatal'. Also run 'ldd "
			^ libraryfilename ^ "' to check its sublibs are available. Also run 'nm -C " ^ libraryfilename ^ "' to check its content.)"
			^ " To unmangle undefined symbols run 'c++filt _ZN6exodus3varC1Ev' for example to to see  exodus::var::var()");
		else
			throw VarError(libraryfilename ^ " does not exist or cannot be found.");
		return false;
	}

	//cache the dlopen result
	{
		//no need for this?
		LOCKDLCACHE

		mv_->dlopen_cache[newlibraryname] = plibrary_;
	}

	libraryname_ = newlibraryname;

#if TRACING >= 3
	std::cout << "exocallable:dlopen()        " << libraryname_ << ", " << plibrary_ << std::endl;
#endif

	return true;
}

bool CallableBase::openfunc(std::string newfunctionname) {

#if TRACING >= 4
	std::cout << "exocallable:openfunc: >>> in >>> " << libraryname_ << std::endl;
#endif

	// find the function and return true/false
	// pfunction_ = (EXODUSFUNCTYPE) dlsym(plibrary_, functionname_.c_str());

	// close any existing function
	closefunc();

#ifdef DLERROR
	dlerror();
#endif

	// var(libraryfilename_)^" "^var(newfunctionname).logputl();

	// pfunction_ = (void*) dlsym((library_t) plibrary_, newfunctionname.c_str());
	pfunction_ = (ExodusProgramBaseCreateDeleteFunction)dlsym((library_t)plibrary_,
															  newfunctionname.c_str());

#ifdef DLERROR
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).errputl();
#endif

	if (pfunction_ == nullptr) {
		//#if TRACING >= 1
		//		std::cout << "exocallable:openfunc: <<< ko <<< " << libraryname_ << " " << newfunctionname
		//			  << std::endl;
		//#endif

		// std::cerr<<functionname_<<" function cannot be found in
		// "<<libraryfilename_<<std::endl;
		throw VarError(var(newfunctionname).replace("exodusprogrambasecreatedelete_", "").quote() ^ " function cannot be found in lib " ^
					  var(libraryfilename_).quote());
		return false;
	}

	functionname_ = newfunctionname;

#if TRACING >= 3
	std::cout << "exocallable:dlsym           " << libraryname_ << ", " << functionname_ << " " << pfunction_ << std::endl;
#endif

	return true;
}

// call shared global function (not used atm)
var CallableBase::callsgf() {
	// dictionaries are libraries of subroutines (ie return void) that
	// have one argument "ExoEnv". They set their response in ANS.
	// they are global functions and receive mv environment reference as their one and only
	// argument.
	using ExodusDynamic = var (*)(ExoEnv & mv);

#if TRACING >= 3
	std::cout << "exocallable:CALLING SGF " << libraryname_ << ", " << functionname_ << std::endl;
#endif

	// call the function via its pointer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
	return ((ExodusDynamic)pfunction_)(*mv_);
#pragma GCC diagnostic pop
	// return;
}

// call shared member function
var CallableBase::callsmf() {
	// define a function type (pExodusProgramBaseMemberFunction)
	// that can call the shared library object member function
	// with the right arguments and returning a var
	//using pExodusProgramBaseMemberFunction = var (ExodusProgramBase::*)();

#if TRACING >= 3
	std::cout << "exocallable:callsmf()       " << libraryname_ << ", " << functionname_ << " " << pobject_ << "," << pmemberfunction_ << std::endl;
#endif

	// call the shared library object main function with the right args, returning a var
	return CALLMEMBERFUNCTION(*(pobject_),
							  ((pExodusProgramBaseMemberFunction)(pmemberfunction_)))();
}

void CallableBase::closelib() {

	//does not actually close any library but does close the function object
	//since libraries are cached in mv_->dlopen_cache

	//TODO close all libraries in exoenv destructor
	//currently the dl cache is never cleared

	// delete any shared object first!
	closefunc();

	// close any existing connection to the (cached) library
	if (plibrary_ != nullptr) {
		//#if TRACING >= 2
		//		std::cout << "exocallable:CLOSED LIBRARY  " << libraryname_ << " " << plibrary_ << std::endl;
		//#endif
		//dlclose((library_t)plibrary_);

		// record the library share no longer exists
		plibrary_ = nullptr;

	} else {
		//#if TRACING >= 5
		//		std::cout << "exocallable:close not required for library : " << libraryname_ << std::endl;
		//#endif
	}
	// flag that this library is no longer connected
	libraryname_ = "";
}

// actually this deletes any shared object created by the shared global function
void CallableBase::closefunc() {
	// the *library* function must be called to delete the object that it created
	//(cant delete the object in the main process since it might have a different memory
	// allocator)
	if (pobject_ != nullptr) {
#if TRACING >= 2
		std::cout << "exocallable:CLOSED function " << libraryname_ << ", " << functionname_ << " " << pobject_ << "," << pmemberfunction_ << std::endl;
#endif
		// second call will delete
		pfunction_(pobject_, *mv_, pmemberfunction_);
		// record the object no longer exists
		pobject_ = nullptr;

	} else {
#if TRACING >= 5
		std::cout << "exocallable:close function not required for function in library : " << libraryname_ << std::endl;
#endif
	}
	// record this function (and object) no longer exists
	// functionname_="";
}

}  // namespace exodus
