:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

set EXODUS_DEV=YES
call config.bat

@echo %EXODUS_DEV%
%EXODUS_DEV%

rem if not EXODUS_BATCHMODE == "" pause