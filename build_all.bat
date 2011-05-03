rem AUTOMATED VCBUILD
rem http://msdn.microsoft.com/en-us/library/cz553aa1%28v=VS.80%29.aspx

rem --- CONFIGURE VCVARS AND MAKENSIS LOCATIONS ---
path %VS80COMNTOOLS%..\..\vc\;%PATH%
path %VS90COMNTOOLS%..\..\vc\;%PATH%
path %VS100COMNTOOLS%..\..\vc\;%PATH%
path %VS110COMNTOOLS%..\..\vc\;%PATH%

rem path \Program Files\NSIS\;%PATH%
path \Program Files (x86)\NSIS\;%PATH%

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