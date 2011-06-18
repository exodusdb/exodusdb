@echo on

@echo ---------------------------------------------------------------------------------------------
@echo Invoking CSHARP to produce cross platform portable .net language library "exodus_library.dll"
@echo "exodus_library.dll" calls platform specific "exodus_wrapper.dll/so" linked to/calls exodus.dll/so
@echo Note: the ".dll" can be loaded by ANY .net runtime! - eg mono on linux
@echo ----------------------------------------------------------------------------------------------

@echo we are going to "sign" it with a "strong name"
copy %EXO_CSHARP_MODULENAME%.snk %EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%

pushd "%EXO_CSHARP32_MODULEDIR%\%EXO_CSHARP_PACKAGE_SUBDIR%"
csc -target:library %EXO_CSHARP_MODULENAME%.cs *.cs /keyfile:%EXO_CSHARP_MODULENAME%.snk

@echo Install available to all users - in "GAC"
gacutil -i %EXO_CSHARP_MODULENAME%.dll

popd

@echo off