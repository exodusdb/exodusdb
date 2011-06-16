call config DEV > config.log
@echo BOOST32=%BOOST32%
@echo BOOST64=%BOOST64%
@echo POSTGRES32=%POSTGRES32%
@echo POSTGRES64=%POSTGRES64%
set PATH=%PATH%;%EXO_JAVA%/bin

call %EXO_DEVPROMPTCMD% %EXO_DEVPROMPTOPT%
cmd
if "%EXO_BATCHMODE%" == "" pause
