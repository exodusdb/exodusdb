/*
Copyright (c) 2009 Stephen John Bush

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

#define MV_NO_NARROW

//portable shared/dynamic library macros
//makes linux dlopen, dlsym, dlclose syntax work on windows
//http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=746&lngWId=3
#if defined(_WIN32) || defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)
# define WIN32_LEAN_AND_MEAN
# define _save_function_ function
//windows.h has function defined so temp undefine and restore afterwards
# undef function
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
# define EXODUSLIBPREFIX "~/lib/lib"+
//# define EXODUSLIBPREFIX "./lib"+
#endif

//needed for getenv
#include <stdlib.h>

#include <exodus/mv.h>
#include <exodus/mvfunctor.h>

namespace exodus {

//using namespace exodus;

/*
//default constructor
ExodusFunctorBase::ExodusFunctorBase()
	: plibrary_(NULL),
	pfunction_(NULL)
{}
*/

/*
ExodusFunctorBase::ExodusFunctorBase(const std::string libname,const std::string funcname)
	: libraryname_(libname),
	functionname_(funcname),
	plibrary_(NULL),
	pfunction_(NULL)
{}
*/

bool ExodusFunctorBase::init(const char* libraryname, const char* functionname, MvEnvironment& mv)
{
	libraryname_=libraryname;
	functionname_=functionname;
	mv_=&mv;
	checkload();

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

ExodusFunctorBase::ExodusFunctorBase()
:
//TODO optimise by only initialise one and detect usage on that only
plibrary_(NULL),
pfunction_(NULL),
pobject_(NULL),
pmemberfunction_(NULL)
{};

  ExodusFunctorBase::ExodusFunctorBase(
	const std::string libname,
	const std::string funcname)
	:
	libraryname_(libname),
	functionname_(funcname),
	//mv_(mv),
	plibrary_(NULL),
	pfunction_(NULL),
	pobject_(NULL),
	pmemberfunction_(NULL)
{}

ExodusFunctorBase::~ExodusFunctorBase()
{
	//the library must be called to delete the object that it created
	if (pobject_!=NULL)
		pfunction_(pobject_,*mv_,pmemberfunction_);

	//close the connection
	if (plibrary_!=NULL)
		dlclose((library_t) plibrary_);
}

bool ExodusFunctorBase::checkload()
{
	if (plibrary_!=0)
		true;

	//find the library or fail
	if (not openlib())
	{
		throw MVException(L"Unable to load " ^ var(libraryfilename_));
		return false;
	}

	//find the function or fail
	if (not openfunc())
	{
		throw MVException(L"Unable to find "
		^ var(functionname_)
		^ L" in "
		^ var(libraryfilename_));
		return false;
	}

	return true;
}

bool ExodusFunctorBase::openlib()
{
	//open the library or return 0
	//dlopen arg2 is ignored by macro on windows

#ifdef dlerror
	dlerror();
#endif

	//TODO optimise with std::string instead of var
	/*
	libraryfilename_=EXODUSLIBPREFIX libraryname_+EXODUSLIBEXT;
	var home;
	home.osgetenv(L"HOME");
	libraryfilename_=var(libraryfilename_).swap(L"~",home).tostring();
	*/
	//#include <stdlib.h>
	libraryfilename_=EXODUSLIBPREFIX libraryname_+EXODUSLIBEXT;
	if (libraryfilename_[0]=='~')
		#pragma warning (disable: 4996)
		//env string is copied into string so following getenv usage is safe
		libraryfilename_.replace(0,1, getenv("HOME"));

	//var(libraryfilename_).outputl();

	plibrary_=(void*) dlopen(libraryfilename_.c_str(),RTLD_NOW);

#ifdef dlerror
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).outputl();
#endif

	return plibrary_!=NULL;
}

bool ExodusFunctorBase::openfunc()
{
	//find the function and return true/false
	//pfunction_ = (EXODUSFUNCTYPE) dlsym(plibrary_, functionname_.c_str());

#ifdef dlerror
	dlerror();
#endif

	//var(libraryfilename_)^L" "^var(functionname_).outputl();

	//pfunction_ = (void*) dlsym((library_t) plibrary_, functionname_.c_str());
	pfunction_ = (ExodusProgramBaseCreateDeleteFunction)
		dlsym((library_t) plibrary_, functionname_.c_str());

#ifdef dlerror
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).outputl();
#endif
	return pfunction_!=NULL;

}

}//namespace exodus
