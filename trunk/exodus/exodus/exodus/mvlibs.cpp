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


//ensure the global libs are defined
#define EXO_MVLIBS_CPP

#define MV_NO_NARROW

#if defined _MSC_VER || defined __CYGWIN__ || defined __MINGW32__
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#else
#include <dlfcn.h>
#endif

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <exodus/mv.h>
#include <exodus/mvenvironment.h>

namespace exodus
{

// var DLL function signature (takes an MvEnvironment and returns an var)
typedef var (*DynamicFunction)(MvEnvironment&);

//MVLIB

//intended to allow "if (MvLib)" syntax but does it really work
//since how can the lib be used if it doesnt exist??
MvLib::operator bool() const {return this!=0;}

//virtual destructor essential for virtual functions otherwise wrong destructors called causing chaos
MvLib::~MvLib(){}

//MVLIBS

void MvLibs::reset()
{
	mvlibs_=new MvLibMap;
}

void MvLibs::set(const var& libraryname, MvLib* mvlib)
{

	//TODO: this doesnt work        if (thread_mvlibs==0) throw MVException(L"MVCommon::set MapOfMVCommon thread_mvcommons is void");
	int ii=0;
	ii=getenvironmentn();
	if (mvlibs_==NULL) reset();
	(*mvlibs_)[libraryname.towstring()]=mvlib;
}

MvLib* MvLibs::get(const var& libraryname)
{
	int ii=0;
	ii=getenvironmentn();
	if (mvlibs_==NULL) reset();
	return (*mvlibs_)[libraryname.towstring()];
}

//portable shared/dynamic library macros makes linux dlopen, dlsym, dlclose syntax ok for windows
//http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=746&lngWId=3
#if defined(_MSC_VER) || defined(__MINGW32__)
typedef HINSTANCE library_t;
typedef DynamicFunction function_t;
#define dlopen(libraryname,dummy) LoadLibrary((LPCWSTR)libraryname)
#define dlsym(plibrary,functionname)  GetProcAddress(plibrary,functionname)
//note FreeLibrary returns non-zero for success and 0 for failure
#define dlclose(plibrary) FreeLibrary(plibrary)
#else
typedef void* library_t;
typedef void* function_t;
#endif

bool var::load(const var& libraryname) const
{

	//just checks if can be loaded and then closes
	//TODO (sb) save library handles
	//TODO (sb) dedup with var::call and var::load
	//macros now make win32 and posix calls identical
	#if 0
		//http://msdn.microsoft.com/en-us/library/ms684175(VS.85).aspx
		// Load DLL file
		//HINSTANCE hinstLib = LoadLibrary("markets.dll");
		HINSTANCE plibrary = LoadLibrary(libraryname.tostring().c_str());
		if (plibrary == NULL) {
			//throw L"Unable to load " ^ libraryname ^ L".dll";
			return false;
		}
		FreeLibrary(plibrary);
	#else
		library_t plibrary;
		if ((plibrary=(library_t) dlopen(libraryname.tostring().c_str(), RTLD_NOW))==0)
		{
			{
				//throw L"Unable to load " ^ libraryname ^ L".dll";
				return false;
			}
		}
		dlclose(plibrary);
	#endif

	//save the libraryname
	//cannot use *this because load is marked const
	//*this="xx";
	//identical in copy ctor and load and call
	var_mvtyp=libraryname.var_mvtyp;
	var_mvstr=libraryname.var_mvstr;
	var_mvint=libraryname.var_mvint;
	var_mvdbl=libraryname.var_mvdbl;

	return true;
}

var var::call(const var& libraryname, const var& functionname) const
{
	return call(libraryname.towstring().c_str(),functionname.tostring().c_str());
}

var var::call(const var& functionname) const
{
	createString();
	var libraryname=field(L"*",1);
	return call(libraryname.towstring().c_str(),functionname.tostring().c_str());
}

var var::call() const
{
	createString();
	var libraryname=field(L"*",1);
	var functionname=field(L"*",2);
	return call(libraryname.towstring().c_str(),functionname.tostring().c_str());
}

inline MvEnvironment& getmvenvironment()
{
	return *global_environments[getenvironmentn()];
}

var var::call(const wchar_t* libraryname, const char* functionname) const
{

	//very detailed dynamic linking in windows and linux 2006
	//http://www.symantec.com/connect/articles/dynamic-linking-linux-and-windows-part-one

	if (wcslen(libraryname)==0)
	{
		throw L"Unable to determine library name for function " ^ var(functionname);
		return 0;
	}

	//TODO dedup with var::call and var::load
	#if 0
		// Load DLL file
		//HINSTANCE hinstLib = LoadLibrary("markets.dll");
		HINSTANCE plibrary = LoadLibrary(var(libraryname).tostring().c_str());
		if (plibrary == NULL) {
			//throw L"Unable to load " ^ var(libraryname) ^ L".dll";
			return false;
		}
	#else
		library_t plibrary;
		if ((plibrary=(library_t)dlopen(var(libraryname).tostring().c_str(), RTLD_NOW))==0)
		{
			//throw L"Unable to load " ^ var(libraryname) ^ L".dll";
			return false;
		}
	#endif

	//save the libraryname
	//cannot use *this because load is marked const
	//*this="xx";
	//identical in copy ctor and load and call
	var var1=libraryname;
	var_mvtyp=var1.var_mvtyp;
	var_mvstr=var1.var_mvstr;
	var_mvint=var1.var_mvint;
	var_mvdbl=var1.var_mvdbl;

	// Get function pointer
	#if 0
		DynamicFunction dictfunction = (DynamicFunction)GetProcAddress(plibrary, functionname);
		if (dictfunction == NULL) {
			dictfunction = (DynamicFunction)GetProcAddress(plibrary, var(functionname).lcase().tostring().c_str());
			if (dictfunction == NULL) {
				//FreeLibrary(plibrary);
				throw L"Unable to find " ^ var(functionname) ^ L" in " ^ var(libraryname) ^ L".dll";
				return L"";
			}
		}
	#else
		DynamicFunction dictfunction = (DynamicFunction) dlsym(plibrary, functionname);
		if (dictfunction == NULL) {
			dictfunction = (DynamicFunction) dlsym(plibrary,var(functionname).lcase().tostring().c_str());
			if (dictfunction == NULL) {
				//dlclose(plibrary);
				throw L"Unable to find " ^ var(functionname) ^ L" in " ^ var(libraryname) ^ L".dll";
				return L"";
			}
		}
	#endif

	//save the functionname
	//cannot use *this because load is marked const
	//*this="xx";
	//identical in copy ctor and load and call
	var1.fieldstorer(L"*",2,1,var(functionname));
	var_mvtyp=var1.var_mvtyp;

	// Call function.u
	return dictfunction(getmvenvironment());

//TODO dlclose(plibrary);
}

} //of namespace exodus
