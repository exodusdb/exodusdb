;initial code is just standard nsis macros that doesnt need to be changed or understood
;exodus build instructions at at the bottom, or search for exodus

/**
 *  EnvVarUpdate.nsh
 *    : Environmental Variables: append, prepend, and remove entries
 *
 *     WARNING: If you use StrFunc.nsh header then include it before this file
 *              with all required definitions. This is to avoid conflicts
 *
 *  Usage:
 *    ${EnvVarUpdate} "ResultVar" "EnvVarName" "Action" "RegLoc" "PathString"
 *
 *  Credits:
 *  Version 1.0 
 *  * Cal Turney (turnec2)
 *  * Amir Szekely (KiCHiK) and e-circ for developing the forerunners of this
 *    function: AddToPath, un.RemoveFromPath, AddToEnvVar, un.RemoveFromEnvVar,
 *    WriteEnvStr, and un.DeleteEnvStr
 *  * Diego Pedroso (deguix) for StrTok
 *  * Kevin English (kenglish_hi) for StrContains
 *  * Hendri Adriaens (Smile2Me), Diego Pedroso (deguix), and Dan Fuhry  
 *    (dandaman32) for StrReplace
 *
 *  Version 1.1 (compatibility with StrFunc.nsh)
 *  * techtonik
 *
 *  http://nsis.sourceforge.net/Environmental_Variables:_append%2C_prepend%2C_and_remove_entries
 *
 */
 
 
!ifndef ENVVARUPDATE_FUNCTION
!define ENVVARUPDATE_FUNCTION
!verbose push
!verbose 3
!include "LogicLib.nsh"
!include "WinMessages.NSH"
!include "StrFunc.nsh"
 
; ---- Fix for conflict if StrFunc.nsh is already includes in main file -----------------------
!macro _IncludeStrFunction StrFuncName
  !ifndef ${StrFuncName}_INCLUDED
    ${${StrFuncName}}
  !endif
  !ifndef Un${StrFuncName}_INCLUDED
    ${Un${StrFuncName}}
  !endif
  !define un.${StrFuncName} "${Un${StrFuncName}}"
!macroend
 
!insertmacro _IncludeStrFunction StrTok
!insertmacro _IncludeStrFunction StrStr
!insertmacro _IncludeStrFunction StrRep
 
; ---------------------------------- Macro Definitions ----------------------------------------
!macro _EnvVarUpdateConstructor ResultVar EnvVarName Action Regloc PathString
  Push "${EnvVarName}"
  Push "${Action}"
  Push "${RegLoc}"
  Push "${PathString}"
    Call EnvVarUpdate
  Pop "${ResultVar}"
!macroend
!define EnvVarUpdate '!insertmacro "_EnvVarUpdateConstructor"'
 
!macro _unEnvVarUpdateConstructor ResultVar EnvVarName Action Regloc PathString
  Push "${EnvVarName}"
  Push "${Action}"
  Push "${RegLoc}"
  Push "${PathString}"
    Call un.EnvVarUpdate
  Pop "${ResultVar}"
!macroend
!define un.EnvVarUpdate '!insertmacro "_unEnvVarUpdateConstructor"'
; ---------------------------------- Macro Definitions end-------------------------------------
 
;----------------------------------- EnvVarUpdate start----------------------------------------
!define hklm_all_users     'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
!define hkcu_current_user  'HKCU "Environment"'
 
!macro EnvVarUpdate UN
 
Function ${UN}EnvVarUpdate
 
  Push $0
  Exch 4
  Exch $1
  Exch 3
  Exch $2
  Exch 2
  Exch $3
  Exch
  Exch $4
  Push $5
  Push $6
  Push $7
  Push $8
  Push $9
  Push $R0
 
  /* After this point:
  -------------------------
     $0 = ResultVar     (returned)
     $1 = EnvVarName    (input)
     $2 = Action        (input)
     $3 = RegLoc        (input)
     $4 = PathString    (input)
     $5 = Orig EnvVar   (read from registry)
     $6 = Len of $0     (temp)
     $7 = tempstr1      (temp)
     $8 = Entry counter (temp)
     $9 = tempstr2      (temp)
     $R0 = tempChar     (temp)  */
 
  ; Step 1:  Read contents of EnvVarName from RegLoc
  ;
  ; Check for empty EnvVarName
  ${If} $1 == ""
    SetErrors
    DetailPrint "ERROR: EnvVarName is blank"
    Goto EnvVarUpdate_Restore_Vars
  ${EndIf}
 
  ; Check for valid Action
  ${If}    $2 != "A"
  ${AndIf} $2 != "P"
  ${AndIf} $2 != "R"
    SetErrors
    DetailPrint "ERROR: Invalid Action - must be A, P, or R"
    Goto EnvVarUpdate_Restore_Vars
  ${EndIf}
 
  ${If} $3 == HKLM
    ReadRegStr $5 ${hklm_all_users} $1     ; Get EnvVarName from all users into $5
  ${ElseIf} $3 == HKCU
    ReadRegStr $5 ${hkcu_current_user} $1  ; Read EnvVarName from current user into $5
  ${Else}
    SetErrors
    DetailPrint 'ERROR: Action is [$3] but must be "HKLM" or HKCU"'
    Goto EnvVarUpdate_Restore_Vars
  ${EndIf}
 
  ; Check for empty PathString
  ${If} $4 == ""
    SetErrors
    DetailPrint "ERROR: PathString is blank"
    Goto EnvVarUpdate_Restore_Vars
  ${EndIf}
 
  ; Make sure we've got some work to do
  ${If} $5 == ""
  ${AndIf} $2 == "R"
    SetErrors
    DetailPrint "$1 is empty - Nothing to remove"
    Goto EnvVarUpdate_Restore_Vars
  ${EndIf}
 
  ; Step 2: Scrub EnvVar
  ;
  StrCpy $0 $5                             ; Copy the contents to $0
  ; Remove spaces around semicolons (NOTE: spaces before the 1st entry or
  ; after the last one are not removed here but instead in Step 3)
  ${If} $0 != ""                           ; If EnvVar is not empty ...
    ${Do}
      ${${UN}StrStr} $7 $0 " ;"
      ${If} $7 == ""
        ${ExitDo}
      ${EndIf}
      ${${UN}StrRep} $0  $0 " ;" ";"         ; Remove '<space>;'
    ${Loop}
    ${Do}
      ${${UN}StrStr} $7 $0 "; "
      ${If} $7 == ""
        ${ExitDo}
      ${EndIf}
      ${${UN}StrRep} $0  $0 "; " ";"         ; Remove ';<space>'
    ${Loop}
    ${Do}
      ${${UN}StrStr} $7 $0 ";;" 
      ${If} $7 == ""
        ${ExitDo}
      ${EndIf}
      ${${UN}StrRep} $0  $0 ";;" ";"
    ${Loop}
 
    ; Remove a leading or trailing semicolon from EnvVar
    StrCpy  $7  $0 1 0
    ${If} $7 == ";"
      StrCpy $0  $0 "" 1                   ; Change ';<EnvVar>' to '<EnvVar>'
    ${EndIf}
    StrLen $6 $0
    IntOp $6 $6 - 1
    StrCpy $7  $0 1 $6
    ${If} $7 == ";"
     StrCpy $0  $0 $6                      ; Change ';<EnvVar>' to '<EnvVar>'
    ${EndIf}
    ; DetailPrint "Scrubbed $1: [$0]"      ; Uncomment to debug
  ${EndIf}
 
  /* Step 3. Remove all instances of the target path/string (even if "A" or "P")
     $6 = bool flag (1 = found and removed PathString)
     $7 = a string (e.g. path) delimited by semicolon(s)
     $8 = entry counter starting at 0
     $9 = copy of $0
     $R0 = tempChar      */
 
  ${If} $5 != ""                           ; If EnvVar is not empty ...
    StrCpy $9 $0
    StrCpy $0 ""
    StrCpy $8 0
    StrCpy $6 0
 
    ${Do}
      ${${UN}StrTok} $7 $9 ";" $8 "0"      ; $7 = next entry, $8 = entry counter
 
      ${If} $7 == ""                       ; If we've run out of entries,
        ${ExitDo}                          ;    were done
      ${EndIf}                             ;
 
      ; Remove leading and trailing spaces from this entry (critical step for Action=Remove)
      ${Do}
        StrCpy $R0  $7 1
        ${If} $R0 != " "
          ${ExitDo}
        ${EndIf}
        StrCpy $7   $7 "" 1                ;  Remove leading space
      ${Loop}
      ${Do}
        StrCpy $R0  $7 1 -1
        ${If} $R0 != " "
          ${ExitDo}
        ${EndIf}
        StrCpy $7   $7 -1                  ;  Remove trailing space
      ${Loop}
      ${If} $7 == $4                       ; If string matches, remove it by not appending it
        StrCpy $6 1                        ; Set 'found' flag
      ${ElseIf} $7 != $4                   ; If string does NOT match
      ${AndIf}  $0 == ""                   ;    and the 1st string being added to $0,
        StrCpy $0 $7                       ;    copy it to $0 without a prepended semicolon
      ${ElseIf} $7 != $4                   ; If string does NOT match
      ${AndIf}  $0 != ""                   ;    and this is NOT the 1st string to be added to $0,
        StrCpy $0 $0;$7                    ;    append path to $0 with a prepended semicolon
      ${EndIf}                             ;
 
      IntOp $8 $8 + 1                      ; Bump counter
    ${Loop}                                ; Check for duplicates until we run out of paths
  ${EndIf}
 
  ; Step 4:  Perform the requested Action
  ;
  ${If} $2 != "R"                          ; If Append or Prepend
    ${If} $6 == 1                          ; And if we found the target
      DetailPrint "Target is already present in $1. It will be removed and"
    ${EndIf}
    ${If} $0 == ""                         ; If EnvVar is (now) empty
      StrCpy $0 $4                         ;   just copy PathString to EnvVar
      ${If} $6 == 0                        ; If found flag is either 0
      ${OrIf} $6 == ""                     ; or blank (if EnvVarName is empty)
        DetailPrint "$1 was empty and has been updated with the target"
      ${EndIf}
    ${ElseIf} $2 == "A"                    ;  If Append (and EnvVar is not empty),
      StrCpy $0 $0;$4                      ;     append PathString
      ${If} $6 == 1
        DetailPrint "appended to $1"
      ${Else}
        DetailPrint "Target was appended to $1"
      ${EndIf}
    ${Else}                                ;  If Prepend (and EnvVar is not empty),
      StrCpy $0 $4;$0                      ;     prepend PathString
      ${If} $6 == 1
        DetailPrint "prepended to $1"
      ${Else}
        DetailPrint "Target was prepended to $1"
      ${EndIf}
    ${EndIf}
  ${Else}                                  ; If Action = Remove
    ${If} $6 == 1                          ;   and we found the target
      DetailPrint "Target was found and removed from $1"
    ${Else}
      DetailPrint "Target was NOT found in $1 (nothing to remove)"
    ${EndIf}
    ${If} $0 == ""
      DetailPrint "$1 is now empty"
    ${EndIf}
  ${EndIf}
 
  ; Step 5:  Update the registry at RegLoc with the updated EnvVar and announce the change
  ;
  ClearErrors
  ${If} $3  == HKLM
    WriteRegExpandStr ${hklm_all_users} $1 $0     ; Write it in all users section
  ${ElseIf} $3 == HKCU
    WriteRegExpandStr ${hkcu_current_user} $1 $0  ; Write it to current user section
  ${EndIf}
 
  IfErrors 0 +4
    MessageBox MB_OK|MB_ICONEXCLAMATION "Could not write updated $1 to $3"
    DetailPrint "Could not write updated $1 to $3"
    Goto EnvVarUpdate_Restore_Vars
 
  ; "Export" our change
  SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
 
  EnvVarUpdate_Restore_Vars:
  ;
  ; Restore the user's variables and return ResultVar
  Pop $R0
  Pop $9
  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Push $0  ; Push my $0 (ResultVar)
  Exch
  Pop $0   ; Restore his $0
 
FunctionEnd
 
!macroend   ; EnvVarUpdate UN
!insertmacro EnvVarUpdate ""
!insertmacro EnvVarUpdate "un."
;----------------------------------- EnvVarUpdate end----------------------------------------
 
!verbose pop
!endif














;Here starts Exodus

SetCompressor /SOLID LZMA

;see following how to install for all users and still support uninstallation
;http://nsis.sourceforge.net/Shortcuts_removal_fails_on_Windows_Vista
;RequestExecutionLevel user
RequestExecutionLevel admin #NOTE: You still need to check user rights with UserInfo!

!define debugorrelease "release"

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  Name "Exodus 11.05.1/64"
  Outfile "exodus-11.05.1-64.exe"
  InstallDir "$PROGRAMFILES\exodus\11.05"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\exodus\11.05" ""

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

  ;should we install this in order to get access to postgres on another server?
  ;File release\libpq.dll
  ;File release\libeay32.dll
  ;File release\ssleay32.dll
  ;File release\libintl-8.dll

;  ;vc2005 should autodownloaded
;  ;redist for vs2005 msvcp80.dll but not msvcp80d.dll
;  ;http://download.microsoft.com/download/d/3/4/d342efa6-3266-4157-a2ec-5174867be706/vcredist_x86.exe
;  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll"
;  File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll"
;
;  ;vc2005 debug
;  ;File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\Debug_NonRedist\x86\Microsoft.VC80.DebugCRT\msvcp80d.dll"

  ;vc2010 x86 - could download
  ;http://www.microsoft.com/downloads/info.aspx?na=41&SrcFamilyId=A7B7A05E-6DE6-4D3A-A423-37BF0912DB84&SrcDisplayLang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f5%2fB%2fC%2f5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E%2fvcredist_x86.exe
  ;File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcp100.dll"
  ;File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll"

  ;vc2010 x86 debug - no download available
  ;File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\Debug_NonRedist\x86\Microsoft.VC100.DebugCRT\msvcp100d.dll"
  ;File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\Debug_NonRedist\x86\Microsoft.VC100.DebugCRT\msvcr100d.dll"
  
  ;vc2010 x64 - could download
  ;http://www.microsoft.com/downloads/info.aspx?na=41&SrcFamilyId=BD512D9E-43C8-4655-81BF-9350143D5867&SrcDisplayLang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f3%2f2%2f2%2f3224B87F-CFA0-4E70-BDA3-3DE650EFEBA5%2fvcredist_x64.exe
  File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcp100.dll"
  File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll"

  ;vc2010 x64 debug - no download available
  File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\Debug_NonRedist\x64\Microsoft.VC100.DebugCRT\msvcp100d.dll"
  File "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\Debug_NonRedist\x64\Microsoft.VC100.DebugCRT\msvcr100d.dll"

  ;dont do exodus.dll since we may want the debug version - below
  File /x exodus.dll release\*.dll
  File release\nano.exe
  File release\nanorc
  
  File ${DebugOrRelease}\*.exe
  File ${DebugOrRelease}\exodus.dll

  SetOutPath "$INSTDIR\lib"

  File ${DebugOrRelease}\exodus.exp
  File ${DebugOrRelease}\exodus.lib
  ;File ${DebugOrRelease}\exodus.pdb

  File /oname=pgexodus-8.dll release\pgexodus-8.dll
  File /oname=pgexodus-9.dll release\pgexodus.dll

  SetOutPath "$INSTDIR\include\exodus"

  File exodus\exodus\exodus\mv*.h
  File exodus\exodus\exodus\exodus*.h
  File exodus\exodus\exodus\program.h
  File exodus\exodus\exodus\mvprogram.h
  File exodus\exodus\exodus\dict.h
  File exodus\exodus\exodus\library.h
  File exodus\exodus\exodus\xfunctor*.h

  SetOutPath "$INSTDIR\projects"

  File cli\src\compile.cpp
  File cli\src\createfile.cpp
  File cli\src\createindex.cpp
  File cli\src\delete.cpp
  File cli\src\deletefile.cpp
  File cli\src\deleteindex.cpp
  File cli\src\edic.cpp
  File cli\src\edir.cpp
  File cli\src\list.cpp
  File cli\src\printtext.h
  File cli\src\listfiles.cpp
  File cli\src\listindexes.cpp
  File cli\src\testsort.cpp
  File cli\src\configexodus.cpp

  ;example of recursive with excludes
  ;File /r /x neosys*.pdb /x imc*.* neosys.net\*.*

  ;append PATH to the binaries in system environment
  ;make available to all users
  ;dont risk prepending because could overwrite system commands
  ;http://nsis.sourceforge.net/Path_Manipulation
  ;Push $INSTDIR
  ;Call AddToPath
  ;${EnvVarUpdate} $0 "PATH" "P" "HKLM" "$INSTDIR\bin"
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\bin"

  SetShellVarContext all

  ;cannot see to add %APPDATA% in local machine path so have to do for current user only :(
  ${EnvVarUpdate} $0 "PATH" "A" "HKCU" "$APPDATA\Exodus"

  ;Store installation folder
  WriteRegStr HKCU "Software\exodus\11.05" "" $INSTDIR
  
  createDirectory "$SMPROGRAMS\Exodus-11.05"
  createShortCut "$SMPROGRAMS\Exodus-11.05\Exodus Console.lnk" "$INSTDIR\bin\exodus.exe"
  createShortCut "$SMPROGRAMS\Exodus-11.05\Exodus Config.lnk" "$INSTDIR\bin\configexodus.exe"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  # create a shortcut named "new shortcut" in the start menu programs directory
  # point the new shortcut at the program uninstaller
  createShortCut "$SMPROGRAMS\Exodus-11.05\Uninstall Exodus.lnk" "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Exodus-11.05" \
   "DisplayName" "Exodus-11.05 (remove only)"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Exodus-11.05" \
   "UninstallString" "$INSTDIR\Uninstall.exe"

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
  Delete "$INSTDIR\lib\pgexodus*.*"
  RMDir "$INSTDIR\lib"

  Delete "$INSTDIR\include\exodus\mv*.h"
  Delete "$INSTDIR\include\exodus\exodus*.h"
  Delete "$INSTDIR\include\exodus\program.h"
  Delete "$INSTDIR\include\exodus\dict.h"
  Delete "$INSTDIR\include\exodus\mvprogram.h"
  Delete "$INSTDIR\include\exodus\library.h"
  Delete "$INSTDIR\include\exodus\xfunctor*.h"
  RMDir "$INSTDIR\include\exodus"
  RMDir "$INSTDIR\include"

  Delete "$INSTDIR\projects\compile.cpp"
  Delete "$INSTDIR\projects\createfile.cpp"
  Delete "$INSTDIR\projects\createindex.cpp"
  Delete "$INSTDIR\projects\delete.cpp"
  Delete "$INSTDIR\projects\deletefile.cpp"
  Delete "$INSTDIR\projects\deleteindex.cpp"
  Delete "$INSTDIR\projects\edic.cpp"
  Delete "$INSTDIR\projects\edir.cpp"
  Delete "$INSTDIR\projects\list.cpp"
  Delete "$INSTDIR\projects\printtext.h"
  Delete "$INSTDIR\projects\listfiles.cpp"
  Delete "$INSTDIR\projects\listindexes.cpp"
  Delete "$INSTDIR\projects\testsort.cpp"
  Delete "$INSTDIR\projects\configexodus.cpp"

  Delete "$INSTDIR\projects\blanksolution\*.*"
  RMDir "$INSTDIR\projects\blanksolution"
  RMDir "$INSTDIR\projects"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\exodus\11.05"

  SetShellVarContext all

  # second, remove the link from the start menu
  delete "$SMPROGRAMS\Exodus-11.05\Uninstall Exodus.lnk"

  delete "$SMPROGRAMS\Exodus-11.05\Exodus Console.lnk"
  delete "$SMPROGRAMS\Exodus-11.05\Exodus Config.lnk"
  RMDir "$SMPROGRAMS\Exodus-11.05"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Exodus-11.05"

  ;remove the path to binaries HKLM=Local Machine and HKCU=Current User
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "$APPDATA\Exodus"

SectionEnd
