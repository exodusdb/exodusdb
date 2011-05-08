rem ----- BOOST32 -----

set BOOST32=C:\Program Files\Boost\boost_1_46_1

rem ----- BOOST64 -----

rem binary installers are available so we BUILD boost/64 libs here
set BOOST64=D:\boost_1_46_1

rem ----- POSTGRESQL32 -----

set POSTGRESQL32=C:\Program Files\PostgreSQL\9.0
rem but when postgresql/32 is installed on a win/64 it is installed here
rem set POSTGRESQL32=C:\Program Files (x86)\PostgreSQL\9.0

rem ----- POSTGRESQL64 -----

set POSTGRESQL64=C:\Program Files\PostgreSQL\9.0

rem ----- BUILD AT CONSOLE OR IN VISUAL STUDIO -----

rem do one of the following depending on your toolset

rem typical commands vc/vs2005 and win32
rem vcbuild /platform:win32 exodus_all2005.sln
rem vcexpress exodus_all2005.sln
rem devenv exodus_all2005.sln

rem typical commands vc/vs2010 and win64
rem vcbuild /platform:win64 exodus_all.sln
rem vcexpress exodus_all.sln
rem devenv exodus_all.sln

rem ----- BUILDING INSTALLER -----

rem "d:\program files\nsis\makensis.exe" exodus_all.nsi

rem ----- INSTALLING ----

rem exodus-x86-11.5.3.exe
rem exodus-x86-11.5.3.exe /S     (silent install *capital* S)
