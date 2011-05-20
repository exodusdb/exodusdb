call config PACK > config.log
@echo EXO_PACKCMD=%EXO_PACKCMD%
@echo EXO_PACKOPT=%EXO_PACKOPT%
%EXO_PACKCMD% %EXO_PACKOPT%
if "%EXO_BATCHMODE%" == "" pause