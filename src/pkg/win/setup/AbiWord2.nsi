;Title          AbiWord for Windows, NSIS v2 series installer script
;Author         Kenneth J. Davis <jeremyd@computer.org> (2002,2003)
;Copyright      Alan Horkan <horkana@tcd.ie> (2002)
;               Michael D. Pritchett <mpritchett@attglobal.net> (2002)
;               [add your name here]
;Version        see AbiSource CVS

; Define this for the older (non 'Modern') user interface, English only
!define CLASSIC_UI

; Define this to not include the optional downloadable components (dictionaries, crtlib, ...)
!define NODOWNLOADS

; To include the C Runtime Library for the compiler used
; Define either the URL for downloadable one or LOCAL to include within setup (include final slash)
; then define the actual crt library filename (both file and (URL or LOCAL) must be defined)
; 1a) the base URL where the crt library can be downloaded from
;!define OPT_CRTL_URL "http://abiword.pchasm.org/microsoft/"
; 1b) alternately where the file may be found on the local filesystem for inclusion
;!define OPT_CRTL_LOCAL "\Program Files\Microsoft Visual Studio\REDIST\"
; 2) the actual filename of the crt library
;!define OPT_CRTL_FILENAME "msvcrt.dll"  ; MSVC 5 and 6 (not for Windows Me, NT 2000, or newer)
;!define OPT_CRTL_FILENAME "msvcr70.dll"  ; MSVC 7

; Define this to include the dictionaries in the installer
; if NODOWNLOADS is defined then the files must be locally available
;!define OPT_DICTIONARIES

; NOT YET AVAILABLE
; Define this to include the standard set of plugins
; if NODOWNLOADS is defined then the files must be locally available
;!define OPT_PLUGINS

; If you have upx available in your PATH, enable this for a smaller setup file
;!define HAVE_UPX

; Specify this one only if you are the TradeMark holder!!!
;!define TRADEMARKED_BUILD



;Declarations
!define PRODUCT "AbiWord"
!ifndef VERSION_MAJOR
!define VERSION_MAJOR "2"
!endif
!ifndef VERSION_MINOR
!define VERSION_MINOR "0"
!endif
!ifndef VERSION_MICRO
!define VERSION_MICRO "0"
!endif
!ifndef VERSION
!define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"
!endif
;!define EXENAME "setup_abiword.${VERSION_MAJOR}-${VERSION_MINOR}-${VERSION_MICRO}.exe"
!define EXENAME "setup_abiword.exe"
!define APPSET  "AbiSuite"
!define MAINPROGRAM "Abiword\bin\Abiword.exe"


; Some checks of user defines
!ifdef NODOWNLOADS & OPT_CRTL_URL
!warning "OPT_CRTL_URL and NODOWNLOADS both defined, ignoring OPT_CRTL_URL"
!undef OPT_CRTL_URL
!endif
!ifdef OPT_CRTL_URL & OPT_CRTL_LOCAL
!error "OPT_CRTL_URL and OPT_CRTL_LOCAL cannot both be defined"
!endif


; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; set the compression algorithm used, zlib | bzip2
SetCompressor bzip2

; where to look for NSIS plugins during setup creation
; default includes ./plugins, but we also want to check current directory
PluginDir .

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

; Specify name of resulting installer
OutFile "${EXENAME}"


; The default installation directory
InstallDir $PROGRAMFILES\${APPSET}${VERSION_MAJOR}

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Install_Dir"

; Support 'Modern' UI and multiple languages
!ifndef CLASSIC_UI

  ; a couple of defines used by Modern, their names may change in future versions
  !define MUI_PRODUCT ${PRODUCT}
  !define MUI_VERSION ${VERSION}

  !include "${NSISDIR}\Contrib\Modern UI\System.nsh"

  !define MUI_WELCOMEPAGE
  !define MUI_LICENSEPAGE
    ;LicenseData ${MUI_LICENSEDATA}
  !define MUI_COMPONENTSPAGE
  !define MUI_DIRECTORYPAGE
  !define MUI_FINISHPAGE
    !define MUI_FINISHPAGE_RUN "$INSTDIR\${MAINPROGRAM}"
    !define MUI_FINISHPAGE_RUN_PARAMETERS  $\"$INSTDIR\readme.txt$\"
    ;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
    !define MUI_FINISHPAGE_NOAUTOCLOSE

  !define MUI_ABORTWARNING

  !define MUI_UNINSTALLER
    !define MUI_UNCONFIRMPAGE

  ; allow insertion of our custom pages
  !define MUI_CUSTOMPAGECOMMANDS


  ; Languages
  ; some are presently commented out as they lack some translations (hence fail to build)
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"    
  !insertmacro MUI_LANGUAGE "Japanese"    
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
;  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
;  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
;  !insertmacro MUI_LANGUAGE "Bulgarian"

  ; license text (for now point to same file, but can be different)
  LicenseData /LANG=${LANG_ENGLISH} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_FRENCH} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_GERMAN} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_SPANISH} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_SIMPCHINESE} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_TRADCHINESE} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_JAPANESE} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_ITALIAN} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_DUTCH} ${MUI_LICENSEDATA}
;  LicenseData /LANG=${LANG_POLISH} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_GREEK} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_RUSSIAN} ${MUI_LICENSEDATA}
;  LicenseData /LANG=${LANG_PORTUGUESEBR} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_UKRAINIAN} ${MUI_LICENSEDATA}
  LicenseData /LANG=${LANG_CZECH} ${MUI_LICENSEDATA}
;  LicenseData /LANG=${LANG_BULGARIAN} ${MUI_LICENSEDATA}


  !define MUI_UI "${NSISDIR}\Contrib\UIs\modern2.exe"

  !insertmacro MUI_SYSTEM 

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
    ReserveFile "${NSISDIR}\Plugins\untgz.dll"
  !endif
!endif ; RESERVE_PLUGINS


!else ; CLASSIC_UI

  ; The name of the installer
  Name "${PRODUCT} ${VERSION}"
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
; descriptions for Sections and SubSections

; English
; Section titles, what user sees to select components for installation
LangString TITLE_ssection_core                 ${LANG_English} "Primary components"
LangString TITLE_section_abi                   ${LANG_English} "Abiword.exe (required)"
Langstring TITLE_section_shellupdate           ${LANG_English} "Update Registry Settings"
LangString TITLE_ssection_shortcuts            ${LANG_English} "Shortcuts"
LangString TITLE_ssection_shortcuts_cu         ${LANG_English} "Shortcuts (Current User)"
LangString TITLE_section_sm_shortcuts_cu       ${LANG_English} "Start Menu Shortcuts (Current User)"
LangString TITLE_section_desktop_shortcuts_cu  ${LANG_English} "Desktop Shortcut (Current User)"
LangString TITLE_ssection_shortcuts_au         ${LANG_English} "Shortcuts (All Users)"
LangString TITLE_section_sm_shortcuts_au       ${LANG_English} "Start Menu Shortcuts (All Users)"
LangString TITLE_section_desktop_shortcuts_au  ${LANG_English} "Desktop Shortcut (All Users)"
LangString TITLE_ssection_gen_file_assoc       ${LANG_English} "General file associations"
LangString TITLE_section_fa_doc                ${LANG_English} "Associate .doc with AbiWord"
LangString TITLE_section_fa_rtf                ${LANG_English} "Associate .rtf with AbiWord"
LangString TITLE_ssection_helper_files         ${LANG_English} "Helper files"
LangString TITLE_section_help                  ${LANG_English} "Help Files"
LangString TITLE_section_templates             ${LANG_English} "Templates"
;LangString TITLE_section_samples               ${LANG_English} "Samples"
LangString TITLE_section_clipart               ${LANG_English} "Clipart"
!ifdef OPT_CRTL_LOCAL
LangString TITLE_section_crtlib_local          ${LANG_English} "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
LangString TITLE_section_crtlib_dl             ${LANG_English} "Download CRTlib ${OPT_CRTL_FILENAME}"
!endif
LangString TITLE_ssection_dictionary           ${LANG_English} "Dictionaries"
LangString TITLE_section_dictinary_def_English ${LANG_English} "en-US  US English (default)"
!ifdef OPT_DICTIONARIES
LangString TITLE_ssection_dl_opt_dict          ${LANG_English} "Download optional dictionaries"
!endif
!ifdef OPT_PLUGINS
LangString TITLE_ssection_plugins              ${LANG_English} "Plugins"
!endif

; Section descriptions displayed to user when mouse hovers over a section
LangString DESC_ssection_core            ${LANG_English} "Primary (core) set of components for AbiWord to run well."
LangString DESC_section_abi              ${LANG_English} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate      ${LANG_English} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts       ${LANG_English} "Installs shortcuts in various places to allow starting AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu    ${LANG_English} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au    ${LANG_English} "Installs shortcuts for all users (or current user on systems without multiple users)."
LangString DESC_ssection_gen_file_assoc  ${LANG_English} "Associates various documents with AbiWord, so AbiWord will be used to open them."
LangString DESC_section_fa_doc           ${LANG_English} "Specifies that AbiWord should be used to open Microsoft Word (R) native format documents."
LangString DESC_section_fa_rtf           ${LANG_English} "Specifies that AbiWord should be used to open Rich Text Files, a 'standard' format for WordProcessors."
LangString DESC_ssection_helper_files    ${LANG_English} "Installs various optional files to aid in using AbiWord."
LangString DESC_section_help             ${LANG_English} "Installs the help documents, no help is available if this is omitted."
LangString DESC_section_templates        ${LANG_English} "Installs templates that can be used to aid in creation of new documents with predefined formatting."
;LangString DESC_section_samples          ${LANG_English} "Samples have been removed."
LangString DESC_section_clipart          ${LANG_English} "Installs pictures (clipart) that can be inserted into documents."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
LangString DESC_section_crtlib           ${LANG_English} "Installs the C Runtime Library used by AbiWord, useful if your system lacks this already."
!endif
LangString DESC_ssection_dictionary      ${LANG_English} "Installs dictionaries for various languages that are used to spell check your document."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
LangString DESC_ssection_plugins         ${LANG_English} "Installs various optional plugins."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
LangString PROMPT_OVERWRITE ${LANG_English} "Overwrite Existing ${PRODUCT}?"
LangString MSG_ABORT        ${LANG_English} "Quitting the install process"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
LangString PROMPT_CRTL_DL_FAILED   ${LANG_English} "Failed to download requested c runtime library (DLL): ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
LangString DLMIRROR_IO_WINDOWTITLE       ${LANG_English} ": Base URL"
LangString TEXT_IO_TITLE                 ${LANG_English} "Optional Downloadable Components Base URL"
LangString TEXT_IO_SUBTITLE              ${LANG_English} "Dictionaries"
LangString MSG_SELECT_DL_MIRROR          ${LANG_English} "Select download mirror..."
LangString MSG_ERROR_SELECTING_DL_MIRROR ${LANG_English} "Error obtaining user choice, using default site!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
LangString SM_PRODUCT_GROUP        ${LANG_English} "${PRODUCT} Word Processor"
LangString SHORTCUT_NAME           ${LANG_English} "${PRODUCT} v${VERSION_MAJOR}"
LangString SHORTCUT_NAME_UNINSTALL ${LANG_English} "Uninstall ${PRODUCT} v${VERSION_MAJOR}"
; repeate these so we can use them in the uninstaller
LangString un.SM_PRODUCT_GROUP     ${LANG_English} "${PRODUCT} Word Processor"
LangString un.SHORTCUT_NAME        ${LANG_English} "${PRODUCT} v${VERSION_MAJOR}"

; Uninstall Strings
LangString un.UNINSTALL_WARNING ${LANG_English} "This will delete $INSTDIR and all subdirectories and files?"


; French ${LANG_French}

; German ${LANG_German}

; Spanish ${LANG_Spanish}

; SimpChinese ${LANG_SimpChinese}

; TradChinese ${LANG_TradChinese}

; Japanese ${LANG_Japanese}

; Italian ${LANG_Italian}

; Dutch ${LANG_Dutch}

; Polish ${LANG_Polish}

; Greek ${LANG_Greek}

; Russian ${LANG_Russian}

; PortugueseBR ${LANG_PortugueseBR}

; Ukrainian ${LANG_Ukrainian}

; Czech ${LANG_Czech}

; Bulgarian ${LANG_Bulgarian}

; End Language descriptions


; Install types 
InstType "Typical (default)"              ;Section 1
InstType "Full (with File Associations)"  ;Section 2
InstType "Minimal"                        ;Section 3
; any other combination is "Custom"

!ifndef CLASSIC_UI
; Page order
  !insertmacro MUI_PAGECOMMAND_WELCOME
  !insertmacro MUI_PAGECOMMAND_LICENSE
  !insertmacro MUI_PAGECOMMAND_COMPONENTS
  !insertmacro MUI_PAGECOMMAND_DIRECTORY
  !ifndef NODOWNLOADS
    !ifdef OPT_DICTIONARIES
      Page custom getDLMirror "$(DLMIRROR_IO_WINDOWTITLE)" ; Custom page to get DL mirror
    !endif
  !endif
  !insertmacro MUI_PAGECOMMAND_INSTFILES
  !insertmacro MUI_PAGECOMMAND_FINISH
!else ; CLASSIC_UI
  ;Page declaration [optional, just specifies default behavior]
  Page license
  Page components
  Page directory
  !ifndef NODOWNLOADS
    !ifdef OPT_DICTIONARIES
      Page custom getDLMirror "$(DLMIRROR_IO_WINDOWTITLE)" ; Custom page to get DL mirror
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
	File "AbiWord.exe"

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

	; Write the language used during installation into the registry for uninstal to use
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

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\AppPaths\AbiWord" "" '"$INSTDIR\AbiWord\bin\AbiWord.exe"'

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
	file /r "..\abisuite\abiword\help"
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
Section "$(TITLE_section_crtlib_dl)" section_crtlib_dl
	SectionIn 2	; select if full installation choosen
	NSISdl::download "${OPT_CRTL_URL}${OPT_CRTL_FILENAME}" "$INSTDIR\${PRODUCT}\bin\${OPT_CRTL_FILENAME}"
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

; RESULT should be one of the R set $R0-$R9
!macro isDLDictSelected RESULT
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
!macroend

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

  !insertmacro isDLDictSelected $R0
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

  ;Font
  Push Tahoma
  Push 8

  ;Languages
  !insertmacro MUI_LANGDLL_PUSH "English"
  !insertmacro MUI_LANGDLL_PUSH "French"
  !insertmacro MUI_LANGDLL_PUSH "German"
  !insertmacro MUI_LANGDLL_PUSH "Spanish"
  !insertmacro MUI_LANGDLL_PUSH "SimpChinese"
  !insertmacro MUI_LANGDLL_PUSH "TradChinese"    
  !insertmacro MUI_LANGDLL_PUSH "Japanese"    
  !insertmacro MUI_LANGDLL_PUSH "Italian"
  !insertmacro MUI_LANGDLL_PUSH "Dutch"
  ;!insertmacro MUI_LANGDLL_PUSH "Polish"
  !insertmacro MUI_LANGDLL_PUSH "Greek"
  !insertmacro MUI_LANGDLL_PUSH "Russian"
  ;!insertmacro MUI_LANGDLL_PUSH "PortugueseBR"
  !insertmacro MUI_LANGDLL_PUSH "Ukrainian"
  !insertmacro MUI_LANGDLL_PUSH "Czech"
  ;!insertmacro MUI_LANGDLL_PUSH "Bulgarian"
  
  ; Pass count of pushed items, if value exceeds items pushed then dialog not shown
  Push 13F ;16F ;16 = number of languages, F = change font

  LangDLL::LangDialog "Installation Language" "Please select a language."

  Pop $LANGUAGE
  StrCmp $LANGUAGE "cancel" 0 +2
    Abort

FunctionEnd


	!insertmacro MUI_SECTIONS_FINISHHEADER


	; section and subsection descriptions (scroll over text)
	!insertmacro MUI_FUNCTIONS_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${section_abi} $(DESC_section_abi)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_shellupdate} $(DESC_section_shellupdate)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts} $(DESC_ssection_shortcuts)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_sm_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_desktop_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_sm_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!insertmacro MUI_DESCRIPTION_TEXT ${section_desktop_shortcuts_au} $(DESC_ssection_shortcuts_au)

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
		loop_start:  
			!insertmacro MUI_DESCRIPTION_TEXT $R1 $(DESC_ssection_dictionary)
			IntOp $R1 $R1 + 1  
			IntCmpU $R1 $R2 loop_end
		Goto loop_start 
		loop_end:
		pop $R2
		pop $R1
!endif


	!insertmacro MUI_FUNCTIONS_DESCRIPTION_END

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
	StrCpy $0 "$(SM_PRODUCT_GROUP)"
	RMDir /r "$SMPROGRAMS\$0"

	; remove desktop shortcut.
	StrCpy $0 "$(SHORTCUT_NAME)"
	Delete "$DESKTOP\$0.lnk"

	; remove directories used (and any files in them)
	RMDir /r "$INSTDIR"

	!ifndef CLASSIC_UI
		!insertmacro MUI_UNFINISHHEADER
	!endif

SectionEnd

!ifndef CLASSIC_UI
;Uninstaller Functions
Function un.onInit

  ;Get language from registry
  ReadRegStr $LANGUAGE HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Installer Language"
  
FunctionEnd
!endif

; End of File
