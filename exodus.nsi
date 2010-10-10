
!define debugorrelease "debug"

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  ;Name "Modern UI Test"
  Name "Exodus 10.10"
  Outfile "exodus-10.10.exe"

  ;Default installation folder
  ;InstallDir "$PROGRAMFILES\Modern UI Test"
  InstallDir "$PROGRAMFILES\exodus\10.10"
  
  ;Get installation folder from registry if available
  ;InstallDirRegKey HKCU "Software\Modern UI Test" ""
  InstallDirRegKey HKCU "Software\exodus\10.10" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  ;!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Contrib\Modern UI\License.txt"
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "All" SecAll

;;;;;;;;;;;;; DEBUG OR RELEASE !!!
;StrCpy $DebugOrRelease "debug"

  ;create a GLOBAL.END file to try and shutdown NEOSYS processes
  ;will be deleted at the end
  ;FileOpen $0 $INSTDIR\neosys\GLOBAL.END w
  ;FileClose $0

  SetOutPath "$INSTDIR\bin"

  ;if this file isnt built then fail  
  File ${DebugOrRelease}\compile.exe

  ;File release\libpq.dll
  ;File release\libeay32.dll
  ;File release\ssleay32.dll
  ;File release\libintl-8.dll
  ;dont do exodus.dll since we may want the debug version - below
  File /x exodus.dll release\*.dll
  File release\nano.exe
  File release\nanorc
  
  File ${DebugOrRelease}\*.exe
  File ${DebugOrRelease}\exodus.dll

  File /oname=pgexodus-8.dll release\pgexodus-8.dll
  File /oname=pgexodus-9.dll release\pgexodus.dll

  SetOutPath "$INSTDIR\lib"

  File ${DebugOrRelease}\exodus.exp
  File ${DebugOrRelease}\exodus.lib
  ;File ${DebugOrRelease}\exodus.pdb

  SetOutPath "$INSTDIR\include\exodus"

  File exodus\exodus\exodus\mv*.h
  File exodus\exodus\exodus\exodus*.h

  SetOutPath "$INSTDIR\projects"

  File cli\src\compile.cpp
  File cli\src\createfile.cpp
  File cli\src\createindex.cpp
  File cli\src\delete.cpp
  File cli\src\deletefile.cpp
  File cli\src\deleteindex.cpp
  File cli\src\edic.cpp
  File cli\src\edit.cpp
  File cli\src\list.cpp
  File cli\src\listfiles.cpp
  File cli\src\listindexes.cpp
  File cli\src\testsort.cpp

  ;example of recursive with excludes
  ;File /r /x neosys*.pdb /x imc*.* neosys.net\*.*

  ;Store installation folder
  WriteRegStr HKCU "Software\exodus\10.10" "" $INSTDIR
  
  createDirectory "$SMPROGRAMS\Exodus-10.10"
  createShortCut "$SMPROGRAMS\Exodus-10.10\Exodus Console.lnk" "$INSTDIR\bin\exodus.exe"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  # create a shortcut named "new shortcut" in the start menu programs directory
  # point the new shortcut at the program uninstaller
  createShortCut "$SMPROGRAMS\Exodus-10.10\Uninstall Exodus.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecAll ${LANG_ENGLISH} "All Software"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAll} $(DESC_SecAll)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\bin\*.*"
  RMDir "$INSTDIR\bin"

  Delete "$INSTDIR\lib\exodus.*"
  RMDir "$INSTDIR\lib"

  Delete "$INSTDIR\include\exodus\mv*.h"
  Delete "$INSTDIR\include\exodus\exodus*.h"
  RMDir "$INSTDIR\include\exodus"
  RMDir "$INSTDIR\include"

  Delete "$INSTDIR\projects\compile.cpp"
  Delete "$INSTDIR\projects\createfile.cpp"
  Delete "$INSTDIR\projects\createindex.cpp"
  Delete "$INSTDIR\projects\delete.cpp"
  Delete "$INSTDIR\projects\deletefile.cpp"
  Delete "$INSTDIR\projects\deleteindex.cpp"
  Delete "$INSTDIR\projects\edic.cpp"
  Delete "$INSTDIR\projects\edit.cpp"
  Delete "$INSTDIR\projects\list.cpp"
  Delete "$INSTDIR\projects\listfiles.cpp"
  Delete "$INSTDIR\projects\listindexes.cpp"
  Delete "$INSTDIR\projects\testsort.cpp"

  Delete "$INSTDIR\projects\blanksolution\*.*"
  RMDir "$INSTDIR\projects\blanksolution"
  RMDir "$INSTDIR\projects"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\exodus\10.10"

  # second, remove the link from the start menu
  delete "$SMPROGRAMS\Exodus-10.10\Uninstall Exodus.lnk"

  delete "$SMPROGRAMS\Exodus-10.10\Exodus Console.lnk"
  RMDir "$SMPROGRAMS\Exodus-10.10"

SectionEnd