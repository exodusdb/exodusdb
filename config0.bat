:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

rem aim to put the most frequently changed parameters first

rem ------------------------
rem --- Product Version  ---
rem ------------------------
set EXODUS_MINOR_VERSION=11.5
set EXODUS_MICRO_VERSION=11.5.3
rem probably can put anything alphanumeric - BUT NO SPACES OR UNUSUAL CHARACTERS

rem --------------------------
rem --- Postgresql version ---
rem --------------------------
    set EXODUS_PGVERSION=9.0
rem eg C:\Program Files\PostgreSQL\9.0\bin

rem ------------------------------
rem --- Boost version and type ---
rem ------------------------------
    set EXODUS_BOOSTVER=1_46_1
    set EXODUS_BOOSTPRO=NO
    set EXODUS_BOOSTDRV=D:
rem ------------------------------
rem eg if EXODUS_BOOSTPRO!="YES" C:\boost_1_46_1\bin
rem or if EXODUS_BOOSTPRO=="YES" C:\Program Files\Boost\1_46_1\bin

rem ----------------------------------------------
rem --- TYPE OF BUILD - PICK *ONE* FROM *EACH* ---
rem ----------------------------------------------
rem set TARGET_CPU=x86
rem
 set TARGET_CPU=x64

rem set Configuration=Release
rem
 set Configuration=Debug

    set EXODUS_TOOLSET=VS2005
rem set EXODUS_TOOLSET=SDK71

rem ------------------------------------
rem --- DRIVE USED - TYPICALLY C: :  ---
rem ------------------------------------
    set PROGRAMS_DRIVE=D:
rem ------------------------------------
rem POSTGRESQL, BOOSTPRO, VISUAL STUDIO, SDK, NSIS must all be stored on one drive
rem otherwise you need to configure "below the line"
rem EXAMPLES: (some may be "Program Files (x86)" on x64 platform)
rem eg D:\Program Files\PostgreSQL\9.0\bin
rem C:\Program Files\Boost\1_46_1\bin
rem C:\Program Files\Microsoft Visual Studio 8\VC\
rem C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin
rem C:\Program Files\NSIS\makensis.exe

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
rem 
rem MSVC
rem C:\Program Files\Microsoft Visual Studio 8\VC\
rem C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin
rem
rem POSTGRESQL32/64
rem C:\Program Files\PostgreSQL\9.0
rem C:\Program Files (x86)\PostgreSQL\9.0
rem 
rem BOOST32/64 eg D: for build drive and C: for 
rem D:\boost_1_46_1
rem If you are using the ready made boost/32 binaries then
rem C:\Program Files\Boost\boost_1_46_1
rem C:\Program Files (x86)\Boost\boost_1_46_1
rem 
rem NSIS
rem rem C:\Program Files\NSIS\makensis.exe
rem rem C:\Program Files (x86)\NSIS\makensis.exe

rem ------------------------------------
rem --- WHERE ARE PROGRAMS INSTALLED ---
rem ------------------------------------
    set EXODUS_PROGRAMFILES32=%PROGRAMS_DRIVE%\Program Files
rem if not "%ProgramFiles(x86)%" == "" set EXODUS_PROGRAMFILES32=%EXODUS_PROGRAMFILES32% (x86)
    if "%PROCESSOR_ARCHITECTURE%" == "AMD64" set EXODUS_PROGRAMFILES32=%EXODUS_PROGRAMFILES32% (x86)
    set EXODUS_PROGRAMFILES64=%PROGRAMS_DRIVE%\Program Files

rem ----- INSTALLED BOOST -----
rem ---------------------------
    set BOOST32=%BOOST_DRV%\boost_%EXODUS_BOOSTVER%
    set BOOST64=%BOOST_DRV%\boost_%EXODUS_BOOSTVER%
    IF BOOSTPRO=="YES" set BOOST32=%EXODUS_PROGRAMFILES32%\Boost\boost_%EXODUS_BOOSTVER%
    IF BOOSTPRO=="YES" set BOOST64=%EXODUS_PROGRAMFILES64%\Boost\boost_%EXODUS_BOOSTVER%

rem NOTE: boost x64 binary libs are not available so we generally build them here
rem D:\boost_1_46_1\stage64\lib
rem D:\boost_1_46_1\stage32\lib
rem exodus project build looks for libs in this order
rem --- xx=32 or 64 ---
rem \libxx
rem \stagexx\lib
rem \lib
rem \stage\lib

rem ----- POSTGRESQL -----
rem ----------------------
    set POSTGRESQL32=%EXODUS_PROGRAMFILES32%\PostgreSQL\%EXODUS_PGVERSION%
rem    set POSTGRESQL64=%EXODUS_PROGRAMFILES64%\PostgreSQL\%EXODUS_PGVERSION%
    set POSTGRESQL64=D:\pg9dev
rem solutions/projects search for includes and libs as follows:
rem \include
rem \stage\lib

rem ---------------------------------------------
rem --- SKIP TOOLSETS IF OPENING DEVELOPER UI ---
rem ---------------------------------------------
    if "%EXODUS_DEV%" == "YES" goto aftertoolsets

    if NOT "%EXODUS_TOOLSET%" == "VS2005" goto toolset2
rem ---------------------------------------------------------------
rem --- VS2005 TOOLSET                                          ---
rem ---------------------------------------------------------------

rem ----------------------------
rem --- VS2005 Configuration ---
rem ----------------------------


rem path %EXODUS_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\;%PATH%
    path %PATH%;%EXODUS_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\

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
set EXODUS_TOOLPATH=C:Windows\system32
 set EXODUS_TOOLPATHREL=%EXODUS_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT
 set EXODUS_TOOLPATHDEB=%EXODUS_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\x86\Microsoft.VC80.DebugCRT
rem set EXODUS_TOOLPATHREL=%EXODUS_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\amd64\Microsoft.VC80.CRT
rem set EXODUS_TOOLPATHDEB=%EXODUS_PROGRAMFILES32%\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\amd64\Microsoft.VC80.DebugCRT
set EXODUS_VCVERSION=80

rem ---------------------
rem --- VS2005 Redist ---
rem ---------------------

if "%TARGET_CPU%" == "x64" goto redist2005x64
rem --- NOTE: QUOTE THE URLS! BUT NOT DESC/SOURCE! ---
set REDIST_DESC=MSVC++ 2005 x86 SP1
set REDIST_SOURCE1=download.microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/e/1/c/e1c773de-73ba-494a-a5ba-f24906ecf088/vcredist_x86.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2005_x86.exe"
goto gotredist2005
:redist2005x64
set REDIST_DESC=MSVC++ 2005 x64 SP1
set REDIST_SOURCE1=download.microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/d/4/1/d41aca8a-faa5-49a7-a5f2-ea0aa4587da0/vcredist_x64.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2005_x64.exe"
:gotredist2005

goto checktoolset

:toolset2
if NOT "%EXODUS_TOOLSET%" == "XXXXXXX" goto toolset3
rem ---------------------------------------------------------------
rem --- xxxxxx TOOLSET                                          ---
rem ---------------------------------------------------------------
rem ...
goto checktoolset


:toolset3
if NOT "%EXODUS_TOOLSET%" == "XXXXXXX" goto toolset4
rem ---------------------------------------------------------------
rem --- xxxxxx TOOLSET                                          ---
rem ---------------------------------------------------------------
rem ...
goto checktoolset

:toolset4
if NOT "%EXODUS_TOOLSET%" == "XXXXXXX" goto toolset5
rem ---------------------------------------------------------------
rem --- xxxxxx TOOLSET                                          ---
rem ---------------------------------------------------------------
rem ...
goto checktoolset

:toolset5
rem -------------------------------------------------------------------------
rem --- SDK71 TOOLSET (the last one and the default)                      ---
rem -------------------------------------------------------------------------

    path %EXODUS_PROGRAMFILES32%\Microsoft SDKs\Windows\v7.1\Bin;%PATH%
rem path %PATH%;%EXODUS_PROGRAMFILES32%\Microsoft SDKs\Windows\v7.1\Bin

rem ---------------------------
rem --- SDK71 Configuration ---
rem ---------------------------
rem call setenv /x86 /debug
rem call setenv /x86 /release
rem call setenv /x64 /debug
rem call setenv /x64 /release
    call setenv /%TARGET_CPU% /%Configuration
echo on

rem ----------------------
rem --- SDK71 Binaries ---
rem ----------------------
rem Runtime version 70 80 90 100 for MSVC2003, 2005, 2008 and 2010 respectively
rem and location of msvcrNNd.dll etc c runtime dlls
set EXODUS_TOOLPATHREL=C:Windows\system32
set EXODUS_TOOLPATHDEB=C:Windows\system32
set EXODUS_VCVERSION=100

rem --------------------
rem --- SDK71 Redist ---
rem --------------------
rem 2008x86 is "http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe"

if "%TARGET_CPU%" == "x64" goto redist2010x64
rem --- NOTE: QUOTE THE URLS! and NOT DESC/SOURCE! ---
set REDIST_DESC=MSVC++ 2010 x86
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe"
set REDIST_SOURCE2=exodusdb.googlecode.com
set REDIST_URL2="http://exodusdb.googlecode.com/files/vcredist_2010_x86.exe"
goto gotredist2010
:redist2010x64
set REDIST_DESC=MSVC++ 2010 x64
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

if not "%Configuration%" == "" goto gotconfiguration
@echo MISSING CONFIGURATION ENVIRONMENT VARIABLE
if "%EXODUS_BATCHMODE%" == "" pause
exit
:gotconfiguration
@echo Configuration=%Configuration%

if not "%TARGET_CPU%" == "" goto gottargetcpu
@echo MISSING TARGET_CPU ENVIRONMENT VARIABLE
if "%EXODUS_BATCHMODE%" == "" pause
exit
:gottargetcpu
@echo TARGET_CPU=%TARGET_CPU%

:aftertoolsets

rem --------------------
rem --- Product Name ---
rem --------------------
rem Name for menus etc and basic name for installfile (without platform)
if "%TARGET_CPU%" == "x86" set EXODUS_PRODUCTNAME=Exodus
if "%TARGET_CPU%" == "x64" set EXODUS_PRODUCTNAME=Exodus64
set EXODUS_CODENAME=exodus

rem ------------------
rem ---- BUILDING ----
rem ------------------

if "%TARGET_CPU%" == "x64" set EXODUS_PLATFORM=x64
if "%TARGET_CPU%" == "x86" set EXODUS_PLATFORM=Win32
if "%TARGET_CPU%" == "x64" set EXODUS_BINARIES=x64\%Configuration%
if "%TARGET_CPU%" == "x86" set EXODUS_BINARIES=%Configuration%

rem --- SOLUTION ---
rem ----------------
rem default project
    set EXODUS_PROJECT=exodus_all
rem VS2005
    if "%EXODUS_TOOLSET%" == "VS2005" set EXODUS_PROJECT=exodus_all2005
rem VS2008 - doesnt exist but could be created from 2005 solution
    if "%EXODUS_TOOLSET%" == "VS2008" set EXODUS_PROJECT=exodus_all2008

rem --- COMMAND TO BUILD ---
rem ------------------------
rem set EXODUS_MAKE=vcbuild /p:Platform=%EXODUS_PLATFORM% /p:Configuration=%Configuration% %EXODUS_PROJECT%.sln
    set EXODUS_MAKE=msbuild /p:Platform=%EXODUS_PLATFORM% /p:Configuration=%Configuration% %EXODUS_PROJECT%.sln

rem --- COMMAND TO CLEAN ---
rem ------------------------
rem set EXODUS_CLEAN=%EXODUS_MAKE% /t:clean
    set EXODUS_CLEAN=%EXODUS_MAKE% /t:clean

rem ignore this for now
rem vcbuild /nocolor /logfile:%EXODUS_PROJECT%_vcb.log /error:ERROR: /warning:WARNING: %EXODUS_PROJECT%.sln
rem if errorlevel 2 set error=yes
rem if error==yes goto upload
rem echo vcbuild done


rem -----------------------
rem --- DEVELOPMENT UI  ---
rem -----------------------
rem search for the latest version of VS Professional or Express
    set EXODUS_VS=%VS110COMNTOOLS%
    if "%EXODUS_VS%" =="" set EXODUS_VS=%VS100COMNTOOLS%
    if "%EXODUS_VS%" =="" set EXODUS_VS=%VS90COMNTOOLS%
    if "%EXODUS_VS%" =="" set EXODUS_VS=%VS80COMNTOOLS%
if exist "%EXODUS_VS%..\IDE\devenv.exe" set EXODUS_DEV="%EXODUS_VS%..\IDE\devenv" %EXODUS_PROJECT%.sln
if exist "%EXODUS_VS%..\IDE\vcexpress.exe" set EXODUS_DEV="%EXODUS_VS%..\IDE\vcexpress" %EXODUS_PROJECT%.sln


rem ---------------------------
rem --- PACKAGING INSTALLER ---
rem ---------------------------
set EXODUS_PACK=%EXODUS_PROGRAMFILES32%\NSIS\makensis.exe
if not exist "%EXODUS_PACK%" set EXODUS_PACK=%EXODUS_PROGRAMFILES64%\NSIS\makensis.exe

if exist "%EXODUS_PACK%" goto gotnsis
@echo MISSING "%EXODUS_PACK%" PROGRAM
if "%EXODUS_BATCHMODE%" == "" pause
exit
:gotnsis
@echo EXODUS_PACK=%EXODUS_PACK%

rem ---------------------------------
rem --- COMMAND TO MAKE INSTALLER ---
rem ---------------------------------
rem there is no exodus_all2005.NSI!
set EXODUS_PACK="%EXODUS_PACK%" exodus_all.nsi

@echo BOOST32=%BOOST32%
@echo BOOST64=%BOOST64%
@echo POSTGRESQL32=%POSTGRESQL32%
@echo POSTGRESQL64=%POSTGRESQL64%
