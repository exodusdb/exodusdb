rem ##################################
rem ### 1. Options for all batches ###
rem ##################################

rem Note:
rem 1. Any options can be moved down to the per batch section
rem 2. These "BATCH" options override any settings in configlocal.cmd

rem If you want to override configlocal.cmd/version.cmd
rem -------------------------------------------------
rem set EXO_BATCH_MAJOR_VER=
rem set EXO_BATCH_MINOR_VER=
rem set EXO_BATCH_MICRO_VER=
rem set EXO_BATCH_BUILD_VER=

rem Usually set per batch in section 4.x below
rem ------------------------------------------
rem set BATCH_TARGET_CPU=
rem set BATCH_CONFIGURATION=

rem Commonly only one toolset etc installed so best defined in
rem configlocal.cmd but can be defined here per batch if multiple
rem tools etc are available
rem -------------------------------------------------------------
rem set EXO_BATCH_TOOLSET=
rem set EXO_BATCH_POSTGRES_VER=
rem set EXO_BATCH_BOOST_VER=
rem set EXO_BATCH_BOOSTPRO=
rem set EXO_BATCH_BUILD_ROOT=
rem set EXO_BATCH_PROGRAMFILES_ROOT=

rem ##########################################
rem ### 2. Change the default batch stages ###
rem ##########################################
rem *=default, un-remark the alternative if desired

rem set EXO_BATCH_CLEAN=NO*
rem set EXO_BATCH_CLEAN=YES

rem set EXO_BATCH_MAKE=YES*
rem set EXO_BATCH_MAKE=NO

rem set EXO_BATCH_PACK=YES*
rem set EXO_BATCH_PACK=NO

rem set EXO_BATCH_UPLOAD=NO*
rem set EXO_BATCH_UPLOAD=YES

rem ################################
rem ### 3. Standard Preparation ####
rem ################################

rem ----------------------------
rem --- clean the batch logs ---
rem ----------------------------
    if exist batch.log del batch.log
    if exist batch.err del batch.err


rem ######################
rem #### 4.x Do a batch ##
rem ######################

    set BATCH_TARGET_CPU=x64
    set BATCH_CONFIGURATION=Release
    call batchone

rem ######################
rem #### 4.x Do a batch ##
rem ######################

    set BATCH_TARGET_CPU=x86
    set BATCH_CONFIGURATION=Release
    call batchone

rem ##########################################
rem #### 5. Store and/or email the results ###
rem ##########################################
pause