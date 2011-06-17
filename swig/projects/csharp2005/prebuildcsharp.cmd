@echo on

@echo Installing into %EXO_JAVA32_MODULEDIR%

@echo Invoking SWIG
if not exist "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%" mkdir "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%"
%SWIG32%\swig.exe -c++ -java -module %EXO_JAVA_MODULENAME% -outdir "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%" -package %EXO_JAVA_PACKAGE_NAME% -I..\..\..\exodus\exodus -outcurrentdir %EXO_SWIG_OPTIONS% ..\..\exodus.i

@echo off