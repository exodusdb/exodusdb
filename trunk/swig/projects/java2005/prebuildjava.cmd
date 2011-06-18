@echo on

@echo -------------------------------------------------------------------------------------
@echo SWIG generating 1. platform independent java library code AND
@echo 2. platform INDEPENDENT wrapper c++ code (that builds into platform DEPENDENT wrapper dll)
@echo -------------------------------------------------------------------------------------

@echo Building into %EXO_JAVA32_MODULEDIR%
if not exist "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%" mkdir "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%"
%SWIG32%\swig.exe -c++ -java -module %EXO_JAVA_MODULENAME% -outdir "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%" -package %EXO_JAVA_PACKAGE_NAME% -I..\..\..\exodus\exodus -outcurrentdir %EXO_SWIG_OPTIONS% ..\..\exodus.i

@echo off