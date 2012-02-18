call config UPLOAD > config.log

if "%UPLOADPASS_EXO%" == ""  goto error
if "%EXO_UPLOADUSER%" == ""  goto error


:ok
@echo EXO_UPLOADCMD=%EXO_UPLOADCMD%
@echo EXO_UPLOADOPT=%EXO_UPLOADOPT%
%EXO_UPLOADCMD% %EXO_UPLOADOPT%
goto exit

:error
echo UPLOADPASS_EXO password environment variable missing.
echo could be set in configlocal.cmd
goto exit

:exit
if "%EXO_BATCHMODE%" == "" pause
