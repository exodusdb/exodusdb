rem --- dev.bat, make.bat, clean.bat, pack.bat and upload.bat all call config.bat to initialise ---

rem --- preconfiguration ---
rem --- you can create preconfig.bat to do any local preconfiguration
    if exist preconfig.bat call preconfig %*

rem --- customisation
rem --- copy this file to configlocal.bat and REMOVE FROM CONFIGLOCAL the following line to avoid recursion
rem --- if "configlocal.bat" is present, use it INSTEAD of config.bat
    if exist configlocal.bat config.bat %*

rem note: exodus project build looks for includes and libs in this order
rem so that architecture specific lib32/lib64 is picked before generic lib
rem and you can keep different architectures in the same tree
rem (xx=32 or 64)
rem include
rem libxx
rem lib

set EXO_CONFIGMODE=%*

rem -------------------------------
rem --- Output Product Version  ---
rem -------------------------------
set EXO_MAJOR_VER=11
set EXO_MINOR_VER=5
set EXO_MICRO_VER=19
set EXO_BUILD_VER=
rem probably can put anything alphanumeric - BUT NO SPACES OR UNUSUAL CHARACTERS

rem --------------------------
rem --- Postgresql version ---
rem --------------------------
    set EXO_POSTGRES_VER=9.0
rem used in X:\Program Files\PostgreSQL\9.0\bin

rem ------------------------------
rem --- Boost version and type ---
rem ------------------------------
    set EXO_BOOST_VER=1_46_1
    set EXO_BOOSTPRO=NO
rem ------------------------------
rem eg if EXO_BOOSTPRO!="YES" X:\boost_1_46_1\bin
rem or if EXO_BOOSTPRO=="YES" X:\Program Files\Boost\1_46_1\bin

rem ----------------------------------------------
rem --- TYPE OF BUILD - PICK *ONE* FROM *EACH* ---
rem ----------------------------------------------
rem set TARGET_CPU=x86
rem
 set TARGET_CPU=x64

rem
 set Configuration=Release
rem set Configuration=Debug

rem
 set EXO_TOOLSET=VS2005
rem set EXO_TOOLSET=SDK71

rem -----------------------------------
rem --- PROGRAM DRIVES TYPICALLY C: ---
rem -----------------------------------
    set EXO_PROGRAMDRV=D:
rem ------------------------------------
rem POSTGRES, BOOSTPRO, VISUAL STUDIO, SDK, NSIS must all be stored on one drive
rem otherwise you need to configure "below the line"
rem EXAMPLES: (some may be "Program Files (x86)" on x64 platform)
rem eg X:\Program Files\PostgreSQL\9.0\bin
rem X:\Program Files\Boost\1_46_1\bin
rem X:\Program Files\Microsoft Visual Studio 8\VC\
rem X:\Program Files\Microsoft SDKs\Windows\v7.1\Bin
rem X:\Program Files\NSIS\makensis.exe

rem -------------------------------------
rem --- DRIVES USED - TYPICALLY C: :  ---
rem -------------------------------------
    set EXO_BUILDDRV=D:
rem ------------------------------------
rem POSTGRES, BOOSTPRO, VISUAL STUDIO, SDK, NSIS must all be stored on one drive
rem otherwise you need to configure "below the line"
rem EXAMPLES: (some may be "Program Files (x86)" on x64 platform)
rem eg X:\Program Files\PostgreSQL\9.0\bin
rem X:\Program Files\Boost\1_46_1\bin
rem X:\Program Files\Microsoft Visual Studio 8\VC\
rem X:\Program Files\Microsoft SDKs\Windows\v7.1\Bin
rem X:\Program Files\NSIS\makensis.exe

rem =================================================================
rem ===================== "THE LINE" ================================
rem =================================================================
rem
rem =================================================================
rem ===                 "BELOW THE LINE"                          ===
rem =================================================================
rem === IF ALL THE PROGRAMS ARE INSTALLED IN THE "USUAL" PLACES   ===
rem === YOU SHOULD NOT HAVE TO MODIFY ANYTHING "BELOW THE LINE"   ===
rem =================================================================

rem ---------------------------------------------------
rem --- GENERAL INFO ABOUT LOCATION OF DEPENDENCIES ---
rem ---------------------------------------------------
rem 1. In case drive paths are hard coded in the config search for ":\" not to miss any
rem 2. Best to install all "programs" eg VS/SDK, postgresql, nsis etc on one drive
rem and to the *standard path structure* even if you install them not on C:
rem 3. Best to install all building stuff like exodus/boost libraries etc on one drive
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

rem ------------------------------------
rem --- WHERE ARE PROGRAMS INSTALLED ---
rem ------------------------------------
    set EXO_PROGRAMFILES32=%EXO_PROGRAMDRV%\Program Files
    set EXO_PROGRAMFILES64=%EXO_PROGRAMDRV%\Program Files
    if "%PROCESSOR_ARCHITECTURE%" EQU "AMD64" set EXO_PROGRAMFILES32=%EXO_PROGRAMFILES32% (x86)

rem ----- INSTALLED BOOST -----
rem ---------------------------
    set BOOST32=%BOOST_DRV%\boost_%EXO_BOOST_VER%
    set BOOST64=%BOOST_DRV%\boost_%EXO_BOOST_VER%
    IF BOOSTPRO=="YES" set BOOST32=%EXO_PROGRAMFILES32%\Boost\boost_%EXO_BOOST_VER%
    IF BOOSTPRO=="YES" set BOOST64=%EXO_PROGRAMFILES64%\Boost\boost_%EXO_BOOST_VER%

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
    set POSTGRES32=%EXO_PROGRAMFILES32%\PostgreSQL\%EXO_POSTGRES_VER%
    set POSTGRES64=%EXO_PROGRAMFILES64%\PostgreSQL\%EXO_POSTGRES_VER%

rem ---------------------------------------------
rem --- SKIP TOOLSETS IF OPENING DEVELOPER UI ---
rem ---------------------------------------------

    if "%EXO_CONFIGMODE%" == "DEV" goto aftertoolsets

    if "%EXO_TOOLSET%" NEQ "VS2005" goto toolset2
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

goto checktoolset

:toolset2
if "%EXO_TOOLSET%" NEQ "XXXXXXX" goto toolset3
rem ---------------------------------------------------------------
rem --- xxxxxx TOOLSET                                          ---
rem ---------------------------------------------------------------
rem ...
goto checktoolset


:toolset3
if "%EXO_TOOLSET%" NEQ "XXXXXXX" goto toolset4
rem ---------------------------------------------------------------
rem --- xxxxxx TOOLSET                                          ---
rem ---------------------------------------------------------------
rem ...
goto checktoolset

:toolset4
if "%EXO_TOOLSET%" NEQ "XXXXXXX" goto toolset5
rem ---------------------------------------------------------------
rem --- xxxxxx TOOLSET                                          ---
rem ---------------------------------------------------------------
rem ...
goto checktoolset

:toolset5
rem -------------------------------------------------------------------------
rem --- SDK71 TOOLSET (the last one and the default)                      ---
rem -------------------------------------------------------------------------

    path %EXO_PROGRAMFILES64%\Microsoft SDKs\Windows\v7.1\Bin;%PATH%
rem path %PATH%;%EXO_PROGRAMFILES64%\Microsoft SDKs\Windows\v7.1\Bin

rem ---------------------------
rem --- SDK71 Configuration ---
rem ---------------------------
    call setenv /%TARGET_CPU% /%Configuration%
ren eg
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
@echo REDIST_DESC=%REDIST_DESC%
@echo REDIST_SOURCE1=%REDIST_SOURCE1%
@echo REDIST_URL1=%REDIST_URL1%

rem all toolsets come here
:checktoolset

rem ---------------------
rem --- SANITY CHECKS ---
rem ---------------------

if "%Configuration%" NEQ "" goto gotconfiguration
"Error: Config: has failed to setup the 'Configuration' Environment variable"
goto exit
:gotconfiguration
@echo Configuration=%Configuration%

if "%TARGET_CPU%" NEQ "" goto gottargetcpu
"Error: Config: has failed to setup the 'TARGET_CPU' Environment variable"
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

rem ------------------
rem ---- BUILDING ----
rem ------------------

if "%TARGET_CPU%" == "x64" set EXO_PLATFORM=x64
if "%TARGET_CPU%" == "x86" set EXO_PLATFORM=Win32
if "%TARGET_CPU%" == "x64" set EXO_BINARIES=x64\%Configuration%
if "%TARGET_CPU%" == "x86" set EXO_BINARIES=%Configuration%

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
rem set EXO_MAKECMD=vcbuild /p:Platform=%EXO_PLATFORM% /p:Configuration=%Configuration% %EXO_PROJECT%.sln
    set EXO_MAKECMD=msbuild /p:Platform=%EXO_PLATFORM% /p:Configuration=%Configuration% %EXO_PROJECT%.sln

rem --- COMMAND TO CLEAN ---
rem ------------------------
rem set EXO_CLEANCMD=%EXO_MAKECMD% /t:clean
    set EXO_CLEANCMD=%EXO_MAKECMD% /t:clean

rem ignore this for now
rem vcbuild /nocolor /logfile:%EXO_PROJECT%_vcb.log /error:ERROR: /warning:WARNING: %EXO_PROJECT%.sln
rem if errorlevel 2 set error=yes
rem if error==yes goto upload
rem echo vcbuild done


rem -----------------------
rem --- DEVELOPMENT UI  ---
rem -----------------------
rem search for the latest version of VS Professional or Express
    set EXO_VS=%VS110COMNTOOLS%
    if "%EXO_VS%" =="" set EXO_VS=%VS100COMNTOOLS%
    if "%EXO_VS%" =="" set EXO_VS=%VS90COMNTOOLS%
    if "%EXO_VS%" =="" set EXO_VS=%VS80COMNTOOLS%

if exist "%EXO_VS%..\IDE\devenv.exe" set EXO_DEVCMD="%EXO_VS%..\IDE\devenv" %EXO_PROJECT%.sln
if exist "%EXO_VS%..\IDE\devenv.exe" goto afterdevcmd

if exist "%EXO_VS%..\IDE\vcexpress.exe" set EXO_DEVCMD="%EXO_VS%..\IDE\vcexpress" %EXO_PROJECT%.sln
if exist "%EXO_VS%..\IDE\vcexpress.exe" goto afterdevcmd

"Error: Config: FAILED TO FIND VS/GUI PROGRAM"
goto exit
:afterdevcmd
@echo EXO_DEVCMD=%EXO_DEVCMD%


if "%EXO_CONFIGMODE%" == "CLEAN" goto afterinstaller

rem ---------------------------
rem --- PACKAGING INSTALLER ---
rem ---------------------------
set EXO_PACKCMD="%EXO_PROGRAMFILES32%\NSIS\makensis.exe"
if not exist %EXO_PACKCMD% set EXO_PACKCMD="%EXO_PROGRAMFILES64%\NSIS\makensis.exe"
if exist %EXO_PACKCMD% goto gotpackexe
if "%EXO_CONFIGMODE%" NEQ "PACK" goto gotpackexe
"Error: Config: MISSING %EXO_PROGRAMFILES32%\NSIS\makensis.exe PROGRAM"
goto afterinstaller
goto exit
:gotpackexe
@echo EXO_PACKCMD=%EXO_PACKCMD%

rem ---------------------------------
rem --- COMMAND TO MAKE INSTALLER ---
rem ---------------------------------
rem there is no exodus_all2005.NSI!
set EXO_PACKOPT=exodus_all.nsi

:afterinstaller

@echo BOOST32=%BOOST32%
@echo BOOST64=%BOOST64%
@echo POSTGRES32=%POSTGRES32%
@echo POSTGRES64=%POSTGRES64%
@set EXO

rem this should be the only exit and pause in the program
:exit
if "%EXO_CONFIGMODE%" EQU "" pause