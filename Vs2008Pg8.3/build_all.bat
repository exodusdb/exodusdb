rem AUTOMATED VCBUILD
rem http://msdn.microsoft.com/en-us/library/cz553aa1%28v=VS.80%29.aspx

rem --- CONFIGURE VCVARS AND MAKENSIS LOCATIONS ---
path c:\program files\microsoft visual studio 8\vc\;%PATH%
path D:\Program Files\NSIS\;%PATH%

rem -- CONFIGURE PROJECT NAME --
set PROJECT=exodus_all

set error=no
if exist %PROJECT%_vcb.log del %PROJECT%_vcb.log
if exist %PROJECT%_nsi.log del %PROJECT%_nsi.log

call vcvarsall
if errorlevel 1 set error=yes
if error==yes goto upload

vcbuild /nocolor /logfile:%PROJECT%_vcb.log /error:ERROR: /warning:WARNING: %PROJECT%.sln
if errorlevel 2 set error=yes
if error==yes goto upload
echo vcbuild done

rem /V2 errors and warnings
MAKENSIS.exe /O%PROJECT%_nsi.log %PROJECT%.nsi
if errorlevel 1 set error=yes
if error==yes goto upload
echo makensis done

:upload
echo upload