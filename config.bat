rem run this from within a VC/VS command prompt
rem COPY AND MODIFY THIS FILE TO SUIT YOUR OWN ENVIRONMENT


rem ----- BOOST32 -----

rem uncomment if you have binaries here but best to build from scratch to avoid 0xc0150002 error
rem set BOOST32=C:\Program Files\Boost\boost_1_46_1

rem assuming we have built libs here (in stage32)
set BOOST32=F:\boost_1_46_1

rem ----- BOOST64 -----

rem binary installers are not available so we have built boost x64 libs (in stage64)
set BOOST64=F:\boost_1_46_1


rem ----- POSTGRESQL32 -----

rem on Win32, postgres is installed here
set POSTGRESQL32=C:\Program Files\PostgreSQL\9.0

rem but on win/64, postgres is installed here
rem set POSTGRESQL32=C:\Program Files (x86)\PostgreSQL\9.0

rem ----- POSTGRESQL64 -----

set POSTGRESQL64=C:\Program Files\PostgreSQL\9.0





rem ----- BUILD AT CONSOLE OR IN VISUAL STUDIO -----

rem do one of the following depending on your toolset

rem typical commands vc/vs2005 and win32
rem vcbuild /platform:win32 exodus_all2005.sln
rem vcexpress exodus_all2005.sln

rem starting up VS2005 pro
rem call "%VS80COMNTOOLS%vsvars32.bat"
rem devenv exodus_all2005.sln

rem typical commands vc/vs2010 and win64
rem vcbuild /platform:win64 exodus_all.sln
rem vcexpress exodus_all.sln

rem 
rem devenv exodus_all.sln

rem ----- BUILDING INSTALLER -----

rem "d:\program files\nsis\makensis.exe" exodus_all.nsi

rem ----- INSTALLING ----

rem exodus-x86-11.5.3.exe
rem exodus-x86-11.5.3.exe /S     (silent install *capital* S)
