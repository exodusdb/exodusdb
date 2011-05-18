:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

    set TARGET_CPU=x86
rem set TARGET_CPU=x64
    set Configuration=Release
rem set Configuration=Debug

rem --------------
rem --- ADVICE ---
rem --------------
rem if you hardcode drive paths here then search for :\ to make sure you dont miss any
rem best to install all "programs" eg sdk, VS, postgresql, nsis etc on one drive
rem and to the *standard path structure* even if you install them not on C:
set PROGRAM_DRIVE=C:
rem best to install all building stuff like exodus/boost libraries etc on one drive
rem maybe the same as the programs, maybe not.
set BUILD_DRIVE=F:

if "%EXODUS_DEV%" == "YES" goto aftersetenv

rem set EXODUS_GENERAL=VS2005
 set EXODUS_GENERAL=SDK71


rem ---------------------------------------------------------------
rem --- VS2005                                                  ---
rem ---------------------------------------------------------------

if NOT "%EXODUS_GENERAL%" == "VS2005" goto defaultbuilder

rem path %PROGRAM_DRIVE%\Program Files\Microsoft Visual Studio 8\VC\;%PATH%
    path %PATH%;%PROGRAM_DRIVE%\Program Files\Microsoft Visual Studio 8\VC\

rem --- x86 command prompt ---
 call vcvarsall.bat x86
 set Configuration=Release
 set TARGET_CPU=x86

rem --- VS2005 cross compiler ---
rem call vcvarsall.bat x86_amd64
rem set Configuration=Release
rem set TARGET_CPU=x64

rem ------------------------
rem --- Binary Toolset ---
rem ------------------------
rem Runtime version 70 80 90 100 for MSVC2003, 2005, 2008 and 2010 respectively
rem and location of msvcrNNd.dll etc c runtime dlls
set EXODUS_TOOLPATH=C:Windows\system32
 set EXODUS_TOOLPATHREL=%PROGRAM_DRIVE%\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT
 set EXODUS_TOOLPATHDEB=%PROGRAM_DRIVE%\Program Files\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\x86\Microsoft.VC80.DebugCRT
rem set EXODUS_TOOLPATHREL=%PROGRAM_DRIVE%\Program Files\Microsoft Visual Studio 8\VC\redist\amd64\Microsoft.VC80.CRT
rem set EXODUS_TOOLPATHDEB=%PROGRAM_DRIVE%\Program Files\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\amd64\Microsoft.VC80.DebugCRT
set EXODUS_VCVERSION=80

rem -----------------------------
rem --- 2005 Redistributables ---
rem -----------------------------

if "%TARGET_CPU%" == "x64" goto redist2005x64
rem --- NOTE: QUOTE THE URLS! and NOT DESC/SOURCE! ---
set REDIST_DESC=MSVC++ 2005 x86 SP1
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/e/1/c/e1c773de-73ba-494a-a5ba-f24906ecf088/vcredist_x86.exe"
goto gotredist2005
:redist2005x64
set REDIST_DESC=MSVC++ 2005 x64 SP1
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/d/4/1/d41aca8a-faa5-49a7-a5f2-ea0aa4587da0/vcredist_x64.exe"
:gotredist2005
set REDIST_SOURCE2=0
set REDIST_URL2=0

goto gototoolset


:defaultbuilder
rem -------------------------------------------------------------------------
rem --- SDK 7.1 the default                                               ---
rem -------------------------------------------------------------------------

    path %PROGRAM_DRIVE%\Program Files\Microsoft SDKs\Windows\v7.1\Bin;%PATH%
rem path %PATH%;%PROGRAM_DRIVE%\Program Files\Microsoft SDKs\Windows\v7.1\Bin

rem ----------------------------------
rem --- Platform and Configuration ---
rem ----------------------------------
rem call setenv /x86 /debug
rem call setenv /x86 /release
rem call setenv /x64 /debug
rem call setenv /x64 /release
    call setenv /%TARGET_CPU% /%Configuration
echo on

rem ------------------------
rem --- Binary Toolset ---
rem ------------------------
rem Runtime version 70 80 90 100 for MSVC2003, 2005, 2008 and 2010 respectively
rem and location of msvcrNNd.dll etc c runtime dlls
set EXODUS_TOOLPATHREL=C:Windows\system32
set EXODUS_TOOLPATHDEB=C:Windows\system32
set EXODUS_VCVERSION=100

rem -----------------------------
rem --- 2010 Redistributables ---
rem -----------------------------
rem 2008x86 is "http://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe"

if "%TARGET_CPU%" == "x64" goto redist2010x64
rem --- NOTE: QUOTE THE URLS! and NOT DESC/SOURCE! ---
set REDIST_DESC=MSVC++ 2010 x86
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe"
goto gotredist2010
:redist2010x64
set REDIST_DESC=MSVC++ 2010 x64
set REDIST_SOURCE1=Microsoft.com
set REDIST_URL1="http://download.microsoft.com/download/3/2/2/3224B87F-CFA0-4E70-BDA3-3DE650EFEBA5/vcredist_x64.exe"
:gotredist2010
@echo REDIST_DESC=%REDIST_DESC%
@echo REDIST_SOURCE1=%REDIST_SOURCE1%
@echo REDIST_URL1=%REDIST_URL1%
set REDIST_SOURCE2=0
set REDIST_URL2=0

:gottoolset

:sanitychecks

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

:aftersetenv

rem ------------------------
rem --- Product Version  ---
rem ------------------------
set EXODUS_MINOR_VERSION=11.5
set EXODUS_MICRO_VERSION=11.5.3

rem --------------------
rem --- Product Name ---
rem --------------------
rem Name for menus etc and basic name for installfile (without platform)
if "%TARGET_CPU%" == "x86" set EXODUS_PRODUCTNAME=Exodus
if "%TARGET_CPU%" == "x64" set EXODUS_PRODUCTNAME=Exodus64
set EXODUS_CODENAME=exodus

rem ----- BOOST32 -----
rem -------------------
rem uncomment if you have binaries here but best to build from scratch to avoid 0xc0150002 error
rem Downloaded Binaries
rem set BOOST32=%PROGRAM_DRIVE%\Program Files\Boost\boost_1_46_1
rem set BOOST32=%PROGRAM_DRIVE%\Program Files (x86)\Boost\boost_1_46_1
rem Built Binaries in something\stage32
    set BOOST32=%BUILD_DRIVE%\boost_1_46_1

rem ----- BOOST64 -----
rem -------------------
rem binary installers are not available so we have built boost x64 libs (in stage64)
    set BOOST64=%BUILD_DRIVE%\boost_1_46_1


rem ----- POSTGRESQL32 -----
rem ------------------------
rem on Win32, postgres is installed here
    set POSTGRESQL32=%PROGRAM_DRIVE%\Program Files\PostgreSQL\9.0

rem but on win/64, postgres is installed here
    if NOT "%ProgramFiles(x86)%" == "" set POSTGRESQL32=%PROGRAM_DRIVE%\Program Files (x86)\PostgreSQL\9.0

rem ----- POSTGRESQL64 -----
rem ------------------------
    set POSTGRESQL64=%PROGRAM_DRIVE%\Program Files\PostgreSQL\9.0

rem ------------------
rem ---- BUILDING ----
rem ------------------
if "%TARGET_CPU%" == "x64" set EXODUS_PLATFORM=x64
if "%TARGET_CPU%" == "x86" set EXODUS_PLATFORM=Win32
if "%TARGET_CPU%" == "x64" set EXODUS_BINARIES=x64\%Configuration%
if "%TARGET_CPU%" == "x86" set EXODUS_BINARIES=%Configuration%

rem --- Solution ---
rem ----------------
rem default prohect
    set EXODUS_PROJECT=exodus_all
rem VS2005
    if "%EXODUS_GENERAL%" == "VS2005" set EXODUS_PROJECT=exodus_all2005

rem --- Build ---
rem -------------
rem set EXODUS_MAKE=vcbuild /p:Platform=%EXODUS_PLATFORM% /p:Configuration=%Configuration% %EXODUS_PROJECT%.sln
    set EXODUS_MAKE=msbuild /p:Platform=%EXODUS_PLATFORM% /p:Configuration=%Configuration% %EXODUS_PROJECT%.sln

rem --- Clean ---
rem -------------
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
set EXODUS_PACK=%PROGRAM_DRIVE%\Program Files (x86)\NSIS\makensis.exe
if not exist "%EXODUS_PACK%" set EXODUS_PACK=%PROGRAM_DRIVE%\Program Files\NSIS\makensis.exe

if exist "%EXODUS_PACK%" goto gotnsis
@echo MISSING "%EXODUS_PACK%" PROGRAM
if "%EXODUS_BATCHMODE%" == "" pause
exit
:gotnsis
@echo EXODUS_PACK=%EXODUS_PACK%

rem ------------------------
rem --- INSTALLER SCRIPT ---
rem ------------------------
rem there is no exodus_all2005.nsi
set EXODUS_PACK="%EXODUS_PACK%" exodus_all.nsi

