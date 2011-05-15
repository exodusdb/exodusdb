:checkconfig
@if exist config.bat goto gotconfig
@echo Please first copy config0.bat to config.bat and 
@echo edit the your new config.bat to your configuration
pause
exit
:gotconfig

if "%EXODUS_DEV%" == "YES" goto aftersetenv

rem -------------------
rem --- BUILDER/SDK ---
rem -------------------
rem path C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin;%PATH%
    path %PATH%;C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin

rem ----------------------------------
rem --- Platform and Configuration ---
rem ----------------------------------
rem call setenv /x86 /debug
rem call setenv /x86 /release
rem call setenv /x64 /debug
 call setenv /x64 /release

:aftersetenv

rem ------------------------
rem --- Binary Toolset ---
rem ------------------------
rem Runtime version 70 80 90 100 for MSVC2003, 2005, 2008 and 2010 respectively
rem and location of msvcrNNd.dll etc c runtime dlls
set EXODUS_VCVERSION=100
set EXODUS_TOOLPATH=C:Windows\system32

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
rem set BOOST32=C:\Program Files\Boost\boost_1_46_1
rem Built Binaries in something\stage32
    set BOOST32=F:\boost_1_46_1

rem ----- BOOST64 -----
rem -------------------
rem binary installers are not available so we have built boost x64 libs (in stage64)
    set BOOST64=F:\boost_1_46_1


rem ----- POSTGRESQL32 -----
rem ------------------------
rem on Win32, postgres is installed here
    set POSTGRESQL32=C:\Program Files\PostgreSQL\9.0

rem but on win/64, postgres is installed here
    if NOT "%ProgramFiles(x86)%" == "" set POSTGRESQL32=C:\Program Files (x86)\PostgreSQL\9.0

rem ----- POSTGRESQL64 -----
rem ------------------------
    set POSTGRESQL64=C:\Program Files\PostgreSQL\9.0

rem ------------------
rem ---- BUILDING ----
rem ------------------
if "%TARGET_CPU%" == "x64" set EXODUS_PLATFORM=x64
if "%TARGET_CPU%" == "x86" set EXODUS_PLATFORM=Win32
if "%TARGET_CPU%" == "x64" set EXODUS_BINARIES=x64\%Configuration%
if "%TARGET_CPU%" == "x86" set EXODUS_BINARIES=%Configuration%

rem --- Solution ---
rem ----------------
    set EXODUS_PROJECT=exodus_all

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
rem set NSIS_PATH=C:\Program Files\NSIS
    set NSIS_PATH=C:\Program Files (x86)\NSIS

