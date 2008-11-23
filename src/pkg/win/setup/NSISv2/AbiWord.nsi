;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Primary NSIS v2 installer script
;Author         Kenneth J. Davis <jeremyd@computer.org> (2002,2003)
;Copyright      Alan Horkan <horkana@tcd.ie> (2002)
;               Michael D. Pritchett <mpritchett@attglobal.net> (2002)
;               Copyright (C) 2008 AbiSource Corporation B.V.
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

; Print out GPL warning
!ifdef OPT_CRTL_LOCAL
!warning "Including a non-GPL compatibly licensed Runtime is probably a violation of GPL!"
!warning "Do NOT distribute installers that included Microsoft's (R) C Runtime DLL!"
!endif


; set application defines, i.e. app name/main executable name/...
!include "abi_appdef.nsh"

; Do a Cyclic Redundancy Check to make sure the installer
; was not corrupted by the download.  
CRCCheck on

; set the compression algorithm used, zlib | bzip2 | lzma
SetCompressor /SOLID lzma

; where to look for NSIS plugins during setup creation
; default includes ./plugins, but we also want to check current directory
;!addplugindir .

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
BrandingText "${PRODUCT} ${VERSION}"

; The default installation directory
InstallDir $PROGRAMFILES\${APPSET}${VERSION_MAJOR}      ; e.g. "C:\Program Files\AbiSuite2"

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Install_Dir"


; Useful inclusions
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "abi_util_winver.nsh"
!include "abi_util_ifexists.nsh"
!include "abi_util_startmenu.nsh"
!include "abi_util_addremove.nsh"
!include "abi_util_deldir.nsh"
!include "abi_parsecmdline.nsh"

; Support 'Modern' UI
!include "abi_mui.nsh"

; support for multiple languages within installer
!include "abi_lng_list.nsh"


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
!define TYPICALSECT 1

InstType "Full"                           ;Section 2
!define FULLSECT 2

InstType "Full (with file associations)"  ;Section 3
!define FULLASSOCSECT 3

InstType "Minimal"                        ;Section 4
!define MINIMALSECT 4

!ifndef NODOWNLOADS
InstType "Full plus Downloads"		;Section 5
!define DLSECT 5
!else
!define DLSECT
!endif
; any other combination is "Custom"


; *********************************************************************


SubSection /e "$(TITLE_ssection_core)" ssection_core

; The stuff that must be installed
Section "" section_abi
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${FULLSECT} ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Display installer command line options
	Call GetParameters
	pop $0
	StrCmp $0 "" +2  ; but only if some actually given
	DetailPrint "Installer command line parameters are ($0)"


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\${PRODUCT}\bin


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Determine if primary executable exists,
	; If we are in modify mode then we expect it to exist
	; (where Not existing is a possible error condition)
	; Else in normal install mode, we expect it to NOT exist
	; (where existing is a possible error condition)
	StrCpy $R0 1	; flag we want to install main program
	${If} ${FileExists} "$INSTDIR\${MAINPROGRAM}"
		${If} $v_modifyinstall == 1 
			DetailPrint "Modify mode: successfully found $INSTDIR\${MAINPROGRAM}"
			StrCpy $R0 0	; skip extraction if already there
		${Else}
			DetailPrint "Install mode: Warning found $INSTDIR\${MAINPROGRAM}"
			MessageBox MB_YESNO "$(PROMPT_OVERWRITE)" /SD IDYES IDYES +2
				Abort "$(MSG_ABORT)"
		${EndIf}
	${Else}	; we need to install the main program
		${If} $v_modifyinstall == 1 
			DetailPrint "Modify mode: Warning failed to find $INSTDIR\${MAINPROGRAM}"
			MessageBox MB_YESNO "$(PROMPT_NOMAINPROGRAM_CONTINUEANYWAY)" /SD IDYES IDYES +2
				Abort "$(MSG_ABORT)"
		${Else}
			DetailPrint "Install mode: "
		${EndIf}
	${EndIf}

	; Actually perform the installation
	${If} $R0 == 1
		; Install main executable
		File "${PROGRAMEXE}"
		File "..\..\..\..\libs\zlib\zlib1.dll"
		File "libglib-2.0-0.dll"
		File "libgobject-2.0-0.dll"
		File "libgthread-2.0-0.dll"
		File "libgsf-1-114.dll"
		File "mingwm10.dll"
		File "bzip2.dll"
		File "iconv.dll"
		File "intl.dll"
		File "libxml2-2.dll"
		

	${EndIf}
SectionEnd
!macro Remove_${section_abi}
	;Removes this component
	DetailPrint "*** Removing Main Component..."
	Delete "$INSTDIR\${MAINPROGRAM}"
	Delete "$INSTDIR\${PRODUCT}\bin\zlib1.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\libglib-2.0-0.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\libgobject-2.0-0.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\libgsf-1-114.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\bzip2.dll"

	; only for MinGW builds
	${IfExists} "iconv.dll"
		Delete "$INSTDIR\${PRODUCT}\bin\iconv.dll"
	${IfExistsEnd}
	${IfExists} "intl.dll"
		Delete "$INSTDIR\${PRODUCT}\bin\intl.dll"
	${IfExistsEnd}
	${IfExists} "libxml2-2.dll"
		Delete "$INSTDIR\${PRODUCT}\bin\libxml2-2.dll"
	${IfExistsEnd}

	; delete the BIN subdirectory
	RMDir "$INSTDIR\${PRODUCT}\bin"
	IfFileExists "$INSTDIR\${PRODUCT}\bin" 0 +2
	DetailPrint "Unable to remove $INSTDIR\${PRODUCT}\bin directory."
!macroend


Section "" section_core_inv ; invisible section that must also be installed, sets installer information
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${FULLSECT} ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	; TODO: determine if we should be using HKCU instead of HKLM for some of these
	; or see if user has permission and ask them...

	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Install_Dir" "$INSTDIR"

	; (User Informational Purposes ONLY!!!)
	; Write the current version installed to the registery
	WriteRegStr HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR} "Version" "${VERSION}"

	; Write the uninstall keys for Windows
	!include "abi_util_reg_uninst.nsh"

SectionEnd
!macro Remove_${section_core_inv}
	; remove registry keys
	DetailPrint "*** Removing Primary Registry Keys..."

	; removes all the uninstaller info we added
	DeleteRegKey HKLM "${REG_UNINSTALL_KEY}"

	; removes all the stuff we store concerning this major revision of AbiWord
	DeleteRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}

!macroend


Section "" section_abi_req
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${FULLSECT} ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	; Image plugin for importers & cut-n-paste of 
      ; various standard image formats (BMP, WMF, JPEG) on Windows
	SetOutPath $INSTDIR\AbiWord\plugins
	File "..\plugins\Abi_IEG_Win32Native.dll"

	SetOutPath $INSTDIR\${PRODUCT}
	File "..\AbiSuite\AbiWord\system.*"
	File /r "..\AbiSuite\AbiWord\strings"

	SetOutPath $INSTDIR
	File /oname=copying.txt "..\AbiSuite\Copying"
	File "..\AbiSuite\readme.txt"
	File "..\AbiSuite\readme.abw"

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
		SetOutPath $INSTDIR\AbiWord\bin
		${lngCreateShortCut} "$SMPROGRAMS" "$STARTMENU_FOLDER" "$(SHORTCUT_NAME)" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
		SetOutPath $TEMP ; needed for removing the directories on uninstall
		${lngCreateShortCut} "$SMPROGRAMS" "$STARTMENU_FOLDER" "$(SHORTCUT_NAME_UNINSTALL)" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	!insertmacro MUI_STARTMENU_WRITE_END

SectionEnd
!macro Remove_${section_abi_req}
	;Removes this component
	DetailPrint "*** Removing Support Files and Start Menu Entry..."

	; remove start menu shortcuts. (tries to get folder name from registry)
	;Delete "$SMPROGRAMS\${SM_PRODUCT_GROUP}\*.*"
	!insertmacro MUI_STARTMENU_GETFOLDER Application $0
	StrCmp "$0" "" +2			; don't accidently delete all start menu entries
      RMDir /r "$SMPROGRAMS\$0"

	; remove desktop shortcut.
	StrCpy $0 "$(SHORTCUT_NAME)"
	Delete "$DESKTOP\$0.lnk"

	; don't bother removing Dingbats font
	; it shouldn't hurt to leave installed and user may now rely on it
	; as its in the System directory

	; remove files we installed in $INSTDIR
	Delete "$INSTDIR\copying.txt"
	Delete "$INSTDIR\readme.txt"
	Delete "$INSTDIR\readme.abw"

	; remove string sets
	Delete "$INSTDIR\${PRODUCT}\strings\*.strings"
	${DeleteDirIfEmpty} "$INSTDIR\${PRODUCT}\strings"

	; remove profile sets
	Delete "$INSTDIR\${PRODUCT}\system.profile*"

	; remove always (for interoperability) installed plugins 
	Delete "$INSTDIR\${PRODUCT}\plugins\Abi_IEG_Win32Native.dll"
	${DeleteDirIfEmpty} "$INSTDIR\${PRODUCT}\plugins"
	IfFileExists "$INSTDIR\${PRODUCT}\plugins" 0 +2
	DetailPrint "Unable to remove plugin directory, please use plugin uninstaller or manually delete."
!macroend

SubSectionEnd ; core
!macro Remove_${ssection_core}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_core"
!macroend
!macro Keeping_${ssection_core}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_core"
!macroend


; *********************************************************************

; OPTIONAL File associations
!include "abi_section_opt_fileassoc.nsh"

; *********************************************************************
; *********************************************************************
!ifdef OPT_PLUGINS

;!include "plugins\abi_misc_plugins.nsh"
 
; OPTIONAL plugins
;SubSection /e "$(TITLE_ssection_plugins)" ssection_plugins
!include "plugins\section_opt_tools.nsh"
!include "plugins\section_opt_importexport.nsh"
;SubSectionEnd ; plugins
;!macro Remove_${ssection_plugins}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
;	DetailPrint "*** ssection_plugins"
;!macroend

!endif
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
!macro Remove_${ssection_helper_files}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_helper_files"
!macroend





;--- Add/Remove callback functions: ---
!define MarkSubSectionStart "DetailPrint"

!macro SectionList MacroName
  ;This macro used to perform operation on multiple sections.
  ;List all of your components in following manner here.
  ;  use ${MarkSubSectionStart} to indicate start of a subsection
  ;  then list all sections (and nested subsections) 
  ;  lastly list the subsection (so it can remove any contents
  ;  shared by multiple sections contained within).

  ${MarkSubSectionStart} "*** ssection_core:"
  !insertmacro "${MacroName}" "section_abi"
  !insertmacro "${MacroName}" "section_core_inv"
  !insertmacro "${MacroName}" "section_abi_req"
  !insertmacro "${MacroName}" "ssection_core"

  ${MarkSubSectionStart} "*** ssection_fa_shellupdate:"
  !insertmacro "${MacroName}" "section_fa_shellupdate_inv"
  !insertmacro "${MacroName}" "section_fa_abw"
  ;insertmacro "${MacroName}" "section_fa_awt"
  ;insertmacro "${MacroName}" "section_fa_zabw"
  !insertmacro "${MacroName}" "section_fa_doc"
  !insertmacro "${MacroName}" "section_fa_rtf"
  !insertmacro "${MacroName}" "ssection_fa_shellupdate"

  ${MarkSubSectionStart} "*** ssection_helper_files:"
  !insertmacro "${MacroName}" "section_help"
  !insertmacro "${MacroName}" "section_templates"
  !insertmacro "${MacroName}" "section_clipart"
!ifdef OPT_CRTL_LOCAL
  !insertmacro "${MacroName}" "section_crtlib_local"
!endif
!ifdef OPT_CRTL_URL
  !insertmacro "${MacroName}" "section_crtlib_dl"
!endif
  ${MarkSubSectionStart} "*** ssection_dictionary:"
  !insertmacro "${MacroName}" "section_dictinary_def_English"
  !ifdef OPT_DICTIONARIES
  ;TODO handle dl dictionaries
  !endif ; OPT_DICTIONARIES
  !insertmacro "${MacroName}" "ssection_dictionary"
  !insertmacro "${MacroName}" "ssection_helper_files"

!ifdef OPT_PLUGINS

  ; Tools plugins
  ${MarkSubSectionStart} "*** ssection_toolsplugins:"
  !insertmacro "${MacroName}" "section_toolsplugins_mathview"
  !insertmacro "${MacroName}" "section_toolsplugins_abicollab"
  !insertmacro "${MacroName}" "section_toolsplugins_grammar"
  !insertmacro "${MacroName}" "section_toolsplugins_urldict"
  !insertmacro "${MacroName}" "section_toolsplugins_google"
  !insertmacro "${MacroName}" "section_toolsplugins_wikipedia"
  !insertmacro "${MacroName}" "section_toolsplugins_babelfish"
  !insertmacro "${MacroName}" "section_toolsplugins_freetranslation"
  !insertmacro "${MacroName}" "section_toolsplugins_scripthappy"
  
  ; Imp/exp plugins
  ${MarkSubSectionStart} "*** ssection_impexpplugins:"

  !insertmacro "${MacroName}" "section_impexpplugins_applix"
  !insertmacro "${MacroName}" "section_impexpplugins_clarisworks"
  !insertmacro "${MacroName}" "section_impexpplugins_docbook"
  !insertmacro "${MacroName}" "section_impexpplugins_officeopenxml"
  !insertmacro "${MacroName}" "section_impexpplugins_opendocument"
  !insertmacro "${MacroName}" "section_impexpplugins_openwriter"
  !insertmacro "${MacroName}" "section_impexpplugins_iscii_text"
  !insertmacro "${MacroName}" "section_impexpplugins_eml"
  !insertmacro "${MacroName}" "section_impexpplugins_palmdoc"
  !insertmacro "${MacroName}" "section_impexpplugins_wml"
  !insertmacro "${MacroName}" "section_impexpplugins_xslfo"

  !insertmacro "${MacroName}" "section_impexpplugins_mswrite"
  !insertmacro "${MacroName}" "section_impexpplugins_opml"
  !insertmacro "${MacroName}" "section_impexpplugins_sdw"
  !insertmacro "${MacroName}" "section_impexpplugins_t602"
  !insertmacro "${MacroName}" "section_impexpplugins_wordperfect"

  !insertmacro "${MacroName}" "section_impexpplugins_hrtext"
  !insertmacro "${MacroName}" "section_impexpplugins_latex"
  
  ;insertmacro "${MacroName}" "ssection_plugins"
!endif

!macroend

Section -FinishComponents
  ;Removes unselected components and writes component status to registry
  !insertmacro SectionList "FinishSection"
SectionEnd
;--- End of Add/Remove callback functions ---

; Perform one time steps done at installer startup, e.g. get installer language, 
; check internet connection/OS/etc and enable/disable options as appropriate
!include "abi_onInit.nsh"


; add descriptions to the component sections
!include "abi_mui_sectdesc.nsh"


; uninstall stuff

;Uninstaller Functions
Function un.onInit
  ;Restore Language selection
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd


; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL $(UNINSTALL_WARNING) IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	;;;;;;;;;;;;;;;;;;;;
; Uninstall IEPlugins if installed
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" \
  "UninstallString"
  StrCmp $R0 "" doneIEPlugins
 
  ClearErrors
  ExecWait '$R0 /S _?=$INSTDIR\AbiWord\plugins' ;Do not copy the uninstaller to a temp files
 
  IfErrors no_remove_uninstaller_IEPlugins
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
	Delete '$R0'
  no_remove_uninstaller_IEPlugins:
  
doneIEPlugins:

;;;;;;;;;;;;;;;;;;;;
; Uninstall ToolsPlugins if installed
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" \
  "UninstallString"
  StrCmp $R0 "" doneToolsPlugins

  ClearErrors
  ExecWait '$R0 /S _?=$INSTDIR\AbiWord\plugins'  ;Do not copy the uninstaller to a temp files
 
  IfErrors no_remove_uninstaller_ToolsPlugins
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
	Delete '$R0'
  no_remove_uninstaller_ToolsPlugins:
  
doneToolsPlugins:

	; removes all optional components
	!insertmacro SectionList "RemoveSection"

	; remove the uninstaller
      Delete "$INSTDIR\${REG_UNINSTALL_FNAME}"

	; attempt to remove actual product directory
	${DeleteDirIfEmpty} "$INSTDIR\AbiWord"
	IfFileExists "$INSTDIR\AbiWord" 0 +2
	DetailPrint "Unable to remove $INSTDIR\AbiWord"

	; attempt to remove install directory
	${DeleteDirIfEmpty} "$INSTDIR"
	IfFileExists "$INSTDIR" 0 +2
	DetailPrint "Unable to remove $INSTDIR"

SectionEnd

; End of File
