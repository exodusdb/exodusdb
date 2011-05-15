:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

call config

rem if not X%EXODUS_PACK%X == XX goto gotexoduspack
rem @echo MISSING EXODUS_PACK ENVIRONMENT VARIABLE
rem if "%EXODUS_BATCHMODE%" == "" pause
rem exit
rem :gotexoduspack
rem @echo EXODUS_PACK=%EXODUS_PACK%

%EXODUS_PACK%

if "%EXODUS_BATCHMODE%" == "" pause

rem ----- INSTALLING ----
rem exodus-x86-11.5.3.exe
rem exodus-x86-11.5.3.exe /S     (silent install *capital* S)
