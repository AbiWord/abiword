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

; set the compression algorithm used, zlib | bzip2 | lzma
SetCompressor lzma

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
  ; specify where to get resources from for UI elements (default)
  !define MUI_UI "${NSISDIR}\Contrib\UIs\modern.exe"

  ; quirk, seems to need to be defined before including Mui.nsh to work
  !define MUI_COMPONENTSPAGE_SMALLDESC

  ; include the Modern UI support
  !include "Mui.nsh"

  ; specify the pages and order to show to user

  ; introduce ourselves
  !insertmacro MUI_PAGE_WELCOME
  ; including the license of AbiWord  (license could be localized, but we lack translations)
  !insertmacro MUI_PAGE_LICENSE $(LicenseTXT)
  ; allow user to select what parts to install
    ; put the description below choices
;    !define MUI_COMPONENTSPAGE_SMALLDESC
  !insertmacro MUI_PAGE_COMPONENTS
  ; and where to install to
  !insertmacro MUI_PAGE_DIRECTORY
  ; where to put in the start menu
    !define MUI_STARTMENUPAGE_DEFAULTFOLDER "$(SM_PRODUCT_GROUP)"
    !define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
    !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}"
    !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
    Var STARTMENU_FOLDER
  !insertmacro MUI_PAGE_STARTMENU "Application" $STARTMENU_FOLDER
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


  ; Languages, include MUI & NSIS language support
  ; then include app install specific language support

    ;Remember the installer language
    !define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
    !define MUI_LANGDLL_REGISTRY_KEY "Software\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}"
    !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

    ; indicate default language definitions to use if a translation is missing a string
    !define DEF_LANG "ENGLISH"

    ; actually sets the LangString
    !macro SETLSTR NAME VALUE	; e.g. English sectID sectDesc
	!echo "${LANG} ( ${LANG_${LANG}} )"
      !define "STRING_ISSET_${LANG}_${NAME}"
      LangString "${NAME}" "${LANG_${LANG}}" "${VALUE}"
    !macroend
    !define SETLSTR "!insertmacro SETLSTR"

    ; macro to set string, assumes LANG already defined (call within context of LANG_LOAD)
    !macro LSTR NAME VALUE	; e.g. sectID sectDesc
      !ifdef SETDEFLANG
        ; if string is already set, we do nothing, otherwise we set to default value and warn user
        !ifndef "STRING_ISSET_${LANG}_${NAME}"
          !ifndef APPSET_LANGUAGEFILE_DEFAULT_USED     ; flag default value must be used
            !define APPSET_LANGUAGEFILE_DEFAULT_USED
          !endif
          ${SETLSTR} "${NAME}" "${VALUE}"  ; set to default value
        !endif
      !else ; just set the value
        ${SETLSTR} "${NAME}" "${VALUE}"
      !endif
    !macroend
    !define LSTR "!insertmacro LSTR"

    ; macro to include necessary language files
    !macro LANG_LOAD LANG
      !insertmacro MUI_LANGUAGE "${LANG}"
      !echo "Loading language ${LANG} ( ${LANG_${LANG}} )"
      ; Specify the license text to use (for multilang support, must come after MUI_LANGUAGE)
      LicenseLangString LicenseTXT "${LANG_${LANG}}" "..\AbiSuite\Copying"
      !verbose push
      !verbose 3
      !include "abi_lng_${LANG}.nsh"   ; Localized Installer Messages (Language Strings)
      !define SETDEFLANG
      !include "abi_lng_${DEF_LANG}.nsh"
      !verbose pop
      !ifdef APPSET_LANGUAGEFILE_DEFAULT_USED
        !undef APPSET_LANGUAGEFILE_DEFAULT_USED
        !warning "${LANG} Installation language file incomplete.  Using default texts for missing strings."
      !endif
      !undef SETDEFLANG
      !echo "End loading language ${LANG}"
      !undef LANG
    !macroend
    !define LANG_LOAD "!insertmacro LANG_LOAD"

  ; load each supported language
  ${LANG_LOAD} "Bulgarian"
  ${LANG_LOAD} "Czech"
  ${LANG_LOAD} "Dutch"
  ${LANG_LOAD} "English"
  ${LANG_LOAD} "French"
  ${LANG_LOAD} "German"
  ${LANG_LOAD} "Greek"
  ${LANG_LOAD} "Italian"
  ${LANG_LOAD} "Japanese"
  ${LANG_LOAD} "Polish"
  ${LANG_LOAD} "Russian"
  ${LANG_LOAD} "PortugueseBR"
  ${LANG_LOAD} "SimpChinese"
  ${LANG_LOAD} "Spanish"
  ${LANG_LOAD} "TradChinese"
  ${LANG_LOAD} "Ukrainian"



!ifdef RESERVE_PLUGINS
  ; Reserve Files to possibly aid in starting faster
  !insertmacro MUI_RESERVEFILE_LANGDLL
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS ;InstallOptions plug-in

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

	; Image plugin for importers & cut-n-paste of 
      ; various standard image formats (BMP, WMF, JPEG) on Windows
	SetOutPath $INSTDIR\AbiWord\plugins
	File "..\plugins\libAbi_IEG_Win32Native.dll"

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
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		;SetShellVarContext current|all???
		${lngCreateSMGroup}  "$STARTMENU_FOLDER"
		${lngCreateShortCut} "$SMPROGRAMS" "$STARTMENU_FOLDER" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
		${lngCreateShortCut} "$SMPROGRAMS" "$STARTMENU_FOLDER" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	!insertmacro MUI_STARTMENU_WRITE_END

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

; optional help section
!include "abi_section_opt_help.nsh"

; optional template section
!include "abi_section_opt_templates.nsh"

; optional clipart section
!include "abi_section_opt_clipart.nsh"

; optional sections for redistributable compontents, e.g. CRTL dll
!include "abi_section_opt_redist.nsh"

; optional sections for dictionaries
!include "abi_section_opt_dictionaries.nsh"

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

;Uninstaller Functions
Function un.onInit
  ;Restore Language selection
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd


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
	; remove start menu shortcuts. (tries to get folder name from registry)
	;Delete "$SMPROGRAMS\${SM_PRODUCT_GROUP}\*.*"
	!insertmacro MUI_STARTMENU_GETFOLDER Application $0
      RMDir /r "$SMPROGRAMS\$0"

	; remove desktop shortcut.
	StrCpy $0 "$(SHORTCUT_NAME)"
	Delete "$DESKTOP\$0.lnk"

	; remove directories used (and any files in them)
	RMDir /r "$INSTDIR"

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

SectionEnd

; End of File
