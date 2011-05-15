:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

call config.bat

if not "%EXODUS_MAKE%" == "" goto gotexodusmake
@echo MISSING EXODUS_MAKE ENVIRONMENT VARIABLE
if "%EXODUS_BATCHMODE%" == "" pause
exit
:gotexodusmake
@echo EXODUS_MAKE=%EXODUS_MAKE%

%EXODUS_MAKE%

if "%EXODUS_BATCHMODE%" == "" pause
