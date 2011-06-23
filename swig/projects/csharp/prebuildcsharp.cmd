@echo on

@echo -------------------------------------------------------------------------------------
@echo SWIG generating 1. platform independent csharp library code AND
@echo 2. platform INDEPENDENT wrapper c++ code (that builds into platform DEPENDENT wrapper dll)
@echo -------------------------------------------------------------------------------------

@echo Building into %EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%
if not exist "%EXO_CSHARP32_MODULEDIR%" mkdir "%EXO_CSHARP32_MODULEDIR%"
if not exist "%EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%" mkdir "%EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%"

%EXO_SWIG32%\swig.exe -c++ -csharp -module %EXO_CSHARP_MODULENAME% -outdir "%EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%" -dllimport %EXO_CSHARP_WRAPPER_NAME% -I..\..\..\exodus\exodus -outcurrentdir %EXO_SWIG_OPTIONS% ..\..\exodus.i

@echo patch to make exodus' ToString override the built-in one - until done properly in exodus.i
cscript ..\replace.vbs "%EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%\mvar.cs" "public string ToString" "public override string ToString"
