@echo on
rem xp 2008 win7
set SETENV_TARGET=2008
set SOURCEDRV=F:
set SOURCEDIR="\icu4c-4_6_1-src\icu\source\"

%SOURCEDRV%
cd %SOURCEDIR%

set SETENV_PLATFORMS=x86 x64
set CONFIGURATIONS=Release Debug
set ACTIONS=clean build
rem set ACTIONS=build
set SUBDIRS=common i18n

if "%SETENV_TARGET%" EQU "xp"   set BUILD_TARGET=v2.0
if "%SETENV_TARGET%" EQU "2008" set BUILD_TARGET=v3.5
if "%SETENV_TARGET%" EQU "win7" set BUILD_TARGET=v4.0

setlocal enableextensions enabledelayedexpansion
if errorlevel==1 pause
set BUILD_PLATFORM=

for %%C in (%CONFIGURATIONS%) do (

 for %%P in (%SETENV_PLATFORMS%) do (

  call setenv /%%P /%%C /%SETENV_TARGET%

  rem this allows x64 debug builds on SDK7.1 if VS2010 is available
  rem must be after setenv above
  if "%%C" EQU "Debug" if "%SETENV_TARGET%" == "win7" vsvars32 %%P

  echo on
  set BUILD_PLATFORM=""
  if "%%P" EQU "x86"  set BUILD_PLATFORM=win32
  if "%%P" EQU "x64"  set BUILD_PLATFORM=x64

  echo SETENV_PLATFORM="%%P" BUILD_PLATFORM="!BUILD_PLATFORM!"
  if "!BUILD_PLATFORM!" EQU "" (
   echo "unknown SETENV_PLATFORM - should be x86 or x64"
   pause
   goto end
  )

  for %%D in (%SUBDIRS%) do (

   cd %SOURCEDIR%\%%D
 
   echo -----------
   echo %%D %%P %%C
   echo -----------
   for %%A in (%ACTIONS%)  do (
    echo msbuild /p:Platform=!BUILD_PLATFORM! /p:Configuration=%%C;TargetFramework=%BUILD_TARGET% /t:%%A
         msbuild /p:Platform=!BUILD_PLATFORM! /p:Configuration=%%C;TargetFramework=%BUILD_TARGET% /t:%%A
   )
  )
 )
)
:end
pause