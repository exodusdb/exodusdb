call config UPLOAD > config.log
@echo EXO_UPLOADCMD=%EXO_UPLOADCMD%
@echo EXO_UPLOADOPT=%EXO_UPLOADOPT%
%EXO_UPLOADCMD% %EXO_UPLOADOPT%
if "%EXO_BATCHMODE%" == "" pause