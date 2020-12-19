call config MAKE > config.log
@echo EXO_MAKE_CMD=%EXO_MAKECMD%
@echo EXO_MAKE_OPT=%EXO_MAKEOPT%
%EXO_MAKECMD% %EXO_MAKEOPT%
if "%EXO_BATCHMODE%" == "" pause
