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

//mvfunctor provides simple function-like syntax xyz() to call main functions in objects
//stored in external shared objects libraries
//
//strategy is to open a pointer to the main function in an object in the library
//
// initialisation
//1. open a pointer to a library (plibrary_)
//2. open a pointer to a global function in that library (pfunction_)
//3. call the global function from 2. to retrieve a new object and pointer
//   to its main function pobject_ and pmemberfunction_)

//usage
//4. call the main function on that object to execute the code

//NOTE that the code can also stop at step 2 to call global functions directly BUT
// although calling global functions gives them mv (via argument) they cant call
// external subroutines since external subroutines are implemented in exodus
// as member functions which require special coding to be called from global functions

#include <iostream>

#define MV_NO_NARROW

//WINDOWS

//portable shared/dynamic library macros
//makes linux dlopen, dlsym, dlclose syntax work on windows
//http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=746&lngWId=3
#if defined(_WIN32) || defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)

//windows.h has function defined so temp undefine and restore afterwards
# define _save_function_ function
# undef function

# define WIN32_LEAN_AND_MEAN
# include "windows.h"

# define function _save_function_

typedef HINSTANCE library_t;
//# define dlopen(arg1,arg2) LoadLibrary(arg1)
# define dlopen(arg1,arg2) LoadLibraryA(arg1)
# define dlsym(arg1,arg2)  GetProcAddress(arg1,arg2)
//note: MS FreeLibrary returns non-zero for success and 0 for failure
# define dlclose(arg1) FreeLibrary(arg1)
# define EXODUSLIBPREFIX
#else
# include <dlfcn.h>
  typedef void* library_t;
# define EXODUSLIBPREFIX "~/lib/lib"
//# define EXODUSLIBPREFIX "./lib"+
#endif


//needed for getenv
#include <stdlib.h>

#include <exodus/mvfunctor.h>

namespace exodus {

//using namespace exodus;

//default constructor.  probably followed by .init(libname,funcname,mv)
ExodusFunctorBase::ExodusFunctorBase()
//TODO optimise by only initialise one and detect usage on that only
: mv_(NULL)
, libraryname_("")
, functionname_("")
, plibrary_(NULL)
, pfunction_(NULL)
, pobject_(NULL)
, pmemberfunction_(NULL)
{pobject_=0;};

//constructor to provide library name and function immediately
ExodusFunctorBase::ExodusFunctorBase(const std::string libname, const std::string funcname)
: mv_(NULL)
, libraryname_(libname)
, functionname_(funcname)
, plibrary_(NULL)
, pfunction_(NULL)
, pobject_(NULL)
, pmemberfunction_(NULL)
{pobject_=0;}

//constructor to provide environment immediately. probably followed by .init(libname,funcname)
ExodusFunctorBase::ExodusFunctorBase(MvEnvironment& mv)
: mv_(&mv)
, libraryname_("")
, functionname_("")
, plibrary_(NULL)
, pfunction_(NULL)
, pobject_(NULL)
, pmemberfunction_(NULL)
{pobject_=0;}

ExodusFunctorBase::~ExodusFunctorBase()
{
	//will delete any shared object first
	closelib();
}

//atm designed to be called once only the first time an external function is called
bool ExodusFunctorBase::init(const char* newlibraryname, const char* newfunctionname, MvEnvironment& mv)
{
	mv_=&mv;
	checkload(newlibraryname, newfunctionname);

	//call a function in the library to create one of its "exodus program" objects
	//nb we MUST call the same library to delete it
	//so that the same memory management routine is called to create and delete it.
	//pfunction_ return a pobject_ if pobject_ is passed in NULL (using mv as an init argument)
	// or deletes a pobject_ if not

	//generate an error here to debug
//	pobject_->main();

	pfunction_(pobject_,*mv_,pmemberfunction_);
	//pobject_->main();
	//((*pobject_).*(pmemberfunction_))();
	//CALLMEMBERFUNCTION(*pobject_,pmemberfunctibon_)();
	if (pobject_==NULL||pmemberfunction_==NULL)
		return false;

	return true;
}

//used in dict/perform/execute ... can be called repeatably since buffers lib and func
bool ExodusFunctorBase::initsmf(const char* newlibraryname, const char* newfunctionname, const bool forcenew) 
{
#if TRACING >= 3
std::cout<<"mvfunctor:initsmf: in:"<<newlibraryname<<" "<<newfunctionname<<std::endl;
#endif
	if (newlibraryname!=libraryname_ && !openlib(newlibraryname)) {
		return false;
	}

	//make sure we have the right program object creation/deletion function
	//forcenew is used in perform/execute to ensure that all global variables
	//are initialised ie performing the same command twice
	//starts from scratch each time
	if (forcenew || newfunctionname!=functionname_)
	{
		if (!openfunc(newfunctionname)) {
			return false;
		}
		//call a function in the library to create one of its "exodus program" objects
		//nb we MUST call the same library to delete it
		//so that the same memory management routine is called to create and delete it.
		//pfunction_ return a pobject_ if pobject_ is passed in NULL (using mv as an init argument)
		// or deletes a pobject_ if not

		//generate an error here to debug
	//	pobject_->main();

		//create a shared object and get pointer to its main member function
		pfunction_(pobject_,*mv_,pmemberfunction_);
		//pobject_->main();
		//((*pobject_).*(pmemberfunction_))();
		//CALLMEMBERFUNCTION(*pobject_,pmemberfunctibon_)();
		if (pobject_==NULL||pmemberfunction_==NULL) {
			return false;
#if TRACING >= 3
std::cout<<"mvfunctor:initsmf: ko: no pobject_"<<libraryname_<<" "<<functionname_<<std::endl;
#endif
		}
	}

#if TRACING >= 3
std::cout<<"mvfunctor:initsmf: ok:"<<libraryname_<<" "<<functionname_<<std::endl;
#endif
	return true;
}

//this version just gets a pointer to an external shared global function
//not used atm?
bool ExodusFunctorBase::initsgf(const char* newlibraryname, const char* newfunctionname)
{
	if (newlibraryname!=libraryname_ and !openlib(newlibraryname)) {
		return false;
	}

	//make sure we have the right program object creation/deletion function
	if (newfunctionname!=functionname_ and !openfunc(newfunctionname)) {
		return false;
	}

	return true;
}

bool ExodusFunctorBase::checkload(std::string newlibraryname, std::string newfunctionname)
{
#if TRACING >= 3
std::cout<<"mvfunctor:checkload: in:"<< newlibraryname<<" "<< newfunctionname<<std::endl;
#endif
	//find the library or fail
	if (not openlib(newlibraryname))
	{
#if TRACING >= 3
std::cout<<"mvfunctor:checkload: ko:"<<newlibraryname<<std::endl;
#endif
		throw MVException(L"Unable to load " ^ var(libraryfilename_));
		return false;
	}

	//find the function or fail
	if (not openfunc(newfunctionname))
	{
#if TRACING >= 3
std::cout<<"mvfunctor:checkload: ko:"<<libraryname_<<" "<< newfunctionname<<std::endl;
#endif
		throw MVException(L"Unable to find "
		^ var(newfunctionname)
		^ L" in "
		^ var(libraryfilename_));
		return false;
	}

#if TRACING >= 3
std::cout<<"mvfunctor:checkload: ok:"<<libraryname_<<" "<<functionname_<<std::endl;
#endif
	return true;
}

bool ExodusFunctorBase::openlib(std::string newlibraryname)
{
#if TRACING >= 3
std::cout<<"mvfunctor:openlib: in:"<<newlibraryname<<std::endl;
#endif
	//open the library or return 0
	//dlopen arg2 is ignored by macro on windows

	//dont reopen if already opened
	if (libraryname_==newlibraryname)
		return true;

	closelib();

#ifdef dlerror
	dlerror();
#endif

	libraryfilename_=EXODUSLIBPREFIX+newlibraryname+EXODUSLIBEXT;
	if (libraryfilename_[0]=='~')
		#pragma warning (disable: 4996)
		//env string is copied into string so following getenv usage is safe
		libraryfilename_.replace(0,1, getenv("HOME"));
	FILE *file;
	if (file=fopen(libraryfilename_.c_str(),"r")) {
		fclose(file);
	} else {
		libraryfilename_="lib"+newlibraryname+EXODUSLIBEXT;
	}
	//var(libraryfilename_).outputl();

	//RTLD_NOW
	//All necessary relocations shall be performed when the object is first loaded.
	//This may waste some processing if relocations are performed for functions that are never referenced.
	//This behavior may be useful for applications that need to know as soon as an object is loaded that all symbols referenced during execution are available.
	//RTLD_LAZY|RTLD_LOCAL may be a better option
	plibrary_=(void*) dlopen(libraryfilename_.c_str(),RTLD_NOW);

#ifdef dlerror
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).outputl();
#endif

	if (plibrary_==NULL)
	{
#if TRACING >= 3
std::cout<<"mvfunctor:openlib: ko:"<< newlibraryname<<std::endl;
#endif
		std::cerr<<libraryfilename_<<" cannot be found or cannot be opened"<<std::endl;
		return false;
	}

	libraryname_=newlibraryname;
#if TRACING >= 3
std::cout<<"mvfunctor:openlib: ok:"<<libraryname_<<std::endl;
#endif
	return true;
}

bool ExodusFunctorBase::openfunc(std::string newfunctionname)
{
#if TRACING >= 3
std::cout<<"mvfunctor:openfunc: in:"<<libraryname_<<" "<< newfunctionname<<std::endl;
#endif
	//find the function and return true/false
	//pfunction_ = (EXODUSFUNCTYPE) dlsym(plibrary_, functionname_.c_str());

	//close any existing function
	closefunc();

#ifdef dlerror
	dlerror();
#endif

	//var(libraryfilename_)^L" "^var(newfunctionname).outputl();

	//pfunction_ = (void*) dlsym((library_t) plibrary_, newfunctionname.c_str());
	pfunction_ = (ExodusProgramBaseCreateDeleteFunction)
		dlsym((library_t) plibrary_, newfunctionname.c_str());

#ifdef dlerror
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).outputl();
#endif

	if (pfunction_==NULL)
	{
#if TRACING >= 3
std::cout<<"mvfunctor:openfunc: ko:"<<libraryname_<<" "<<newfunctionname<<std::endl;
#endif
		std::cerr<<functionname_<<" function cannot be found in "<<libraryfilename_<<std::endl;
		return false;
	}

	functionname_=newfunctionname;
#if TRACING >= 3
std::cout<<"mvfunctor:openfunc: ok:"<<libraryname_<<" "<< functionname_<<std::endl;
#endif
	return true;

}

//call shared global function (not used atm)
var ExodusFunctorBase::callsgf()
{
	//dictionaries are libraries of subroutines (ie return void) that 
	//have one argument "MvEnvironment". They set their response in ANS.
	//they are global functions and receive mv environment reference as their one and only argument.
	typedef var (*ExodusDynamic)(MvEnvironment& mv);

#if TRACING >= 3
std::cout<<"mvfunctor:callsgf: in:"<<libraryname_<<" "<< functionname_<<std::endl;
#endif
	//call the function via its pointer
	return ((ExodusDynamic) pfunction_)(*mv_);
	//return;
}

//call shared member function
var ExodusFunctorBase::callsmf()
{
	//define a function type (pExodusProgramBaseMemberFunction)
	//that can call the shared library object member function
	//with the right arguments and returning a var
	typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

#if TRACING >= 3
std::cout<<"mvfunctor:callsmf: in:" << libraryname_<< " " << functionname_<<std::endl;
#endif
	//call the shared library object main function with the right args, returning a var
	return CALLMEMBERFUNCTION(*(
			pobject_),
			((pExodusProgramBaseMemberFunction) (pmemberfunction_)))
					();

}

void ExodusFunctorBase::closelib()
{
	//delete any shared object first!
	closefunc();

	//close any existing connection to the library
	if (plibrary_!=NULL)
	{
		dlclose((library_t) plibrary_);
		//record the library share no longer exists
		plibrary_=NULL;
#if TRACING >= 3
std::cout<<"mvfunctor:closed libraryname_:" <<libraryname_<<std::endl;
#endif
	}
	//record this library is no longer connected
	libraryname_="";
}

//actually this deletes any shared object created by the shared global function
void ExodusFunctorBase::closefunc()
{
	//the *library* function must be called to delete the object that it created
	//(cant delete the object in the main process since it might have a different memory allocator)
	if (pobject_!=NULL)
	{
		//second call will delete
		pfunction_(pobject_,*mv_,pmemberfunction_);
		//record the object no longer exists
		pobject_=NULL;
#if TRACING >= 3
std::cout<< "mvfunctor:closed functionname:" << functionname_ << std::endl;
#endif
	}
	//record this function (and object) no longer exists
	functionname_="";
}

}//namespace exodus
