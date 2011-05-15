:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

call config

"%NSIS_PATH%\makensis.exe" %EXODUS_PROJECT%.nsi

if not EXODUS_BATCHMODE == "" pause

rem ----- INSTALLING ----
rem exodus-x86-11.5.3.exe
rem exodus-x86-11.5.3.exe /S     (silent install *capital* S)
