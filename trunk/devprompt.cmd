call config DEV > config.log
@echo BOOST32=%BOOST32%
@echo BOOST64=%BOOST64%
@echo POSTGRES32=%POSTGRES32%
@echo POSTGRES64=%POSTGRES64%
@echo NOTE: ..\release AND ..\x64\release *FIRST* IN THE PATH
set PATH=..\release;..\x64\release;%PATH%

call %EXO_DEVPROMPTCMD% %EXO_DEVPROMPTOPT%
cmd
rem if "%EXO_BATCHMODE%" == "" pause
