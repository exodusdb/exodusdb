; top: parameters
; middle: standard nsis macros that dont need to be changed
; bottom: exodus build instructions
; ------------------------------------------------------------
; requires AT LEAST the following environment variables PRESET
; ------------------------------------------------------------
;  TARGET_CPU           eg x64 or x86 only
;  Configuration        eg Release or Debug only
;
;  EXO_VCVERSION     eg 70 80 90 100 etc
;  EXO_TOOLPATH      eg C:\Windows\system32 - location of msvcrNNd.dll etc
;
;  EXO_MAJOR_VER eg 11
;  EXO_MINOR_VER eg 5
;  EXO_MICRO_VER eg 3
;  EXO_BUILD_VER eg 
;  EXO_PRODUCTNAME   eg Exodus
;  SOURCE_PATH          eg debug release x64\debug x64\release
;
;  EXO_INSTALLFILENAME
;
;  REDIST_DESC          eg "MSVC++ 2010"
;  REDIST_SOURCE1       eg "Microsoft.com"
;  REDIST_URL1          eg "http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe"
;  REDIST_SOURCE2       optional
;  REDIST_URL2          optional
;  REDIST_SOURCE3       optional
;  REDIST_URL3          optional

;;;;;;;;; package parameters ;;;;;;;;;;;;;;;;;;;;

;---------------------------------------------------------------------------
; EXO_VCVERSION=WHICH VC RUNTIME VERSION
; MUST have been built with this toolset
; 70  is VS2003
; 80  is VS2005
; 90  is VS2008
; 100 is VS2010 (NB 100 NOT JUST 10!!!)
;----------------------------------------------------------------------------
;!define EXO_VCVERSION "70"
;!define EXO_VCVERSION "80"
;!define EXO_VCVERSION "90"
;!define EXO_VCVERSION "100"
 !define EXO_VCVERSION "$%EXO_VCVERSION%"
 !define EXO_REDISTDOTVER "$%EXO_REDISTDOTVER%"
;------------------------------------------
; WHERE IS VC RUNTIME VERSION TO DISTRIBUTE
; currently hard coded?! otherwise
; MUST! be present as an environment variable pointing to the MSVC toolset
;------------------------------------------
;!define EXO_TOOLPATH "$%VS700COMNTOOLS%"
;!define EXO_TOOLPATH "$%VS800COMNTOOLS%"
;!define EXO_TOOLPATH "$%VS90COMNTOOLS%"
;!define EXO_TOOLPATH "$%VS100COMNTOOLS%"
;!define EXO_TOOLPATH c:\windows\system32
 !define EXO_TOOLPATHREL "$%EXO_TOOLPATHREL%"
 !define EXO_TOOLPATHDEB "$%EXO_TOOLPATHDEB%"

;----------------------------------------------------------------
; EXO_PLATFORM MUST be x86 or x64
; DETERMINES PART OF THE INSTALLER FILE NAME!
; ALSO WHICH VC RUNTIME PLATFORM TO DISTRIBUTE (if not hard coded)
;----------------------------------------------------------------
;!define EXO_PLATFORM "x86"
;!define EXO_PLATFORM "x64"
 !define EXO_PLATFORM "$%TARGET_CPU%"

;------------------------------------------------------------------
; WHAT NAME TO GIVE EXODUS
;------------------------------------------------------------------
 !define EXO_PRODUCTNAME "$%EXO_PRODUCTNAME%"

;-------------------------------------------------------------------
; VERSION OF EXODUS FOR SEPARATE MENU ITEM AND INSTALLATION DIRECTORY
; majorminor versions each get installed in a separate directory
; micro versions get uninstalled/overwritten
;-------------------------------------------------------------------
 !define EXO_MAJOR_VER "$%EXO_MAJOR_VER%"
 !define EXO_MINOR_VER "$%EXO_MINOR_VER%"
 !define EXO_MICRO_VER "$%EXO_MICRO_VER%"
; !define EXO_BUILD_VER "$%EXO_MINOR_VER%"

 !define EXO_DOTTEDMINORVERSION "$%EXO_MAJOR_VER%.$%EXO_MINOR_VER%"
 !define EXO_DOTTEDMICROVERSION "$%EXO_MAJOR_VER%.$%EXO_MINOR_VER%.$%EXO_MICRO_VER%"

 !define EXO_PRODUCTNAME_AND_DOTTEDMICROVERSION "$%EXO_PRODUCTNAME% $%EXO_MAJOR_VER%.$%EXO_MINOR_VER%.$%EXO_MICRO_VER%-$%TARGET_CPU%"

 !define EXO_INSTDIR "C:\$%EXO_PRODUCTNAME%$%EXO_MAJOR_VER%$%EXO_MINOR_VER%"

 ;output file name
 !define EXO_OUTFILE "$%EXO_INSTALLFILENAME%"

 !define EXO_REGKEY_ROOT "$%EXO_PRODUCTNAME%"
 !define EXO_REGKEY_VER  "$%EXO_PRODUCTNAME%\$%EXO_MAJOR_VER%.$%EXO_MINOR_VER%"

;%LOCALAPPDATA%  writable like appdata (if present)
;%APPDATA% not writable - in vista+?

; another try "$PROFILE" eg C:\User\John
; !!! use $PROFILE not environment variables like %USERPROFILE% to get the installing user
;otherwise you will get the "runas" user like administrator while installing
 !define EXO_USERHOME "$PROFILE\$%EXO_PRODUCTNAME%"
 !define EXO_USERBIN  "$PROFILE\$%EXO_PRODUCTNAME%\bin"

;-------------------------
;BASIC FILENAME FOR EXODUS
;-------------------------
;!define EXO_CODENAME "exodus"
 !define EXO_CODENAME "$%EXO_CODENAME%"

 !define debugorrelease "$%EXO_BINARIES%"

;----------------
;MENU ITEM TITLES
;----------------
!define EXO_MENUDIR                        "$%EXO_PRODUCTNAME% $%EXO_MAJOR_VER%.$%EXO_MINOR_VER%"
!define EXO_MENUITEM_TITLE_COMMANDLINE     "$%EXO_PRODUCTNAME% (command line)"
!define EXO_MENUITEM_TITLE_CONFIGURE       "$%EXO_PRODUCTNAME% (configure)"
!define EXO_MENUITEM_TITLE_UNINSTALL_TITLE "Uninstall"
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



  ;VC2010 x86 - could download
  ;http://www.microsoft.com/downloads/info.aspx?na=41&SrcFamilyId=A7B7A05E-6DE6-4D3A-A423-37BF0912DB84&SrcDisplayLang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f5%2fB%2fC%2f5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E%2fvcredist_x86.exe
  ;VC2010 x86 debug - no download available

  ;VC2010 x64 - could download
  ;http://www.microsoft.com/downloads/info.aspx?na=41&SrcFamilyId=BD512D9E-43C8-4655-81BF-9350143D5867&SrcDisplayLang=en&u=http%3a%2f%2fdownload.microsoft.com%2fdownload%2f3%2f2%2f2%2f3224B87F-CFA0-4E70-BDA3-3DE650EFEBA5%2fvcredist_x64.exe
  ;VC2010 x64 debug - no download available

  ;VC2005 x86 - could download
  ;http://download.microsoft.com/download/d/3/4/d342efa6-3266-4157-a2ec-5174867be706/vcredist_x86.exe
  ;VC2005 x86 debug - no download available


;;;;;;;;;;;;; end of package parameters ;;;;;;;;;;;;;;;;;;;;








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



!define STRFUNC_VERBOSITY 2
;!include "StrFunc.nsh"
;here we include StrFunc DIRECTLY instead of including it
;why? so we can hack out various forced verbosity 4(?!) that cannot be suppressed in the original

/*
o-----------------------------------------------------------------------------o
|String Functions Header File 1.09                                            |
(-----------------------------------------------------------------------------)
| By deguix                                     / A Header file for NSIS 2.01 |
| <cevo_deguix@yahoo.com.br>                   -------------------------------|
|                                                                             |
|    This header file contains NSIS functions for string manipulation.        |
o-----------------------------------------------------------------------------o
*/

!verbose push
!verbose 3
!ifndef STRFUNC_VERBOSITY
  !define STRFUNC_VERBOSITY 3
!endif
!define _STRFUNC_VERBOSITY ${STRFUNC_VERBOSITY}
!undef STRFUNC_VERBOSITY
!verbose ${_STRFUNC_VERBOSITY}

!include LogicLib.nsh

!ifndef STRFUNC

  !define FALSE 0
  !define TRUE 1

  ;Header File Identification

  !define STRFUNC `String Functions Header File`
  !define STRFUNC_SHORT `StrFunc`
  !define STRFUNC_CREDITS `2004 Diego Pedroso`

  ;Header File Version

  !define STRFUNC_VERMAJ 1
  !define STRFUNC_VERMED 09
 ;!define STRFUNC_VERMIN 0
 ;!define STRFUNC_VERBLD 0

  !define STRFUNC_VER `${STRFUNC_VERMAJ}.${STRFUNC_VERMED}`

  ;Header File Init Message Prefix and Postfix

  !define STRFUNC_INITMSGPRE `----------------------------------------------------------------------$\r$\n`
  !define STRFUNC_INITMSGPOST `$\r$\n----------------------------------------------------------------------$\r$\n`

  ;Header File Init Message

  !verbose push
  ;exodus !verbose 4
  !echo `${STRFUNC_INITMSGPRE}NSIS ${STRFUNC} ${STRFUNC_VER} - Copyright ${STRFUNC_CREDITS}${STRFUNC_INITMSGPOST}`
  !verbose pop

  ;Header File Function Init Message Prefix and Postfix

  !define STRFUNC_FUNCMSGPRE ``
  !define STRFUNC_FUNCMSGPOST ``
  
  ;Header File Function Macros

  !macro STRFUNC_FUNCLIST_INSERT Name
    !ifdef StrFunc_List
      !define StrFunc_List2 `${StrFunc_List}`
      !undef StrFunc_List
      !define StrFunc_List `${StrFunc_List2}|${Name}`
      !undef StrFunc_List2
    !else
      !define StrFunc_List `${Name}`
    !endif
  !macroend

  !macro STRFUNC_DEFFUNC Name
    !insertmacro STRFUNC_FUNCLIST_INSERT ${Name}
  
    !define `${Name}` `!insertmacro FUNCTION_STRING_${Name}`
    !define `Un${Name}` `!insertmacro FUNCTION_STRING_Un${Name}`
  !macroend
  
  !macro STRFUNC_FUNC ShortName Credits
    !verbose push
    ;!exodus verbose 4

    !ifndef `Un${ShortName}`
      !echo `${STRFUNC_FUNCMSGPRE}$ {Un${ShortName}} - Copyright ${Credits}${STRFUNC_FUNCMSGPOST}`
      !verbose pop
      !define `Un${ShortName}` `!insertmacro FUNCTION_STRING_Un${ShortName}_Call`
      !define `Un${ShortName}_INCLUDED`
      Function `un.${ShortName}`
    !else
      !echo `${STRFUNC_FUNCMSGPRE}$ {${ShortName}} - Copyright ${Credits}${STRFUNC_FUNCMSGPOST}`
      !verbose pop
      !undef `${ShortName}`
      !define `${ShortName}` `!insertmacro FUNCTION_STRING_${ShortName}_Call`
      !define `${ShortName}_INCLUDED`
      Function `${ShortName}`
    !endif
  !macroend

  ;Function Names Startup Definition

  !insertmacro STRFUNC_DEFFUNC StrCase
  !define StrCase_List `ResultVar|String|Type`
  !define StrCase_TypeList `Output|Text|Option  U L T S <>`
  !macro `FUNCTION_STRING_UnStrCase`
    !undef UnStrCase
    !insertmacro FUNCTION_STRING_StrCase
  !macroend
  
  !insertmacro STRFUNC_DEFFUNC StrClb
  !define StrClb_List `ResultVar|String|Action`
  !define StrClb_TypeList `Output|Text|Option  > < <>`
  !macro `FUNCTION_STRING_UnStrClb`
    !undef UnStrClb
    !insertmacro FUNCTION_STRING_StrClb
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrIOToNSIS
  !define StrIOToNSIS_List `ResultVar|String`
  !define StrIOToNSIS_TypeList `Output|Text`
  !macro `FUNCTION_STRING_UnStrIOToNSIS`
    !undef UnStrIOToNSIS
    !insertmacro FUNCTION_STRING_StrIOToNSIS
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrLoc
  !define StrLoc_List `ResultVar|String|StrToSearchFor|CounterDirection`
  !define StrLoc_TypeList `Output|Text|Text|Option > <`
  !macro `FUNCTION_STRING_UnStrLoc`
    !undef UnStrLoc
    !insertmacro FUNCTION_STRING_StrLoc
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrNSISToIO
  !define StrNSISToIO_List `ResultVar|String`
  !define StrNSISToIO_TypeList `Output|Text`
  !macro `FUNCTION_STRING_UnStrNSISToIO`
    !undef UnStrNSISToIO
    !insertmacro FUNCTION_STRING_StrNSISToIO
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrRep
  !define StrRep_List `ResultVar|String|StrToReplace|ReplacementString`
  !define StrRep_TypeList `Output|Text|Text|Text`
  !macro `FUNCTION_STRING_UnStrRep`
    !undef UnStrRep
    !insertmacro FUNCTION_STRING_StrRep
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrSort
  !define StrSort_List `ResultVar|String|LeftStr|CenterStr|RightStr|IncludeLeftStr|IncludeCenterStr|IncludeRightStr`
  !define StrSort_TypeList `Output|Text|Text|Text|Text|Option 1 0|Option 1 0|Option 1 0`
  !macro `FUNCTION_STRING_UnStrSort`
    !undef UnStrSort
    !insertmacro FUNCTION_STRING_StrSort
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrStr
  !define StrStr_List `ResultVar|String|StrToSearchFor`
  !define StrStr_TypeList `Output|Text|Text`
  !macro `FUNCTION_STRING_UnStrStr`
    !undef UnStrStr
    !insertmacro FUNCTION_STRING_StrStr
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrStrAdv
  !define StrStrAdv_List `ResultVar|String|StrToSearchFor|SearchDirection|ResultStrDirection|DisplayStrToSearch|Loops|CaseSensitive`
  !define StrStrAdv_TypeList `Output|Text|Text|Option > <|Option > <|Option 1 0|Text|Option 0 1`
  !macro `FUNCTION_STRING_UnStrStrAdv`
    !undef UnStrStrAdv
    !insertmacro FUNCTION_STRING_StrStrAdv
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrTok
  !define StrTok_List `ResultVar|String|Separators|ResultPart|SkipEmptyParts`
  !define StrTok_TypeList `Output|Text|Text|Mixed L|Option 1 0`
  !macro `FUNCTION_STRING_UnStrTok`
    !undef UnStrTok
    !insertmacro FUNCTION_STRING_StrTok
  !macroend

  !insertmacro STRFUNC_DEFFUNC StrTrimNewLines
  !define StrTrimNewLines_List `ResultVar|String`
  !define StrTrimNewLines_TypeList `Output|Text`
  !macro `FUNCTION_STRING_UnStrTrimNewLines`
    !undef UnStrTrimNewLines
    !insertmacro FUNCTION_STRING_StrTrimNewLines
  !macroend

  ;Function Codes for Install and Uninstall

  # Function StrCase
  ################

  !macro FUNCTION_STRING_StrCase
    !insertmacro STRFUNC_FUNC `StrCase` `2004 Diego Pedroso - Based on functions by Dave Laundon`

    /*After this point:
      ------------------------------------------
       $0 = String (input)
       $1 = Type (input)
       $2 = StrLength (temp)
       $3 = StartChar (temp)
       $4 = EndChar (temp)
       $5 = ResultStr (temp)
       $6 = CurrentChar (temp)
       $7 = LastChar (temp)
       $8 = Temp (temp)*/

      ;Get input from user
      Exch $1
      Exch
      Exch $0
      Exch
      Push $2
      Push $3
      Push $4
      Push $5
      Push $6
      Push $7
      Push $8

      ;Initialize variables
      StrCpy $2 ""
      StrCpy $3 ""
      StrCpy $4 ""
      StrCpy $5 ""
      StrCpy $6 ""
      StrCpy $7 ""
      StrCpy $8 ""

      ;Upper and lower cases are simple to use
      ${If} $1 == "U"

        ;Upper Case System:
        ;------------------
        ; Convert all characters to upper case.

        System::Call "User32::CharUpper(t r0 r5)i"
        Goto StrCase_End
      ${ElseIf} $1 == "L"

        ;Lower Case System:
        ;------------------
        ; Convert all characters to lower case.

        System::Call "User32::CharLower(t r0 r5)i"
        Goto StrCase_End
      ${EndIf}

      ;For the rest of cases:
      ;Get "String" length
      StrLen $2 $0

      ;Make a loop until the end of "String"
      ${For} $3 0 $2
        ;Add 1 to "EndChar" counter also
        IntOp $4 $3 + 1

        # Step 1: Detect one character at a time

        ;Remove characters before "StartChar" except when
        ;"StartChar" is the first character of "String"
        ${If} $3 <> 0
          StrCpy $6 $0 `` $3
        ${EndIf}

        ;Remove characters after "EndChar" except when
        ;"EndChar" is the last character of "String"
        ${If} $4 <> $2
          ${If} $3 = 0
            StrCpy $6 $0 1
          ${Else}
            StrCpy $6 $6 1
          ${EndIf}
        ${EndIf}

        # Step 2: Convert to the advanced case user chose:

        ${If} $1 == "T"

          ;Title Case System:
          ;------------------
          ; Convert all characters after a non-alphabetic character to upper case.
          ; Else convert to lower case.

          ;Use "IsCharAlpha" for the job
          System::Call "*(&t1 r7) i .r8"
          System::Call "*$8(&i1 .r7)"
          System::Free $8
          System::Call "user32::IsCharAlpha(i r7) i .r8"
          
          ;Verify "IsCharAlpha" result and convert the character
          ${If} $8 = 0
            System::Call "User32::CharUpper(t r6 r6)i"
          ${Else}
            System::Call "User32::CharLower(t r6 r6)i"
          ${EndIf}
        ${ElseIf} $1 == "S"

          ;Sentence Case System:
          ;------------------
          ; Convert all characters after a ".", "!" or "?" character to upper case.
          ; Else convert to lower case. Spaces or tabs after these marks are ignored.

          ;Detect current characters and ignore if necessary
          ${If} $6 == " "
          ${OrIf} $6 == "$\t"
            Goto IgnoreLetter
          ${EndIf}

          ;Detect last characters and convert
          ${If} $7 == "."
          ${OrIf} $7 == "!"
          ${OrIf} $7 == "?"
          ${OrIf} $7 == ""
            System::Call "User32::CharUpper(t r6 r6)i"
          ${Else}
            System::Call "User32::CharLower(t r6 r6)i"
          ${EndIf}
        ${ElseIf} $1 == "<>"

          ;Switch Case System:
          ;------------------
          ; Switch all characters cases to their inverse case.

          ;Use "IsCharUpper" for the job
          System::Call "*(&t1 r6) i .r8"
          System::Call "*$8(&i1 .r7)"
          System::Free $8
          System::Call "user32::IsCharUpper(i r7) i .r8"
          
          ;Verify "IsCharUpper" result and convert the character
          ${If} $8 = 0
            System::Call "User32::CharUpper(t r6 r6)i"
          ${Else}
            System::Call "User32::CharLower(t r6 r6)i"
          ${EndIf}
        ${EndIf}

        ;Write the character to "LastChar"
        StrCpy $7 $6

        IgnoreLetter:
        ;Add this character to "ResultStr"
        StrCpy $5 `$5$6`
      ${Next}

      StrCase_End:

    /*After this point:
      ------------------------------------------
       $0 = OutVar (output)*/

      ; Copy "ResultStr" to "OutVar"
      StrCpy $0 $5

      ;Return output to user
      Pop $8
      Pop $7
      Pop $6
      Pop $5
      Pop $4
      Pop $3
      Pop $2
      Pop $1
      Exch $0
    FunctionEnd

  !macroend

  !macro FUNCTION_STRING_StrClb
    !insertmacro STRFUNC_FUNC `StrClb` `2004 Diego Pedroso - Based on functions by Nik Medved`

    /*After this point:
      ------------------------------------------
       $0 = String (input)
       $1 = Action (input)
       $2 = Lock/Unlock (temp)
       $3 = Temp (temp)
       $4 = Temp2 (temp)*/

      ;Get input from user

      Exch $1
      Exch
      Exch $0
      Exch
      Push $2
      Push $3
      Push $4
      
      StrCpy $2 ""
      StrCpy $3 ""
      StrCpy $4 ""

      ;Open the clipboard to do the operations the user chose (kichik's fix)
      System::Call 'user32::OpenClipboard(i $HWNDPARENT)'

      ${If} $1 == ">" ;Set

        ;Step 1: Clear the clipboard
        System::Call 'user32::EmptyClipboard()'

        ;Step 2: Allocate global heap
        StrLen $2 $0
        IntOp $2 $2 + 1
        System::Call 'kernel32::GlobalAlloc(i 2, i r2) i.r2'

        ;Step 3: Lock the handle
        System::Call 'kernel32::GlobalLock(i r2) i.r3'

        ;Step 4: Copy the text to locked clipboard buffer
        System::Call 'kernel32::lstrcpyA(i r3, t r0)'

        ;Step 5: Unlock the handle again
        System::Call 'kernel32::GlobalUnlock(i r2)'

        ;Step 6: Set the information to the clipboard
        System::Call 'user32::SetClipboardData(i 1, i r2)'

        StrCpy $0 ""

      ${ElseIf} $1 == "<" ;Get

        ;Step 1: Get clipboard data
        System::Call 'user32::GetClipboardData(i 1) i .r2'

        ;Step 2: Lock and copy data (kichik's fix)
        System::Call 'kernel32::GlobalLock(i r2) t .r0'

        ;Step 3: Unlock (kichik's fix)
        System::Call 'kernel32::GlobalUnlock(i r2)'

      ${ElseIf} $1 == "<>" ;Swap

        ;Step 1: Get clipboard data
        System::Call 'user32::GetClipboardData(i 1) i .r2'

        ;Step 2: Lock and copy data (kichik's fix)
        System::Call 'kernel32::GlobalLock(i r2) t .r4'

        ;Step 3: Unlock (kichik's fix)
        System::Call 'kernel32::GlobalUnlock(i r2)'

        ;Step 4: Clear the clipboard
        System::Call 'user32::EmptyClipboard()'

        ;Step 5: Allocate global heap
        StrLen $2 $0
        IntOp $2 $2 + 1
        System::Call 'kernel32::GlobalAlloc(i 2, i r2) i.r2'

        ;Step 6: Lock the handle
        System::Call 'kernel32::GlobalLock(i r2) i.r3'

        ;Step 7: Copy the text to locked clipboard buffer
        System::Call 'kernel32::lstrcpyA(i r3, t r0)'

        ;Step 8: Unlock the handle again
        System::Call 'kernel32::GlobalUnlock(i r2)'

        ;Step 9: Set the information to the clipboard
        System::Call 'user32::SetClipboardData(i 1, i r2)'
        
        StrCpy $0 $4
      ${Else} ;Clear

        ;Step 1: Clear the clipboard
        System::Call 'user32::EmptyClipboard()'

        StrCpy $0 ""
      ${EndIf}

      ;Close the clipboard
      System::Call 'user32::CloseClipboard()'

    /*After this point:
      ------------------------------------------
       $0 = OutVar (output)*/

      ;Return result to user
      Pop $4
      Pop $3
      Pop $2
      Pop $1
      Exch $0
    FunctionEnd

  !macroend

  # Function StrIOToNSIS
  ####################

  !macro FUNCTION_STRING_StrIOToNSIS
    !insertmacro STRFUNC_FUNC `StrIOToNSIS` `2004 "bluenet" - Based on functions by Amir Szekely, Joost Verburg, Dave Laundon and Diego Pedroso`

    /*After this point:
      ------------------------------------------
       $R0 = String (input/output)
       $R1 = StartCharPos (temp)
       $R2 = StrLen (temp)
       $R3 = TempStr (temp)
       $R4 = TempRepStr (temp)*/

      ;Get input from user
      Exch $R0
      Push $R1
      Push $R2
      Push $R3
      Push $R4
      
      ;Get "String" length
      StrLen $R2 $R0

      ;Loop until "String" end is reached
      ${For} $R1 0 $R2
        ;Get the next "String" characters
        StrCpy $R3 $R0 2 $R1
        
        ;Detect if current character is:
        ${If} $R3 == "\\" ;Back-slash
          StrCpy $R4 "\"
        ${ElseIf} $R3 == "\r" ;Carriage return
          StrCpy $R4 "$\r"
        ${ElseIf} $R3 == "\n" ;Line feed
          StrCpy $R4 "$\n"
        ${ElseIf} $R3 == "\t" ;Tab
          StrCpy $R4 "$\t"
        ${Else} ;Anything else
          StrCpy $R4 ""
        ${EndIf}

        ;Detect if "TempRepStr" is not empty
        ${If} $R4 != ""
          ;Replace the old characters with the new one
          StrCpy $R3 $R0 $R1
          IntOp $R1 $R1 + 2
          StrCpy $R0 $R0 "" $R1
          StrCpy $R0 "$R3$R4$R0"
          IntOp $R2 $R2 - 1 ;Decrease "StrLen"
          IntOp $R1 $R1 - 2 ;Go back to the next character
        ${EndIf}
      ${Next}
      Pop $R4
      Pop $R3
      Pop $R2
      Pop $R1
      Exch $R0
    FunctionEnd
  !macroend

  # Function StrLoc
  ###############

  !macro FUNCTION_STRING_StrLoc
    !insertmacro STRFUNC_FUNC `StrLoc` `2004 Diego Pedroso - Based on functions by Ximon Eighteen`

    /*After this point:
      ------------------------------------------
       $R0 = OffsetDirection (input)
       $R1 = StrToSearch (input)
       $R2 = String (input)
       $R3 = StrToSearchLen (temp)
       $R4 = StrLen (temp)
       $R5 = StartCharPos (temp)
       $R6 = TempStr (temp)*/

      ;Get input from user
      Exch $R0
      Exch
      Exch $R1
      Exch 2
      Exch $R2
      Push $R3
      Push $R4
      Push $R5
      Push $R6

      ;Get "String" and "StrToSearch" length
      StrLen $R3 $R1
      StrLen $R4 $R2
      ;Start "StartCharPos" counter
      StrCpy $R5 0

      ;Loop until "StrToSearch" is found or "String" reaches its end
      ${Do}
        ;Remove everything before and after the searched part ("TempStr")
        StrCpy $R6 $R2 $R3 $R5

        ;Compare "TempStr" with "StrToSearch"
        ${If} $R6 == $R1
          ${If} $R0 == `<`
            IntOp $R6 $R3 + $R5
            IntOp $R0 $R4 - $R6
          ${Else}
            StrCpy $R0 $R5
          ${EndIf}
          ${ExitDo}
        ${EndIf}
        ;If not "StrToSearch", this could be "String" end
        ${If} $R5 >= $R4
          StrCpy $R0 ``
          ${ExitDo}
        ${EndIf}
        ;If not, continue the loop
        IntOp $R5 $R5 + 1
      ${Loop}

      ;Return output to user
      Pop $R6
      Pop $R5
      Pop $R4
      Pop $R3
      Pop $R2
      Exch
      Pop $R1
      Exch $R0
    FunctionEnd

  !macroend

  # Function StrNSISToIO
  ####################

  !macro FUNCTION_STRING_StrNSISToIO
    !insertmacro STRFUNC_FUNC `StrNSISToIO` `2004 "bluenet" - Based on functions by Amir Szekely, Joost Verburg, Dave Laundon and Diego Pedroso`

    /*After this point:
      ------------------------------------------
       $R0 = String (input/output)
       $R1 = StartCharPos (temp)
       $R2 = StrLen (temp)
       $R3 = TempStr (temp)
       $R4 = TempRepStr (temp)*/

      ;Get input from user
      Exch $R0
      Push $R1
      Push $R2
      Push $R3
      Push $R4
      
      ;Get "String" length
      StrLen $R2 $R0

      ;Loop until "String" end is reached
      ${For} $R1 0 $R2
        ;Get the next "String" character
        StrCpy $R3 $R0 1 $R1

        ;Detect if current character is:
        ${If} $R3 == "$\r" ;Back-slash
          StrCpy $R4 "\r"
        ${ElseIf} $R3 == "$\n" ;Carriage return
          StrCpy $R4 "\n"
        ${ElseIf} $R3 == "$\t" ;Line feed
          StrCpy $R4 "\t"
        ${ElseIf} $R3 == "\" ;Tab
          StrCpy $R4 "\\"
        ${Else} ;Anything else
          StrCpy $R4 ""
        ${EndIf}

        ;Detect if "TempRepStr" is not empty
        ${If} $R4 != ""
          ;Replace the old character with the new ones
          StrCpy $R3 $R0 $R1
          IntOp $R1 $R1 + 1
          StrCpy $R0 $R0 "" $R1
          StrCpy $R0 "$R3$R4$R0"
          IntOp $R2 $R2 + 1 ;Increase "StrLen"
        ${EndIf}
      ${Next}

      ;Return output to user
      Pop $R4
      Pop $R3
      Pop $R2
      Pop $R1
      Exch $R0
    FunctionEnd
  !macroend

  # Function StrRep
  ###############

  !macro FUNCTION_STRING_StrRep
    !insertmacro STRFUNC_FUNC `StrRep` `2004 Diego Pedroso - Based on functions by Hendri Adriaens`

    /*After this point:
      ------------------------------------------
       $R0 = ReplacementString (input)
       $R1 = StrToSearch (input)
       $R2 = String (input)
       $R3 = RepStrLen (temp)
       $R4 = StrToSearchLen (temp)
       $R5 = StrLen (temp)
       $R6 = StartCharPos (temp)
       $R7 = TempStrL (temp)
       $R8 = TempStrR (temp)*/

      ;Get input from user
      Exch $R0
      Exch
      Exch $R1
      Exch
      Exch 2
      Exch $R2
      Push $R3
      Push $R4
      Push $R5
      Push $R6
      Push $R7
      Push $R8

      ;Return "String" if "StrToSearch" is ""
      ${IfThen} $R1 == "" ${|} Goto Done ${|}

      ;Get "ReplacementString", "String" and "StrToSearch" length
      StrLen $R3 $R0
      StrLen $R4 $R1
      StrLen $R5 $R2
      ;Start "StartCharPos" counter
      StrCpy $R6 0

      ;Loop until "StrToSearch" is found or "String" reaches its end
      ${Do}
        ;Remove everything before and after the searched part ("TempStrL")
        StrCpy $R7 $R2 $R4 $R6

        ;Compare "TempStrL" with "StrToSearch"
        ${If} $R7 == $R1
          ;Split "String" to replace the string wanted
          StrCpy $R7 $R2 $R6 ;TempStrL

          ;Calc: "StartCharPos" + "StrToSearchLen" = EndCharPos
          IntOp $R8 $R6 + $R4

          StrCpy $R8 $R2 "" $R8 ;TempStrR

          ;Insert the new string between the two separated parts of "String"
          StrCpy $R2 $R7$R0$R8
          ;Now calculate the new "StrLen" and "StartCharPos"
          StrLen $R5 $R2
          IntOp $R6 $R6 + $R3
          ${Continue}
        ${EndIf}

        ;If not "StrToSearch", this could be "String" end
        ${IfThen} $R6 >= $R5 ${|} ${ExitDo} ${|}
        ;If not, continue the loop
        IntOp $R6 $R6 + 1
      ${Loop}

      Done:

    /*After this point:
      ------------------------------------------
       $R0 = OutVar (output)*/

      ;Return output to user
      StrCpy $R0 $R2
      Pop $R8
      Pop $R7
      Pop $R6
      Pop $R5
      Pop $R4
      Pop $R3
      Pop $R2
      Pop $R1
      Exch $R0
    FunctionEnd

  !macroend

  # Function StrSort
  ################

  !macro FUNCTION_STRING_StrSort
    !insertmacro STRFUNC_FUNC `StrSort` `2004 Diego Pedroso - Based on functions by Stuart Welch`

    /*After this point:
      ------------------------------------------
       $R0 = String (input)
       $R1 = LeftStr (input)
       $R2 = CenterStr (input)
       $R3 = RightStr (input)
       $R4 = IncludeLeftStr (input)
       $R5 = IncludeCenterStr (input)
       $R6 = IncludeRightStr (input)

       $0 = StrLen (temp)
       $1 = LeftStrLen (temp)
       $2 = CenterStrLen (temp)
       $3 = RightStrLen (temp)
       $4 = StartPos (temp)
       $5 = EndPos (temp)
       $6 = StartCharPos (temp)
       $7 = EndCharPos (temp)
       $8 = TempStr (temp)*/

      ;Get input from user
      Exch $R6
      Exch
      Exch $R5
      Exch
      Exch 2
      Exch $R4
      Exch 2
      Exch 3
      Exch $R3
      Exch 3
      Exch 4
      Exch $R2
      Exch 4
      Exch 5
      Exch $R1
      Exch 5
      Exch 6
      Exch $R0
      Exch 6
      Push $0
      Push $1
      Push $2
      Push $3
      Push $4
      Push $5
      Push $6
      Push $7
      Push $8

      ;Parameter defaults
      ${IfThen} $R4 == `` ${|} StrCpy $R4 `1` ${|}
      ${IfThen} $R5 == `` ${|} StrCpy $R5 `1` ${|}
      ${IfThen} $R6 == `` ${|} StrCpy $R6 `1` ${|}

      ;Get "String", "CenterStr", "LeftStr" and "RightStr" length
      StrLen $0 $R0
      StrLen $1 $R1
      StrLen $2 $R2
      StrLen $3 $R3
      ;Start "StartCharPos" counter
      StrCpy $6 0
      ;Start "EndCharPos" counter based on "CenterStr" length
      IntOp $7 $6 + $2

      ;Loop until "CenterStr" is found or "String" reaches its end
      ${Do}
        ;Remove everything before and after the searched part ("TempStr")
        StrCpy $8 $R0 $2 $6

        ;Compare "TempStr" with "CenterStr"
        ${IfThen} $8 == $R2 ${|} ${ExitDo} ${|}
        ;If not, this could be "String" end
        ${IfThen} $7 >= $0 ${|} Goto Done ${|}
        ;If not, continue the loop
        IntOp $6 $6 + 1
        IntOp $7 $7 + 1
      ${Loop}

      # "CenterStr" was found

      ;Remove "CenterStr" from "String" if the user wants
      ${If} $R5 = ${FALSE}
        StrCpy $8 $R0 $6
        StrCpy $R0 $R0 `` $7
        StrCpy $R0 $8$R0
      ${EndIf}

      ;"StartPos" and "EndPos" will record "CenterStr" coordinates for now
      StrCpy $4 $6
      StrCpy $5 $7
      ;"StartCharPos" and "EndCharPos" should be before "CenterStr"
      IntOp $6 $6 - $1
      IntOp $7 $6 + $1

      ;Loop until "LeftStr" is found or "String" reaches its start
      ${Do}
        ;Remove everything before and after the searched part ("TempStr")
        StrCpy $8 $R0 $1 $6

        ;If "LeftStr" is empty
        ${If} $R1 == ``
          StrCpy $6 0
          StrCpy $7 0
          ${ExitDo}
        ${EndIf}

        ;Compare "TempStr" with "LeftStr"
        ${IfThen} $8 == $R1 ${|} ${ExitDo} ${|}
        ;If not, this could be "String" start
        ${IfThen} $6 <= 0 ${|} ${ExitDo} ${|}
        ;If not, continue the loop
        IntOp $6 $6 - 1
        IntOp $7 $7 - 1
      ${Loop}

      # "LeftStr" is found or "String" start was reached

      ;Remove "LeftStr" from "String" if the user wants
      ${If} $R4 = ${FALSE}
        IntOp $6 $6 + $1
      ${EndIf}

      ;Record "LeftStr" first character position on "TempStr" (temporarily)
      StrCpy $8 $6

      ;"StartCharPos" and "EndCharPos" should be after "CenterStr"
      ${If} $R5 = ${FALSE}
        StrCpy $6 $4
      ${Else}
        IntOp $6 $4 + $2
      ${EndIf}
      IntOp $7 $6 + $3
      
      ;Record "LeftStr" first character position on "StartPos"
      StrCpy $4 $8

      ;Loop until "RightStr" is found or "String" reaches its end
      ${Do}
        ;Remove everything before and after the searched part ("TempStr")
        StrCpy $8 $R0 $3 $6

        ;If "RightStr" is empty
        ${If} $R3 == ``
          StrCpy $6 $0
          StrCpy $7 $0
          ${ExitDo}
        ${EndIf}

        ;Compare "TempStr" with "RightStr"
        ${IfThen} $8 == $R3 ${|} ${ExitDo} ${|}
        ;If not, this could be "String" end
        ${IfThen} $7 >= $0 ${|} ${ExitDo} ${|}
        ;If not, continue the loop
        IntOp $6 $6 + 1
        IntOp $7 $7 + 1
      ${Loop}

      ;Remove "RightStr" from "String" if the user wants
      ${If} $R6 = ${FALSE}
        IntOp $7 $7 - $3
      ${EndIf}

      ;Record "RightStr" last character position on "StartPos"
      StrCpy $5 $7

      ;As the positionment is relative...
      IntOp $5 $5 - $4

      ;Write the string and finish the job
      StrCpy $R0 $R0 $5 $4
      Goto +2

      Done:
      StrCpy $R0 ``

    /*After this point:
      ------------------------------------------
       $R0 = OutVar (output)*/

      ;Return output to user
      Pop $8
      Pop $7
      Pop $6
      Pop $5
      Pop $4
      Pop $3
      Pop $2
      Pop $1
      Pop $0
      Pop $R6
      Pop $R5
      Pop $R4
      Pop $R3
      Pop $R2
      Pop $R1
      Exch $R0
    FunctionEnd

  !macroend
  
  # Function StrStr
  ###############

  !macro FUNCTION_STRING_StrStr
    !insertmacro STRFUNC_FUNC `StrStr` `2004 Diego Pedroso - Based on functions by Ximon Eighteen`

    /*After this point:
      ------------------------------------------
       $R0 = StrToSearch (input)
       $R1 = String (input)
       $R2 = StrToSearchLen (temp)
       $R3 = StrLen (temp)
       $R4 = StartCharPos (temp)
       $R5 = TempStr (temp)*/

      ;Get input from user
      Exch $R0
      Exch
      Exch $R1
      Push $R2
      Push $R3
      Push $R4
      Push $R5

      ;Get "String" and "StrToSearch" length
      StrLen $R2 $R0
      StrLen $R3 $R1
      ;Start "StartCharPos" counter
      StrCpy $R4 0

      ;Loop until "StrToSearch" is found or "String" reaches its end
      ${Do}
        ;Remove everything before and after the searched part ("TempStr")
        StrCpy $R5 $R1 $R2 $R4

        ;Compare "TempStr" with "StrToSearch"
        ${IfThen} $R5 == $R0 ${|} ${ExitDo} ${|}
        ;If not "StrToSearch", this could be "String" end
        ${IfThen} $R4 >= $R3 ${|} ${ExitDo} ${|}
        ;If not, continue the loop
        IntOp $R4 $R4 + 1
      ${Loop}

    /*After this point:
      ------------------------------------------
       $R0 = OutVar (output)*/

      ;Remove part before "StrToSearch" on "String" (if there has one)
      StrCpy $R0 $R1 `` $R4

      ;Return output to user
      Pop $R5
      Pop $R4
      Pop $R3
      Pop $R2
      Pop $R1
      Exch $R0
    FunctionEnd

  !macroend

  # Function StrStrAdv
  ##################

  !macro FUNCTION_STRING_StrStrAdv
    !insertmacro STRFUNC_FUNC `StrStrAdv` `2003-2004 Diego Pedroso`

    /*After this point:
      ------------------------------------------
       $0 = String (input)
       $1 = StringToSearch (input)
       $2 = DirectionOfSearch (input)
       $3 = DirectionOfReturn (input)
       $4 = ShowStrToSearch (input)
       $5 = NumLoops (input)
       $6 = CaseSensitive (input)
       $7 = StringLength (temp)
       $8 = StrToSearchLength (temp)
       $9 = CurrentLoop (temp)
       $R0 = EndCharPos (temp)
       $R1 = StartCharPos (temp)
       $R2 = OutVar (output)
       $R3 = Temp (temp)*/

      ;Get input from user

      Exch $6
      Exch
      Exch $5
      Exch
      Exch 2
      Exch $4
      Exch 2
      Exch 3
      Exch $3
      Exch 3
      Exch 4
      Exch $2
      Exch 4
      Exch 5
      Exch $1
      Exch 5
      Exch 6
      Exch $0
      Exch 6
      Push $7
      Push $8
      Push $9
      Push $R3
      Push $R2
      Push $R1
      Push $R0

      ; Clean $R0-$R3 variables
      StrCpy $R0 ""
      StrCpy $R1 ""
      StrCpy $R2 ""
      StrCpy $R3 ""

      ; Verify if we have the correct values on the variables
      ${If} $0 == ``
        SetErrors ;AdvStrStr_StrToSearch not found
        Goto AdvStrStr_End
      ${EndIf}

      ${If} $1 == ``
        SetErrors ;No text to search
        Goto AdvStrStr_End
      ${EndIf}

      ${If} $2 != <
        StrCpy $2 >
      ${EndIf}

      ${If} $3 != <
        StrCpy $3 >
      ${EndIf}

      ${If} $4 <> 0
        StrCpy $4 1
      ${EndIf}

      ${If} $5 <= 0
        StrCpy $5 0
      ${EndIf}

      ${If} $6 <> 1
        StrCpy $6 0
      ${EndIf}

      ; Find "AdvStrStr_String" length
      StrLen $7 $0

      ; Then find "AdvStrStr_StrToSearch" length
      StrLen $8 $1

      ; Now set up basic variables

      ${If} $2 == <
        IntOp $R1 $7 - $8
        StrCpy $R2 $7
      ${Else}
        StrCpy $R1 0
        StrCpy $R2 $8
      ${EndIf}

      StrCpy $9 0 ; First loop

      ;Let's begin the search

      ${Do}
        ; Step 1: If the starting or ending numbers are negative
        ;         or more than AdvStrStr_StringLen, we return
        ;         error

        ${If} $R1 < 0
          StrCpy $R1 ``
          StrCpy $R2 ``
          StrCpy $R3 ``
          SetErrors ;AdvStrStr_StrToSearch not found
          Goto AdvStrStr_End
        ${ElseIf} $R2 > $7
          StrCpy $R1 ``
          StrCpy $R2 ``
          StrCpy $R3 ``
          SetErrors ;AdvStrStr_StrToSearch not found
          Goto AdvStrStr_End
        ${EndIf}

        ; Step 2: Start the search depending on
        ;         AdvStrStr_DirectionOfSearch. Chop down not needed
        ;         characters.

        ${If} $R1 <> 0
          StrCpy $R3 $0 `` $R1
        ${EndIf}

        ${If} $R2 <> $7
          ${If} $R1 = 0
            StrCpy $R3 $0 $8
          ${Else}
            StrCpy $R3 $R3 $8
          ${EndIf}
        ${EndIf}

        ; Step 3: Make sure that's the string we want

        ; Case-Sensitive Support <- Use "AdvStrStr_Temp"
        ; variable because it won't be used anymore

        ${If} $6 == 1
          System::Call `kernel32::lstrcmpA(ts, ts) i.s` `$R3` `$1`
          Pop $R3
          ${If} $R3 = 0
            StrCpy $R3 1 ; Continue
          ${Else}
            StrCpy $R3 0 ; Break
          ${EndIf}
        ${Else}
          ${If} $R3 == $1
            StrCpy $R3 1 ; Continue
          ${Else}
            StrCpy $R3 0 ; Break
          ${EndIf}
        ${EndIf}

        ; After the comparasion, confirm that it is the
        ; value we want.

        ${If} $R3 = 1

          ;We found it, return except if the user has set up to
          ;search for another one:
          ${If} $9 >= $5

            ;Now, let's see if the user wants
            ;AdvStrStr_StrToSearch to appear:
            ${If} $4 == 0
              ;Return depends on AdvStrStr_DirectionOfReturn
              ${If} $3 == <
                ; RTL
                StrCpy $R0 $0 $R1
              ${Else}
                ; LTR
                StrCpy $R0 $0 `` $R2
              ${EndIf}
              ${Break}
            ${Else}
              ;Return depends on AdvStrStr_DirectionOfReturn
              ${If} $3 == <
                ; RTL
                StrCpy $R0 $0 $R2
              ${Else}
                ; LTR
                StrCpy $R0 $0 `` $R1
              ${EndIf}
              ${Break}
            ${EndIf}
          ${Else}
            ;If the user wants to have more loops, let's do it so!
            IntOp $9 $9 + 1

            ${If} $2 == <
              IntOp $R1 $R1 - 1
              IntOp $R2 $R2 - 1
            ${Else}
              IntOp $R1 $R1 + 1
              IntOp $R2 $R2 + 1
            ${EndIf}
          ${EndIf}
        ${Else}
          ; Step 4: We didn't find it, so do steps 1 thru 3 again

          ${If} $2 == <
            IntOp $R1 $R1 - 1
            IntOp $R2 $R2 - 1
          ${Else}
            IntOp $R1 $R1 + 1
            IntOp $R2 $R2 + 1
          ${EndIf}
        ${EndIf}
      ${Loop}

      AdvStrStr_End:

      ;Add 1 to AdvStrStr_EndCharPos to be supportable
      ;by "StrCpy"

      IntOp $R2 $R2 - 1

      ;Return output to user

      Exch $R0
      Exch
      Pop $R1
      Exch
      Pop $R2
      Exch
      Pop $R3
      Exch
      Pop $9
      Exch
      Pop $8
      Exch
      Pop $7
      Exch
      Pop $6
      Exch
      Pop $5
      Exch
      Pop $4
      Exch
      Pop $3
      Exch
      Pop $2
      Exch
      Pop $1
      Exch
      Pop $0

    FunctionEnd

  !macroend

  # Function StrTok
  ###############

  !macro FUNCTION_STRING_StrTok
    !insertmacro STRFUNC_FUNC `StrTok` `2004 Diego Pedroso - Based on functions by "bigmac666"`
    /*After this point:
      ------------------------------------------
       $0 = SkipEmptyParts (input)
       $1 = ResultPart (input)
       $2 = Separators (input)
       $3 = String (input)
       $4 = StrToSearchLen (temp)
       $5 = StrLen (temp)
       $6 = StartCharPos (temp)
       $7 = TempStr (temp)
       $8 = CurrentLoop
       $9 = CurrentSepChar
       $R0 = CurrentSepCharNum
       */

      ;Get input from user
      Exch $0
      Exch
      Exch $1
      Exch
      Exch 2
      Exch $2
      Exch 2
      Exch 3
      Exch $3
      Exch 3
      Push $4
      Push $5
      Push $6
      Push $7
      Push $8
      Push $9
      Push $R0

      ;Parameter defaults
      ${IfThen} $2 == `` ${|} StrCpy $2 `|` ${|}
      ${IfThen} $1 == `` ${|} StrCpy $1 `L` ${|}
      ${IfThen} $0 == `` ${|} StrCpy $0 `0` ${|}

      ;Get "String" and "StrToSearch" length
      StrLen $4 $2
      StrLen $5 $3
      ;Start "StartCharPos" and "ResultPart" counters
      StrCpy $6 0
      StrCpy $8 -1

      ;Loop until "ResultPart" is met, "StrToSearch" is found or
      ;"String" reaches its end
      ResultPartLoop: ;"CurrentLoop" Loop

        ;Increase "CurrentLoop" counter
        IntOp $8 $8 + 1

        StrSearchLoop:
        ${Do} ;"String" Loop
          ;Remove everything before and after the searched part ("TempStr")
          StrCpy $7 $3 1 $6

          ;Verify if it's the "String" end
          ${If} $6 >= $5
            ;If "CurrentLoop" is what the user wants, remove the part
            ;after "TempStr" and itself and get out of here
            ${If} $8 == $1
            ${OrIf} $1 == `L`
              StrCpy $3 $3 $6
            ${Else} ;If not, empty "String" and get out of here
              StrCpy $3 ``
            ${EndIf}
            StrCpy $R0 `End`
            ${ExitDo}
          ${EndIf}

          ;Start "CurrentSepCharNum" counter (for "Separators" Loop)
          StrCpy $R0 0

          ${Do} ;"Separators" Loop
            ;Use one "Separators" character at a time
            ${If} $R0 <> 0
              StrCpy $9 $2 1 $R0
            ${Else}
              StrCpy $9 $2 1
            ${EndIf}

            ;Go to the next "String" char if it's "Separators" end
            ${IfThen} $R0 >= $4 ${|} ${ExitDo} ${|}

            ;Or, if "TempStr" equals "CurrentSepChar", then...
            ${If} $7 == $9
              StrCpy $7 $3 $6

              ;If "String" is empty because this result part doesn't
              ;contain data, verify if "SkipEmptyParts" is activated,
              ;so we don't return the output to user yet

              ${If} $7 == ``
              ${AndIf} $0 = ${TRUE}
                IntOp $6 $6 + 1
                StrCpy $3 $3 `` $6
                StrCpy $6 0
                Goto StrSearchLoop
              ${ElseIf} $8 == $1
                StrCpy $3 $3 $6
                StrCpy $R0 "End"
                ${ExitDo}
              ${EndIf} ;If not, go to the next result part
              IntOp $6 $6 + 1
              StrCpy $3 $3 `` $6
              StrCpy $6 0
              Goto ResultPartLoop
            ${EndIf}

            ;Increase "CurrentSepCharNum" counter
            IntOp $R0 $R0 + 1
          ${Loop}
          ${IfThen} $R0 == "End" ${|} ${ExitDo} ${|}
          
          ;Increase "StartCharPos" counter
          IntOp $6 $6 + 1
        ${Loop}

    /*After this point:
      ------------------------------------------
       $3 = OutVar (output)*/

      ;Return output to user

      Pop $R0
      Pop $9
      Pop $8
      Pop $7
      Pop $6
      Pop $5
      Pop $4
      Pop $0
      Pop $1
      Pop $2
      Exch $3
    FunctionEnd

  !macroend

  # Function StrTrimNewLines
  ########################

  !macro FUNCTION_STRING_StrTrimNewLines
    !insertmacro STRFUNC_FUNC `StrTrimNewLines` `2004 Diego Pedroso - Based on functions by Ximon Eighteen`

    /*After this point:
      ------------------------------------------
       $R0 = String (input)
       $R1 = TrimCounter (temp)
       $R2 = Temp (temp)*/

      ;Get input from user
      Exch $R0
      Push $R1
      Push $R2
      
      ;Initialize trim counter
      StrCpy $R1 0

      loop:
        ;Subtract to get "String"'s last characters
        IntOp $R1 $R1 - 1

        ;Verify if they are either $\r or $\n
        StrCpy $R2 $R0 1 $R1
        ${If} $R2 == `$\r`
        ${OrIf} $R2 == `$\n`
          Goto loop
        ${EndIf}

      ;Trim characters (if needed)
      IntOp $R1 $R1 + 1
      ${If} $R1 < 0
        StrCpy $R0 $R0 $R1
      ${EndIf}

    /*After this point:
      ------------------------------------------
       $R0 = OutVar (output)*/

      ;Return output to user
      Pop $R2
      Pop $R1
      Exch $R0
    FunctionEnd

  !macroend

  ;Function Calls for Install and Uninstall

  !macro FUNCTION_STRING_StrCase_Call ResultVar String Type
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrCase} "${ResultVar}" "${String}" "${Type}"`
    !verbose pop

    Push `${String}`
    Push `${Type}`
    Call StrCase
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrCase_Call ResultVar String Type
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrCase} "${ResultVar}" "${String}" "${Type}"`
    !verbose pop

    Push `${String}`
    Push `${Type}`
    Call un.StrCase
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrClb_Call ResultVar String Action
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrClb} "${ResultVar}" "${String}" "${Action}"`
    !verbose pop

    Push `${String}`
    Push `${Action}`
    Call StrClb
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrClb_Call ResultVar String Action
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrClb} "${ResultVar}" "${String}" "${Action}"`
    !verbose pop

    Push `${String}`
    Push `${Action}`
    Call un.StrClb
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrIOToNSIS_Call ResultVar String
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrIOToNSIS} "${ResultVar}" "${String}"`
    !verbose pop

    Push `${String}`
    Call StrIOToNSIS
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrIOToNSIS_Call ResultVar String
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrIOToNSIS} "${ResultVar}" "${String}"`
    !verbose pop

    Push `${String}`
    Call un.StrIOToNSIS
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrLoc_Call ResultVar String StrToSearchFor OffsetDirection
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrLoc} "${ResultVar}" "${String}" "${StrToSearchFor}" "${OffsetDirection}"`
    !verbose pop

    Push `${String}`
    Push `${StrToSearchFor}`
    Push `${OffsetDirection}`
    Call StrLoc
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrLoc_Call ResultVar String StrToSearchFor OffsetDirection
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrLoc} "${ResultVar}" "${String}" "${StrToSearchFor}" "${OffsetDirection}"`
    !verbose pop

    Push `${String}`
    Push `${StrToSearchFor}`
    Push `${OffsetDirection}`
    Call un.StrLoc
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrNSISToIO_Call ResultVar String
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrNSISToIO} "${ResultVar}" "${String}"`
    !verbose pop

    Push `${String}`
    Call StrNSISToIO
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrNSISToIO_Call ResultVar String
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrNSISToIO} "${ResultVar}" "${String}"`
    !verbose pop

    Push `${String}`
    Call un.StrNSISToIO
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrRep_Call ResultVar String StringToReplace ReplacementString
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrRep} "${ResultVar}" "${String}" "${StringToReplace}" "${ReplacementString}"`
    !verbose pop

    Push `${String}`
    Push `${StringToReplace}`
    Push `${ReplacementString}`
    Call StrRep
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrRep_Call ResultVar String StringToReplace ReplacementString
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrRep} "${ResultVar}" "${String}" "${StringToReplace}" "${ReplacementString}"`
    !verbose pop

    Push `${String}`
    Push `${StringToReplace}`
    Push `${ReplacementString}`
    Call un.StrRep
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrSort_Call ResultVar String CenterStr LeftStr RightStr IncludeCenterStr IncludeLeftStr IncludeRightStr
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrSort} "${ResultVar}" "${String}" "${CenterStr}" "${LeftStr}" "${RightStr}" "${IncludeCenterStr}" "${IncludeLeftStr}" "${IncludeRightStr}"`
    !verbose pop

    Push `${String}`
    Push `${CenterStr}`
    Push `${LeftStr}`
    Push `${RightStr}`
    Push `${IncludeCenterStr}`
    Push `${IncludeLeftStr}`
    Push `${IncludeRightStr}`
    Call StrSort
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrSort_Call ResultVar String CenterStr LeftStr RightStr IncludeCenterStr IncludeLeftStr IncludeRightStr
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrSort} "${ResultVar}" "${String}" "${CenterStr}" "${LeftStr}" "${RightStr}" "${IncludeCenterStr}" "${IncludeLeftStr}" "${IncludeRightStr}"`
    !verbose pop

    Push `${String}`
    Push `${CenterStr}`
    Push `${LeftStr}`
    Push `${RightStr}`
    Push `${IncludeCenterStr}`
    Push `${IncludeLeftStr}`
    Push `${IncludeRightStr}`
    Call un.StrSort
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrStr_Call ResultVar String StrToSearchFor
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrStr} "${ResultVar}" "${String}" "${StrToSearchFor}"`
    !verbose pop

    Push `${String}`
    Push `${StrToSearchFor}`
    Call StrStr
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrStr_Call ResultVar String StrToSearchFor
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrStr} "${ResultVar}" "${String}" "${StrToSearchFor}"`
    !verbose pop

    Push `${String}`
    Push `${StrToSearchFor}`
    Call un.StrStr
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrStrAdv_Call ResultVar String StrToSearchFor SearchDirection ResultStrDirection DisplayStrToSearch Loops CaseSensitive
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrStrAdv} "${ResultVar}" "${String}" "${StrToSearchFor}" "${SearchDirection}" "${ResultStrDirection}" "${DisplayStrToSearch}" "${Loops}" "${CaseSensitive}"`
    !verbose pop

    Push `${String}`
    Push `${StrToSearchFor}`
    Push `${SearchDirection}`
    Push `${ResultStrDirection}`
    Push `${DisplayStrToSearch}`
    Push `${Loops}`
    Push `${CaseSensitive}`
    Call StrStrAdv
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrStrAdv_Call ResultVar String StrToSearchFor SearchDirection ResultStrDirection DisplayStrToSearch Loops CaseSensitive
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrStrAdv} "${ResultVar}" "${String}" "${StrToSearchFor}" "${SearchDirection}" "${ResultStrDirection}" "${DisplayStrToSearch}" "${Loops}" "${CaseSensitive}"`
    !verbose pop

    Push `${String}`
    Push `${StrToSearchFor}`
    Push `${SearchDirection}`
    Push `${ResultStrDirection}`
    Push `${DisplayStrToSearch}`
    Push `${Loops}`
    Push `${CaseSensitive}`
    Call un.StrStrAdv
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrTok_Call ResultVar String Separators ResultPart SkipEmptyParts
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrTok} "${ResultVar}" "${String}" "${Separators}" "${ResultPart}" "${SkipEmptyParts}"`
    !verbose pop

    Push `${String}`
    Push `${Separators}`
    Push `${ResultPart}`
    Push `${SkipEmptyParts}`
    Call StrTok
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrTok_Call ResultVar String Separators ResultPart SkipEmptyParts
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrTok} "${ResultVar}" "${String}" "${Separators}" "${ResultPart}" "${SkipEmptyParts}"`
    !verbose pop

    Push `${String}`
    Push `${Separators}`
    Push `${ResultPart}`
    Push `${SkipEmptyParts}`
    Call un.StrTok
    Pop `${ResultVar}`
  !macroend

  !macro FUNCTION_STRING_StrTrimNewLines_Call ResultVar String
    !verbose push
    ;!exodus verbose 4
    !echo `$ {StrTrimNewLines} "${ResultVar}" "${String}"`
    !verbose pop

    Push `${String}`
    Call StrTrimNewLines
    Pop `${ResultVar}`
  !macroend
  !macro FUNCTION_STRING_UnStrTrimNewLines_Call ResultVar String
    !verbose push
    ;!exodus verbose 4
    !echo `$ {UnStrTrimNewLines} "${ResultVar}" "${String}"`
    !verbose pop

    Push `${String}`
    Call un.StrTrimNewLines
    Pop `${ResultVar}`
  !macroend

!endif
!verbose 3
!define STRFUNC_VERBOSITY ${_STRFUNC_VERBOSITY}
!undef _STRFUNC_VERBOSITY
!verbose pop

;----------------------------------------
;--------- END OF StrFunc.nsh
;----------------------------------------



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

  Name "${EXO_PRODUCTNAME_AND_DOTTEDMICROVERSION}"
  Outfile "${EXO_OUTFILE}"
  InstallDir "${EXO_INSTDIR}"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\${EXO_REGKEY_VER}" ""

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

; StrCmp "${EXO_PLATFORM}" "x86" 0 nobla
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

  ;VC runtime without bloated redist package (maybe can do this for vs2005 vc8 only?)
  ;also delivering debug versions?! so exodus programs can be developed with stackwalker
  ;File "${EXO_TOOLPATH}..\..\VC\redist\${EXO_PLATFORM}\Microsoft.VC${EXO_VCVERSION}.CRT\*"
  ;File "${EXO_TOOLPATH}..\..\VC\redist\Debug_NonRedist\${EXO_PLATFORM}\Microsoft.VC${EXO_VCVERSION}.DebugCRT\*"
  ;hard coded!
  File "${EXO_TOOLPATHREL}\msvcr${EXO_VCVERSION}.dll"
  File "${EXO_TOOLPATHREL}\msvcp${EXO_VCVERSION}.dll"
  File "${EXO_TOOLPATHDEB}\msvcr${EXO_VCVERSION}d.dll"
  File "${EXO_TOOLPATHDEB}\msvcp${EXO_VCVERSION}d.dll"

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
  createDirectory "${EXO_USERHOME}"
  createDirectory "${EXO_USERHOME}\src"
  createDirectory "${EXO_USERHOME}\bin"
  SetOutPath "${EXO_USERHOME}\src"

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
  File test\src\testmain.cpp

  ;example of recursive with excludes
  ;File /r /x neosys*.pdb /x imc*.* neosys.net\*.*

  ;append PATH to the binaries in system environment
  ;make available to all users
  ;http://nsis.sourceforge.net/Path_Manipulation
  ;dont risk prepending because could overwrite system commands
  ;${EnvVarUpdate} $0 "PATH" "P" "HKLM" "$INSTDIR\bin"
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\bin"

  ;prepend and INCLUDE and LIBPATH to enable development for all users
  ;http://nsis.sourceforge.net/Path_Manipulation
  ;
  ;LIBPATH=c:\Exodus115\lib
  ;EXO_LIBPATH=c:\Exodus115\lib
  ;EXO_LIBPATH_11.5=c:\Exodus115\lib
  ;
  ;INCLUDE=c:\Exodus115\include
  ;EXO_INCLUDE=c:\Exodus115\include
  ;EXO_INCLUDE_11.5=c:\Exodus115\include
  ;
  ${EnvVarUpdate} $0 "LIBPATH" "P" "HKLM" "$INSTDIR\lib"
  ${EnvVarUpdate} $0 "INCLUDE" "P" "HKLM" "$INSTDIR\include"
  ${EnvVarUpdate} $0 "EXO_LIBPATH" "P" "HKLM" "$INSTDIR\lib"
  ${EnvVarUpdate} $0 "EXO_INCLUDE" "P" "HKLM" "$INSTDIR\include"
  ${EnvVarUpdate} $0 "EXO_LIBPATH_${EXO_DOTTEDMINORVERSION}" "P" "HKLM" "$INSTDIR\lib"
  ${EnvVarUpdate} $0 "EXO_INCLUDE_${EXO_DOTTEDMINORVERSION}" "P" "HKLM" "$INSTDIR\include"

  SetShellVarContext all

;  ;create a path to the user compiled binaries HKCU=Current User (add to "all users user??)
;  ;cannot see to add %APPDATA% in local machine path so have to do for current user only :(
;appending ... not sure why not prepending
  ${EnvVarUpdate} $0 "PATH" "A" "HKCU" "${EXO_USERBIN}"

  ;record the installation folder in the registry
  WriteRegStr HKLM "Software\${EXO_REGKEY_VER}" "" $INSTDIR

  createDirectory "$SMPROGRAMS\${EXO_MENUDIR}"

  createShortCut "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_COMMANDLINE}.lnk"     "$INSTDIR\bin\exodus.exe"

  createShortCut "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_CONFIGURE}.lnk" "$INSTDIR\bin\configexodus.exe"
  ;make configure "run as" so it can copy dll to postgresql bin
  push "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_CONFIGURE}.lnk"
  call ShellLinkSetRunAs
  pop $0
  DetailPrint HR=$0

  ;Create uninstaller
  ;WriteUninstaller "$INSTDIR\bin\Uninstall.exe" appears we cannot do this
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  # create a shortcut in the start menu programs directory to the uninstaller
  createShortCut "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_UNINSTALL_TITLE}.lnk" "$INSTDIR\Uninstall.exe"

  #create a Windows Uninstall Item (title and path to uninstaller)
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXO_MENUDIR}" \
   "DisplayName" "${EXO_MENUDIR} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXO_MENUDIR}" \
   "UninstallString" "$INSTDIR\Uninstall.exe"

SectionEnd


!define REDIST_FILENAME "vcredist_tmp.exe"
Section "${REDIST_DESC} Redist (req.)" SEC_CRT

;  SectionIn RO

;  ; Detection made easy: Unlike previous redists, VC2010 now generates a platform
;  ; independent key for checking availability.
;  
  ;VCVER is like "10.0"
  ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\$%EXO_REDISTDOTVER%\VC\Runtimes\$%TARGET_CPU%" "Installed"
;  IfErrors done
  StrCmp $R0 "1" done

  ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\$%EXO_REDISTDOTVER%\VC\VCRedist\$%TARGET_CPU%" "Installed"
;  IfErrors done
  StrCmp $R0 "1" done

  SetOutPath "$TEMP"

  ;download source 1
  DetailPrint "Downloading ${REDIST_DESC} Redistributable Setup..."
  DetailPrint "Contacting ${REDIST_SOURCE1} ..."
  NSISdl::download /TIMEOUT=15000 "${REDIST_URL1}" "${REDIST_FILENAME}"

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

  LangString DESC_CRT    ${LANG_ENGLISH} "Should only be downloaded if you don't already have it installed but if you know you have it uncheck this if pre-2010 to save time."

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

  Delete "${EXO_USERHOME}\src\compile.cpp"
  Delete "${EXO_USERHOME}\src\createfile.cpp"
  Delete "${EXO_USERHOME}\src\createindex.cpp"
  Delete "${EXO_USERHOME}\src\delete.cpp"
  Delete "${EXO_USERHOME}\src\deletefile.cpp"
  Delete "${EXO_USERHOME}\src\deleteindex.cpp"
  Delete "${EXO_USERHOME}\src\edic.cpp"
  Delete "${EXO_USERHOME}\src\edir.cpp"
  Delete "${EXO_USERHOME}\src\list.cpp"
  Delete "${EXO_USERHOME}\src\printtext.h"
  Delete "${EXO_USERHOME}\src\listfiles.cpp"
  Delete "${EXO_USERHOME}\src\listindexes.cpp"
  Delete "${EXO_USERHOME}\src\testsort.cpp"
  Delete "${EXO_USERHOME}\src\configexodus.cpp"
  Delete "${EXO_USERHOME}\src\testmain.cpp"

  Delete "${EXO_USERHOME}\src\blanksolution\*.*"
  RMDir "${EXO_USERHOME}\src\blanksolution"

  ;src and bin may remain if they created any programs
  RMDir "${EXO_USERHOME}\src"
  RMDir "${EXO_USERHOME}\bin"

  ;Exodus is likely to remain since we will not delete .exodus
  RMDir "${EXO_USERHOME}"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  RMDir "$PROGRAMFILES\${EXO_PRODUCTNAME}"

  ;unrecord the installation folder in the registry
  DeleteRegKey /ifempty HKLM "Software\${EXO_REGKEY_VER}"
  DeleteRegKey /ifempty HKLM "Software\${EXO_REGKEY_ROOT}"

  SetShellVarContext all

  # second, remove the link from the start menu
  delete "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_UNINSTALL_TITLE}.lnk"
  delete "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_COMMANDLINE}.lnk"
  delete "$SMPROGRAMS\${EXO_MENUDIR}\${EXO_MENUITEM_TITLE_CONFIGURE}.lnk"

  RMDir "$SMPROGRAMS\${EXO_MENUDIR}"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXO_MENUDIR}"

  ${un.EnvVarUpdate} $0 "LIBPATH" "R" "HKLM" "$INSTDIR\lib"
  ${un.EnvVarUpdate} $0 "INCLUDE" "R" "HKLM" "$INSTDIR\include"
  ${un.EnvVarUpdate} $0 "EXO_LIBPATH" "R" "HKLM" "$INSTDIR\lib"
  ${un.EnvVarUpdate} $0 "EXO_INCLUDE" "R" "HKLM" "$INSTDIR\include"
  ${un.EnvVarUpdate} $0 "EXO_LIBPATH_${EXO_DOTTEDMINORVERSION}" "R" "HKLM" "$INSTDIR\lib"
  ${un.EnvVarUpdate} $0 "EXO_INCLUDE_${EXO_DOTTEDMINORVERSION}" "R" "HKLM" "$INSTDIR\include"

  ;remove the path to installation binaries HKLM=Local Machine
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"

  ;remove the path to the user compiled binaries HKCU=Current User
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "${EXO_USERBIN}"

SectionEnd


;--------------------------------
;onInit Function

Function .onInit

  ;skip out if no uninstaller found
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${EXO_MENUDIR}" \
  "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "${EXO_MENUDIR} is already installed. $\n$\nClick `OK` to remove the \
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



