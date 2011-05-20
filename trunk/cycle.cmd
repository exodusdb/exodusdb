set EXO_BATCHMODE=YES

if exist cycle.log del cycle.log
if exist cycle.err del cycle.err

set CONFIGURATION=Release

set TARGET_CPU=x86
call :build

set TARGET_CPU=x64
rem call :build

goto :xyz

rem ------
:build
rem ------

date /t >> cycle.log
time /t >> cycle.log
echo %EXO_CONFIGURATION%/%EXO_PLATFORM% >> cycle.log

rem call clean 1>> cycle.log
call make 1>> cycle.log 2>>cycle.err
call pack 1>> cycle.log 2>>cycle.err
rem call upload 1>> cycle.log

:xyz