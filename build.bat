rem run this from within a VC/VS command prompt
rem COPY AND MODIFY THIS FILE TO SUIT YOUR OWN ENVIRONMENT
rem make a shortcut to the copy so you can quickly develop and build
rem with the right environment variables set


call config.bat

rem ----- BUILD AT CONSOLE OR IN VISUAL STUDIO -----

rem do one of the following depending on your toolset

rem typical commands vc/vs2005 and win32
rem vcbuild /platform:win32 exodus_all2005.sln
rem vcexpress exodus_all2005.sln

rem starting up VS2005 pro
call "%VS80COMNTOOLS%vsvars32.bat"
devenv exodus_all2005.sln
pause

rem vcbuild /nocolor /logfile:%PROJECT%_vcb.log /error:ERROR: /warning:WARNING: %PROJECT%.sln
rem if errorlevel 2 set error=yes
rem if error==yes goto upload
rem echo vcbuild done

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
