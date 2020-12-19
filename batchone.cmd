@echo --------------------------------------------------------- >> batch.log
date /t >> batch.log
time /t >> batch.log
@echo EXO_BATCH_MAJOR_VERSION  = %EXO_BATCH_MAJOR_VER% >>batch.log
@echo EXO_BATCH_MINOR_VERSION  = %EXO_BATCH_MINOR_VER% >>batch.log
@echo EXO_BATCH_MICRO_VERSION  = %EXO_BATCH_MICRO_VER% >>batch.log
@echo EXO_BATCH_BUILD_VERSION  = %EXO_BATCH_BUILD_VER% >>batch.log
@echo BATCH_TARGET_CPU         =%BATCH_TARGET_CPU% >> batch.log
@echo BATCH_CONFIGURATION      =%BATCH_CONFIGURATION% >> batch.log

set EXO_BATCHMODE=YES

if "%EXO_BATCH_CLEAN%"  EQU "YES" call clean  1>> batch.log 2>>batch.err
if "%EXO_BATCH_MAKE%"   NEQ "NO"  call make   1>> batch.log 2>>batch.err
if "%EXO_BATCH_PACK%"   NEQ "NO"  call pack   1>> batch.log 2>>batch.err
if "%EXO_BATCH_UPLOAD%" EQU "YES" call upload 1>> batch.log 2>>batch.err
