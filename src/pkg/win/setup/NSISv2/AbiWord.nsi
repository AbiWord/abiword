;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Primary NSIS v2 installer script
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
!warning "OPT_CRTL_URL and OPT_CRTL_LOCAL should not both be defined"
!warning "disabling OPT_CRTL_URL and using OPT_CRTL_LOCAL"
!undef OPT_CRTL_URL
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
!ifdef VERSION
!undef VERSION
!endif
!define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"

!define INSTALLERNAME "setup_abiword.${VERSION_MAJOR}-${VERSION_MINOR}-${VERSION_MICRO}.exe"

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


; Useful inclusions
!include Sections.nsh


; Support 'Modern' UI and multiple languages
!ifndef CLASSIC_UI

  ; include the Modern UI support
  !include "Mui.nsh"

  ; specify where to get resources from for UI elements (default)
  !define MUI_UI "${NSISDIR}\Contrib\UIs\modern.exe"

  ; specify the pages and order to show to user

  ; introduce ourselves
  !insertmacro MUI_PAGE_WELCOME
  ; including the license of AbiWord  (license could be localized, but we don't)
  !insertmacro MUI_PAGE_LICENSE '${MUI_LICENSEDATA}'
  ; allow user to select what parts to install
  !define MUI_COMPONENTSPAGE_SMALLDESC  ; but put the description below choices
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
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES


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


!ifdef RESERVE_PLUGINS
  ; Reserve Files to possibly aid in starting faster
  !insertmacro MUI_RESERVEFILE_LANGDLL
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
  !insertmacro MUI_RESERVEFILE_SPECIALINI
  !insertmacro MUI_RESERVEFILE_SPECIALBITMAP

  ; reserve room at start of compressed archive for plugins (not already reserved by Modern UI)
  ; ReserveFile [/nonfatal] [/r] file [file...]
  !ifndef NODOWNLOADS
    ReserveFile "${NSISDIR}\Plugins\dialer.dll"
    ReserveFile "${NSISDIR}\Plugins\NSISdl.dll"
  !endif
  !ifdef OPT_DICTIONARIES
    ReserveFile "${NSISDIR}\Plugins\untgz.dll"
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


; Localized Installer Messages (Language Strings)
!macro LSTR sectID sectDesc
  LangString "${sectID}" "${LANG_X}" "${sectDesc}"
!macroend
!define LSTR "!insertmacro LSTR"
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


; add a version resource to installer corresponding to version of app we're installing
!include "abi_version.nsh"


; Install types 
InstType "Typical (default)"              ;Section 1
InstType "Full (with File Associations)"  ;Section 2
InstType "Minimal"                        ;Section 3
InstType "Tiny - ${PROGRAMEXE} only"	;Section 4
!ifndef NODOWNLOADS
InstType "Full plus Downloads"		;Section 5
!define DLSECT 5
!else
!define DLSECT
!endif
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
!define CreateFileAssociation "!insertmacro CreateFileAssociation"


; Create a language localized Start Menu group
!macro lngCreateSMGroup group
	push $0
	StrCpy $0 "${group}"
	CreateDirectory "$SMPROGRAMS\$0"
	pop $0
!macroend
!define lngCreateSMGroup "!insertmacro lngcreateSMGroup"

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
!define lngCreateShortCut "!insertmacro lngCreateShortCut"


; *********************************************************************


SubSection /e "$(TITLE_ssection_core)" ssection_core

; The stuff that must be installed
Section "$(TITLE_section_abi)" section_abi
	SectionIn 1 2 3 4 ${DLSECT} RO	; included in Typical, Full, Minimal, Required
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
SectionEnd


Section "" ; invisible section that must also be installed, sets installer information
	SectionIn 1 2 3 4 ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	; TODO: determine if we should be using HKCU instead of HKLM for some of these
	; or see if user has permission and ask them...

	; Write the language used during installation into the registry for uninstall to use
	WriteRegStr HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Installer Language" "$LANGUAGE"

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

	; New Uninstaller
	WriteUninstaller "Uninstall${PRODUCT}${VERSION_MAJOR}.exe"

SectionEnd


Section "$(TITLE_section_abi_req)" section_abi_req
	SectionIn 1 2 3 ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	; We need BMP plugin for cut-n-paste of images on Windows
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\libAbi_IEG_BMP.dll"

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
  
	; Write the start menu entry (if user selected to do so)
	!ifndef CLASSIC_UI
	  !insertmacro MUI_STARTMENU_WRITE_BEGIN
		;SetShellVarContext current|all???
		${lngCreateSMGroup}  "$MUI_STARTMENU_FOLDER"
		${lngCreateShortCut} "$SMPROGRAMS" "$MUI_STARTMENU_FOLDER" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
		${lngCreateShortCut} "$SMPROGRAMS" "$MUI_STARTMENU_FOLDER" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	  !insertmacro MUI_STARTMENU_WRITE_END
	!endif

SectionEnd

; OPTIONAL Registry Settings
Section "$(TITLE_section_shellupdate)" section_shellupdate
	SectionIn 1 2 3 ${DLSECT}
	; Write the AbiSuite.AbiWord Keys
	WriteRegStr HKCR "${APPSET}.${PRODUCT}" "" "${PRODUCT} Document"
	WriteRegStr HKCR "${APPSET}.${PRODUCT}\DefaultIcon" "" "$INSTDIR\${MAINPROGRAM},2"
	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\command" "" '"$INSTDIR\${MAINPROGRAM}" "%1"'
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\command" "" "$INSTDIR\${MAINPROGRAM}"
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\ddeexec" "" '[Open("%1")]'
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\ddeexec\application" "" "${PRODUCT}"
;	WriteRegStr HKCR "${APPSET}.${PRODUCT}\shell\open\ddeexec\topic" "" "System"

	; Write File Associations
	${CreateFileAssociation} ".abw"  "${APPSET}.${PRODUCT}" "application/abiword"
	${CreateFileAssociation} ".awt"  "${APPSET}.${PRODUCT}" "application/abiword-template"
	${CreateFileAssociation} ".zabw" "${APPSET}.${PRODUCT}" "application/abiword-compressed"

SectionEnd

SubSectionEnd ; core


; *********************************************************************
!ifdef CLASSIC_UI


SubSection /e "$(TITLE_ssection_shortcuts)" ssection_shortcuts

SubSection /e "$(TITLE_ssection_shortcuts_cu)" ssection_shortcuts_cu

; OPTIONAL Start Menu Shortcut for the current user profile
Section "$(TITLE_section_sm_shortcuts_cu)" section_sm_shortcuts_cu
	SectionIn 1 2 3 ${DLSECT}
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	${lngCreateSMGroup}  "$(SM_PRODUCT_GROUP)"
	${lngCreateShortCut} "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	${lngCreateShortCut} "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
SectionEnd

; OPTIONAL Desktop Shortcut for the current user profile
Section "$(TITLE_section_desktop_shortcuts_cu)" section_desktop_shortcuts_cu
	SectionIn 1 2 3 ${DLSECT}
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	${lngCreateShortCut} "$DESKTOP" "" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
SectionEnd


SubSectionEnd ; Shortcuts (Current User)
SubSection /e "$(TITLE_ssection_shortcuts_au)" ssection_shortcuts_au


; OPTIONAL Start Menu Shortcut for the special All User profile (not used in win9x) 
Section "$(TITLE_section_sm_shortcuts_au)" section_sm_shortcuts_au
	SectionIn 2	${DLSECT}	; off by default, included in 2 Full Install
	SetShellVarContext all  	; set to all, reset at end of section
	${lngCreateSMGroup}  "$(SM_PRODUCT_GROUP)"
	${lngCreateShortCut} "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	${lngCreateShortCut} "$SMPROGRAMS" "$(SM_PRODUCT_GROUP)" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	SetShellVarContext current  	; This is pro'ly overkill
SectionEnd


; OPTIONAL Desktop Shortcut for All Users
Section "$(TITLE_section_desktop_shortcuts_au)" section_desktop_shortcuts_au
	SectionIn 2	${DLSECT}	; not in default, included in 2 Full Install
	SetShellVarContext all  	;  All users 
	${lngCreateShortCut} "$DESKTOP" "" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	SetShellVarContext current  	; reset to current user
SectionEnd

SubSectionEnd ; Shortcuts (All Users)"

SubSectionEnd ; Shortcuts


!endif ;CLASSIC_UI
; *********************************************************************


SubSection /e "$(TITLE_ssection_gen_file_assoc)" ssection_gen_file_assoc

; OPTIONAL 
Section "$(TITLE_section_fa_doc)" section_fa_doc
	SectionIn 2 ${DLSECT}
	${CreateFileAssociation} ".doc"  "${APPSET}.${PRODUCT}" "application/abiword"
SectionEnd

; OPTIONAL 
Section "$(TITLE_section_fa_rtf)" section_fa_rtf
	SectionIn 2 ${DLSECT}
	${CreateFileAssociation} ".rtf"  "${APPSET}.${PRODUCT}" "application/abiword"
SectionEnd

SubSectionEnd ; general file associations


; *********************************************************************


SubSection /e "$(TITLE_ssection_helper_files)" ssection_helper_files

; MORE OPTIONS
; language packs, clipart, help docs, templates etc.   

; OPTIONAL Installation of Help Files
Section "$(TITLE_section_help)" section_help
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR\AbiWord
	; help documents may not be created if peer abiword-docs not found
	File /nonfatal /r "..\abisuite\abiword\help"
SectionEnd

; OPTIONAL Installation of Templates
Section "$(TITLE_section_templates)" section_templates
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\templates"
SectionEnd

; OPTIONAL Installation of Samples - REMOVED
;Section "$(TITLE_section_samples)" section_samples
;	SectionIn 1 2 ${DLSECT}
;	SetOutPath $INSTDIR\AbiWord
;	File /r "..\AbiSuite\AbiWord\sample"
;SectionEnd

; OPTIONAL Installation of Clipart
Section "$(TITLE_section_clipart)" section_clipart
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\clipart"
SectionEnd


; include function to determine if user is connected to Internet/has networking enabled computer
!include "abi_util_connected.nsh"


!ifdef OPT_CRTL_LOCAL
; OPTIONAL Installation of c runtime library dll
; Hidden if for Win95 only (e.g msvcrt.dll)
Section "$(TITLE_section_crtlib_local)" section_crtlib_local
	SectionIn 2 ${DLSECT}	; select if full installation choosen
	SetOutPath $INSTDIR\${PRODUCT}\bin

	File "${OPT_CRTL_LOCAL}${OPT_CRTL_FILENAME}"

	!ifdef OPT_CPPL_FILENAME
	File "${OPT_CRTL_LOCAL}${OPT_CPPL_FILENAME}"
	!endif
SectionEnd
!endif ; OPT_CRTL_LOCAL

; we only enable this option if a url to connect to was
; specified during installation building; this should
; only be enabled for release builds if your server (where
; the url points) can handle the load and you need
; a crtlib other than msvcrt.dll (or to support Win95)
!ifdef OPT_CRTL_URL
; OPTIONAL Installation of c runtime library dll
; Hidden if for Win95 only (e.g msvcrt.dll)
Section "$(TITLE_section_crtlib_dl)" section_crtlib_dl
	SectionIn 2	${DLSECT}	; select if full installation choosen
	Call ConnectInternet	; try to establish connection if not connected
	StrCmp $0 "online" 0 dlDone
	NSISdl::download "${OPT_CRTL_URL}${OPT_CRTL_FILENAME}" "$INSTDIR\${PRODUCT}\bin\${OPT_CRTL_FILENAME}"
	Pop $0 ;Get the return value
	StrCmp $0 "success" dlDone
		; Couldn't download the file
		DetailPrint "$(PROMPT_CRTL_DL_FAILED)"
		DetailPrint "NSISdl::download return $0"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "$(PROMPT_CRTL_DL_FAILED)"

	!ifdef OPT_CPPL_FILENAME
	NSISdl::download "${OPT_CRTL_URL}${OPT_CRTL_FILENAME}" "$INSTDIR\${PRODUCT}\bin\${OPT_CPPL_FILENAME}"
	Pop $0 ;Get the return value
	StrCmp $0 "success" dlDone
		; Couldn't download the file
		DetailPrint "*$(PROMPT_CRTL_DL_FAILED)"
		DetailPrint "NSISdl::download return $0"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "$(PROMPT_CRTL_DL_FAILED)"
	!endif

	dlDone:
SectionEnd
!endif ; OPT_CRTL_URL


SubSection /e "$(TITLE_ssection_dictionary)" ssection_dictionary

; OPTIONAL Installation of Default Dictionary
Section "$(TITLE_section_dictinary_def_English)" section_dictinary_def_English
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
SectionEnd


!ifdef OPT_DICTIONARIES
!include "abi_dict_misc.nsh"
!endif ; OPT_DICTIONARIES


SubSectionEnd ; Dictionaries

SubSectionEnd ; helper files


; *********************************************************************


!include "abi_util_winver.nsh"


!macro SectionDisable SECTIONID
  !insertmacro UnselectSection ${SECTIONID}	; mark section as unselected
  SectionSetText ${SECTIONID} ""	; and make invisible so user doesn't see it
!macroend
!define SectionDisable "!insertmacro SectionDisable"

Function .onInit

!ifndef CLASSIC_UI
  ;Language selection
  !insertmacro MUI_LANGDLL_DISPLAY
!endif


!ifndef NODOWNLOADS
  ; Disable all downloads if not connected
  Call ConnectInternet	; try to establish connection if not connected
  StrCmp $0 "online" connected
  !ifdef OPT_DICTIONARIES
	${SectionDisable} ${ssection_dl_opt_dict}
	!insertmacro cycle_over_dictionary_sections "${SectionDisable} $R1"
  !endif
  !ifdef OPT_CRTL_URL
  	${SectionDisable} ${section_crtlib_dl}
  !endif
  connected:
!endif ;NODOWNLOADS

; Disable Windows 95 specific sections
!ifdef OPT_CRTL_WIN95ONLY
Call GetWindowsVersion
Pop $R0
StrCmp $R0 '95' skipDisableW95dl 0	; disable for all but Windows 95
  !ifdef OPT_CRTL_URL
     ${SectionDisable} ${section_crtlib_dl}
  !endif
  !ifdef OPT_CRTL_LOCAL
     ${SectionDisable} ${section_crtlib_local}
  !endif
skipDisableW95dl:
!endif ;OPT_CRTL_WIN95ONLY

FunctionEnd


!ifndef CLASSIC_UI
	; section and subsection descriptions (scroll over text)
	!define MUI_DESCRIPTION_TEXT "!insertmacro MUI_DESCRIPTION_TEXT"
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		${MUI_DESCRIPTION_TEXT} ${section_abi} $(DESC_section_abi)
		${MUI_DESCRIPTION_TEXT} ${section_abi_req} $(DESC_section_abi_req)
		${MUI_DESCRIPTION_TEXT} ${section_shellupdate} $(DESC_section_shellupdate)

		!ifdef CLASSIC_UI
		${MUI_DESCRIPTION_TEXT} ${ssection_shortcuts} $(DESC_ssection_shortcuts)
		${MUI_DESCRIPTION_TEXT} ${ssection_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		${MUI_DESCRIPTION_TEXT} ${section_sm_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		${MUI_DESCRIPTION_TEXT} ${section_desktop_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		${MUI_DESCRIPTION_TEXT} ${ssection_shortcuts_au} $(DESC_ssection_shortcuts_au)
		${MUI_DESCRIPTION_TEXT} ${section_sm_shortcuts_au} $(DESC_ssection_shortcuts_au)
		${MUI_DESCRIPTION_TEXT} ${section_desktop_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!endif

		${MUI_DESCRIPTION_TEXT} ${ssection_core} $(DESC_ssection_core)
		${MUI_DESCRIPTION_TEXT} ${ssection_gen_file_assoc} $(DESC_ssection_gen_file_assoc)
		${MUI_DESCRIPTION_TEXT} ${section_fa_doc} $(DESC_section_fa_doc)
		${MUI_DESCRIPTION_TEXT} ${section_fa_rtf} $(DESC_section_fa_rtf)
		${MUI_DESCRIPTION_TEXT} ${ssection_helper_files} $(DESC_ssection_helper_files)
		${MUI_DESCRIPTION_TEXT} ${section_help} $(DESC_section_help)
		${MUI_DESCRIPTION_TEXT} ${section_templates} $(DESC_section_templates)
;		${MUI_DESCRIPTION_TEXT} ${section_samples} $(DESC_section_samples)
		${MUI_DESCRIPTION_TEXT} ${section_clipart} $(DESC_section_clipart)

!ifdef OPT_CRTL_LOCAL
		${MUI_DESCRIPTION_TEXT} ${section_crtlib_local} $(DESC_section_crtlib)
!endif

!ifndef NODOWNLOADS
!ifdef OPT_CRTL_URL
		${MUI_DESCRIPTION_TEXT} ${section_crtlib_dl} $(DESC_section_crtlib)
!endif

		${MUI_DESCRIPTION_TEXT} ${ssection_dictionary} $(DESC_ssection_dictionary)
		${MUI_DESCRIPTION_TEXT} ${section_dictinary_def_English} $(DESC_ssection_dictionary)
!ifdef OPT_DICTIONARIES
		${MUI_DESCRIPTION_TEXT} ${ssection_dl_opt_dict} $(DESC_ssection_dictionary)
!endif ; OPT_DICTIONARIES
!endif ; NODOWNLOADS

!ifdef OPT_DICTIONARIES
		!insertmacro cycle_over_dictionary_sections "${MUI_DESCRIPTION_TEXT} $R1 $(DESC_ssection_dictionary)"
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
	ReadRegStr $0 HKCR "${extension}" ""
	StrCmp $0 "${appType}" 0 Skip_Del_File_Assoc.${extension}
		; actually remove file assoications
		DeleteRegKey HKCR "${extension}"
	Skip_Del_File_Assoc.${extension}:
	pop $0
!macroend
!define un.RemoveFileAssociation "!insertmacro un.RemoveFileAssociation"


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
	${un.RemoveFileAssociation} ".abw"  "${APPSET}.${PRODUCT}"
	${un.RemoveFileAssociation} ".awt"  "${APPSET}.${PRODUCT}"
	${un.RemoveFileAssociation} ".zabw" "${APPSET}.${PRODUCT}"
	; other common ones
	${un.RemoveFileAssociation} ".doc"  "${APPSET}.${PRODUCT}"
	${un.RemoveFileAssociation} ".rtf"  "${APPSET}.${PRODUCT}"

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
