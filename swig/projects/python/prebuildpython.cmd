echo on

@echo -------------------------------------------------------------------------------------
@echo SWIG generating 1. platform independent csharp library code AND
@echo 2. platform INDEPENDENT wrapper c++ code (that builds into platform DEPENDENT wrapper dll)
@echo -------------------------------------------------------------------------------------

@echo Building into %EXO_PYTHON32_MODULEDIR%
if not exist "%EXO_PYTHON32_MODULEDIR%" mkdir "%EXO_PYTHON32_MODULEDIR%"

%EXO_SWIG32%\swig.exe -c++ -python -module %EXO_PYTHON_MODULENAME% -I..\..\..\exodus\libexodus -outcurrentdir -outdir "%EXO_PYTHON32_MODULEDIR%" %EXO_SWIG_OPTIONS% ..\..\exodus.i

@echo off
@echo patch to make python debug use non-python lib
rem sadly gives error: fatal error C1189: #error :  You have included some C++/C library header files with _DEBUG defined and some with _DEBUG not defined. This will not work correctly. Please have _DEBUG set or clear consistently.
rem cscript ..\replace.vbs "exodus_wrap.cxx" "#include <Python.h>" "#ifdef _DEBUG\\n#undef _DEBUG\\n#define SUSPEND_PYTHON_DEBUG\\n#endif\\n#include <Python.h>\\n#ifdef SUSPEND_PYTHON_DEBUG\\n#define _DEBUG\\n#endif\\n"
