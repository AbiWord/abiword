;Title          AbiWord for Windows, NSIS v2 series installer script
;Author         Kenneth J. Davis <jeremyd@computer.org> (2002,2003)
;Copyright      Alan Horkan <horkana@tcd.ie> (2002)
;               Michael D. Pritchett <mpritchett@attglobal.net> (2002)
;               [add your name here]
;Version        see AbiSource CVS

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


; Define this to not include the optional downloadable components (dictionaries, crtlib, ...)
;!define NODOWNLOADS

; Define this for the older (non 'Modern') user interface, English only
!define CLASSIC_UI


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
!packhdr tmp.dat "upx -9 tmp.dat"
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
    ; !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt" ; doesn't work for me, oh well
    !define MUI_FINISHPAGE_NOAUTOCLOSE

  !define MUI_ABORTWARNING

  !define MUI_UNINSTALLER
    !define MUI_UNCONFIRMPAGE

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

!else

  ; The name of the installer
  Name "${PRODUCT} ${VERSION}"
  Caption "${PRODUCT} ${VERSION} Setup"

  ; no WindowsXP manifest stuff
  XPStyle off
  
  ;Page declaration [optional, just specifies default behavior]
  Page license
  Page components
  Page directory
  Page instfiles

  ; The text to prompt the user to enter a directory
  DirText "Choose a directory to install in to:"

  ; set the icons to use
  Icon ${MUI_ICON}
  UninstallIcon ${MUI_UNICON}

  ; set the checkmark bitmaps to use
  CheckBitmap ${MUI_CHECKBITMAP}

  ; The text to prompt the user to enter a directory
  ComponentText "This will install Abiword on your computer. Select which optional components you want installed."

  ; License Information
  LicenseText "This program is Licensed under the GNU General Public License (GPL)."
  LicenseData ${MUI_LICENSEDATA}

!endif

; Install types 
InstType "Typical (default)"              ;Section 1
InstType "Full (with File Associations)"  ;Section 2
InstType "Minimal"                        ;Section 3
; any other combination is "Custom"


; Language Strings
; descriptions for Sections and SubSections

; English
LangString DESC_section_abi           ${LANG_English} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_English} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_English} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_English} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_English} "Installs shortcuts for all users (or current user on systems without multiple users)."

; French
LangString DESC_section_abi           ${LANG_French} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_French} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_French} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_French} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_French} "Installs shortcuts for all users (or current user on systems without multiple users)."

; German
LangString DESC_section_abi           ${LANG_German} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_German} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_German} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_German} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_German} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Spanish
LangString DESC_section_abi           ${LANG_Spanish} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Spanish} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Spanish} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Spanish} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Spanish} "Installs shortcuts for all users (or current user on systems without multiple users)."

; SimpChinese
LangString DESC_section_abi           ${LANG_SimpChinese} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_SimpChinese} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_SimpChinese} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_SimpChinese} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_SimpChinese} "Installs shortcuts for all users (or current user on systems without multiple users)."

; TradChinese
LangString DESC_section_abi           ${LANG_TradChinese} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_TradChinese} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_TradChinese} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_TradChinese} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_TradChinese} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Japanese
LangString DESC_section_abi           ${LANG_Japanese} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Japanese} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Japanese} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Japanese} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Japanese} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Italian
LangString DESC_section_abi           ${LANG_Italian} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Italian} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Italian} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Italian} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Italian} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Dutch
LangString DESC_section_abi           ${LANG_Dutch} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Dutch} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Dutch} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Dutch} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Dutch} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Polish
;LangString DESC_section_abi           ${LANG_Polish} "Required.  Installs the actual AbiWord.exe program."
;Langstring DESC_section_shellupdate   ${LANG_Polish} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
;LangString DESC_ssection_shortcuts    ${LANG_Polish} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
;LangString DESC_ssection_shortcuts_cu ${LANG_Polish} "Installs shortcuts for the currently logged on user."
;LangString DESC_ssection_shortcuts_au ${LANG_Polish} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Greek
LangString DESC_section_abi           ${LANG_Greek} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Greek} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Greek} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Greek} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Greek} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Russian
LangString DESC_section_abi           ${LANG_Russian} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Russian} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Russian} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Russian} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Russian} "Installs shortcuts for all users (or current user on systems without multiple users)."

; PortugueseBR
;LangString DESC_section_abi           ${LANG_PortugueseBR} "Required.  Installs the actual AbiWord.exe program."
;Langstring DESC_section_shellupdate   ${LANG_PortugueseBR} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
;LangString DESC_ssection_shortcuts    ${LANG_PortugueseBR} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
;LangString DESC_ssection_shortcuts_cu ${LANG_PortugueseBR} "Installs shortcuts for the currently logged on user."
;LangString DESC_ssection_shortcuts_au ${LANG_PortugueseBR} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Ukrainian
LangString DESC_section_abi           ${LANG_Ukrainian} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Ukrainian} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Ukrainian} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Ukrainian} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Ukrainian} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Czech
LangString DESC_section_abi           ${LANG_Czech} "Required.  Installs the actual AbiWord.exe program."
Langstring DESC_section_shellupdate   ${LANG_Czech} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts    ${LANG_Czech} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu ${LANG_Czech} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au ${LANG_Czech} "Installs shortcuts for all users (or current user on systems without multiple users)."

; Bulgarian
;LangString DESC_section_abi           ${LANG_Bulgarian} "Required.  Installs the actual AbiWord.exe program."
;Langstring DESC_section_shellupdate   ${LANG_Bulgarian} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
;LangString DESC_ssection_shortcuts    ${LANG_Bulgarian} "Installs shortcuts in various places to allow staring AbiWord through additional locations."
;LangString DESC_ssection_shortcuts_cu ${LANG_Bulgarian} "Installs shortcuts for the currently logged on user."
;LangString DESC_ssection_shortcuts_au ${LANG_Bulgarian} "Installs shortcuts for all users (or current user on systems without multiple users)."

; End Language descriptions


; The stuff that must be installed
Section "Abiword.exe (required)" section_abi
	SectionIn 1 2 3 RO	; included in Typical, Full, Minimal, Required
	;;
	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\${MAINPROGRAM}" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing ${PRODUCT}?" IDYES DoInstall
	
	Abort "Quitting the install process"

	DoInstall:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\bin
	File "AbiWord.exe"

	SetOutPath $INSTDIR\AbiWord
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

	; New Uninstaller 
	WriteUninstaller "Uninstall${PRODUCT}${VERSION_MAJOR}.exe"

SectionEnd

; OPTIONAL Registry Settings
Section "Update Registry Settings" section_shellupdate
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
	WriteRegStr HKCR ".abw" "" "${APPSET}.${PRODUCT}"
	WriteRegStr HKCR ".abw" "Content Type" "application/abiword"
	WriteRegStr HKCR ".awt" "" "${APPSET}.${PRODUCT}"
	WriteRegStr HKCR ".awt" "Content Type" "application/abiword-template"
	WriteRegStr HKCR ".zabw" "" "${APPSET}.${PRODUCT}"
	WriteRegStr HKCR ".zabw" "Content Type" "application/abiword-compressed"

SectionEnd

SubSection /e "Shortcuts" ssection_shortcuts

SubSection /e "Shortcuts (Current User)" ssection_shortcuts_cu

; OPTIONAL Start Menu Shortcut for the current user profile
Section "Start Menu Shortcuts (Current User)"
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	CreateDirectory "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}\Uninstall ${PRODUCT}.lnk" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	CreateShortCut "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}\${PRODUCT}.lnk" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
SectionEnd

; OPTIONAL Desktop Shortcut for the current user profile
Section "Desktop Shortcut (Current User)"
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
SectionEnd


SubSectionEnd ; Shortcuts (Current User)
SubSection /e "Shortcuts (All Users)" ssection_shortcuts_au


; OPTIONAL Start Menu Shortcut for the special All User profile (not used in win9x) 
Section "Start Menu Shortcuts (All Users)"
	SectionIn 2		; off by default, included in 2 Full Install
	SetShellVarContext all  	; set to all, reset at end of section
	CreateDirectory "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}\Uninstall ${PRODUCT}.lnk" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${PRODUCT}${VERSION_MAJOR}.exe" 0
	CreateShortCut "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}\${PRODUCT}.lnk" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	SetShellVarContext current  	; This is pro'ly overkill
SectionEnd


; OPTIONAL Desktop Shortcut for All Users
Section "Desktop Shortcut (All Users)"
	SectionIn 2	; not in default, included in 2 Full Install
	SetShellVarContext all  	;  All users 
	CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\${MAINPROGRAM}" "" "$INSTDIR\${MAINPROGRAM}" 0
	SetShellVarContext current  	; reset to current user
SectionEnd

SubSectionEnd ; Shortcuts (All Users)"

SubSectionEnd ; Shortcuts


;SectionDivider " general file associations "
SubSection /e "General file associations"


; OPTIONAL 
Section "Associate .doc with AbiWord"
	SectionIn 2
	; Write File Associations
	WriteRegStr HKCR ".doc" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".doc" "Content Type" "application/abiword"
SectionEnd

; OPTIONAL 
Section "Associate .rtf with AbiWord"
	SectionIn 2
	; Write File Associations
	WriteRegStr HKCR ".rtf" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".rtf" "Content Type" "application/abiword"
SectionEnd

SubSectionEnd ; general file associations

;SectionDivider " helper files "
SubSection /e "Helper files"

; MORE OPTIONS
; language packs, clipart, help docs, templates etc.   
;Section "Help Files"
;SectionEnd

; OPTIONAL Installation of Help Files
Section "Help Files"
	SectionIn 1 2
	SetOutPath $INSTDIR\AbiWord
	file /r "..\abisuite\abiword\help"
SectionEnd

; OPTIONAL Installation of Templates
Section "Templates"
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\templates"
SectionEnd

; OPTIONAL Installation of Samples - REMOVED
;Section "Samples"
;	SectionIn 1 2
;	SetOutPath $INSTDIR\AbiWord
;	File /r "..\AbiSuite\AbiWord\sample"
;SectionEnd

; OPTIONAL Installation of Clipart
Section "Clipart"
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\clipart"
SectionEnd

; we only enable this option if a url to connect to was
; specified during installation building; this should
; only be enabled for release builds if your server (where
; the url points) can handle the load and you need
; a crtlib other than msvcrt.dll (or to support Win95)
!ifndef NODOWNLOADS
!ifdef CRTL_URL
; OPTIONAL Installation of c runtime library dll
Section "Download CRTlib ${CRTL_FILENAME}"
	SectionIn 2	; select if full installation choosen
	NSISdl::download "${CRTL_URL}${CRTL_FILENAME}" "$INSTDIR\AbiWord\bin\${CRTL_FILENAME}"
	StrCmp $0 "success" Finish
		; Couldn't download the file
		DetailPrint "Could not download requested c runtime library (DLL): ${CRTL_URL}${CRTL_FILENAME}"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "Failed to download ${CRTL_URL}${CRTL_FILENAME}"
	Finish:
SectionEnd
!endif
!endif

SubSection /e "Dictionaries"

; OPTIONAL Installation of Default Dictionary
Section "en-US  US English (default)"
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
SectionEnd

!ifndef NODOWNLOADS
; NOTE: these just reference files for download then installs them
SubSection /e "Download optional dictionaries"

;TODO make a string and figure out how to let user pick another
!ifndef DICTIONARY_BASE
!define DICTIONARY_BASE "http://dl.sourceforge.net/abiword"
!endif

; $R3 is set to the filename used
Function getDictionary
	!define DICT_LANG $R0
	!define DICT_LOCALE $R1
	!define DICT_ARCH $R2

	; set filename, handle files without locale portion (represented by Xx)
	StrCmp ${DICT_LOCALE} "Xx" noLocale 0
		StrCpy $R3 "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
      Goto Skip1
	noLocale:
		StrCpy $R3 "abispell-${DICT_LANG}.${DICT_ARCH}.tar.gz"
	Skip1:
	!define DICT_FILENAME $R3

	; Quietly download the file
	NSISdl::download "${DICTIONARY_BASE}/${DICT_FILENAME}" "$TEMP\${DICT_FILENAME}"
	StrCmp $0 "success" doDictInst
		; Couldn't download the file
		DetailPrint "Could not download requested dictionary:"
		DetailPrint "  ${DICTIONARY_BASE}/${DICT_FILENAME}"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "Failed to download ${DICTIONARY_BASE}/${DICT_FILENAME}"
	Goto Finish

	doDictInst:
		; Unzip dictionary into dictionary subdirecotry
		untgz::extract "-j" "$TEMP\${DICT_FILENAME}" "-d" "$INSTDIR\dictionary"
		
		; Delete temporary files
		Delete "$TEMP\${DICT_FILENAME}"

	Finish:
		!undef DICT_LANG
		!undef DICT_LOCALE
		!undef DICT_ARCH
FunctionEnd

!macro SectionDict DICT_NAME DICT_LANG DICT_LOCALE DICT_ARCH DICT_SIZE
Section '${DICT_LANG}-${DICT_LOCALE}  ${DICT_NAME}'
;	SectionIn 2	; Full only
	AddSize ${DICT_SIZE}
	StrCpy $R0 ${DICT_LANG}
	StrCpy $R1 ${DICT_LOCALE}
	StrCpy $R2 ${DICT_ARCH}
	Call getDictionary
SectionEnd
!macroend

; These are listed alphabetically based on English LANG-LOCALE
; NOTE: if the dictinaries are updated so to should these sizes (KB)
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

SubSectionEnd ; Optional downloads
!endif  ; NODOWNLOADS

SubSectionEnd ; Dictionaries

SubSectionEnd ; helper files


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

  LangDLL::LangDialog "Installer Language" "Please select a language."

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
		!insertmacro MUI_DESCRIPTION_TEXT ${ssection_shortcuts_au} $(DESC_ssection_shortcuts_au)
	!insertmacro MUI_FUNCTIONS_DESCRIPTION_END
!endif


; uninstall stuff
!ifdef CLASSIC_UI
	UninstallText "This will uninstall ${PRODUCT} v${VERSION_MAJOR}. Hit next to continue."
!endif

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "This will delete $INSTDIR and all subdirectories and files?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}${VERSION_MAJOR}"
	DeleteRegKey HKLM SOFTWARE\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}

	; remove file assoications
	DeleteRegKey HKCR "${APPSET}.${PRODUCT}"
	DeleteRegKey HKCR ".abw"
	DeleteRegKey HKCR ".awt"
	DeleteRegKey HKCR ".zabw"

	ReadRegStr $0 HKCR ".doc" "(Default)"
	StrCmp $0 "${APPSET}.${PRODUCT}" Del_Word_Assoc Skip_Del_Word
	Del_Word_Assoc:
	DeleteRegKey HKCR ".doc"
	Skip_Del_Word:
	
	ReadRegStr $0 HKCR ".rtf" "(Default)"
	StrCmp $0 "${APPSET}.${PRODUCT}" Del_RTF_Assoc Skip_Del_RTF
	Del_RTF_Assoc:
	DeleteRegKey HKCR ".rtf"
	Skip_Del_RTF:
	
	; remove start menu shortcuts.
	;Delete "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}\*.*"
	RMDir /r "$SMPROGRAMS\${PRODUCT} v${VERSION_MAJOR}"

	; remove desktop shortcut.
	Delete "$DESKTOP\${PRODUCT}.lnk"

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
