@echo chain of command:
@echo "test.exe -> exodus_library.dll -> exodus_wrapper.dll -> exodus.dll"
@echo test.exe - platform independent dot executable on all dot net platforms
@echo exodus_library_dll ... ditto
@echo exodus_wrapper.dll platform dependent dotnet<->native interface
@echo exodus.dll platform dependent standard exodus function library
@echo
@echo Assume
@echo 1. you have installed exodus_library using
@echo    gacutil -i exodus_library.dll
@echo    or that it is in the current directory
@echo
@echo 2. that exodus_wrapper.dll is in the same directory
@echo    as exodus_library.dll
@echo    ... or is available in the path
@echo
@echo 3. exodus.dll ... is available in the path

del test.exe

csc test.cs /r:\csharp\exodus_library.dll

set path=\csharp\;%PATH%

.\test
