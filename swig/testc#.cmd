@echo -----------------------------------------------------------------------
@echo chain of command:
@echo "test.exe -> exodus_library.dll -> exodus_wrapper.dll -> exodus.dll"
@echo test.exe - platform independent dot executable on all dot net platforms
@echo exodus_library_dll ... ditto
@echo exodus_wrapper.dll platform dependent dotnet<->native interface
@echo exodus.dll platform dependent standard exodus function library
@echo
@echo Assume
@echo 1. MODULE LIBRARY
@echo    you have installed exodus_library using
@echo    gacutil -i exodus_library.dll (BUT NOT SO EASY)
@echo    OR
@echo    *** IT IS IN THE CURRENT DIRECTORY !!!
@echo    http://stackoverflow.com/questions/629712/how-to-set-c-library-path-for-an-application
@echo    http://stackoverflow.com/questions/629459/c-cannot-find-library-during-runtime
@echo    http://msdn.microsoft.com/en-us/library/823z9h8w%28v=VS.100%29.aspx
@echo
@echo 2. INTERFACE LIBRARY
@echo    that exodus_wrapper.dll is in the same directory
@echo    as exodus_library.dll
@echo    ... or is available in the path
@echo
@echo 3. exodus.dll ... is available in the path
@echo -------------------------------------------

if exist test.exe del test.exe

csc test.cs /r:\csharp\exodus_library.dll

@echo get access to module: exodus_library
copy \csharp\exodus_library.dll .

@echo assume exodus_wrapper.dll and exodus.dll are somewhere on the PATH

.\test.exe