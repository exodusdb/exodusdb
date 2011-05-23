
rem --------------------------------------------------------------
rem --- your local configuration is in configlocal.cmd         ---
rem --- (copy configlocalEXAMPLE.cmd TO configlocal.cmd)       ---
rem --------------------------------------------------------------
rem * indicates the default. Uncomment alternatives or add your own
rem sorry but you MUST remove trailing spaces on lines!!!
rem
rem EXODUS VERSION (no spaces or unusual characters!)
rem --------------
    if exist version.cmd call version.cmd
rem set DEFAULT_EXO_MAJOR_VER=0
rem set DEFAULT_EXO_MINOR_VER=0
rem set DEFAULT_EXO_MICRO_VER=0
rem set DEFAULT_EXO_BUILD_VER=0
rem
rem TARGET_CPU
rem ----------
rem set DEFAULT_TARGET_CPU=x64*
rem set DEFAULT_TARGET_CPU=x86
rem 
rem CONFIGURATION
rem -------------
rem set DEFAULT_CONFIGURATION=Release*
rem set DEFAULT_CONFIGURATION=Debug
rem
rem TOOLSET
rem -------
rem set DEFAULT_EXO_TOOLSET=SDK71*
rem set DEFAULT_EXO_TOOLSET=VS2005
rem
rem --------------------------------------------------------------
rem 
rem PROGRAM DRIVE - for the "EXO_PROGRAM_DRIVE" bit of EXO_PROGRAM_DRIVE\Program Files\Postgresql\9.0\bin etc
rem -------------
rem set EXO_PROGRAMFILES_ROOT=C:*
rem set EXO_PROGRAMFILES_ROOT=D:
rem
rem NO TRAILING SLASH!
rem POSTGRES, BOOSTPRO, VISUAL STUDIO, SDK, NSIS must all be stored on one drive
rem otherwise you need to configure "below the line" in the config.cmd/configcustom.cmd
rem eg X:\Program Files\PostgreSQL\9.0\bin
rem X:\Program Files\Boost\1_46_1\bin
rem X:\Program Files\Microsoft Visual Studio 8\VC\
rem X:\Program Files\Microsoft SDKs\Windows\v7.1\Bin
rem X:\Program Files\NSIS\makensis.exe
rem some may be "Program Files (x86)" on x64 platform
rem
rem BUILD ROOT - if not using BOOSTPRO, for the "EXO_BUILD_ROOT" bit of EXO_BUILD_ROOT\Boost\1_46_1\bin
rem ----------
rem set EXO_BUILD_ROOT=* (blank means root relative to location of Exodus source)
rem set EXO_BUILD_ROOT=D:
rem
rem NO TRAILING SLASH! NO RELATIVE PATHS - blank means root of exodus drive
rem dependencies = currently only BOOST. BOOSTPRO isnt here. it is assumed to be on the PROGRAM_DRIVE above
rem 
rem POSTGRES - for the "9.0" bit in X:\Program Files\PostgreSQL\9.0\bin
rem --------
rem set EXO_POSTGRES_VER=9.0*
rem set EXO_POSTGRES_VER=8.4
rem set EXO_POSTGRES_VER=9.0a
rem 
rem BOOSTPRO - Yes/No
rem --------
rem set EXO_BOOSTPRO=NO*
rem set EXO_BOOSTPRO=YES
rem YES means x86-only libs from boostpro in X:\Program Files\boost_1_46_1\bin
rem ELSE libs are in %EXO_BUILD_ROOT%\boost_1_46_1\bin
rem 
rem BOOST - for the "1_46_1" bit in D:\boost_1_46_1\bin or C:\Program Files\boost_1_46_1\bin
rem -----
rem set EXO_BOOST_VER=1_46_1 *
rem set EXO_BOOST_VER=1_44_0
rem
rem UPLOADER - only examples, there is no default.
rem --------
rem set EXO_UPLOADUSER=neosys.com@gmail.com
rem set UPLOADPASS_EXO=somesillysecret

rem Note about include and lib search paths
rem ---------------------------------------
rem exodus project build looks for includes and libs in this order
rem so that architecture specific lib32/lib64 is picked before generic lib
rem and you can keep different architectures in the same tree
rem (xx=32 or 64)
rem libxx
rem stagexx\lib
rem lib
rem stage\lib
