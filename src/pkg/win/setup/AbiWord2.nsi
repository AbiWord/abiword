; This is a file for creating an installer for Abiword using NSIS v2 series
; originally for NSIS v1 by Alan Horkan <horkana@tcd.ie>
; modified by Michael D. Pritchett <mpritchett@attglobal.net>
; Kenneth J. Davis <jeremyd@computer.org> (2002,2003)
; [add your name here]

!define UI_PRODUCT "AbiWord"
!ifndef UI_VERSION
!define UI_VERSION "2.0"
!endif
!ifndef UI_VERSION_MAJOR
!define UI_VERSION_MAJOR "2"
!endif

; To not include the list of dictionaries ... to download define this
;!define NODOWNLOADS

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; set the compression algorithm used, zlib | bzip2
SetCompressor bzip2

;where to look for NSIS plugins during setup creation
PluginDir .

; no WindowsXP manifest stuff
XPStyle off

; The name of the installer
Name "${UI_PRODUCT} ${UI_VERSION}"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

;OutFile "setup_abiword.X-Y-Z.exe
OutFile "setup_abiword.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite${UI_VERSION_MAJOR}

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite\${UI_PRODUCT}\v${UI_VERSION_MAJOR} "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install Abiword on your computer. Select which optional components you want installed."

; Install types 
InstType "Typical (default)"			; Section 1
InstType "Full (with File Associations)" 	; Section 2
InstType "Minimal"				; Section 3
; any other combination is "Custom"

; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

CheckBitmap ..\..\pkg\win\setup\modern.bmp


; The stuff that must be installed
Section "Abiword.exe (required)"
	SectionIn 1 2 3	; included in Typical, Full, Minimal 
	;;
	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\bin\AbiWord.exe" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWord?" IDYES DoInstall
	
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
  
	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\Abisuite\${UI_PRODUCT}\v${UI_VERSION_MAJOR} "Install_Dir" "$INSTDIR"

	; (User Informational Purposes ONLY!!!)
	; Write the current version installed to the registery
	WriteRegStr HKLM SOFTWARE\Abisuite\${UI_PRODUCT}\v${UI_VERSION_MAJOR} "Version" "${UI_VERSION}"

	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UI_PRODUCT}${UI_VERSION_MAJOR}" "DisplayName" "${UI_PRODUCT} ${UI_VERSION} (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UI_PRODUCT}${UI_VERSION_MAJOR}" "UninstallString" '"$INSTDIR\Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe"'

	; New Uninstaller 
	WriteUninstaller "Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe"

SectionEnd

; OPTIONAL Registry Settings
Section "Update Registry Settings"
	SectionIn 1 2 3
	; Write the AbiSuite.AbiWord Keys
	WriteRegStr HKCR "AbiSuite.AbiWord" "" "AbiWord Document"
	WriteRegStr HKCR "AbiSuite.AbiWord\DefaultIcon" "" "$INSTDIR\AbiWord\bin\AbiWord.exe,2"
	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\command" "" '"$INSTDIR\AbiWord\bin\AbiWord.exe" "%1"'
;	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\command" "" "$INSTDIR\AbiWord\bin\AbiWord.exe"
;	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\ddeexec" "" '[Open("%1")]'
;	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\ddeexec\application" "" "AbiWord"
;	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\ddeexec\topic" "" "System"

	; Write File Associations
	WriteRegStr HKCR ".abw" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".abw" "Content Type" "application/abiword"
	WriteRegStr HKCR ".awt" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".awt" "Content Type" "application/abiword-template"
	WriteRegStr HKCR ".zabw" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".zabw" "Content Type" "application/abiword-compressed"

SectionEnd

SubSection /e "Shortcuts"

SubSection /e "Shortcuts (Current User)"

; OPTIONAL Start Menu Shortcut for the current user profile
Section "Start Menu Shortcuts (Current User)"
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	CreateDirectory "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}"
	CreateShortCut "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}\Uninstall ${UI_PRODUCT}.lnk" "$INSTDIR\Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe" 0
	CreateShortCut "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}\${UI_PRODUCT}.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd

; OPTIONAL Desktop Shortcut for the current user profile
Section "Desktop Shortcut (Current User)"
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	CreateShortCut "$DESKTOP\${UI_PRODUCT}.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd


SubSectionEnd ; Shortcuts (Current User)
SubSection /e "Shortcuts (All Users)"


; OPTIONAL Start Menu Shortcut for the special All User profile (not used in win9x) 
Section "Start Menu Shortcuts (All Users)"
	SectionIn 2		; off by default, included in 2 Full Install
	SetShellVarContext all  	; set to all, reset at end of section
	CreateDirectory "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}"
	CreateShortCut "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}\Uninstall ${UI_PRODUCT}.lnk" "$INSTDIR\Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe" "" "$INSTDIR\Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe" 0
	CreateShortCut "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}\${UI_PRODUCT}.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
	SetShellVarContext current  	; This is pro'ly overkill
SectionEnd


; OPTIONAL Desktop Shortcut for All Users
Section "Desktop Shortcut (All Users)"
	SectionIn 2	; not in default, included in 2 Full Install
	SetShellVarContext all  	;  All users 
	CreateShortCut "$DESKTOP\${UI_PRODUCT}.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
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

Function getDictionary
	!define DICT_LANG $R0
	!define DICT_LOCALE $R1
	!define DICT_ARCH $R2

	; Quietly download the file
	NSISdl::download "${DICTIONARY_BASE}/abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz" "$TEMP\abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
	StrCmp $0 "success" doDictInst
		; Couldn't download the file
		DetailPrint "Could not download requested dictionary:"
		DetailPrint "  ${DICTIONARY_BASE}/abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "Failed to download ${DICTIONARY_BASE}/abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
	Goto Finish

	doDictInst:
		; Unzip dictionary into dictionary subdirecotry
		untgz::extract "-j" "$TEMP\abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz" "-d" "$INSTDIR\dictionary"
		
		; Delete temporary files
		Delete "$TEMP\abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"

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
;insertmacro SectionDict "Esperanto"    "eo" "  " "i386"   942  ;no locale...
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


;SubSection /e " uninstall "

; uninstall stuff
UninstallText "This will uninstall ${UI_PRODUCT} v${UI_VERSION_MAJOR}. Hit next to continue."
;UninstallExeName "Uninstall${UI_PRODUCT}${UI_VERSION_MAJOR}.exe"

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "This will delete $INSTDIR and all subdirectories and files?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UI_PRODUCT}${UI_VERSION_MAJOR}"
	DeleteRegKey HKLM SOFTWARE\Abisuite\${UI_PRODUCT}\v${UI_VERSION_MAJOR}

	; remove file assoications
	DeleteRegKey HKCR "AbiSuite.AbiWord"
	DeleteRegKey HKCR ".abw"
	DeleteRegKey HKCR ".awt"
	DeleteRegKey HKCR ".zabw"

	ReadRegStr $0 HKCR ".doc" "(Default)"
	StrCmp $0 "AbiSuite.AbiWord" Del_Word_Assoc Skip_Del_Word
	Del_Word_Assoc:
	DeleteRegKey HKCR ".doc"
	Skip_Del_Word:
	
	ReadRegStr $0 HKCR ".rtf" "(Default)"
	StrCmp $0 "AbiSuite.AbiWord" Del_RTF_Assoc Skip_Del_RTF
	Del_RTF_Assoc:
	DeleteRegKey HKCR ".rtf"
	Skip_Del_RTF:
	
	; remove start menu shortcuts.
	Delete "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}\*.*"

	; remove desktop shortcut.
	Delete "$DESKTOP\${UI_PRODUCT}.lnk"

	; remove directories used
	RMDir "$SMPROGRAMS\${UI_PRODUCT} v${UI_VERSION_MAJOR}"
	RMDir /r "$INSTDIR"
	 
SectionEnd

;SubSectionEnd ; uninstall

; eof
