;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Primary NSIS installer script
;Author         Kenneth J. Davis <jeremyd@computer.org> (2002,2003)
;Copyright      Alan Horkan <horkana@tcd.ie> (2002)
;               Michael D. Pritchett <mpritchett@attglobal.net> (2002)
;               [add your name here]
;Version        see AbiSource CVS


; Include user settable values (compile time options)
!include "abi_options.nsh"


; Some checks of user defines
!ifdef NODOWNLOADS & OPT_CRTL_URL
!warning "OPT_CRTL_URL and NODOWNLOADS both defined, ignoring OPT_CRTL_URL"
!undef OPT_CRTL_URL
!endif
!ifdef OPT_CRTL_URL & OPT_CRTL_LOCAL
!error "OPT_CRTL_URL and OPT_CRTL_LOCAL cannot both be defined"
!endif


; Declarations
!define PRODUCT "AbiWord"

!ifndef VERSION_MAJOR
!define VERSION_MAJOR "2"
!endif
!ifndef VERSION_MINOR
!define VERSION_MINOR "1"
!endif
!ifndef VERSION_MICRO
!define VERSION_MICRO "0"
!endif
!ifndef VERSION
!define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"
!endif
!define INSTALLERNAME "setup_abiword.${VERSION_MAJOR}-${VERSION_MINOR}-${VERSION_MICRO}.exe"
;!define INSTALLERNAME "setup_abiword.exe"

!define APPSET  "AbiSuite"
!define PROGRAMEXE "AbiWord.exe"
!define MAINPROGRAM "AbiWord\bin\${PROGRAMEXE}"


; Do a Cyclic Redundancy Check to make sure the installer
; was not corrupted by the download.  
CRCCheck on

; set the compression algorithm used, zlib | bzip2
SetCompressor bzip2

; where to look for NSIS plugins during setup creation
; default includes ./plugins, but we also want to check current directory
!addplugindir .

; compresses the header
!ifdef HAVE_UPX
!packhdr tmp.dat "upx --best tmp.dat"
!endif

; Specify the icons to use
!ifndef TRADEMARKED_BUILD
; Personal build (to be used by all!!! except SourceGear Inc.)
!define MUI_ICON "..\..\pkg\win\setup\setup.ico"
!define MUI_UNICON "..\..\pkg\win\setup\setup.ico"
!else
; Trademarked build
!define MUI_ICON "..\..\pkg\win\setup\setup_tm.ico"
!define MUI_UNICON "..\..\pkg\win\setup\setup_tm.ico"
!endif

; Specify the bitmap to use
!define MUI_CHECKBITMAP "..\..\pkg\win\setup\modern.bmp"

; Specify the license text to use
!define MUI_LICENSEDATA "..\AbiSuite\Copying"

; Specify filename of resulting installer
OutFile "${INSTALLERNAME}"

; The name displayed by the installer
Name "${PRODUCT} ${VERSION}"

; The default installation directory
InstallDir $PROGRAMFILES\${APPSET}${VERSION_MAJOR}      ; e.g. "C:\Program Files\AbiSuite2"

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Install_Dir"


; Support 'Modern' UI and multiple languages
!ifndef CLASSIC_UI

  ; include the Modern UI support
  !include "Mui.nsh"

  ; specify the pages and order to show to user

  ; introduce ourselves
  !insertmacro MUI_PAGE_WELCOME
  ; including the license of AbiWord  (could be localized, but we don't)
  !insertmacro MUI_PAGE_LICENSE '${MUI_LICENSEDATA}'
  ; allow user to select what parts to install
  !insertmacro MUI_PAGE_COMPONENTS
  ; and where to install to
  !insertmacro MUI_PAGE_DIRECTORY
  ; where to put in the start menu
    !define MUI_STARTMENUPAGE_DEFAULTFOLDER "$(SM_PRODUCT_GROUP)"
    !define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
    !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}"
    !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenu Name"
  !insertmacro MUI_PAGE_STARTMENU
  ; allow insertion of our custom pages
  !define MUI_CUSTOMPAGECOMMANDS
  ; query user for download mirror to use (only if dl item was selected though)
  !ifndef NODOWNLOADS
    !ifdef OPT_DICTIONARIES
      Page custom getDLMirror ; Custom page to get DL mirror
    !endif
  !endif
  ; actually install the files
  !insertmacro MUI_PAGE_INSTFILES
  ; specify Finish Page settings
    ; prompt to run AbiWord (start with readme.txt)
    !define MUI_FINISHPAGE_RUN "$INSTDIR\${MAINPROGRAM}"
    !define MUI_FINISHPAGE_RUN_PARAMETERS  $\"$INSTDIR\readme.txt$\"
    ; or uncomment to allow viewing readme with default text editor
    ;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
    ; force user to close so they can see install done & not start readme
    !define MUI_FINISHPAGE_NOAUTOCLOSE
  !insertmacro MUI_PAGE_FINISH

  ; warn user if they try to quit the install before it completes
  !define MUI_ABORTWARNING

  ; create an uninstaller and specify the pages it should have
;  !define MUI_UNINSTALLER
;    !define MUI_UNCONFIRMPAGE
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  ; specify where to get resources from for UI elements (default)
  !define MUI_UI "${NSISDIR}\Contrib\UIs\modern.exe"
  ;!define MUI_UI_SMALLDESCRIPTION "${NSISDIR}\Contrib\UIs\modern-smalldesc.exe"


  ; Languages
  ; some are presently commented out as they lack some translations (hence fail to build)
;  !insertmacro MUI_LANGUAGE "Bulgarian"
;  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "TradChinese"
;  !insertmacro MUI_LANGUAGE "Ukrainian"

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
  !define MUI_LANGDLL_REGISTRY_KEY "Software\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}"
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"


  ; Reserve Files to possibly aid in starting faster
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
  !insertmacro MUI_RESERVEFILE_SPECIALINI
  !insertmacro MUI_RESERVEFILE_SPECIALBITMAP

; reserve room at start of compressed archive for plugins (not already reserved by Modern UI)
!define RESERVE_PLUGINS
!ifdef RESERVE_PLUGINS
  ; ReserveFile [/nonfatal] [/r] file [file...]
  ReserveFile "${NSISDIR}\Plugins\LangDLL.dll"
  !ifndef NODOWNLOADS
    ReserveFile "${NSISDIR}\Plugins\NSISdl.dll"
  !endif
  !ifdef OPT_DICTIONARIES
    ;ReserveFile "${NSISDIR}\Plugins\untgz.dll"
    ReserveFile "untgz.dll"
  !endif
!endif ; RESERVE_PLUGINS


!else ; CLASSIC_UI

  Caption "${PRODUCT} ${VERSION} Setup"

  ; no WindowsXP manifest stuff
  XPStyle off
  
  ; The text to prompt the user to enter a directory
  DirText "Choose a directory to install in to:"

  ; set the icons to use
  Icon ${MUI_ICON}
  UninstallIcon ${MUI_UNICON}

  ; set the checkmark bitmaps to use
  CheckBitmap ${MUI_CHECKBITMAP}

  ; The text to prompt the user to select options for installation
  ComponentText "This will install Abiword on your computer. Select which optional components you want installed."

  ; License Information
  LicenseText "This program is Licensed under the GNU General Public License (GPL)."
  LicenseData ${MUI_LICENSEDATA}

!endif


; Language Strings
!define LANG_X					; used to prevent a warning about not being defined
;!include "abi_lng_Bulgarian.nsh"
;!include "abi_lng_Czech.nsh"
!include "abi_lng_Dutch.nsh"
!include "abi_lng_English.nsh"
!include "abi_lng_French.nsh"
!include "abi_lng_German.nsh"
!include "abi_lng_Greek.nsh"
!include "abi_lng_Italian.nsh"
!include "abi_lng_Japanese.nsh"
!include "abi_lng_Polish.nsh"
!include "abi_lng_PortugueseBR.nsh"
!include "abi_lng_Russian.nsh"
!include "abi_lng_SimpChinese.nsh"
!include "abi_lng_Spanish.nsh"
!include "abi_lng_TradChinese.nsh"
;!include "abi_lng_Ukrainian.nsh"


; Install types 
InstType "Typical (default)"              ;Section 1
InstType "Full (with File Associations)"  ;Section 2
InstType "Minimal"                        ;Section 3
; any other combination is "Custom"


!ifdef CLASSIC_UI
  ;Page declaration [optional, just specifies default behavior]
  Page license
  Page components
  Page directory
  !ifndef NODOWNLOADS
    !ifdef OPT_DICTIONARIES
      Page custom getDLMirror ; Custom page to get DL mirror
    !endif
  !endif
  Page instfiles
!endif



; Associate file extension and content type with an application entry
!macro CreateFileAssociation extension appType contentType
	WriteRegStr HKCR "${extension}" "" "${appType}"
	WriteRegStr HKCR "${extension}" "Content Type" "${contentType}"
!macroend


; Create a language localized Start Menu group
!macro lngCreateSMGroup group
	push $0
	StrCpy $0 "${group}"
	CreateDirectory "$SMPROGRAMS\$0"
	pop $0
!macroend

; Create a language localized Start Menu ShortCut
; we split the link.lnk up so we can use localized components without confusing NSIS
!macro lngCreateShortCut basedir group linkname target.file parameters icon.file icon_index_number
	push $0
	push $1
	push $2
	StrCpy $0 "${basedir}"
	StrCpy $0 "$0\"
	; if group is empty skip past it
	StrCpy $1 "${group}"
	StrLen $2 "$1"
	IntCmp $2 0 +3
	StrCpy $0 "$0$1"
	StrCpy $0 "$0\"
	;skipGroup:
	StrCpy $1 ${linkname}
	StrCpy $0 "$0$1"
	StrCpy $0 "$0.lnk"
	CreateShortCut $0 "${target.file}" "${parameters}" "${icon.file}" ${icon_index_number}
	pop $2
	pop $1
	pop $0
!macroend


; *********************************************************************


SubSection /e "$(TITLE_ssection_core)" ssection_core

; The stuff that must be installed
Section "$(TITLE_section_abi)" section_abi
	SectionIn 1 2 3 RO	; included in Typical, Full, Minimal, Required
	;;
	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\${MAINPROGRAM}" 0 DoInstall
	
	MessageBox MB_YESNO "$(PROMPT_OVERWRITE)" IDYES DoInstall
	
	Abort "$(MSG_ABORT)"

	DoInstall:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\${PRODUCT}\bin
	File "${PROGRAMEXE}"

	!ifdef MINGW32
		File "libAbiWord.dll"
	!endif

	SetOutPath $INSTDIR\${PRODUCT}
	File "..\AbiSuite\AbiWord\system.*"
	File /r "..\AbiSuite\AbiWord\strings"

	SetOutPath $INSTDIR
	File /oname=copying.txt "..\AbiSuite\Copying"
	File "..\AbiSuite\readme.txt"

	; Special Install of Dingbats font
	SetOutPath $TEMP
	File "..\..\pkg\win\setup\Dingbats.ttf"
	IfFileExists "$WINDIR\Fonts\Dingbats.ttf" EraseTemp 0
		CopyFiles /SILENT "$TEMP\Dingbats.ttf" "$WINDIR\Fonts" 
	EraseTemp:
	Delete $TEMP\Dingbats.ttf
  
	; TODO: determine if we should be using HKCU instead of HKLM for some of these

	; Write the language used during installation into the registry for uninstall to use
	; WriteRegStr HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Installer Language" "$LANGUAGE"

	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Install_Dir" "$INSTDIR"

	; (User Informational Purposes ONLY!!!)
	; Write the current version installed to the registery
	WriteRegStr HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Version" "${VERSION}"

	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}${VERSION_MAJOR}" "DisplayName" "${PRODUCT} ${VERSION} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}${VERSION_MAJOR}" "UninstallString" '"$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe"'

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${PROGRAMEXE}" "" '"$INSTDIR\${MAINPROGRAM}"'


        ; Write the start menu entry (if user selected to do so)
        !ifndef CLASSIC_UI
          !insertmacro MUI_STARTMENU_WRITE_BEGIN
  	  ;SetShellVarContext current|all???
	  !insertmacro lngCreateSMGroup  "$MUI_STARTMENU_FOLDER"
	  !insertmacro lngCreateShortCut "$SMPROGRAMS" "$MUI_STARTMENU_FOLDER" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	  !insertmacro lngCreateShortCut "$SMPROGRAMS" "$MUI_STARTMENU_FOLDER" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
          !insertmacro MUI_STARTMENU_WRITE_END
        !endif

	; New Uninstaller
	WriteUninstaller "Uninstall${PRODUCT}${VERSION_MAJOR}.exe"

SectionEnd

; OPTIONAL Registry Settings
Section "$(TITLE_section_shellupdate)" section_shellupdate
	SectionIn 1 2 3
	; Write the AbiSuite.AbiWord Keys
	WriteRegStr HKCR "${APPSET}.${PRODUCT}" "" "${PRODUCT} Document"
	WriteRegStr HKCR "${APPSET}.${PRODUCT}\DefaultIcon" "" "$INSTDIR\${MAINPROGRAM},2"
	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\command" "" '"$INSTDIR\${MAINPROGRAM}" "%1"'
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\command" "" "$INSTDIR\${MAINPROGRAM}"
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\ddeexec" "" '[Open("%1")]'
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\ddeexec\application" "" "${PRODUCT}"
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\ddeexec\topic" "" "System"

	; Write File Associations
	!insertmacro CreateFileAssociation ".abw"  "${APPSET}.${PRODUCT}" "application/abiword"
	!insertmacro CreateFileAssociation ".awt"  "${APPSET}.${PRODUCT}" "application/abiword-template"
	!insertmacro CreateFileAssociation ".zabw" "${APPSET}.${PRODUCT}" "application/abiword-compressed"

SectionEnd

SubSectionEnd ; core


; *********************************************************************
!ifdef CLASSIC_UI


SubSection /e "$(TITLE_ssection_shortcuts)" ssection_shortcuts

SubSection /e "$(TITLE_ssection_shortcuts_cu)" ssection_shortcuts_cu

; OPTIONAL Start Menu Shortcut for the current user profile
Section "$(TITLE_section_sm_shortcuts_cu)" section_sm_shortcuts_cu
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	!insertmacro lngCreateSMGroup  "$(SM_PRODUCT_GROUP)"
	!insertmacro lngCreateShortCut "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	!insertmacro lngCreateShortCut "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
SectionEnd

; OPTIONAL Desktop Shortcut for the current user profile
Section "$(TITLE_section_desktop_shortcuts_cu)" section_desktop_shortcuts_cu
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	!insertmacro lngCreateShortCut "$DESKTOP" "" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
SectionEnd


SubSectionEnd ; Shortcuts (Current User)
SubSection /e "$(TITLE_ssection_shortcuts_au)" ssection_shortcuts_au


; OPTIONAL Start Menu Shortcut for the special All User profile (not used in win9x) 
Section "$(TITLE_section_sm_shortcuts_au)" section_sm_shortcuts_au
	SectionIn 2		; off by default, included in 2 Full Install
	SetShellVarContext all  	; set to all, reset at end of section
	!insertmacro lngCreateSMGroup  "$(SM_PRODUCT_GROUP)"
	!insertmacro lngCreateShortCut "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	!insertmacro lngCreateShortCut "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	SetShellVarContext current  	; This is pro'ly overkill
SectionEnd


; OPTIONAL Desktop Shortcut for All Users
Section "$(TITLE_section_desktop_shortcuts_au)" section_desktop_shortcuts_au
	SectionIn 2	; not in default, included in 2 Full Install
	SetShellVarContext all  	;  All users 
	!insertmacro lngCreateShortCut "$DESKTOP" "" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	SetShellVarContext current  	; reset to current user
SectionEnd

SubSectionEnd ; Shortcuts (All Users)"

SubSectionEnd ; Shortcuts


!endif ;CLASSIC_UI
; *********************************************************************


SubSection /e "$(TITLE_ssection_gen_file_assoc)" ssection_gen_file_assoc

; OPTIONAL 
Section "$(TITLE_section_fa_doc)" section_fa_doc
	SectionIn 2
	!insertmacro CreateFileAssociation ".doc"  "${APPSET}.${PRODUCT}" "application/abiword"
SectionEnd

; OPTIONAL 
Section "$(TITLE_section_fa_rtf)" section_fa_rtf
	SectionIn 2
	!insertmacro CreateFileAssociation ".rtf"  "${APPSET}.${PRODUCT}" "application/abiword"
SectionEnd

SubSectionEnd ; general file associations


; *********************************************************************


SubSection /e "$(TITLE_ssection_helper_files)" ssection_helper_files

; MORE OPTIONS
; language packs, clipart, help docs, templates etc.   

; OPTIONAL Installation of Help Files
Section "$(TITLE_section_help)" section_help
	SectionIn 1 2
	SetOutPath $INSTDIR\AbiWord
	; help documents may not be created if peer abiword-docs not found
	File /nonfatal /r "..\abisuite\abiword\help"
SectionEnd

; OPTIONAL Installation of Templates
Section "$(TITLE_section_templates)" section_templates
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\templates"
SectionEnd

; OPTIONAL Installation of Samples - REMOVED
;Section "$(TITLE_section_samples)" section_samples
;	SectionIn 1 2
;	SetOutPath $INSTDIR\AbiWord
;	File /r "..\AbiSuite\AbiWord\sample"
;SectionEnd

; OPTIONAL Installation of Clipart
Section "$(TITLE_section_clipart)" section_clipart
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\clipart"
SectionEnd


!ifdef OPT_CRTL_LOCAL
; OPTIONAL Installation of c runtime library dll
; TODO: this is really only needed for Win95, so hide on all other OSes
Section "$(TITLE_section_crtlib_local)" section_crtlib_local
	SectionIn 2	; select if full installation choosen
	SetOutPath $INSTDIR\${PRODUCT}\bin
	File "${OPT_CRTL_LOCAL}${OPT_CRTL_FILENAME}"
SectionEnd
!endif ; OPT_CRTL_LOCAL

; we only enable this option if a url to connect to was
; specified during installation building; this should
; only be enabled for release builds if your server (where
; the url points) can handle the load and you need
; a crtlib other than msvcrt.dll (or to support Win95)
!ifdef OPT_CRTL_URL
; OPTIONAL Installation of c runtime library dll
; TODO: this is really only needed for Win95, so hide on all other OSes
Section "$(TITLE_section_crtlib_dl)" section_crtlib_dl
	SectionIn 2	; select if full installation choosen
	NSISdl::download "${OPT_CRTL_URL}${OPT_CRTL_FILENAME}" "$INSTDIR\${PRODUCT}\bin\${OPT_CRTL_FILENAME}"
        Pop $0 ;Get the return value
	StrCmp $0 "success" Finish
		; Couldn't download the file
		DetailPrint "$(PROMPT_CRTL_DL_FAILED)"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "$(PROMPT_CRTL_DL_FAILED)"
	Finish:
SectionEnd
!endif ; OPT_CRTL_URL


SubSection /e "$(TITLE_ssection_dictionary)" ssection_dictionary

; OPTIONAL Installation of Default Dictionary
Section "$(TITLE_section_dictinary_def_English)" section_dictinary_def_English
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
SectionEnd


!ifdef OPT_DICTIONARIES

!ifndef NODOWNLOADS
; NOTE: these just reference files for download, once download installs(extracts) them

; WARNING: ${ssection_dl_opt_dict}+1 is assumed to be 1st section of downloadable dictionaries
SubSection /e "$(TITLE_ssection_dl_opt_dict)" ssection_dl_opt_dict

; we attempt to let user pick, but this is our fallback/default entry
!ifndef DICTIONARY_BASE_DEFAULT
!define DICTIONARY_BASE_DEFAULT "http://dl.sourceforge.net/abiword"
!endif

; this is the count of dictionaries available for download [count of sections defined]
!define DICTIONARY_COUNT 27	; used to query sections & set description text

; determine if section selected by user
!define SF_SELECTED   1

; RESULT is returned in $R0, $R0 set to 0 is none selected, else will be nonzero
Function isDLDictSelected
  !define RESULT $R0
  
  ; we !!!assume!!! that section# of 1st dictionary section is # of dictionary subsection + 1
  push $0	; the 1st section
  push $1	; the last section

  StrCpy ${RESULT} 0
  StrCpy $0 ${ssection_dl_opt_dict}
  IntOp $0 $0 + 1                   ; order here
  IntOp $1 $0 + ${DICTIONARY_COUNT} ; matters  $1 = ${ssection_dl_opt_dict} + 1 + ${DICTIONARY_COUNT}
  ; $0=section of 1st downloadable dictionary
  loop_start:
    ; check if flag set
    SectionGetFlags $0 ${RESULT}
    IntOp ${RESULT} ${RESULT} & ${SF_SELECTED}
    IntCmp ${RESULT} ${SF_SELECTED} loop_end 0 0
    ; loop through sections
    IntOp $0 $0 + 1  
    IntCmpU $0 $1 loop_end
  Goto loop_start
  loop_end:

  pop $1
  pop $0
FunctionEnd

; creates an .ini file for use by custom download InstallOption dialog
; $R0 is set to temp file used
; $R1 is default/fallback entry
; $R2 is URL list except for default entry
Function createDLIni
  ; create .ini file used for custom dialog
  GetTempFileName $R0

  ; write out our fields  
  WriteINIStr $R0 "Settings" NumFields "2"
  WriteINIStr $R0 "Settings" CancelEnabled "1"
  WriteINIStr $R0 "Settings" CancelShow "1"
  WriteINIStr $R0 "Settings" BackEnabled "0"

  WriteINIStr $R0 "Field 1" Type "Label"
  WriteINIStr $R0 "Field 1" Text "$(MSG_SELECT_DL_MIRROR)"
  WriteINIStr $R0 "Field 1" Left   "0"
  WriteINIStr $R0 "Field 1" Right  "-1"
  WriteINIStr $R0 "Field 1" Top    "15"
  WriteINIStr $R0 "Field 1" Bottom "35"

  WriteINIStr $R0 "Field 2" Type "combobox"
  WriteINIStr $R0 "Field 2" Text "sel"
  WriteINIStr $R0 "Field 2" Left   "0"
  WriteINIStr $R0 "Field 2" Right  "-1"
  WriteINIStr $R0 "Field 2" Top    "39"
  WriteINIStr $R0 "Field 2" Bottom "-1"
  WriteINIStr $R0 "Field 2" minLen "8"
  WriteINIStr $R0 "Field 2" listItems "$R1|$R2"
  WriteINIStr $R0 "Field 2" state "$R1"
FunctionEnd

; determines where to download from
; $R9 is set to base URL
Function getDLMirror
  ; save callees registers
  Push $R0
  Push $R1
  Push $R2

  Call isDLDictSelected ; returns result in $R0
  IntCmp $R0 0 noUpDate 0 0

!ifndef CLASSIC_UI
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"
!else
  ;Set text on the white rectangle
  Push $R0

    GetDlgItem $R0 $HWNDPARENT 1037
    SendMessage $R0 ${WM_SETTEXT} 0 "STR:$(TEXT_IO_TITLE)"
    GetDlgItem $R0 $HWNDPARENT 1038
    SendMessage $R0 ${WM_SETTEXT} 0 "STR:$(TEXT_IO_SUBTITLE)"

  Pop $R0
!endif


  ; "Selecting download mirror ..."
  ; use install options to allow custom entry or select from list

  ; create the inifile used to specify our custom dialog
  StrCpy $R1 "${DICTIONARY_BASE_DEFAULT}"
  StrCpy $R2 "http://unc.dl.sourceforge.net/abiword|http://telia.dl.sourceforge.net/abiword|http://umn.dl.sourceforge.net/abiword|http://twtelecom.dl.sourceforge.net/abiword|http://easynews.dl.sourceforge.net/abiword|http://belnet.dl.sourceforge.net/abiword|http://cesnet.dl.sourceforge.net/abiword|http://switch.dl.sourceforge.net/abiword"
  Call createDLIni ; sets $R0 to inifilename

  ; create the dialog and wait for user's response
!ifndef CLASSIC_UI
  ; for now manually call, as the macro doesn't work as expected
  ;  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "$R0"
  InstallOptions::dialog $R0
!else
  InstallOptions::dialog $R0
!endif

  ; pop return status and use default value on anything other than success
  ; else read back user's choice
  Pop $R9
  StrCmp $R9 "success" 0 useDefaultURL
    ReadINIStr $R9 $R0 "Field 2" State ; $R9 = URL field's state
    Goto Next1
  useDefaultURL:
    DetailPrint "$(MSG_ERROR_SELECTING_DL_MIRROR)"
    StrCpy $R9 "${DICTIONARY_BASE_DEFAULT}"
  Next1:

  ; remove .ini file
  Delete $R0

  ; $R9 is the base URL chosen

  noUpDate:

  ; restore calless registers
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd


!endif  ; NODOWNLOADS

; $R3 is set to the filename used
Function getDictionary
      !define DICTIONARY_BASE $R9

	!define DICT_LANG $R0
	!define DICT_LOCALE $R1
	!define DICT_ARCH $R2

!ifndef NODOWNLOADS
	; set filename, handle files without locale portion (represented by Xx)
	StrCmp ${DICT_LOCALE} "Xx" noLocale 0
		StrCpy $R3 "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
      Goto Skip1
	noLocale:
		StrCpy $R3 "abispell-${DICT_LANG}.${DICT_ARCH}.tar.gz"
	Skip1:
!else ; for files included in setup we just leave the -Xx tacked on
		StrCpy $R3 "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
!endif
	!define DICT_FILENAME $R3


!ifndef NODOWNLOADS
	; download the file
	DetailPrint "NSISdl::download '${DICTIONARY_BASE}/${DICT_FILENAME}' '$TEMP\${DICT_FILENAME}'"
	NSISdl::download "${DICTIONARY_BASE}/${DICT_FILENAME}" "$TEMP\${DICT_FILENAME}"
	Pop $0 ;Get the return value
	StrCmp $0 "success" doDictInst
		; Couldn't download the file
		DetailPrint "Could not download requested dictionary:"
		DetailPrint "  ${DICTIONARY_BASE}/${DICT_FILENAME}"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "Failed to download ${DICTIONARY_BASE}/${DICT_FILENAME}"
	Goto Finish
!endif

	doDictInst:
		; Unzip dictionary into dictionary subdirecotry
		untgz::extract "-j" "-d" "$INSTDIR\dictionary" "$TEMP\${DICT_FILENAME}"
		StrCmp $0 "success" doCleanup
			DetailPrint "  Failed to extract ${DICT_FILENAME}"
			MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "  Failed to extract ${DICT_FILENAME}"
		
		doCleanup:
		; Delete temporary files
		Delete "$TEMP\${DICT_FILENAME}"

	Finish:
		!undef DICT_LANG
		!undef DICT_LOCALE
		!undef DICT_ARCH
FunctionEnd

; used to define a section containing an optional dictionary for downloading & installation
!macro SectionDict DICT_NAME DICT_LANG DICT_LOCALE DICT_ARCH DICT_SIZE
Section '${DICT_LANG}-${DICT_LOCALE}  ${DICT_NAME}' section_dl_opt_dict_${DICT_LANG}_${DICT_LOCALE}
!ifdef NODOWNLOADS
	SectionIn 2	; Full only
	SetOutPath $TEMP
	File "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
!else
	AddSize ${DICT_SIZE}
!endif

	DetailPrint "Installing dictionary for: '${DICT_LANG}-${DICT_LOCALE}  ${DICT_NAME}'"

	StrCpy $R0 ${DICT_LANG}
	StrCpy $R1 ${DICT_LOCALE}
	StrCpy $R2 ${DICT_ARCH}
	Call getDictionary
SectionEnd
!macroend


; These are listed alphabetically based on English LANG-LOCALE
; NOTE: if the dictinaries are updated so to should these sizes (KB)
; Be sure to update DICTIONARY_COUNT above (so description & selection query work correctly)
!insertmacro SectionDict "Catalan"      "ca" "ES" "i386"  4324
!insertmacro SectionDict "Czech"        "cs" "DZ" "i386"  2558
!insertmacro SectionDict "Danish"       "da" "DK" "i386"  1580
!insertmacro SectionDict "Swiss"        "de" "CH" "i386"  8501
!insertmacro SectionDict "Deutsch"      "de" "DE" "i386"  2277
!insertmacro SectionDict "Ellhnika"     "el" "GR" "i386"  2049  ;Greek
!insertmacro SectionDict "English"      "en" "GB" "i386"  2109
!insertmacro SectionDict "Esperanto"    "eo" "Xx" "i386"   942  ;no locale...
!insertmacro SectionDict "Español"      "es" "ES" "i386"  2632
!insertmacro SectionDict "Finnish"      "fi" "FI" "i386" 10053
!insertmacro SectionDict "Français"     "fr" "FR" "i386"  1451
!insertmacro SectionDict "Hungarian"    "hu" "HU" "i386"  8086
!insertmacro SectionDict "Irish gaelic" "ga" "IE" "i386"   587
!insertmacro SectionDict "Galician"     "gl" "ES" "i386"   807
!insertmacro SectionDict "Italian"      "it" "IT" "i386"  1638
!insertmacro SectionDict "Latin"        "la" "IT" "i386"  2254  ;mlatin
!insertmacro SectionDict "Lietuviu"     "lt" "LT" "i386"  1907  ;Lithuanian
!insertmacro SectionDict "Dutch"        "nl" "NL" "i386"  1079  ;nederlands
!insertmacro SectionDict "Norsk"        "nb" "NO" "i386"  2460  ;Norwegian
!insertmacro SectionDict "Nynorsk"      "nn" "NO" "i386"  3001  ;Norwegian(nynorsk)
!insertmacro SectionDict "Polish"       "pl" "PL" "i386"  4143
!insertmacro SectionDict "Portugues"    "pt" "PT" "i386"  1117  ;Portuguese
!insertmacro SectionDict "Brazilian"    "pt" "BR" "i386"  1244  ;Portuguese
!insertmacro SectionDict "Russian"      "ru" "RU" "i386"  8307
!insertmacro SectionDict "Slovensko"    "sl" "SI" "i386"   857  ;Slovenian
!insertmacro SectionDict "Svenska"      "sv" "SE" "i386"   753  ;Swedish
!insertmacro SectionDict "Ukrainian"    "uk" "UA" "i386"  3490


!ifndef NODOWNLOADS
SubSectionEnd ; DL Optional downloads
!endif

!endif ; OPT_DICTIONARIES


SubSectionEnd ; Dictionaries

SubSectionEnd ; helper files


; *********************************************************************


!ifndef CLASSIC_UI

Function .onInit
  ;Language selection
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd


;////blah MUI_HEADERDESCRITPION or SOMETHING

	; section and subsection descriptions (scroll over text)
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${section_abi} $(DESC_section_abi)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_shellupdate} $(DESC_section_shellupdate)

                !ifdef CLASSIC_UI
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts} $(DESC_ssection_shortcuts)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_sm_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_desktop_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_sm_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_desktop_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!endif

		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_core} $(DESC_ssection_core)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_gen_file_assoc} $(DESC_ssection_gen_file_assoc)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_fa_doc} $(DESC_section_fa_doc)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_fa_rtf} $(DESC_section_fa_rtf)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_helper_files} $(DESC_ssection_helper_files)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_help} $(DESC_section_help)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_templates} $(DESC_section_templates)
;		!insertmacro MUI_DESCRIPTION_TEXT ${section_samples} $(DESC_section_samples)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_clipart} $(DESC_section_clipart)

!ifdef OPT_CRTL_LOCAL
		!insertmacro MUI_DESCRIPTION_TEXT ${section_crtlib_local} $(DESC_section_crtlib)
!endif

!ifndef NODOWNLOADS
!ifdef OPT_CRTL_URL
		!insertmacro MUI_DESCRIPTION_TEXT ${section_crtlib_dl} $(DESC_section_crtlib)
!endif

		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_dictionary} $(DESC_ssection_dictionary)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_dictinary_def_English} $(DESC_ssection_dictionary)
!ifdef OPT_DICTIONARIES
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_dl_opt_dict} $(DESC_ssection_dictionary)
!endif ; OPT_DICTIONARIES
!endif ; NODOWNLOADS

!ifdef OPT_DICTIONARIES
		; we !!!assume!!! that section# of 1st dictionary section is # of dictionary subsection + 1
		push $R1	; the 1st section
		push $R2	; the last section
		StrCpy $R1 ${ssection_dl_opt_dict}
		IntOp $R1 $R1 + 1                   ; order here
		IntOp $R2 $R1 + ${DICTIONARY_COUNT}	; matters  $R2 = ${ssection_dl_opt_dict} + 1 + ${DICTIONARY_COUNT}
		; $R1=section of 1st downloadable dictionary
		/*loop_start:*/
			!insertmacro MUI_DESCRIPTION_TEXT $R1 $(DESC_ssection_dictionary)
			IntOp $R1 $R1 + 1  
			IntCmpU $R1 $R2 +2/*loop_end*/
		Goto -3/*loop_start*/
		/*loop_end:*/
		pop $R2
		pop $R1
!endif


	!insertmacro MUI_FUNCTION_DESCRIPTION_END


!ifndef NODOWNLOADS
!endif

!endif



; uninstall stuff
!ifdef CLASSIC_UI
	UninstallText "This will uninstall ${PRODUCT} v${VERSION_MAJOR}. Hit next to continue."
!endif


; check if a file extension is associated with us and if so delete it
!macro un.RemoveFileAssociation extension appType
	push $0
	ReadRegStr $0 HKCR "${extension}" "(Default)"
	StrCmp $0 "${appType}" 0 Skip_Del_File_Assoc.${extension}
		; actually remove file assoications
		DeleteRegKey HKCR "${extension}"
	Skip_Del_File_Assoc.${extension}:
	pop $0
!macroend


; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL $(UNINSTALL_WARNING) IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}${VERSION_MAJOR}"
	DeleteRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}

	; remove file assoications
	; our native ones
	!insertMacro un.RemoveFileAssociation ".abw"  "${APPSET}.${PRODUCT}"
	!insertMacro un.RemoveFileAssociation ".awt"  "${APPSET}.${PRODUCT}"
	!insertMacro un.RemoveFileAssociation ".zabw" "${APPSET}.${PRODUCT}"
	; other common ones
	!insertMacro un.RemoveFileAssociation ".doc"  "${APPSET}.${PRODUCT}"
	!insertMacro un.RemoveFileAssociation ".rtf"  "${APPSET}.${PRODUCT}"

	; actual apptype entry
	DeleteRegKey HKCR "${APPSET}.${PRODUCT}"

	; remove start menu shortcuts.
	;Delete "$SMPROGRAMS\${SM_PRODUCT_GROUP}\*.*"
        !ifndef CLASSIC_UI
                !insertmacro MUI_STARTMENU_GETFOLDER $0
                RMDir /r "$0"
        !else
             StrCpy $0 "$(SM_PRODUCT_GROUP)"
	     RMDir /r "$SMPROGRAMS\$0"
        !endif

	; remove desktop shortcut.
	StrCpy $0 "$(SHORTCUT_NAME)"
	Delete "$DESKTOP\$0.lnk"

	; remove directories used (and any files in them)
	RMDir /r "$INSTDIR"

SectionEnd

!ifndef CLASSIC_UI
;Uninstaller Functions
Function un.onInit
  ;Restore Language selection
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd
!endif

; End of File

