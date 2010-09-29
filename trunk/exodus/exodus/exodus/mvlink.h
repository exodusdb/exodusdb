
#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)
# define WIN32_LEAN_AND_MEAN
# define _save_function_ function
# undef function
# include "windows.h"
# define function _save_function_
# define EXODUSLIBEXT ".dll"
#else
# include <dlfcn.h>
# define EXODUSLIBEXT ".so"
#endif

// declare DLL/SO function type
typedef EXODUSFUNCRETURN (*EXODUSFUNCTYPE)(EXODUSFUNCARGS);

//portable shared/dynamic library macros makes linux dlopen, dlsym, dlclose syntax ok for windows
//http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=746&lngWId=3
#if defined(_MSC_VER) || defined(__MINGW32__)
  typedef HINSTANCE library_t;
  //typedef EXODUSFUNCTYPE function_t;
# define dlopen(arg1,arg2) LoadLibrary(arg1)
# define dlsym(arg1,arg2)  GetProcAddress(arg1,arg2)
//note MS FreeLibrary returns non-zero for success and 0 for failure
# define dlclose(arg1) FreeLibrary(arg1)
#else
  typedef void* library_t;
  //typedef void* function_t;
#endif

class EXODUSFUNCTORCLASSNAME
{

public:

 EXODUSFUNCTORCLASSNAME(const std::string libname,const std::string funcname)
             : _libraryname(libname),
               _functionname(funcname),
               _plibrary(NULL),
               _pfunction(NULL)
              {}

 EXODUSFUNCRETURN operator() (EXODUSFUNCARGS)
 {
	checkload();
#if EXODUSFUNCRETURNVOID == 1
	_pfunction(EXODUSFUNCARGS2);
	return;
#else
	return _pfunction(EXODUSFUNCARGS2);
#endif
 }

 virtual ~EXODUSFUNCTORCLASSNAME()
 {
        if (_plibrary!=0)
                dlclose(_plibrary);
 }

private:

 library_t _plibrary;

 EXODUSFUNCTYPE _pfunction;

 std::string _libraryname;
 std::string _functionname;

 void checkload()
 {
        if (_plibrary!=0)
		return;

        //find the library or fail
        if (not openlib())
                throw MVException(L"Unable to load " ^ var(_libraryname) ^ L".dll/so");

        //find the function or fail
        if (not openfunc())
                throw MVException(L"Unable to find " ^ var(_functionname) ^ L" in " ^ var(_libraryname) ^ 

L".dll/so");
 }

 bool openlib()
 {
        //open the library or return 0
//        println(_libraryname+EXODUSLIBEXT);
        _plibrary=(library_t)
		 dlopen((_libraryname+EXODUSLIBEXT).c_str(), RTLD_NOW);
        return _plibrary!=NULL;
 }

 bool openfunc()
 {
        //find the function and return true/false
        _pfunction = (EXODUSFUNCTYPE)
		dlsym(_plibrary, _functionname.c_str());
        return _pfunction!=NULL;
 }

};

//define a functor object so the function can be called like xyz(a,b,c)
EXODUSFUNCTORCLASSNAME EXODUSFUNCNAME(EXODUSLIBNAMEQQ,EXODUSFUNCNAMEQQ);

