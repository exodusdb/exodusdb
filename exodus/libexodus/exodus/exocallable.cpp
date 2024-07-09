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
// 2. open a pointer to a global function in that library (pfunc_)
// 3. call the global function from 2. to retrieve a new object and pointer
//   to its main function plibobject_ and pmemberfunc_)

// usage
// 4. call the main function on that object to execute the code

// NOTE that the code can also stop at step 2 to call global functions directly BUT
// although calling global functions gives them mv (via argument) they cant call
// external subroutines since external subroutines are implemented in exodus
// as member functions which require special coding to be called from global functions

constexpr int TRACING = 0;

#include <iostream>
#include <string>

//////////
// WINDOWS
//////////

// portable shared/dynamic library macros
// makes linux dlopen, dlsym, dlclose syntax work on windows
// http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=746&lngWId=3
#if defined(_WIN32) || defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)

	// windows.h has function defined so save function definition, undefine it for restoring
#	define _save_function_ function
#	undef function

#	define WIN32_LEAN_AND_MEAN
#	include "windows.h"

	// Restore function definition
#	define function _save_function_

	using library_t = HINSTANCE;
//#	define dlopen(arg1,arg2) LoadLibrary(arg1)
#	define dlopen(arg1, arg2) LoadLibraryA(arg1)
#	define dlsym(arg1, arg2) GetProcAddress(arg1, arg2)
	//Note: MS FreeLibrary returns non-zero for success and 0 for failure
#	define dlclose(arg1) FreeLibrary(arg1)
#	define EXODUSLIBPREFIX
#	define EXODUSLIBEXT ".dll"

/////////////////////////////////////////////
// LINUX
/////////////////////////////////////////////

#else
#	define DLERROR
#	include <dlfcn.h>
	using library_t = void*;
#	define EXODUSLIBPREFIX "~/lib/lib"
//# define EXODUSLIBPREFIX "./lib"+
# define EXODUSLIBEXT ".so"

#endif

// Needed for getenv
//#include <stdlib.h>

// Probably no need for LOCKDLCACHE since we have a separate cache per exoenv
#include <mutex>
static std::mutex global_mutex_lockdlcache;
//#define LOCKDLCACHE std::lock_guard<std::mutex> guard(global_mutex_lockdlcache);
#define LOCKDLCACHE std::lock_guard guard(global_mutex_lockdlcache);

#include <exodus/varimpl.h>
#include <exodus/exoenv.h>
#include <exodus/exocallable.h>
namespace exo {

// using namespace exo;

// Default constructor.  probably followed by .init(libname,funcname,mv)
Callable::Callable()
	// TODO optimise by only initialise one and detect usage on that only
	: mv_(nullptr), libname_(""), funcname_(""), plib_(nullptr), pfunc_(nullptr), plibobject_(nullptr), pmemberfunc_(nullptr) {
	plibobject_ = nullptr;
}

// Constructor to provide everything immediately
Callable::Callable(const std::string_view libname, const std::string_view funcname, ExoEnv& mv)
	: mv_(&mv), libname_(libname), funcname_(funcname), plib_(nullptr), pfunc_(nullptr), plibobject_(nullptr), pmemberfunc_(nullptr) {
	plibobject_ = nullptr;
}

// Constructor to provide library name and function immediately
Callable::Callable(const std::string_view libname, const std::string_view funcname)
	: mv_(nullptr), libname_(libname), funcname_(funcname), plib_(nullptr), pfunc_(nullptr), plibobject_(nullptr), pmemberfunc_(nullptr) {
	plibobject_ = nullptr;
}

// Constructor to provide environment immediately. probably followed by .init(libname,funcname)
Callable::Callable(ExoEnv& mv)
	: mv_(&mv), libname_(""), funcname_(""), plib_(nullptr), pfunc_(nullptr), plibobject_(nullptr), pmemberfunc_(nullptr) {
	plibobject_ = nullptr;
}

// Destructor
Callable::~Callable() {
	// will delete any shared object first
	closelib();
}

//// Designed to be called once only the first time an callable object is called
//bool Callable::attach_shared_object(const char* newlibname, const char* newfuncname, ExoEnv& mv) {
//
//	if (TRACING >= 4) {
//		std::cout << "exocallable:init(libname,funcname,mv)  " << libname_ << ", " << funcname_ << std::endl;
//	}
//
//	// Acquire the program environment that will be passed to all libobjects during creation
//	mv_ = &mv;
//
//	// Load the library and get access to the function that will create/delete a libobject
//	checkload(newlibname, newfuncname);
//
//	// Call a function in the shared library to create one of its "exodus program" objects
//	// Note We MUST call the same library function to delete the object
//	// so that the same memory management routine is called to create and delete it.
//	//
//	// pfunc_ either
//	// 1. returns a plibobject_ if plibobject_ is passed in nullptr (using mv as an init argument)
//	// 2. deletes a plibobject_ if not
//
//	// Create a libobject by calling the given function
//	pfunc_(plibobject_, *mv_, pmemberfunc_);
//	if (plibobject_ == nullptr || pmemberfunc_ == nullptr)
//		return false;
//
//	if (TRACING >= 3) {
//		std::cout << "exocallable:init(libname,funcname,mv) OK" << libname_ << ", " << funcname_ << " " << plibobject_ << "," << pmemberfunc_ << std::endl;
//	}
//
//	return true;
//}

// Designed to be called once only the first time an callable object is called
bool Callable::attach(const char* newlibname) {

	if (TRACING >= 4) {
		std::cout << "exocallable::attach(libname)  " << libname_ << std::endl;
	}

	// Acquire the program environment that will be passed to all libobjects during creation
	//mv_ = &mv;

	// Load the library and acquire pfunc_ that will create/delete a libobject
	checkload(newlibname, "exodusprogrambasecreatedelete_");

	// Call a function in the shared library to create one of its "exodus program" objects
	// Note We MUST call the same library function to delete the object
	// so that the same memory management routine is called to create and delete it.
	//
	// pfunc_ either
	// 1. returns a plibobject_ if plibobject_ is passed in nullptr (using mv as an init argument)
	// 2. deletes a plibobject_ if not

	// Create a libobject by calling the given function
	pfunc_(plibobject_, *mv_, pmemberfunc_);
	if (plibobject_ == nullptr || pmemberfunc_ == nullptr)
		return false;

	if (TRACING >= 3) {
		std::cout << "exocallable:attach(libname) OK" << libname_ << " " << plibobject_ << ", createdelete " << pmemberfunc_ << std::endl;
	}

	return true;
}

//// Called from every library .h file if _pmemberfunction is nullptr
//// atm designed to be called once only the first time an external function is called
//// assuming library and function names and mv are already set
//bool Callable::init() {
//	if (TRACING >= 4) {
//		std::cout << "exocallable:init()          " << libname_ << ", " << funcname_ << std::endl;
//	}
//	checkload(libname_, funcname_);
//
//	// Call a function in the library to create one of its "exodus program" objects
//	// nb we MUST call the same library to delete it
//	// so that the same memory management routine is called to create and delete it.
//	// pfunc_ return a plibobject_ if plibobject_ is passed in nullptr (using mv as an init argument)
//	// or deletes a plibobject_ if not
//
//	// generate an error here to debug
//	//	plibobject_->main();
//
//	pfunc_(plibobject_, *mv_, pmemberfunc_);
//	if (plibobject_ == nullptr || pmemberfunc_ == nullptr) {
//	if (TRACING >= 1) {
//		std::cout << "exocallable:init() NOK " << libname_ << ", " << funcname_ << std::endl;
//	}
//		return false;
//	}
//	if (TRACING >= 3) {
//		std::cout << "exocallable:init()          " << libname_ << ", " << funcname_ << " " << plibobject_ << "," << pmemberfunc_ << std::endl;
//	}
//	return true;
//}

// initsmf is called by dict/perform/execute ... can be called repeatably since buffers lib and func
// perform/execute call with forcenew=true to ensure global variables are unassigned initially
//
// "forcenew" is used in PERFORM/EXECUTE to ensure that the ExodusProgram globals
// are initialised i.e. performing the same command twice
// starts from scratch each time.
//
// This function is not called by standard "callable" external lib function calls based on #include files
// They use Callable::init() and then directly call the member function.
// They do *NOT* clear ExodusProgram globals so can reuse state and behave like a lambda function or co-routine.
//
bool Callable::initsmf(ExoEnv& mv, const char* newlibname, const char* newfuncname, const bool forcenew) {

	if (TRACING >= 4) {
		std::cout << "exocallable:initsmf: === in === " << newlibname << std::endl;
	}

	// Get our environment in place first
	// because the library/function cache used by openlib is within it
	mv_ = &mv;

	// Open the library or fail
	if (newlibname != libname_ && !openlib(newlibname)) {
		return false;
	}

	// Make sure we have the right program object creation/deletion function
	if (forcenew || newfuncname != funcname_) {

		//////////////////////////////////////////////////////
		// Get access to the object creation/deletion function
		//////////////////////////////////////////////////////
		if (!openfunc(newfuncname)) {
			return false;
		}

		// Call a function in the library to create one of its "exodus program" objects
		// nb we MUST call the same library to delete it
		// so that the same memory management routine is called to create and delete it.
		// pfunc_ return a plibobject_ if plibobject_ is passed in nullptr (using mv as an init
		// argument)
		// or deletes a plibobject_ if not

		// generate an error here to debug
		//	plibobject_->main();

		////////////////////////////////////////////////////////////////////////////
		// Create a ExodusProgram object and get pointer to its main member function
		////////////////////////////////////////////////////////////////////////////
		// 1. plibobject_ is returned and is an ExodusProgram object
		// 2. Pass in the current mv_ ExoEnv environment object
		// 3. pmemberfunc_ is returned and is the function main of the ExodusProgram
		pfunc_(plibobject_, *mv_, pmemberfunc_);
		if (plibobject_ == nullptr || pmemberfunc_ == nullptr) {
			if (TRACING >= 1) {
				std::cout << "exocallable:initsmf: ko: no plibobject_" << libname_ << " "
					  << funcname_ << std::endl;
			}
			return false;
		}
	}

	if (TRACING >= 3) {
		std::cout << "exocallable:initsmf()       " << libname_ << ", " << funcname_ << " " << plibobject_ << "," << pmemberfunc_ << std::endl;
	}

	return true;
}

//// this version just gets a pointer to an external shared global function
//// not used atm?
//bool Callable::initsgf(const char* newlibname, const char* newfuncname) {
//	if (newlibname != libname_ and !openlib(newlibname)) {
//		return false;
//	}
//
//	// make sure we have the right program object creation/deletion function
//	if (newfuncname != funcname_ and !openfunc(newfuncname)) {
//		return false;
//	}
//
//	return true;
//}

// assign (case sensitive)
// pickos xxx="funcname"; call @xxx
void Callable::operator=(const char* newlibname) {
	if (newlibname != libname_) {
		closelib();
		// Attach immediately to prevent init("xxxxx") being called
		// and ease debugging at point of assignment in case of error
		if (!*newlibname)
			// After setting libname to "', a call will result in the init("xxxxxx") in the .h file being called
			// i.e. the original library name will be called.
			// e.g. the systemsubs.cpp stub provided for general calls
			libname_ = newlibname;
		else
			this->attach(newlibname);
	}
	return;
}

bool Callable::checkload(const std::string newlibname, const std::string newfuncname) {

	if (TRACING >= 4) {
		std::cout << "exocallable:checkload: in>" << newlibname << " " << newfuncname
			  << std::endl;
	}

	// find the library or fail
	if (not openlib(newlibname)) {

		if (TRACING >= 1) {
			std::cout << "exocallable:checkload: ko:" << newlibname << std::endl;
		}
		UNLIKELY
		throw VarError("Unable to load " ^ var(libfilepath_));
		//std::unreachable();
		//return false;
	}

	// find the function or fail
	if (not openfunc(newfuncname)) {

		if (TRACING >= 1) {
			std::cout << "exocallable:checkload: ko:" << libname_ << " " << newfuncname
					  << std::endl;
		}
		UNLIKELY
		throw VarError("Unable to find function " ^ var(newfuncname) ^ " in " ^
					  var(libfilepath_));
		//std::unreachable();
		//return false;
	}

	if (TRACING >= 4) {
		std::cout << "exocallable:checkload: ok<" << libname_ << std::endl;
	}

	return true;
}

std::string Callable::libfilepath(const std::string_view libname) const {

	//look for lib file in ~/lib/libXXXXXX.so
	std::string libfilepath = EXODUSLIBPREFIX;
	libfilepath += libname;
	libfilepath += EXODUSLIBEXT;
	if (libfilepath[0] == '~') {
		// env string is copied into string so following getenv usage is safe
		var exo_HOME;
		if (not exo_HOME.osgetenv("EXO_HOME") and not exo_HOME.osgetenv("HOME"))
			exo_HOME = "";
		//std::string replace
		libfilepath.replace(0, 1, exo_HOME.toString());
	}

	return libfilepath;
}

bool Callable::openlib(const std::string newlibname) {

	//uses a cache in mv_->dlopen_cache

	if (TRACING >= 4) {
		std::cout << "exocallable:openlib: >>> in >>> " << newlibname << std::endl;
	}

	// open the library or return 0
	// dlopen arg2 is ignored by macro on windows

	closelib();

#ifdef DLERROR
	dlerror();
#endif

	{
		// No need for this if dlcache is per mv and is therefore effectively threadlocal?
		LOCKDLCACHE

		// Look for library in cache
		auto cacheentry = mv_->dlopen_cache.find(newlibname);
		if (cacheentry != mv_->dlopen_cache.end()) {
			//std::cout << "using dlopen cache for " << newlibraryname << std::endl;
			plib_ = mv_->dlopen_cache.at(newlibname);
			libname_ = newlibname;
			return true;
		}
	}

	libfilepath_ = libfilepath(newlibname);

	if (TRACING >= 4) {
		std::cout << "exocallable:openlib: libfilepath_ now " << libfilepath_ << std::endl;
	}

	// var(libfilepath_).logputl();

	// Using "RTLD_NOW"
	// Alternatives are RTLD_LAZY|RTLD_LOCAL
	// All necessary relocations shall be performed when the object is first loaded.
	// This may waste some processing if relocations are performed for functions that are never
	// referenced. This behavior may be useful for applications that need to know as soon as an
	// object is loaded that all symbols referenced during execution are available.
	plib_ = static_cast<void*>(dlopen(libfilepath_.c_str(), RTLD_NOW));

	// Try without path in case the library is system installed e.g. in /usr/local/lib
	std::size_t fnpos0;
	std::string purelibfilename;
	if (plib_ == nullptr) {

		fnpos0 = libfilepath_.rfind(OSSLASH_);
		if (fnpos0 != std::string::npos && fnpos0 != libfilepath_.size() - 1) {

			// In two places
			purelibfilename = libfilepath_.substr(fnpos0 + 1);

			//TRACE(purelibraryfilename)
			plib_ = static_cast<void*>(dlopen(purelibfilename.c_str(), RTLD_NOW));

//			// Try on the same path as the executable
//			if (plib_ == nullptr) {
//			}
		}
	}

#ifdef DLERROR
	///root/lib/libdict_invoices.so: undefined symbol: _ZN6exodus14ostempfilenameEv
	// /root/lib/libhtmllib2.so: cannot open shared object file: No such file or directory
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).errputl();
#endif

	if (plib_ == nullptr) {
		var libfilepath = libfilepath_;
		if (libfilepath.osfile()) {
			throw VarError(libfilepath ^ " Cannot be linked/wrong version. Run with LD_DEBUG=libs for more info. Look for 'fatal'. Also run 'ldd "
			^ libfilepath ^ "' to check its sublibs are available. Also run 'nm -C " ^ libfilepath ^ "' to check its content.)"
			^ " To unmangle undefined symbols run 'c++filt _ZN6exodus3varC1Ev' for example to to see  exo::var::var()");
		} else {
			throw VarError(libfilepath ^ " does not exist or cannot be found, or " ^ purelibfilename ^ " cannot be linked/wrong version?");
		}
		//std::unreachable();
		//return false;
	}

	// Cache the dlopen result
	{
		//no need for this?
		LOCKDLCACHE

		mv_->dlopen_cache[newlibname] = plib_;
	}

	// Record the library name of the cached lib
	libname_ = newlibname;

	if (TRACING >= 3) {
		std::cout << "exocallable:dlopen()        " << libname_ << ", " << plib_ << std::endl;
	}

	return true;
}

// This currently mostly called to get access to the exodusprogrambasecreatedelete_ function
// but could also be called to directly access any free functions in the library
bool Callable::openfunc(const std::string newfuncname) {

	if (TRACING >= 4) {
		std::cout << "exocallable:openfunc: >>> in >>> " << libname_ << std::endl;
	}

	// Close any existing function
	delete_shared_object();

#ifdef DLERROR
	dlerror();
#endif

	// This is the only call to dlsym
	/////////////////////////////////
	pfunc_ = reinterpret_cast<ExodusProgramBaseCreateDeleteFunction>(
		dlsym(static_cast<library_t>(plib_), newfuncname.c_str())
	);

	// Log any error message
#ifdef DLERROR
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).errputl();
#endif

	// Throw if symbol does not exist
	if (pfunc_ == nullptr) {
		UNLIKELY
		throw VarError(var(newfuncname).replace("exodusprogrambasecreatedelete_", "").quote() ^ " function cannot be found in lib " ^
					  var(libfilepath_).quote());
	}

	// Record the function name
	funcname_ = newfuncname;

	if (TRACING >= 3) {
		std::cout << "exocallable:dlsym           " << libname_ << ", " << funcname_ << " " << pfunc_ << std::endl;
	}

	return true;
}

//// call shared global function (not used atm)
//var Callable::callsgf() {
//	// dictionaries are libraries of subroutines (ie return void) that
//	// have one argument "ExoEnv". They set their response in ANS.
//	// they are global functions and receive mv environment reference as their one and only
//	// argument.
//	using ExodusDynamic = var (*)(ExoEnv & mv);
//
//		if (TRACING >= 3) {
//			std::cout << "exocallable:CALLING SGF " << libname_ << ", " << funcname_ << std::endl;
//		}
//
//	// call the function via its pointer
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wcast-function-type"
//	return ((ExodusDynamic)pfunc_)(*mv_);
//#pragma GCC diagnostic pop
//	// return;
//}

// Call shared member function
// Call the libobject's main member function with no args, returning a var
// Define a function type (pExodusProgramBaseMemberFunction)
// that can call the shared library object member function
// with the right arguments and returning a var
//using pExodusProgramBaseMemberFunction = var (ExodusProgramBase::*)();
var Callable::callsmf() {

	if (TRACING >= 3) {
		std::cout << "exocallable:callsmf()       " << libname_ << ", " << funcname_ << " " << plibobject_ << "," << pmemberfunc_ << std::endl;
	}

	return CALLMEMBERFUNCTION(*(plibobject_),
							  //((pExodusProgramBaseMemberFunction)(pmemberfunc_)))();
							  ( static_cast<pExodusProgramBaseMemberFunction>(pmemberfunc_) ))();
}

// Does not actually close any library but does delete the libprogram object
// Libraries are cached in mv_->dlopen_cache
// Currently the dl cache is never cleared
//TODO Close all libraries in exoenv destructor
void Callable::closelib() {

	// Delete any shared object first!
	delete_shared_object();

	// Close any existing connection to the (cached) library
	if (plib_ != nullptr) {
		//if (TRACING >= 2) {
		//		std::cout << "exocallable:CLOSED LIBRARY  " << libname_ << " " << plib_ << std::endl;
		//}
		//dlclose((lib_t)plib_);

		// record the library share no longer exists
		plib_ = nullptr;

	} else {
		//	if (TRACING >= 5) {
		//		std::cout << "exocallable:close not required for library : " << libname_ << std::endl;
		//	}
	}

	// Flag that this callable is no longer associated with any library
	libname_ = "";
}

// Forcibly delete ExodusProgram object
// Actually this deletes a shared object created by the shared global function in exodus libraries
// It does not close the dlsym link to the creator/deleter function.
// The *library* creator/deleter function must be called to delete the object that it created
// Cannot delete the object in the main process since it might have a different memory allocator)
void Callable::delete_shared_object() {

	// Skip if there is no plibobject_
	if (plibobject_ == nullptr) {
		if (TRACING >= 5) {
			std::cout << "exocallable::delete_shared_object: Not required for function in library : " << libname_ << std::endl;
		}
		return;
	}

	if (TRACING >= 2) {
		std::cout << "exocallable::delete_shared_object: " << libname_ << ", " << funcname_ << " " << plibobject_ << "," << pmemberfunc_ << std::endl;
	}

	// First call will have created the object on the heap
	// This second call will delete the object from the heap
	pfunc_(plibobject_, *mv_, pmemberfunc_);

	// Record that the object no longer exists on the heap
	plibobject_ = nullptr;

	// Dont lose the function name in case we need it again
	// Record this function (and object) no longer exists
	// funcname_="";
}

}  // namespace exo
