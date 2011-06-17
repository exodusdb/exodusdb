rem WHICH COMMAND IN WINDOWS!
rem for %i in (YOURPROGRAM.exe) do @echo %~$PATH:i


rem ------------------------------
rem --- quick and dirty config ---
rem ------------------------------
rem this is stuff that has yet to be properly positioned in config.cmd and configlocalEXAMPLE.cmd

rem NOTE: watch out for module names causing conflicting dll names to avoid overwriting.

rem ---------------------------------------------------------
rem --- dev.cmd, make.cmd, clean.cmd, pack.cmd and upload.cmd
rem ---  all call this config.cmd to initialise           ---
rem --- NO GLOBAL ENVVAR CONFIGURATION SHOULD BE REQUIRED ---
rem ---------------------------------------------------------

set EXO_CONFIGMODE=%*

rem --------------------------------------------------------------
rem --- your local configuration is in configlocal.cmd         ---
rem --- (copy configlocalEXAMPLE.cmd TO configlocal.cmd)       ---
rem --------------------------------------------------------------
    if exist version.cmd call version.cmd %*
    if exist configlocal.cmd call configlocal.cmd %*

rem you should not have to edit this config.cmd but, if you do
rem see "Completely customising configuration" below

rem -------------------------------
rem --- Batch Run Configuration ---
rem -------------------------------

    if "%EXO_BATCH_MAJOR_VER%" NEQ "" set EXO_MAJOR_VER=%EXO_BATCH_MAJOR_VER%
    if "%EXO_BATCH_MINOR_VER%" NEQ "" set EXO_MINOR_VER=%EXO_BATCH_MINOR_VER%
    if "%EXO_BATCH_MICRO_VER%" NEQ "" set EXO_MICRO_VER=%EXO_BATCH_MICRO_VER%
    if "%EXO_BATCH_BUILD_VER%" NEQ "" set EXO_BUILD_VER=%EXO_BATCH_BUILD_VER%

    if "%BATCH_TARGET_CPU%"    NEQ "" set TARGET_CPU=%BATCH_TARGET_CPU%
    if "%BATCH_CONFIGURATION%" NEQ "" set CONFIGURATION=%BATCH_CONFIGURATION%
    if "%EXO_BATCH_TOOLSET%"   NEQ "" set EXO_TOOLSET=%EXO_BATCH_TOOLSET%

    if "%EXO_BATCH_POSTGRES_VER%" NEQ "" set EXO_POSTGRES_VER=%EXO_BATCH_POSTGRES_VER%

    if "%EXO_BATCH_BOOST_VER%" NEQ "" set EXO_BOOST_VER=%EXO_BATCH_BOOST_VER%
    if "%EXO_BATCH_BOOSTPRO%"  NEQ "" set EXO_BOOSTPRO=%EXO_BATCH_BOOSTPRO%

    if "%EXO_BATCH_BUILD_ROOT%"        NEQ "" set EXO_BUILD_ROOT=%EXO_BATCH_BUILD_ROOT%

    if "%EXO_BATCH_PROGRAMFILES_ROOT%" NEQ "" set EXO_PROGRAMFILES_ROOT=%EXO_BATCH_PROGRAMFILES_ROOT%

rem ----------------
rem --- Defaults ---
rem ----------------

    if "%EXO_MAJOR_VER%" EQU "" set EXO_MAJOR_VER=0
    if "%EXO_MINOR_VER%" EQU "" set EXO_MINOR_VER=0
    if "%EXO_MICRO_VER%" EQU "" set EXO_MICRO_VER=0
    if "%EXO_BUILD_VER%" EQU "" set EXO_BUILD_VER=0

    if "%TARGET_CPU%"    EQU "" set TARGET_CPU=x64
    if "%CONFIGURATION%" EQU "" set CONFIGURATION=Release
    if "%EXO_TOOLSET%"   EQU "" set EXO_TOOLSET=SDK71

    rem eg C:\Program Files\PostgreSQL\9.0
    if "%EXO_POSTGRES_PREFIX%" EQU "" set EXO_POSTGRES_PREFIX=
    if "%EXO_POSTGRES_VER%"    EQU "" set EXO_POSTGRES_VER=9.0

    rem eg C:\boost_1_46_1
    if "%EXO_BOOST_PREFIX%"  EQU "" set EXO_BOOST_PREFIX=Boost_
    if "%EXO_BOOST_VER%"     EQU "" set EXO_BOOST_VER=1_46_1

    rem eg C:\Program Files\boost_1_46_1
    if "%EXO_BOOSTPRO%"      EQU "" set EXO_BOOSTPRO=NO

    rem eg c:\icu\source
    if "%EXO_ICU_PREFIX%" EQU "" set EXO_ICU_PREFIX=icu
    if "%EXO_ICU_VER%"    EQU "" set EXO_ICU_VER=
    if "%EXO_ICU_SUFFIX%" EQU "" set EXO_ICU_SUFFIX=\source

    rem eg C:\swigwin-2.0.4
    if "%EXO_SWIG_PREFIX%"   EQU "" set EXO_SWIG_PREFIX=swigwin-
    if "%EXO_SWIG_VER%"      EQU "" set EXO_SWIG_VER=2.0.4

    rem eg C:\Python27
    if "%EXO_PYTHON_PREFIX%" EQU "" set EXO_PYTHON_PREFIX=Python
    if "%EXO_PYTHON_VER%"    EQU "" set EXO_PYTHON_VER=27

    rem eg C:\Perl
    if "%EXO_PERL_PREFIX%"   EQU "" set EXO_PERL_PREFIX=Perl
    if "%EXO_PERL_VER%"      EQU "" set EXO_PERL_VER=

    rem eg C:\php-5.3.6
    if "%EXO_PHP_PREFIX%" EQU "" set EXO_PHP_PREFIX=php-
    if "%EXO_PHP_VER%"    EQU "" set EXO_PHP_VER=5.3.6

    rem eg D:\Program Files\Java\jdk1.6.0_26
    if "%EXO_JAVA_PREFIX%" EQU "" set EXO_JAVA_PREFIX=jdk
    if "%EXO_JAVA_VER%"    EQU "" set EXO_JAVA_VER=1.6.0_26

    if "%EXO_BUILD_ROOT%"    EQU "" set EXO_BUILD_ROOT=

    if "%EXO_PROGRAMFILES_ROOT%" EQU "" set EXO_PROGRAMFILES_ROOT=C:

rem ------------------
rem -- Module Name ---
rem ------------------

    rem EG PYTHON exo.pm and exo.dll
    set EXO_PERL_MODULENAME=exo

    rem EG PYTHON exodus.py and _exodus.dll
    set EXO_PYTHON_MODULENAME=exodus

    rem EG PHP exo.php and exo.dll
    set EXO_PHP_MODULENAME=exo

    rem module name determines name of the SO/DLL and loadlibrary statement
    rem EG JAVA org.exodus.jar and libexodus.dll?
    set EXO_JAVA_MODULENAME=jexodus

    rem package name determines the name of the jar and import statement
rem set EXO_JAVA_PACKAGE_NAME=org.exodus
rem set EXO_JAVA_PACKAGE_SUBDIR=org\exodus
    set EXO_JAVA_PACKAGE_NAME=exodus
    set EXO_JAVA_PACKAGE_SUBDIR=exodus

rem -----------------------------------------------------
rem --- completely customising configuration          ---
rem -----------------------------------------------------
rem 1. Copy config.cmd to configcustom.cmd
rem 2. Edit configcustom.cmd as desired
rem 3. EDIT OUT ALL LINES DOWN TO "BELOW THE LINE" FROM CONFIGCUSTOM.CMD
rem    TO AVOID CONFIGCUSTOM.CMD FROM CALLING CONFIGCUSTOM.CMD RECURSIVELY!
rem -----------------------------------------------------
    if exist configcustom.cmd call configcustom.cmd %*
    if exist configcustom.cmd goto exit

rem ================================================================
rem ===                 "BELOW THE LINE"                         ===
rem ================================================================
rem === IF ALL THE PROGRAMS ARE INSTALLED IN THE USUAL PLACES    ===
rem === YOU SHOULD NOT HAVE TO MODIFY ANYTHING "BELOW THE LINE"  ===
rem ================================================================

rem ---------------------------------------------------
rem --- GENERAL INFO ABOUT LOCATION OF DEPENDENCIES ---
rem ---------------------------------------------------
rem 1. STANDARD build environment assumes all "programs" eg VS/SDK,
rem  postgresql, nsis etc are installed on ONE drive
rem  (and to the *standard path structure* even if you install them not on C:)
rem 2. Best to install all building stuff like exodus/boost libraries etc on one drive
rem maybe the same as the programs, maybe not.
rem
rem We need to know the location of MSVC, Boost, PostgreSQL and NSIS
rem the following is standard except drives and versions
rem 
rem MSVC
rem X:\Program Files\Microsoft Visual Studio 8\VC\
rem X:\Program Files\Microsoft SDKs\Windows\v7.1\Bin
rem
rem POSTGRES32/64
rem X:\Program Files\PostgreSQL\9.0
rem X:\Program Files (x86)\PostgreSQL\9.0
rem 
rem BOOST32/64 eg D: for build drive and C: for 
rem Y:\boost_1_46_1
rem If you are using the ready made boost/32 binaries then
rem X:\Program Files\Boost\boost_1_46_1
rem X:\Program Files (x86)\Boost\boost_1_46_1
rem 
rem NSIS
rem rem X:\Program Files\NSIS\makensis.exe
rem rem X:\Program Files (x86)\NSIS\makensis.exe

rem ------------------------
rem --- check target cpu ---
rem ------------------------
    if "%TARGET_CPU%" EQU "x86" goto validtargetcpu
    if "%TARGET_CPU%" EQU "x64" goto validtargetcpu
	echo "Error: config.cmd: TARGET_CPU:"%TARGET_CPU%" is invalid. Should be x86 or x64" 1>&2
	goto exit
:validtargetcpu

rem ---------------------------
rem --- check configuration ---
rem ---------------------------
    if "%CONFIGURATION%" EQU "Release" goto validconfiguration
    if "%CONFIGURATION%" EQU "Debug" goto validconfiguration
	echo "Error: config.cmd: CONFIGURATION:"%CONFIGURATION%" is invalid. Should be Release or Debug" 1>&2
	goto exit
:validconfiguration

rem ------------------------------------
rem --- WHERE ARE PROGRAMS INSTALLED ---
rem ------------------------------------
    set EXO_PROGRAMFILES32=%EXO_PROGRAMFILES_ROOT%\Program Files
    set EXO_PROGRAMFILES64=%EXO_PROGRAMFILES_ROOT%\Program Files
    if "%PROCESSOR_ARCHITECTURE%" EQU "AMD64" set EXO_PROGRAMFILES32=%EXO_PROGRAMFILES32% (x86)

rem check programs directory exist
    if "%TARGET_CPU%" EQU "x86" set EXO_PROGRAMFILES=%EXO_PROGRAMFILES32%
    if "%TARGET_CPU%" EQU "x64" set EXO_PROGRAMFILES=%EXO_PROGRAMFILES64%
    if exist "%EXO_PROGRAMFILES%" goto gotprogramfiles
	echo "Error: config.cmd: EXO_PROGRAMFILES="%EXO_PROGRAMFILES%" does not exist" 1>&2
	goto exit
:gotprogramfiles

rem clean, pack and upload do not need to know where libraries are
if "%EXO_CONFIGMODE%" EQU "CLEAN"  goto afterlibs
if "%EXO_CONFIGMODE%" EQU "PACK"   goto afterlibs
if "%EXO_CONFIGMODE%" EQU "UPLOAD" goto afterlibs

rem ----- ICU -----
rem ---------------
    set ICU32=%EXO_BUILD_ROOT%\%EXO_ICU_PREFIX%%EXO_ICU_VER%%EXO_ICU_SUFFIX%
    set ICU64=%EXO_BUILD_ROOT%\%EXO_ICU_PREFIX%%EXO_ICU_VER%%EXO_ICU_SUFFIX%

rem ----- BUILT BOOST -----
rem -----------------------
    set BOOST32=%EXO_BUILD_ROOT%\%EXO_BOOST_PREFIX%%EXO_BOOST_VER%
    set BOOST64=%EXO_BUILD_ROOT%\%EXO_BOOST_PREFIX%%EXO_BOOST_VER%


rem ----- INSTALLED BOOST -----
rem ---------------------------
    IF "%BOOSTPRO%" EQU "YES" set BOOST32=%EXO_PROGRAMFILES32%\Boost\%EXO_BOOST_PREFIX%%EXO_BOOST_VER%
    IF "%BOOSTPRO%" EQU "YES" set BOOST64=%EXO_PROGRAMFILES64%\Boost\%EXO_BOOST_PREFIX%%EXO_BOOST_VER%

rem --- CHECK BOOST ---
rem -------------------
    if "%TARGET_CPU%" EQU "x86" set EXO_BOOST=%BOOST32%
    if "%TARGET_CPU%" EQU "x64" set EXO_BOOST=%BOOST64%

rem check boost exists
    if exist "%EXO_BOOST%" goto gotboost
	echo "Error: config.cmd: EXO_BOOST="%EXO_BOOST%" does not exist" 1>&2
	goto exit
:gotboost

rem check boost include exists (assume lib can be found there too)
    if exist "%EXO_BOOST%\boost\version.hpp" goto gotboostinclude
	echo "Error: config.cmd: EXO_BOOST="%EXO_BOOST%\boost\version.hpp" does not exist" 1>&2
	goto exit
:gotboostinclude

rem NOTE: boost x64 binary libs are not available so we generally build them here
rem D:\boost_1_46_1\stage64\lib
rem D:\boost_1_46_1\stage32\lib
rem exodus project build looks for includes and libs in this order
rem so that architecture specific lib32/lib64 is picked before generic lib
rem and you can keep different architectures in the same tree
rem --- xx=32 or 64 ---
rem include
rem libxx
rem stagexx\lib
rem lib
rem stage\lib

rem ----- POSTGRES -----
rem --------------------
    set POSTGRES32=%EXO_PROGRAMFILES32%\PostgreSQL\%EXO_POSTGRES_PREFIX%%EXO_POSTGRES_VER%
    set POSTGRES64=%EXO_PROGRAMFILES64%\PostgreSQL\%EXO_POSTGRES_PREFIX%%EXO_POSTGRES_VER%

rem check postgres master directory exists
    if "%TARGET_CPU%" EQU "x86" set EXO_POSTGRES=%EXO_PROGRAMFILES32%\PostgreSQL\
    if "%TARGET_CPU%" EQU "x64" set EXO_POSTGRES=%EXO_PROGRAMFILES64%\PostgreSQL\
    if exist "%EXO_POSTGRES%" goto gotpostgres
	echo "Error: config.cmd: EXO_POSTGRES="%EXO_POSTGRES%" does not exist" 1>&2
	goto exit
:gotpostgres

rem check postgres ver include (assume libs can be found there too)
    if "%TARGET_CPU%" EQU "x86" set EXO_POSTGRES_INCLUDE=%POSTGRES32%\include
    if "%TARGET_CPU%" EQU "x64" set EXO_POSTGRES_INCLUDE=%POSTGRES64%\include
    if exist "%EXO_POSTGRES_INCLUDE%\*.*" goto gotpostgresinclude
	echo "Error: config.cmd: EXO_POSTGRES_INCLUDE="%EXO_POSTGRES_INCLUDE%" does not exist" 1>&2
	goto exit
:gotpostgresinclude

rem ----- SWIG -----
rem ----------------
    set EXO_SWIG_OPTIONS="-w503,314,389,361,362,370,383,384"

    set SWIG32=%EXO_BUILD_ROOT%\%EXO_SWIG_PREFIX%%EXO_SWIG_VER%
    set SWIG64=%EXO_BUILD_ROOT%\%EXO_SWIG_PREFIX%%EXO_SWIG_VER%

rem check postgres master directory exists
    if "%TARGET_CPU%" EQU "x86" set EXO_SWIG=%EXO_SWIG32%
    if "%TARGET_CPU%" EQU "x64" set EXO_SWIG=%EXO_SWIG64%
rem dont check for now in case they are not building the SWIG bindings
rem    if exist "%EXO_SWIG%" goto gotSWIG
rem	echo "Error: config.cmd: EXO_SWIG="%EXO_SWIG%" does not exist" 1>&2
rem	goto exit
:gotSWIG

rem --------------
rem --- Python ---
rem --------------
    set EXO_PYTHON32=%EXO_BUILD_ROOT%\%EXO_PYTHON_PREFIX%%EXO_PYTHON_VER%
    set EXO_PYTHON64=%EXO_BUILD_ROOT%\%EXO_PYTHON_PREFIX%%EXO_PYTHON_VER%

rem check python master directory exists
    if "%TARGET_CPU%" EQU "x86" set EXO_PYTHON=%EXO_PYTHON32%
    if "%TARGET_CPU%" EQU "x64" set EXO_PYTHON=%EXO_PYTHON64%
rem dont check for now in case they are not building the python bindings
rem    if exist "%EXO_PYTHON%" goto gotpython
rem	echo "Error: config.cmd: EXO_PYTHON="%EXO_PYTHON%" does not exist" 1>&2
rem	goto exit
:gotpython

goto gotpythoninclude
rem check python ver include (assume libs can be found there too)
    if "%TARGET_CPU%" EQU "x86" set EXO_PYTHON_INCLUDE=%EXO_PYTHON32%\include
    if "%TARGET_CPU%" EQU "x64" set EXO_PYTHON_INCLUDE=%EXO_PYTHON64%\include
    if exist "%EXO_PYTHON_INCLUDE%\*.*" goto gotpythoninclude
	echo "Error: config.cmd: EXO_PYTHON_INCLUDE="%EXO_PYTHON_INCLUDE%" does not exist" 1>&2
	goto exit
:gotpythoninclude

set EXO_PYTHON32_INCLUDE=%EXO_PYTHON32%\Include
set EXO_PYTHON64_INCLUDE=%EXO_PYTHON64%\Include

set EXO_PYTHON32_LIB=%EXO_PYTHON32%\Libs
set EXO_PYTHON64_LIB=%EXO_PYTHON64%\Libs

set EXO_PYTHON32_MODULEDIR=%EXO_PYTHON32%\Lib\site-packages
set EXO_PYTHON64_MODULEDIR=%EXO_PYTHON64%\Lib\site-packages

rem ------------
rem --- Perl ---
rem ------------

set EXO_PERL32=%EXO_BUILD_ROOT%\%EXO_PERL_PREFIX%%EXO_PERL_VER%
set EXO_PERL64=%EXO_BUILD_ROOT%\%EXO_PERL_PREFIX%%EXO_PERL_VER%

set EXO_PERL32_INCLUDE=%EXO_PERL32%\LIB\CORE
set EXO_PERL64_INCLUDE=%EXO_PERL64%\LIB\CORE

set EXO_PERL32_LIB=%EXO_PERL32%\LIB\CORE
set EXO_PERL64_LIB=%EXO_PERL64%\LIB\CORE

set EXO_PERL32_MODULEDIR=%EXO_PERL32%\site\lib
set EXO_PERL64_MODULEDIR=%EXO_PERL64%\site\lib


rem ----- PHP -----
rem ------------------
    set PHP32=%EXO_BUILD_ROOT%\%EXO_PHP_PREFIX%%EXO_PHP_VER%
    set PHP64=%EXO_BUILD_ROOT%\%EXO_PHP_PREFIX%%EXO_PHP_VER%

rem check php master directory exists
    if "%TARGET_CPU%" EQU "x86" set EXO_PHP=%EXO_PHP32%
    if "%TARGET_CPU%" EQU "x64" set EXO_PHP=%EXO_PHP64%
rem dont check for now in case they are not building the php bindings
rem    if exist "%EXO_PHP%" goto gotphp
rem	echo "Error: config.cmd: EXO_PHP="%EXO_PHP%" does not exist" 1>&2
rem	goto exit
:gotphp

goto gotphpinclude
rem check php ver include (assume libs can be found there too)
    if "%TARGET_CPU%" EQU "x86" set EXO_PHP_INCLUDE=%PHP32%\include
    if "%TARGET_CPU%" EQU "x64" set EXO_PHP_INCLUDE=%PHP64%\include
    if exist "%EXO_PHP_INCLUDE%\*.*" goto gotphpinclude
	echo "Error: config.cmd: EXO_PHP_INCLUDE="%EXO_PHP_INCLUDE%" does not exist" 1>&2
	goto exit
:gotphpinclude


rem ------------
rem --- Java ---
rem ------------

set EXO_JAVA32=%EXO_PROGRAMFILES32%\Java\%EXO_JAVA_PREFIX%%EXO_JAVA_VER%
set EXO_JAVA64=%EXO_PROGRAMFILES64%\Java\%EXO_JAVA_PREFIX%%EXO_JAVA_VER%

set EXO_JAVA32_BIN=%EXO_JAVA32%\bin
set EXO_JAVA64_BIN=%EXO_JAVA64%\bin

set EXO_JAVA32_INCLUDE=%EXO_JAVA32%\include
set EXO_JAVA64_INCLUDE=%EXO_JAVA64%\include

set EXO_JAVA32_LIB=%EXO_JAVA32%\lib
set EXO_JAVA64_LIB=%EXO_JAVA64%\lib

set EXO_JAVA32_MODULEDIR=%EXO_JAVA32%\lib
set EXO_JAVA64_MODULEDIR=%EXO_JAVA64%\lib

if "%TARGET_CPU%" EQU "x86" set EXO_JAVA=%EXO_JAVA32%
if "%TARGET_CPU%" EQU "x64" set EXO_JAVA=%EXO_JAVA64%

rem --- quick test of config ---
rem ...
rem pause

:afterlibs

rem ----------------
rem --- TOOLSETS ---
rem ----------------

    if "%EXO_CONFIGMODE%" EQU "DEV" goto aftertoolsets

    if "%EXO_TOOLSET%" EQU "VS2005" goto VS2005
    if "%EXO_TOOLSET%" EQU "SDK71" goto SDK71

    echo "Error: config.cmd: Invalid EXO_TOOLSET="%EXO_TOOLSET%". Valid toolsets are VS2005 SDK71" 1>&2
    goto exit



:VS2005
rem ---------------------------------------------------------------
rem --- VS2005 TOOLSET                                          ---
rem ---------------------------------------------------------------

rem ----------------------------
rem --- VS2005 Configuration ---
rem ----------------------------

rem path %EXO_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\;%PATH%
    path %PATH%;%EXO_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\

rem --- x86 command prompt ---
    if %TARGET_CPU%==x86 call vcvarsall.bat x86

rem --- VS2005 cross compiler ---
    if %TARGET_CPU%==x64 call vcvarsall.bat x86_amd64

rem -----------------------
rem --- VS2005 Binaries ---
rem -----------------------
rem these are probably going to be removed at least for release versions that have redist packages ... debug versions may stay
rem Runtime version 70 80 90 100 for MSVC2003, 2005, 2008 and 2010 respectively
rem and location of msvcrNNd.dll etc c runtime dlls
set EXO_TOOLPATH=C:Windows\system32
 set EXO_TOOLPATHREL=%EXO_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT
 set EXO_TOOLPATHDEB=%EXO_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\x86\Microsoft.VC80.DebugCRT
rem set EXO_TOOLPATHREL=%EXO_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\amd64\Microsoft.VC80.CRT
rem set EXO_TOOLPATHDEB=%EXO_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\amd64\Microsoft.VC80.DebugCRT
set EXO_VCVERSION=80

rem ---------------------
rem --- VS2005 Redist ---
rem ---------------------

if "%TARGET_CPU%" == "x64" goto redist2005x64
rem --- NOTE: QUOTE THE URLS! BUT NOT DESC/SOURCE! ---
set REDIST_DESC=MSVC++ 2005 Redist. x86 SP1
set REDIST_SOURCE1=download.microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/e/1/c/e1c773de-73ba-494a-a5ba-f24906ecf088/vcredist_x86.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2005_x86.exe"
goto gotredist2005
:redist2005x64
set REDIST_DESC=MSVC++ 2005 Redist. x64 SP1
set REDIST_SOURCE1=download.microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/d/4/1/d41aca8a-faa5-49a7-a5f2-ea0aa4587da0/vcredist_x64.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2005_x64.exe"
:gotredist2005
set EXO_REDISTDOTVER=8.0

goto checktoolset



:SDK71
rem -------------------------------------------------------------------------
rem --- SDK71 TOOLSET (the last one and the default)                      ---
rem -------------------------------------------------------------------------

rem    path %EXO_PROGRAMFILES64%\Microsoft SDKs\Windows\v7.1\Bin;%PATH%
rem path %PATH%;%EXO_PROGRAMFILES64%\Microsoft SDKs\Windows\v7.1\Bin

rem ---------------------------
rem --- SDK71 Configuration ---
rem ---------------------------
rem echo    call "%EXO_PROGRAMFILES64%\Microsoft SDKs\Windows\v7.1\Bin\setenv" /%TARGET_CPU% /%CONFIGURATION%
rem pause
    call "%EXO_PROGRAMFILES64%\Microsoft SDKs\Windows\v7.1\Bin\setenv" /%TARGET_CPU% /%CONFIGURATION% 2> nul
rem examples:
rem call setenv /x86 /debug
rem call setenv /x86 /release
rem call setenv /x64 /debug
rem call setenv /x64 /release

rem setenv seems to switch off echo. switch echo back on
echo on

rem ----------------------
rem --- SDK71 Binaries ---
rem ----------------------
rem Runtime version 70 80 90 100 for MSVC2003, 2005, 2008 and 2010 respectively
rem and location of msvcrNNd.dll etc c runtime dlls
set EXO_TOOLPATHREL=C:Windows\system32
set EXO_TOOLPATHDEB=C:Windows\system32
set EXO_VCVERSION=100

rem --------------------
rem --- SDK71 Redist ---
rem --------------------
rem 2008x86 is "http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe"

if "%TARGET_CPU%" == "x64" goto redist2010x64
rem --- NOTE: QUOTE THE URLS! and NOT DESC/SOURCE! ---
set REDIST_DESC=MSVC++ 2010 Redist. x86
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2010_x86.exe"
goto gotredist2010
:redist2010x64
set REDIST_DESC=MSVC++ 2010 Redist. x64
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/3/2/2/3224B87F-CFA0-4E70-BDA3-3DE650EFEBA5/vcredist_x64.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2010_x64.exe"
:gotredist2010
set EXO_REDISTDOTVER=10.0
@echo REDIST_DESC=%REDIST_DESC%
@echo REDIST_SOURCE1=%REDIST_SOURCE1%
@echo REDIST_URL1=%REDIST_URL1%
@echo EXO_REDISTDOTVER=%EXO_REDISTDOTVER%



:checktoolset
rem -----------------------
rem --- CHECK TOOLSETOK ---
rem -----------------------

if "%CONFIGURATION%" NEQ "" goto gotconfiguration
echo "Error: config.cmd: has failed to setup the CONFIGURATION Environment variable" 1>&2
goto exit
:gotconfiguration
@echo Configuration=%CONFIGURATION%

if "%TARGET_CPU%" NEQ "" goto gottargetcpu
echo "Error: config.cmd: has failed to setup the 'TARGET_CPU' Environment variable" 1>&2
goto exit
:gottargetcpu
@echo TARGET_CPU=%TARGET_CPU%

:aftertoolsets

rem --------------------
rem --- Product Name ---
rem --------------------
rem Name for menus etc and basic name for installfile (without platform)
rem if "%TARGET_CPU%" == "x86" set EXO_PRODUCTNAME=Exodus
rem if "%TARGET_CPU%" == "x64" set EXO_PRODUCTNAME=Exodus64
rem stick with one name and installdir per minor version regardless of x86/x64 - like python
set EXO_PRODUCTNAME=Exodus
set EXO_CODENAME=exodus

rem ------------------------
rem --- INSTALLFILE NAME ---
rem ------------------------
    set EXO_INSTALL_FILENAME=%EXO_CODENAME%-%EXO_MAJOR_VER%.%EXO_MINOR_VER%.%EXO_MICRO_VER%-%TARGET_CPU%.exe

rem ------------------
rem ---- BUILDING ----
rem ------------------

if "%TARGET_CPU%" == "x64" set EXO_PLATFORM=x64
if "%TARGET_CPU%" == "x86" set EXO_PLATFORM=Win32

if "%TARGET_CPU%" == "x64" set EXO_BINARIES=x64\%CONFIGURATION%
if "%TARGET_CPU%" == "x86" set EXO_BINARIES=%CONFIGURATION%

rem --- SOLUTION ---
rem ----------------
rem default project
    set EXO_PROJECT=exodus_all
rem VS2005
    if "%EXO_TOOLSET%" == "VS2005" set EXO_PROJECT=exodus_all2005
rem VS2008 - doesnt exist but could be created from 2005 solution
    if "%EXO_TOOLSET%" == "VS2008" set EXO_PROJECT=exodus_all2008

rem --- COMMAND TO BUILD ---
rem ------------------------
rem set EXO_MAKECMD=vcbuild /p:Platform=%EXO_PLATFORM% /p:Configuration=%CONFIGURATION% %EXO_PROJECT%.sln
    set EXO_MAKECMD=msbuild /p:Platform=%EXO_PLATFORM% /p:Configuration=%CONFIGURATION% %EXO_PROJECT%.sln

rem --- COMMAND TO CLEAN ---
rem ------------------------
rem set EXO_CLEANCMD=%EXO_MAKECMD% /t:clean
    set EXO_CLEANCMD=%EXO_MAKECMD% /t:clean

rem ignore this for now
rem vcbuild /nocolor /logfile:%EXO_PROJECT%_vcb.log /error:ERROR: /warning:WARNING: %EXO_PROJECT%.sln
rem if errorlevel 2 set error=yes
rem if error==yes goto upload
rem echo vcbuild done


rem clean, pack and upload do not need to know what the devcmd is
if "%EXO_CONFIGMODE%" EQU "CLEAN"  goto afterdevcmd
if "%EXO_CONFIGMODE%" EQU "PACK"   goto afterdevcmd
if "%EXO_CONFIGMODE%" EQU "UPLOAD" goto afterdevcmd

rem -----------------------
rem --- DEVELOPMENT UI  ---
rem -----------------------
rem search for the latest version of VS Professional or Express
    set EXO_VS=%VS110COMNTOOLS%
    if "%EXO_VS%" =="" set EXO_VS=%VS100COMNTOOLS%
    if "%EXO_VS%" =="" set EXO_VS=%VS90COMNTOOLS%
    if "%EXO_VS%" =="" set EXO_VS=%VS80COMNTOOLS%

if exist "%EXO_VS%..\..\VC\vcvarsall.bat" set EXO_DEVPROMPTCMD="%EXO_VS%..\..\VC\vcvarsall.bat"

if exist "%EXO_VS%..\IDE\devenv.exe" set EXO_DEVCMD="%EXO_VS%..\IDE\devenv" %EXO_PROJECT%.sln
if exist "%EXO_VS%..\IDE\devenv.exe" goto afterdevcmd

if exist "%EXO_VS%..\IDE\vcexpress.exe" set EXO_DEVCMD="%EXO_VS%..\IDE\vcexpress" %EXO_PROJECT%.sln
if exist "%EXO_VS%..\IDE\vcexpress.exe" goto afterdevcmd

echo "Error: config.cmd: FAILED TO FIND VS/GUI PROGRAM" 1>&2
goto exit
:afterdevcmd


if "%EXO_CONFIGMODE%" == "CLEAN" goto afteruploader
if "%EXO_CONFIGMODE%" == "MAKE" goto afteruploader
if "%EXO_CONFIGMODE%" == "DEV" goto afteruploader

rem ---------------------------
rem --- PACKAGING INSTALLER ---
rem ---------------------------
set EXO_PACKCMD="%EXO_PROGRAMFILES32%\NSIS\makensis.exe"
if not exist %EXO_PACKCMD% set EXO_PACKCMD="%EXO_PROGRAMFILES64%\NSIS\makensis.exe"
if exist %EXO_PACKCMD% goto gotpackexe
if "%EXO_CONFIGMODE%" NEQ "PACK" goto gotpackexe
echo "Error: config.cmd: MISSING %EXO_PROGRAMFILES32%\NSIS\makensis.exe PROGRAM" 1>&2
goto afterinstaller
goto exit
:gotpackexe

rem ---------------------------------
rem --- COMMAND TO MAKE INSTALLER ---
rem ---------------------------------
rem there is no exodus_all2005.NSI!
rem /2 means output only errors or warnings
rem NSIS outputs errors and warnings to standard output not standard error
rem so we output only errors and warnings to standard output /V2
rem
 set EXO_PACKOPT=/V2 exodus_all.nsi
rem set EXO_PACKOPT=/V3 exodus_all.nsi
rem set EXO_PACKOPT=exodus_all.nsi

:afterinstaller


rem needs EXO_UPLOADUSER UPLOADPASS_EXO EXO_INSTALL_FILENAME
rem -------------------------
rem --- COMMAND TO UPLOAD ---
rem -------------------------
rem TODO MODIFY THE CMD TO FAIL NOT PROMPT FOR USER/PASS
rem NOTE THAT IT ASKS FOR USER AND PASS FOR ANY TYPE OF FAILURE INCLUDE "FILE ALREADY EXISTS" - THERE IS NO OVERWRITE OPTION
set EXO_UPLOADCMD=googlecode_upload.py
set EXO_UPLOADOPT=-s "Windows %TARGET_CPU% Installer" --project=exodusdb --user=%EXO_UPLOADUSER% --password=%UPLOADPASS_EXO% -l "Type-Package,OpSys-Win,Featured" %EXO_INSTALL_FILENAME%

rem ------------------------------------------
rem --- DUMP ALL THE ENVIRONMENT VARIABLES ---
rem ------------------------------------------
@echo BOOST32=%BOOST32%
@echo BOOST64=%BOOST64%
@echo POSTGRES32=%POSTGRES32%
@echo POSTGRES64=%POSTGRES64%
@set EXO

:afteruploader

rem this should be the only exit and pause in the program
:exit
if "%EXO_CONFIGMODE%" EQU "" pause
