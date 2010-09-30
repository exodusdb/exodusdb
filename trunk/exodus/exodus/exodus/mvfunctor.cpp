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

#include <exodus/mv.h>
#include <exodus/mvfunctor.h>

namespace exodus {

ExodusFunctorBase::ExodusFunctorBase(const std::string libname,const std::string funcname)
	: _libraryname(libname),
	_functionname(funcname),
	_plibrary(NULL),
	_pfunction(NULL)
{}

ExodusFunctorBase::~ExodusFunctorBase()
{
	if (_plibrary!=0)
		dlclose((library_t) _plibrary);
}

void ExodusFunctorBase::checkload()
{
	if (_plibrary!=0)
		return;

	//find the library or fail
	if (not openlib())
		throw MVException(L"Unable to load " ^ var(_libraryfilename));

	//find the function or fail
	if (not openfunc())
		throw MVException(L"Unable to find "
		^ var(_functionname)
		^ L" in "
		^ var(_libraryfilename)
		^ L".dll/so");
}

bool ExodusFunctorBase::openlib()
{
	//open the library or return 0
	//dlopen arg2 is ignored by macro on windows

#ifdef dlerror
	dlerror();
#endif
	//TODO optimise with std::string instead of var
	_libraryfilename=EXODUSLIBPREFIX _libraryname+EXODUSLIBEXT;
	var home;
	home.osgetenv(L"HOME");
_libraryfilename=var(_libraryfilename).swap(L"~",home).tostring();

	_plibrary=(void*) dlopen(_libraryfilename.c_str(),RTLD_NOW);

#ifdef dlerror
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).outputln();
#endif
	return _plibrary!=NULL;
}

bool ExodusFunctorBase::openfunc()
{
	//find the function and return true/false
	//_pfunction = (EXODUSFUNCTYPE) dlsym(_plibrary, _functionname.c_str());

#ifdef dlerror
	dlerror();
#endif
	_pfunction = (void*) dlsym((library_t) _plibrary, _functionname.c_str());

#ifdef dlerror
	const char* dlsym_error = dlerror();
	if (dlsym_error)
		var(dlsym_error).outputln();
#endif
	return _pfunction!=NULL;
}

}//namespace exodus
