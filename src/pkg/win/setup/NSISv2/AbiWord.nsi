;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Primary NSIS v2 installer script
;Author         Kenneth J. Davis <jeremyd@computer.org> (2002,2003)
;Copyright      Alan Horkan <horkana@tcd.ie> (2002)
;               Michael D. Pritchett <mpritchett@attglobal.net> (2002)
;               Copyright (C) 2008 AbiSource Corporation B.V.
;Version        see AbiSource CVS


; Include user settable values (compile time options)
!include "abi_options.nsh"

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
; Personal build (to be used by all!!! except AbiSource Corporation B.V.)
!define MUI_ICON "${NSIS_SCRIPT_PATH}\setup.ico"
!define MUI_UNICON "${NSIS_SCRIPT_PATH}\setup.ico"
!else
; Trademarked build
!define MUI_ICON "${NSIS_SCRIPT_PATH}\setup_tm.ico"
!define MUI_UNICON "${NSIS_SCRIPT_PATH}\setup_tm.ico"
!endif

; Specify the bitmap to use
!define MUI_CHECKBITMAP "${NSIS_SCRIPT_PATH}\modern.bmp"

; Specify filename of resulting installer
OutFile "${INSTALLERNAME}"

; The name displayed by the installer
Name "${PRODUCT} ${VERSION}"
BrandingText "${PRODUCT} ${VERSION}"

; The default installation directory
InstallDir $PROGRAMFILES\${PRODUCT}      ; e.g. "C:\Program Files\AbiWord"

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\${PRODUCT} "Install_Dir"


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
	SetOutPath $INSTDIR\bin


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Determine if primary executable exists,
	; If we are in modify mode then we expect it to exist
	; (where Not existing is a possible error condition)
	; Else in normal install mode, we expect it to NOT exist
	; (where existing is a possible error condition)
	StrCpy $R0 1	; flag we want to install main program
	${If} ${FileExists} "$INSTDIR\${PROGRAMEXE}"
		${If} $v_modifyinstall == 1 
			DetailPrint "Modify mode: successfully found $INSTDIR\${PROGRAMEXE}"
			StrCpy $R0 0	; skip extraction if already there
		${Else}
			DetailPrint "Install mode: Warning found $INSTDIR\${PROGRAMEXE}"
			MessageBox MB_YESNO "$(PROMPT_OVERWRITE)" /SD IDYES IDYES +2
				Abort "$(MSG_ABORT)"
		${EndIf}
	${Else}	; we need to install the main program
		${If} $v_modifyinstall == 1 
			DetailPrint "Modify mode: Warning failed to find $INSTDIR\${PROGRAMEXE}"
			MessageBox MB_YESNO "$(PROMPT_NOMAINPROGRAM_CONTINUEANYWAY)" /SD IDYES IDYES +2
				Abort "$(MSG_ABORT)"
		${Else}
			DetailPrint "Install mode: "
		${EndIf}
	${EndIf}

	; Actually perform the installation
	${If} $R0 == 1
		; Install main executable
		File "${ABIWORD_COMPILED_PATH}\bin\${PROGRAMEXE}"
		File "${ABIWORD_COMPILED_PATH}\bin\LibAbiWord.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\bz2-1.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libenchant.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libfribidi-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libglib-2.0-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libgobject-2.0-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libgio-2.0-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libgthread-2.0-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libgmodule-2.0-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libgsf-1-114.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libintl-8.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libpng12-0.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libxml2-2.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\libwv-1-2-3.dll"
		File "${ABIWORD_COMPILED_PATH}\bin\zlib1.dll"
	${EndIf}
	
	; install the msvc redistributable; yes, we are allowed
	; to ship this:
	File "${NSIS_SCRIPT_PATH}\vcredist_x86.exe"
	ExecWait '"$INSTDIR\bin\vcredist_x86.exe" /q:a /c:"VCREDI~1.EXE /q:a /c:""msiexec /i vcredist.msi /qb!"" "'
	Delete "$INSTDIR\bin\vcredist_x86.exe"
SectionEnd
!macro Remove_${section_abi}
	;Removes this component
	DetailPrint "*** Removing Main Component..."
	Delete "$INSTDIR\bin\${PROGRAMEXE}"
	Delete "$INSTDIR\bin\LibAbiWord.dll"
	Delete "$INSTDIR\bin\bz2-1.dll"
	Delete "$INSTDIR\bin\libenchant.dll"
	Delete "$INSTDIR\bin\libfribidi-0.dll"
	Delete "$INSTDIR\bin\libglib-2.0-0.dll"
	Delete "$INSTDIR\bin\libgobject-2.0-0.dll"
	Delete "$INSTDIR\bin\libgio-2.0-0.dll"
	Delete "$INSTDIR\bin\libgthread-2.0-0.dll"
	Delete "$INSTDIR\bin\libgmodule-2.0-0.dll"
	Delete "$INSTDIR\bin\libgsf-1-114.dll"
	Delete "$INSTDIR\bin\libintl-8.dll"
	Delete "$INSTDIR\bin\libpng12-0.dll"
	Delete "$INSTDIR\bin\libxml2-2.dll"
	Delete "$INSTDIR\bin\libwv-1-2-3.dll"
	Delete "$INSTDIR\bin\zlib1.dll"

	; delete the BIN subdirectory
	RMDir "$INSTDIR\bin"
	IfFileExists "$INSTDIR\bin" 0 +2
	DetailPrint "Unable to remove $INSTDIR\bin directory."
!macroend


Section "" section_core_inv ; invisible section that must also be installed, sets installer information
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${FULLSECT} ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	; TODO: determine if we should be using HKCU instead of HKLM for some of these
	; or see if user has permission and ask them...

	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\${PRODUCT}\v${VERSION_MAJOR} "Install_Dir" "$INSTDIR"

	; (User Informational Purposes ONLY!!!)
	; Write the current version installed to the registery
	WriteRegStr HKLM SOFTWARE\${PRODUCT}\v${VERSION_MAJOR} "Version" "${VERSION}"

	; Write the uninstall keys for Windows
	!include "abi_util_reg_uninst.nsh"

SectionEnd
!macro Remove_${section_core_inv}
	; remove registry keys
	DetailPrint "*** Removing Primary Registry Keys..."

	; removes all the uninstaller info we added
	DeleteRegKey HKLM "${REG_UNINSTALL_KEY}"

	; removes all the stuff we store concerning this major revision of AbiWord
	DeleteRegKey HKLM SOFTWARE\${PRODUCT}\v${VERSION_MAJOR}

!macroend


Section "" section_abi_req
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${FULLSECT} ${DLSECT} RO	; included in Typical, Full, Minimal, Required

	; Image plugin for importers & cut-n-paste of 
	; various standard image formats (BMP, WMF, JPEG) on Windows
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginWin32gfx.dll"

	SetOutPath $INSTDIR\profiles
	File "${ABIWORD_COMPILED_PATH}\profiles\system.profile*"
  
  SetOutPath $INSTDIR/share/locale/am_ET/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/am_ET.mo
  SetOutPath $INSTDIR/share/locale/ar/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ar.mo
  SetOutPath $INSTDIR/share/locale/ast_ES/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ast_ES.mo
  SetOutPath $INSTDIR/share/locale/ayc_BO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ayc_BO.mo
  SetOutPath $INSTDIR/share/locale/aym_BO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/aym_BO.mo
  SetOutPath $INSTDIR/share/locale/be_BY/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/be_BY.mo
  SetOutPath $INSTDIR/share/locale/be@latin/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/be@latin.mo
  SetOutPath $INSTDIR/share/locale/bg_BG/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/bg_BG.mo
  SetOutPath $INSTDIR/share/locale/br_FR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/br_FR.mo
  SetOutPath $INSTDIR/share/locale/ca_ES/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ca_ES.mo
  SetOutPath $INSTDIR/share/locale/cs_CZ/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/cs_CZ.mo
  SetOutPath $INSTDIR/share/locale/cy_GB/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/cy_GB.mo
  SetOutPath $INSTDIR/share/locale/da_DK/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/da_DK.mo
  SetOutPath $INSTDIR/share/locale/de_CH/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/de_CH.mo
  SetOutPath $INSTDIR/share/locale/de_DE/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/de_DE.mo
  SetOutPath $INSTDIR/share/locale/el_GR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/el_GR.mo
  SetOutPath $INSTDIR/share/locale/en_AU/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/en_AU.mo
  SetOutPath $INSTDIR/share/locale/en_CA/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/en_CA.mo
  SetOutPath $INSTDIR/share/locale/en_GB/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/en_GB.mo
  SetOutPath $INSTDIR/share/locale/en_IE/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/en_IE.mo
  SetOutPath $INSTDIR/share/locale/eo/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/eo.mo
  SetOutPath $INSTDIR/share/locale/es_ES/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/es_ES.mo
  SetOutPath $INSTDIR/share/locale/es_MX/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/es_MX.mo
  SetOutPath $INSTDIR/share/locale/et/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/et.mo
  SetOutPath $INSTDIR/share/locale/eu_ES/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/eu_ES.mo
  SetOutPath $INSTDIR/share/locale/fi_FI/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/fi_FI.mo
  SetOutPath $INSTDIR/share/locale/fr_FR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/fr_FR.mo
  SetOutPath $INSTDIR/share/locale/ga_IE/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ga_IE.mo
  SetOutPath $INSTDIR/share/locale/gl/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/gl.mo
  SetOutPath $INSTDIR/share/locale/he_IL/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/he_IL.mo
  SetOutPath $INSTDIR/share/locale/hr_HR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/hr_HR.mo
  SetOutPath $INSTDIR/share/locale/hu_HU/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/hu_HU.mo
  SetOutPath $INSTDIR/share/locale/id_ID/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/id_ID.mo
  SetOutPath $INSTDIR/share/locale/it_IT/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/it_IT.mo
  SetOutPath $INSTDIR/share/locale/ja_JP/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ja_JP.mo
  SetOutPath $INSTDIR/share/locale/jbo/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/jbo.mo
  SetOutPath $INSTDIR/share/locale/ko/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ko.mo
  SetOutPath $INSTDIR/share/locale/ku/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ku.mo
  SetOutPath $INSTDIR/share/locale/lt_LT/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/lt_LT.mo
  SetOutPath $INSTDIR/share/locale/lv_LV/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/lv_LV.mo
  SetOutPath $INSTDIR/share/locale/mg_MG/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/mg_MG.mo
  SetOutPath $INSTDIR/share/locale/mh_MH/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/mh_MH.mo
  SetOutPath $INSTDIR/share/locale/mk_MK/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/mk_MK.mo
  SetOutPath $INSTDIR/share/locale/ms_MY/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ms_MY.mo
  SetOutPath $INSTDIR/share/locale/nb_NO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/nb_NO.mo
  SetOutPath $INSTDIR/share/locale/ne_NP/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ne_NP.mo
  SetOutPath $INSTDIR/share/locale/nl_NL/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/nl_NL.mo
  SetOutPath $INSTDIR/share/locale/nn_NO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/nn_NO.mo
  SetOutPath $INSTDIR/share/locale/pl_PL/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/pl_PL.mo
  SetOutPath $INSTDIR/share/locale/ps/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ps.mo
  SetOutPath $INSTDIR/share/locale/pt_BR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/pt_BR.mo
  SetOutPath $INSTDIR/share/locale/pt_PT/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/pt_PT.mo
  SetOutPath $INSTDIR/share/locale/quh_BO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/quh_BO.mo
  SetOutPath $INSTDIR/share/locale/qul_BO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/qul_BO.mo
  SetOutPath $INSTDIR/share/locale/ro_RO/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ro_RO.mo
  SetOutPath $INSTDIR/share/locale/ru_RU/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ru_RU.mo
  SetOutPath $INSTDIR/share/locale/sc_IT/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sc_IT.mo
  SetOutPath $INSTDIR/share/locale/sk_SK/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sk_SK.mo
  SetOutPath $INSTDIR/share/locale/sl_SI/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sl_SI.mo
  SetOutPath $INSTDIR/share/locale/sq_AL/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sq_AL.mo
  SetOutPath $INSTDIR/share/locale/sr/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sr.mo
  SetOutPath $INSTDIR/share/locale/sr_SR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sr_SR.mo
  SetOutPath $INSTDIR/share/locale/sv_SE/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/sv_SE.mo
  SetOutPath $INSTDIR/share/locale/ta_IN/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ta_IN.mo
  SetOutPath $INSTDIR/share/locale/tr_TR/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/tr_TR.mo
  SetOutPath $INSTDIR/share/locale/uk_UA/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/uk_UA.mo
  SetOutPath $INSTDIR/share/locale/ur/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/ur.mo
  SetOutPath $INSTDIR/share/locale/vi_VN/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/vi_VN.mo
  SetOutPath $INSTDIR/share/locale/wo_SN/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/wo_SN.mo
  SetOutPath $INSTDIR/share/locale/yi/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/yi.mo
  SetOutPath $INSTDIR/share/locale/zh_CN/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/zh_CN.mo
  SetOutPath $INSTDIR/share/locale/zh_HK/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/zh_HK.mo
  SetOutPath $INSTDIR/share/locale/zh_TW/LC_MESSAGES
  File /oname=abiword.mo ${ABIWORD_MODULE_PATH}/po/zh_TW.mo

	SetOutPath $INSTDIR
	File /oname=copying.txt "${ABIWORD_MODULE_PATH}\Copying"

	; Special Install of Dingbats font
	SetOutPath $TEMP
	File "${NSIS_SCRIPT_PATH}\Dingbats.ttf"
	IfFileExists "$WINDIR\Fonts\Dingbats.ttf" EraseTemp 0
		CopyFiles /SILENT "$TEMP\Dingbats.ttf" "$WINDIR\Fonts" 
	EraseTemp:
	Delete $TEMP\Dingbats.ttf
  
	; Write the start menu entry (if user selected to do so)
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
		;SetShellVarContext current|all???
		${lngCreateSMGroup}  "$STARTMENU_FOLDER"
		SetOutPath $INSTDIR\bin
		${lngCreateShortCut} "$SMPROGRAMS" "$STARTMENU_FOLDER" "$(SHORTCUT_NAME)" "$INSTDIR\${PROGRAMEXE}" "" "$INSTDIR\${PROGRAMEXE}" 0
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
	Delete "$INSTDIR\share\locale\*\LC_MESSAGES\*.mo"
	${DeleteDirIfEmpty} "$INSTDIR\share\locale\*\LC_MESSAGES"
	${DeleteDirIfEmpty} "$INSTDIR\share\locale\*"
	${DeleteDirIfEmpty} "$INSTDIR\share\locale"
	${DeleteDirIfEmpty} "$INSTDIR\share"

	; remove profile sets
	Delete "$INSTDIR\profiles\system.profile*"
	${DeleteDirIfEmpty} "$INSTDIR\profiles"

	; remove always (for interoperability) installed plugins 
	Delete "$INSTDIR\plugins\PluginWin32gfx.dll"
	${DeleteDirIfEmpty} "$INSTDIR\plugins"
	IfFileExists "$INSTDIR\plugins" 0 +2
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

; OPTIONAL plugins
;SubSection /e "$(TITLE_ssection_plugins)" ssection_plugins
!include "section_opt_tools.nsh"
!include "section_opt_importexport.nsh"
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
  ;!insertmacro "${MacroName}" "section_toolsplugins_mathview"
  !insertmacro "${MacroName}" "section_toolsplugins_abicollab"
  !insertmacro "${MacroName}" "section_toolsplugins_grammar"
  !insertmacro "${MacroName}" "section_toolsplugins_urldict"
  !insertmacro "${MacroName}" "section_toolsplugins_google"
  !insertmacro "${MacroName}" "section_toolsplugins_wikipedia"
  !insertmacro "${MacroName}" "section_toolsplugins_babelfish"
  !insertmacro "${MacroName}" "section_toolsplugins_freetranslation"
  ;!insertmacro "${MacroName}" "section_toolsplugins_scripthappy"
  
  ; Imp/exp plugins
  ${MarkSubSectionStart} "*** ssection_impexpplugins:"

  !insertmacro "${MacroName}" "section_impexpplugins_applix"
  !insertmacro "${MacroName}" "section_impexpplugins_clarisworks"
  !insertmacro "${MacroName}" "section_impexpplugins_docbook"
  !insertmacro "${MacroName}" "section_impexpplugins_officeopenxml"
  !insertmacro "${MacroName}" "section_impexpplugins_opendocument"
  ;!insertmacro "${MacroName}" "section_impexpplugins_openwriter"
  !insertmacro "${MacroName}" "section_impexpplugins_iscii_text"
  !insertmacro "${MacroName}" "section_impexpplugins_eml"
  ;!insertmacro "${MacroName}" "section_impexpplugins_palmdoc"
  !insertmacro "${MacroName}" "section_impexpplugins_wml"
  !insertmacro "${MacroName}" "section_impexpplugins_xslfo"

  ;!insertmacro "${MacroName}" "section_impexpplugins_mswrite"
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
  ExecWait '$R0 /S _?=$INSTDIR\plugins' ;Do not copy the uninstaller to a temp files
 
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
  ExecWait '$R0 /S _?=$INSTDIR\plugins'  ;Do not copy the uninstaller to a temp files
 
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
	${DeleteDirIfEmpty} "$INSTDIR"
	IfFileExists "$INSTDIR" 0 +2
	DetailPrint "Unable to remove $INSTDIR"

	; attempt to remove install directory
	${DeleteDirIfEmpty} "$INSTDIR"
	IfFileExists "$INSTDIR" 0 +2
	DetailPrint "Unable to remove $INSTDIR"

SectionEnd

; End of File
