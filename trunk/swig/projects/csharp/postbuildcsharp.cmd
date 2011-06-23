@echo on

@echo ---------------------------------------------------------------------------------------------
@echo Invoking CSHARP to produce cross platform portable .net language library "exodus_library.dll"
@echo "exodus_library.dll" calls platform specific "exodus_wrapper.dll/so" linked to/calls exodus.dll/so
@echo Note: the ".dll" can be loaded by ANY .net runtime! - eg mono on linux
@echo ----------------------------------------------------------------------------------------------

@echo SKIP THIS FOR NOW we are going to "sign" it with a "strong name"
@echo SKIP THIS FOR NOW copy ..\..\..\%EXO_CSHARP_MODULENAME%.snk %EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%

pushd "%EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%"
csc -target:library %EXO_CSHARP_MODULENAME%.cs *.cs
@echo SKIP KEYFILE FOR NOW csc -target:library %EXO_CSHARP_MODULENAME%.cs *.cs /keyfile:%EXO_CSHARP_MODULENAME%.snk

@echo SKIP THIS FOR NOW Install available to all users - in "GAC"
@echo SKIP THIS FOR NOW gacutil -i %EXO_CSHARP_MODULENAME%.dll

popd

@echo off