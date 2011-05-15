:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

call config.bat

@echo %EXODUS_MAKE%
%EXODUS_MAKE%

if not EXODUS_BATCHMODE == "" pause