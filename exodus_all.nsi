;initial code is just standard nsis macros that doesnt need to be changed or understood
;exodus build instructions at at the bottom, or search for exodus

;-----------------------------------------------
;requires following environment variables PRESET
;-----------------------------------------------
; TARGET_CPU           eg x64 or x86 only
; Configuration        eg Release or Debug only
;
; EXODUS_VCVERSION     eg 70 80 90 100 etc
; EXODUS_TOOLPATH      eg C:\Windows\system32 - location of msvcrNNd.dll etc
;
; EXODUS_MINOR_VERSION eg 11.5
; EXODUS_MICRO_VERSION eg 11.5.3
; EXODUS_PRODUCTNAME   eg Exodus or Exodus64
; SOURCE_PATH          eg debug release x64\debug x64\release

; REDIST_DESC          eg "MSVC++ 2010"
; REDIST_SOURCE1       eg "Microsoft.com"
; REDIST_URL1          eg "http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe"
; REDIST_SOURCE2       optional
; REDIST_URL2          optional
; REDIST_SOURCE3       optional
; REDIST_URL2          optional

;;;;;;;;; package parameters ;;;;;;;;;;;;;;;;;;;;

;---------------------------------------------------------------------------
; EXODUS_VCVERSION=WHICH VC RUNTIME VERSION
; MUST have been built with this toolset
; 70  is VS2003
; 80  is VS2005
; 90  is VS2008
; 100 is VS2010 (NB 100 NOT JUST 10!!!)
;----------------------------------------------------------------------------
;!define EXODUS_VCVERSION "70"
;!define EXODUS_VCVERSION "80"
;!define EXODUS_VCVERSION "90"
;!define EXODUS_VCVERSION "100"
 !define EXODUS_VCVERSION "$%EXODUS_VCVERSION%"
;------------------------------------------
; WHERE IS VC RUNTIME VERSION TO DISTRIBUTE
; currently hard coded?! otherwise
; MUST! be present as an environment variable pointing to the MSVC toolset
;------------------------------------------
;!define EXODUS_TOOLPATH "$%VS700COMNTOOLS%"
;!define EXODUS_TOOLPATH "$%VS800COMNTOOLS%"
;!define EXODUS_TOOLPATH "$%VS90COMNTOOLS%"
;!define EXODUS_TOOLPATH "$%VS100COMNTOOLS%"
;!define EXODUS_TOOLPATH c:\windows\system32
 !define EXODUS_TOOLPATHREL "$%EXODUS_TOOLPATHREL%"
 !define EXODUS_TOOLPATHDEB "$%EXODUS_TOOLPATHDEB%"

;----------------------------------------------------------------
; EXODUS_PLATFORM MUST be x86 or x64
; DETERMINES PART OF THE INSTALLER FILE NAME!
; ALSO WHICH VC RUNTIME PLATFORM TO DISTRIBUTE (if not hard coded)
;----------------------------------------------------------------
;!define EXODUS_PLATFORM "x86"
;!define EXODUS_PLATFORM "x64"
 !define EXODUS_PLATFORM "$%TARGET_CPU%"

;------------------------------------------------------------------
; WHAT NAME TO GIVE EXODUS ON START MENU ETC (OMIT VERSION!)
; use Exodus for x86 since it will install on 32 or 64 bit machines
; and Exodus64 for x64 since it will only install on 64 bit machines
; it will be used as a folder name
;------------------------------------------------------------------
;!define EXODUS_PRODUCTNAME "Exodus"
;!define EXODUS_PRODUCTNAME "Exodus64"
 !define EXODUS_PRODUCTNAME "$%EXODUS_PRODUCTNAME%"

;-------------------------------------------------------------------
; VERSION OF EXODUS FOR SEPARATE MENU ITEM AND INSTALLATION DIRECTORY
; minor versions each get installed in a separate directory
;-------------------------------------------------------------------
;!define EXODUS_MINOR_VERSION "11.5"
 !define EXODUS_MINOR_VERSION "$%EXODUS_MINOR_VERSION%"

;-------------------------------------------------------------
; WHAT MINOR VERSION NUMBER (FOR INFORMATION ONLY)
; micro versions are for information and will simply overwrite
; previous minor versions
; TODO check we dont double add to path etc.
;-------------------------------------------------------------
;!define EXODUS_MICRO_VERSION "11.5.3"
 !define EXODUS_MICRO_VERSION "$%EXODUS_MICRO_VERSION%"

;-------------------------
;BASIC FILENAME FOR EXODUS
;-------------------------
;!define EXODUS_CODENAME "exodus"
 !define EXODUS_CODENAME "$%EXODUS_CODENAME%"

 !define debugorrelease "$%EXODUS_BINARIES%"

;-----------------------------------------
;NAME, SOURCES AND URLS OF REDISTRIBUTABLE
;-----------------------------------------
;!define REDIST_DESC MSVC++ 2010
;!define REDIST_SOURCE1 Microsoft.com
;!define REDIST_URL1 http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe

!define REDIST_DESC "$%REDIST_DESC%"

!define REDIST_SOURCE1 "$%REDIST_SOURCE1%"
!define REDIST_URL1 $%REDIST_URL1%

!define REDIST_SOURCE2 "$%REDIST_SOURCE2%"
!define REDIST_URL2 $%REDIST_URL2%


;;;;;;;;;;;;; end of package parameters ;;;;;;;;;;;;;;;;;;;;


  ;VC2010 x86 - could download
  ;http://www.microsoft.com/downloads/info.aspx?na=41&SrcFamilyId=A7B7A05E-6DE6-4D3A-A423-37BF0912DB84&SrcDisplayLang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f5%2fB%2fC%2f5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E%2fvcredist_x86.exe
  ;VC2010 x86 debug - no download available

  ;VC2010 x64 - could download
  ;http://www.microsoft.com/downloads/info.aspx?na=41&SrcFamilyId=BD512D9E-43C8-4655-81BF-9350143D5867&SrcDisplayLang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f3%2f2%2f2%2f3224B87F-CFA0-4E70-BDA3-3DE650EFEBA5%2fvcredist_x64.exe
  ;VC2010 x64 debug - no download available

  ;VC2005 x86 - could download
  ;http://download.microsoft.com/download/d/3/4/d342efa6-3266-4157-a2ec-5174867be706/vcredist_x86.exe
  ;VC2005 x86 debug - no download available




!include LogicLib.nsh
 
!ifndef IPersistFile
!define IPersistFile {0000010b-0000-0000-c000-000000000046}
!endif
!ifndef CLSID_ShellLink
!define CLSID_ShellLink {00021401-0000-0000-C000-000000000046}
!define IID_IShellLinkA {000214EE-0000-0000-C000-000000000046}
!define IID_IShellLinkW {000214F9-0000-0000-C000-000000000046}
!define IShellLinkDataList {45e2b4ae-b1c3-11d0-b92f-00a0c90312e1}
	!ifdef NSIS_UNICODE
	!define IID_IShellLink ${IID_IShellLinkW}
	!else
	!define IID_IShellLink ${IID_IShellLinkA}
	!endif
!endif
 
 
 
Function ShellLinkSetRunAs
System::Store S
pop $9
System::Call "ole32::CoCreateInstance(g'${CLSID_ShellLink}',i0,i1,g'${IID_IShellLink}',*i.r1)i.r0"
${If} $0 = 0
	System::Call "$1->0(g'${IPersistFile}',*i.r2)i.r0" ;QI
	${If} $0 = 0
		System::Call "$2->5(w '$9',i 0)i.r0" ;Load
		${If} $0 = 0
			System::Call "$1->0(g'${IShellLinkDataList}',*i.r3)i.r0" ;QI
			${If} $0 = 0
				System::Call "$3->6(*i.r4)i.r0" ;GetFlags
				${If} $0 = 0
					System::Call "$3->7(i $4|0x2000)i.r0" ;SetFlags ;SLDF_RUNAS_USER
					${If} $0 = 0
						System::Call "$2->6(w '$9',i1)i.r0" ;Save
					${EndIf}
				${EndIf}
				System::Call "$3->2()" ;Release
			${EndIf}
		System::Call "$2->2()" ;Release
		${EndIf}
	${EndIf}
	System::Call "$1->2()" ;Release
${EndIf}
push $0
System::Store L
FunctionEnd

/* use like this
CreateShortcut "$temp\test.lnk" "calc.exe"
push "$temp\test.lnk"
call ShellLinkSetRunAs
pop $0
DetailPrint HR=$0
*/



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

;SetCompressor /SOLID LZMA

;see following how to install for all users and still support uninstallation
;http://nsis.sourceforge.net/Shortcuts_removal_fails_on_Windows_Vista
;RequestExecutionLevel user
RequestExecutionLevel admin #NOTE: You still need to check user rights with UserInfo!

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  Name "${EXODUS_PRODUCTNAME} ${EXODUS_MICRO_VERSION}"
  Outfile "${EXODUS_CODENAME}-${EXODUS_PLATFORM}-${EXODUS_MICRO_VERSION}.exe"
  InstallDir "$PROGRAMFILES\${EXODUS_PRODUCTNAME}\${EXODUS_MINOR_VERSION}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${EXODUS_PRODUCTNAME}\${EXODUS_MINOR_VERSION}" ""

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

;!include "x64.nsh"

;var DebugOrRelease
;!define location1 "$%Configuration%"
;!define location2 "$%TARGET_CPU%\$%Configuration%"
;--------------------------------------------------------
; SOURCE/LOCATION OF THE BUILT BINARIES TO BE PACKAGED UP
; PICK ONE!
;--------------------------------------------------------
;!define debugorrelease "debug"
;!define debugorrelease "release"
;!define debugorrelease "x64\debug"
;!define debugorrelease x64\release"

Section "All" SecAll

; StrCmp "${EXODUS_PLATFORM}" "x86" 0 nobla
;  strcpy $DebugOrRelease "${location1}"
;  Goto donee
;nobla:
;  strcpy $DebugOrRelease "${location2}"
;donee:

;${If} ${RunningX64}
;    File ..\x64\blah.exe
;${Else}
;    File ..\x86\blah.exe
;${EndIf}

  ;create a GLOBAL.END file to try and shutdown NEOSYS processes
  ;will be deleted at the end
  ;FileOpen $0 $INSTDIR\neosys\GLOBAL.END w
  ;FileClose $0

  SetOutPath "$INSTDIR\usr\share\terminfo\63"
  File release\cygwin\usr\share\terminfo\63\*

  SetOutPath "$INSTDIR\bin"

  ;if this file isnt built then fail  
  File ${DebugOrRelease}\compile.exe

  ;should we install this in order to get access to postgres on another server?
  ;File release\libpq32\*

  ;VC runtime without bloated redist package
  ;also delivering debug versions?! so exodus programs can be developed with stackwalker
  ;File "${EXODUS_TOOLPATH}..\..\VC\redist\${EXODUS_PLATFORM}\Microsoft.VC${EXODUS_VCVERSION}.CRT\*"
  ;File "${EXODUS_TOOLPATH}..\..\VC\redist\Debug_NonRedist\${EXODUS_PLATFORM}\Microsoft.VC${EXODUS_VCVERSION}.DebugCRT\*"
  ;hard coded!
  File "${EXODUS_TOOLPATHREL}\msvcr${EXODUS_VCVERSION}.dll"
  File "${EXODUS_TOOLPATHREL}\msvcp${EXODUS_VCVERSION}.dll"
  File "${EXODUS_TOOLPATHDEB}\msvcr${EXODUS_VCVERSION}d.dll"
  File "${EXODUS_TOOLPATHDEB}\msvcp${EXODUS_VCVERSION}d.dll"

  ;dont do exodus.dll since we may want the debug version - below
  ;File /x exodus.dll /x libpq.dll release\*.dll
  File /x exodus.dll release\*.dll

  File release\cygwin\bin\nano.exe
  File release\cygwin\bin\*.dll
  File release\cygwin\bin\nanorc
  
  File ${DebugOrRelease}\*.exe
  File ${DebugOrRelease}\exodus.dll
  File ${DebugOrRelease}\pgexodus.dll

  ;also output exodus_cli.exe as exodus.exe for ease of access to the end user
  File /oname=exodus.exe ${DebugOrRelease}\exodus_cli.exe

  SetOutPath "$INSTDIR\lib"
	
  File ${DebugOrRelease}\exodus.exp
  File ${DebugOrRelease}\exodus.lib
  ;File ${DebugOrRelease}\exodus.pdb

  File /oname=pgexodus-8.dll release\pgexodus-8.dll
  File /oname=pgexodus-9.dll ${DebugOrRelease}\pgexodus.dll

  SetOutPath "$INSTDIR\include\exodus"

  File exodus\exodus\exodus\mv*.h
  File exodus\exodus\exodus\exodus*.h
  File exodus\exodus\exodus\program.h
  File exodus\exodus\exodus\mvprogram.h
  File exodus\exodus\exodus\dict.h
  File exodus\exodus\exodus\library.h
  File exodus\exodus\exodus\xfunctor*.h

  ;SetOutPath "$INSTDIR\src"
  createDirectory "$%APPDATA%\Exodus"
  createDirectory "$%APPDATA%\Exodus\src"
  createDirectory "$%APPDATA%\Exodus\bin"
  SetOutPath "$%APPDATA%\Exodus\src"

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
  ;dont forget to remove the same in uninstall!
  ${EnvVarUpdate} $0 "PATH" "A" "HKCU" "$APPDATA\${EXODUS_PRODUCTNAME}\bin"

  ;Store installation folder
  WriteRegStr HKCU "Software\${EXODUS_PRODUCTNAME}\${EXODUS_MINOR_VERSION}" "" $INSTDIR
  
  createDirectory "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}"
  createShortCut "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\${EXODUS_PRODUCTNAME} Console.lnk" "$INSTDIR\bin\exodus.exe"

  createShortCut "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\${EXODUS_PRODUCTNAME} Config.lnk" "$INSTDIR\bin\configexodus.exe"

  ;make it "run as"
  push "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\${EXODUS_PRODUCTNAME} Config.lnk"
  call ShellLinkSetRunAs
  pop $0
  DetailPrint HR=$0

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  # create a shortcut named "new shortcut" in the start menu programs directory
  # point the new shortcut at the program uninstaller
  createShortCut "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\Uninstall ${EXODUS_PRODUCTNAME}.lnk" "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}" \
   "DisplayName" "${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION} (remove only)"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}" \
   "UninstallString" "$INSTDIR\Uninstall.exe"

SectionEnd


!define REDIST_FILENAME "vcredist_tmp.exe"
Section "${REDIST_DESC} Redist (req.)" SEC_CRT

;  SectionIn RO

;dont bother attempting to detect since it doesnt seem to appear to work everywhere
;  ; Detection made easy: Unlike previous redists, VC2010 now generates a platform
;  ; independent key for checking availability.
;  
;  ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86" "Installed"
;  IfErrors done
;  StrCmp $R0 "1" done

  SetOutPath "$TEMP"

  ;download source 1
  DetailPrint "Downloading ${REDIST_DESC} Redistributable Setup..."
  DetailPrint "Contacting ${REDIST_SOURCE1} ..."
  NSISdl::download /TIMEOUT=15000 "${REDIST_URL1}xxxxxxxxxxxx" "${REDIST_FILENAME}"

  Pop $R0 ;Get the return value
  StrCmp $R0 "success" OnSuccess
  DetailPrint "Could not contact ${REDIST_SOURCE1}, or the file has been (re)moved!"

  ;download source2
  DetailPrint "Contacting ${REDIST_SOURCE2} ..."
  NSISdl::download /TIMEOUT=20000 "${REDIST_URL2}" "${REDIST_FILENAME}"

  ;[TODO] Provide 3rd Source
  ;Pop $R0 ;Get the return value
  ;StrCmp $R0 "success" +2
  ;DetailPrint "Contacting ${REDIST_SOURCE3} ..."
  ;NSISdl::download /TIMEOUT=30000 "${REDIST_URL3}" "${REDIST_FILENAME}"

  Pop $R0 ;Get the return value
 ; StrCmp $R0 "success" +2
  StrCmp $R0 "success" OnSuccess
    MessageBox MB_OK "Could not download ${REDIST_DESC}, none of the mirrors appear to be functional. Please download it from Microsoft."
    Goto done

OnSuccess:
  DetailPrint "Running ${REDIST_DESC} Setup..."
  ;gives command line option syntax error on vs2005
;  ExecWait '"${REDIST_FILENAME}" /qb'
  ExecWait '"${REDIST_FILENAME}" /q'
  DetailPrint "Finished ${REDIST_DESC} Setup"
  
  Delete "$TEMP\${REDIST_FILENAME}"

done:
SectionEnd





;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecAll ${LANG_ENGLISH} "All Software"

  LangString DESC_CRT    ${LANG_ENGLISH} "Will only be downloaded if you don't already have it installed."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAll} $(DESC_SecAll)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_CRT2008}     $(DESC_CRT2008)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_CRT}     $(DESC_CRT)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\usr\share\terminfo\63\*.*"
  RMDir  "$INSTDIR\usr\share\terminfo\63"
  RMDir  "$INSTDIR\usr\share\terminfo"
  RMDir  "$INSTDIR\usr\share"
  RMDir  "$INSTDIR\usr"

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

  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\compile.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\createfile.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\createindex.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\delete.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\deletefile.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\deleteindex.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\edic.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\edir.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\list.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\printtext.h"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\listfiles.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\listindexes.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\testsort.cpp"
  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\configexodus.cpp"

  Delete "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\blanksolution\*.*"
  RMDir "$%APPDATA%\${EXODUS_PRODUCTNAME}\src\blanksolution"

  ;src and bin may remain if they created any programs
  RMDir "$%APPDATA%\${EXODUS_PRODUCTNAME}\src"
  RMDir "$%APPDATA%\${EXODUS_PRODUCTNAME}\bin"

  ;Exodus is likely to remain since we will not delete .exodus
  RMDir "$%APPDATA%\${EXODUS_PRODUCTNAME}"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  RMDir "$PROGRAMFILES\${EXODUS_PRODUCTNAME}"

  DeleteRegKey /ifempty HKCU "Software\${EXODUS_PRODUCTNAME}\${EXODUS_MINOR_VERSION}"

  SetShellVarContext all

  # second, remove the link from the start menu
  delete "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\Uninstall ${EXODUS_PRODUCTNAME}.lnk"

  delete "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\${EXODUS_PRODUCTNAME} Console.lnk"
  delete "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}\${EXODUS_PRODUCTNAME} Config.lnk"
  RMDir "$SMPROGRAMS\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}"

  ;remove the path to binaries HKLM=Local Machine and HKCU=Current User
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "$APPDATA\${EXODUS_PRODUCTNAME}\bin"

SectionEnd


;--------------------------------
;onInit Function

Function .onInit
 
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION}" \
  "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "${EXODUS_PRODUCTNAME}-${EXODUS_MINOR_VERSION} is already installed. $\n$\nClick `OK` to remove the \
  previous version or `Cancel` to cancel this upgrade." \
  IDOK uninst
  Abort
 
;Run the uninstaller
uninst:
  ClearErrors

  ;The following variant lets the uninstaller remove itself, so the IfErrors block is not needed any more:
  ;Exec $INSTDIR\uninst.exe ; instead of the ExecWait line
  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
done:
 
FunctionEnd



