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


; set application defines, i.e. app name/main executable name/...
!include "abi_appdef.nsh"

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
!include "abi_util_ifexists.nsh"
!include "abi_util_startmenu.nsh"


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

	; only for MinGW builds
	${IfExists} "libAbiWord.dll"
		File "libAbiWord.dll"
	${IfExistsEnd}
SectionEnd


Section "" ; invisible section that must also be installed, sets installer information
	SectionIn 1 2 3 4 ${DLSECT} RO	; included in Typical, Full, Minimal, Required

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

SubSectionEnd ; core


; *********************************************************************

; OPTIONAL File associations
!include "abi_section_opt_fileassoc.nsh"

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

; OPTIONAL plugins
${IfExists} system.dll.log
!error exists
${IfExistsEnd}

; *********************************************************************

!include "abi_util_winver.nsh"


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
	; remove start menu shortcuts. (tries to get folder name from registry)
	;Delete "$SMPROGRAMS\${SM_PRODUCT_GROUP}\*.*"
	!insertmacro MUI_STARTMENU_GETFOLDER Application $0
	StrCmp "$0" "" +2			; don't accidently delete all start menu entries
      RMDir /r "$SMPROGRAMS\$0"

	; remove desktop shortcut.
	StrCpy $0 "$(SHORTCUT_NAME)"
	Delete "$DESKTOP\$0.lnk"

	; remove directories used (and any files in them)
	RMDir /r "$INSTDIR"

	; remove registry keys

	; removes all the uninstaller info we added
	DeleteRegKey HKLM ${UninstallerKeyName}

	; removes all the stuff we store concerning this major revision of AbiWord
	DeleteRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}

	; remove file assoications (only removes if still registered with ${appType}, tries to restore prior one
      !define appType "${APPSET}.${PRODUCT}"
	; our native ones
	${RemoveFileAssociation} ".abw"  "${appType}"
	${RemoveFileAssociation} ".awt"  "${appType}"
	${RemoveFileAssociation} ".zabw" "${appType}"
	; other common ones
	${RemoveFileAssociation} ".doc"  "${appType}"
	${RemoveFileAssociation} ".rtf"  "${appType}"

	; actual apptype entry
	DeleteRegKey HKCR "${appType}"

SectionEnd

; End of File
