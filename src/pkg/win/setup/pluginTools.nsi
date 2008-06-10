; Tools plugins
; This is a file for creating an installer for Abiword Plugins using NSIS 
; Based on .nsi file created by Alan Horkan <horkana@tcd.ie>
; and modified by Michael D. Pritchett <mpritchett@attglobal.net>
; modified by Kenneth J Davis <jeremyd@computer.org>

!ifndef VERSION_MAJOR
!define VERSION_MAJOR "2"
!endif

!ifndef VERSION_MINOR
!define VERSION_MINOR "1"
!endif

!ifndef VERSION_MICRO
!define VERSION_MICRO "6"
!endif

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; set the compression algorithm used, zlib | bzip2 | lzma
SetCompressor /SOLID lzma

; The name of the installer
Name "AbiWord Tools Plugins"
BrandingText "AbiWord 2.6 Series"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

OutFile "abiword-plugins-tools-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)." "$(^NextBtn)"
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite${VERSION_MAJOR}

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite\AbiWord\v${VERSION_MAJOR} "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install AbiWord's Tools Plugins on your computer."

; Different installation types (usual or with a plugin specific uninstaller)
InstType "Typical (default)"
InstType "Full"

; The text to prompt the user to enter a directory
DirText "Choose the AbiSuite directory where you previously installed Abiword:"

; For NSIS 2.xx
CheckBitmap ..\..\pkg\win\setup\modern.bmp

; The stuff that must be installed
; binary, license, or whatever
;Section "Tools Plugins (required)"
Section
	SectionIn 1 2

	;;;;
	; Testing clause to abort if required AbiWord.exe DLL does not exist
	IfFileExists "$INSTDIR\AbiWord\bin\AbiWord.exe" DoInstall 0

	MessageBox MB_ICONSTOP "Quitting the install process - AbiWord.exe not found"
	Quit

	DoInstall:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\plugins
  
	; Write the installation path into the registry
	;;WriteRegStr HKLM SOFTWARE\Abisuite "Install_Dir" "$INSTDIR"

SectionEnd



!macro dlFileMacro remoteFname localFname errMsg
	!define retryDLlbl retryDL_${__FILE__}${__LINE__}
	!define dlDonelbl dlDoneDL_${__FILE__}${__LINE__}

	;Call ConnectInternet	; try to establish connection if not connected
	;StrCmp $0 "online" 0 ${dlDonelbl}

	${retryDLlbl}:
	NSISdl::download "${remoteFname}" "${localFname}"
	Pop $0 ;Get the return value
	StrCmp $0 "success" ${dlDonelbl}
		; Couldn't download the file
		DetailPrint "${errMsg}"
		DetailPrint "Remote URL: ${remoteFname}"
		DetailPrint "Local File: ${localFname}"
		DetailPrint "NSISdl::download returned $0"
		MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "${errMsg}" IDRETRY ${retryDLlbl}
	${dlDonelbl}:
	!undef retryDLlbl
	!undef dlDonelbl
!macroend
!define dlFile "!insertmacro dlFileMacro"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Macro for unzipping a file from an archive with error reporting
!macro unzipFileMacro archiveFname destinationPath fnameToExtract errMsg
	!define uzDonelbl uzDone_${__FILE__}${__LINE__}

	ZipDLL::extractfile "${archiveFname}" "${destinationPath}" "${fnameToExtract}"
	Pop $0 ; Get return value
	StrCmp $0 "success" ${uzDonelbl}
		; Couldn't unzip the file
		DetailPrint "${errMsg}"
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "${errMsg}" IDOK
	${uzDonelbl}:
	!undef uzDonelbl
!macroend
!define unzipFile "!insertmacro unzipFileMacro"


Section "Equation Editor"
	SectionIn 1 2
	

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiMathView.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Equation Editor Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Equation Editor Plugin (already exists)!"
	Goto End

	DoInstall:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Unzip libmathview into same directory as AbiWord.exe
	SetOutPath $INSTDIR\AbiWord\bin
	File /r "..\bin\libmathview-0.dll"
	File /r "..\bin\libmathview_frontend_libxml2-0.dll"
	
	;Install Configuration Files - This better work...
	SetOutPath $INSTDIR\math
	File /r "..\AbiSuite\math\gtkmathview.conf.xml"
	File /r "..\AbiSuite\math\dictionary-local.xml"
	File /r "..\AbiSuite\math\dictionary.xml"
	
	SetOutPath $INSTDIR\AbiWord\plugins
	File "AbiMathView.dll"

	End:
SectionEnd



SubSection /e "Collaboration Tools"
Section "AbiCollab Real-Time Collaboration"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiCollab.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiCollab Collaboration Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiCollab Collaboration Plugin (already exists)!"
	Goto End

	DoInstall:
	SetOutPath $INSTDIR\AbiWord\bin
	File "..\bin\libsoup-2.4-1.dll"
	File "..\bin\libgthread-2.0-0.dll"
	SetOutPath $INSTDIR\AbiWord\Plugins
	File "AbiCollab.dll"
	WriteRegStr HKCR ".abicollab" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".abicollab" "Content Type" "application/abiword"
	End:
SectionEnd
SubSectionEnd

SubSection /e "Mechanics and Grammar Tools"

!ifdef 0
SubSection "AikSaurus Thesaurus Plugin"
; OPTIONAL
Section "The AikSaurus Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AikSaurusABI.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AikSaurus Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AikSaurus Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AikSaurusABI.dll"

	End:
SectionEnd

; OPTIONAL but needed Registry Settings & data files if AikSaurus (program) not installed
Section "AikSaurus Data Files && Update Registry)"
	SectionIn 1 2

	; TODO actually determine if already exists or not and 
	;      prompt if to use existing data files if there.

	; Skip adding registry entries if plugin doesn't exist
	IfFileExists "$INSTDIR\AbiWord\plugins\AikSaurusABI.dll" 0 End

	; Add data files
	File "meanings.dat"
	File "words.dat"

	; Write out AikSaurus data file directory to registry
	; TODO check if AikSaurus program installed (shares this registry key)
	WriteRegStr HKLM SOFTWARE\Aiksaurus "Data_Dir" "$INSTDIR\AbiWord\plugins\"

	End:
SectionEnd
; AikSaurus
SubSectionEnd
!endif

Section "Grammar Checker"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiGrammar.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Grammar Checker Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Grammar Checker Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiGrammar.dll"
	SetOutPath $INSTDIR\AbiWord\bin
	File /r "..\bin\liblink-grammar-4.dll"
	SetOutPath $INSTDIR\AbiWord\bin\en
	File /r "..\bin\en\*"
	SetOutPath $INSTDIR\AbiWord\Plugins
  
	End:
SectionEnd

; OPTIONAL
Section "Web Dictionary Integration"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiURLDict.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Web Dictionary Integration Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Web Dictionary Integration Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiURLDict.dll"

	End:
SectionEnd

; Dictionary, thesaurus, encyclopedia, etc.
SubSectionEnd


SubSection /e "Reference Tools"

; OPTIONAL
Section "Google Search"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiGoogle.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Google Search Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Google Search Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiGoogle.dll"

	End:  
SectionEnd

; OPTIONAL
Section "Wikipedia Lookup"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiWikipedia.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Wikipedia Lookup Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Wikipedia Lookup Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiWikipedia.dll"

	End:  
SectionEnd

;Reference Tools
SubSectionEnd


SubSection /e "Translation Plugins"

; OPTIONAL
Section "Babelfish Translation"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiBabelfish.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Babelfish Translation Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Babelfish Translation Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiBabelfish.dll"

	End:  
SectionEnd

; OPTIONAL
Section "FreeTranslation.com Translation"
	SectionIn 1

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiFreeTranslation.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing FreeTranslation.com Translation Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping FreeTranslation.com Translation Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiFreeTranslation.dll"

	End:
SectionEnd

SubSectionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Disabled Plugins
!ifdef 0

SubSection /e "Image Manipulation"

; OPTIONAL
;Section "AbiGimp Plugin"
;	SectionIn 2
;
;	; Testing clause to Overwrite Existing Version - if exists
;	IfFileExists "$INSTDIR\AbiWord\plugins\AbiGimp.dll" 0 DoInstall
;	
;	MessageBox MB_YESNO "Overwrite Existing AbiGimp Plugin?" IDYES DoInstall
;	
;	DetailPrint "Skipping AbiGimp Plugin (already exists)!"
;	Goto End
;
;	DoInstall:
;	File "AbiGimp.dll"
;
;	End:  
;SectionEnd
;
;SectionDivider

; OPTIONAL

;conflicts with AbiMathView
Section "AbiPaint Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiPaint.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPaint Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPaint Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiPaint.dll"
  
	End:
SectionEnd


;SectionDivider
SubSectionEnd
!endif



Section "Command Output Scripting"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiScriptHappy.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Command Output Scripting Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Command Output Scripting Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiScriptHappy.dll"
  
	End:
SectionEnd



; uncomment [here and in uninstall] & change .ext if this plugin adds support for new type (with new extension)
; OPTIONAL Registry Settings
;Section "Update Registry (Add File Associations)"
;	SectionIn 1 2
;	; Write File Associations
;	WriteRegStr HKCR ".ext" "" "AbiSuite.AbiWord"
;	WriteRegStr HKCR ".ext" "Content Type" "application/abiword"
;
;SectionEnd


; REQUIRED Create Uninstaller for Plugin
Section "-CreateUninstaller"
	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" "DisplayName" "AbiWord Tools Plugins"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" "UninstallString" '"$INSTDIR\AbiWord\plugins\UninstallAbiWordToolsPlugins.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" "NoRepair" "1"
	; New Uninstaller 
	WriteUninstaller "AbiWord\plugins\UninstallAbiWordToolsPlugins.exe"
SectionEnd


; uninstall stuff
UninstallText "This will uninstall AbiWord's Tools Plugins. Hit next to continue."

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "Are you sure you want to fully remove all AbiWord Tools Plugins?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins"
	;;DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove file assoications
	;DeleteRegKey HKCR ".ext"

!ifdef 0
	; AikSaurus
	Delete "$INSTDIR\AikSaurusABI.dll"
	Delete "$INSTDIR\meanings.dat"
	Delete "$INSTDIR\words.dat"
	; TODO: this could screw up AikSaurus if installed as a program and not just plugin
	;DeleteRegKey HKLM SOFTWARE\Aiksaurus
!endif

	; AbiBabelfish
	Delete "$INSTDIR\AbiBabelfish.dll"
	
	; AbiCollab
	Delete "$INSTDIR\AbiCollab.dll"
	Delete "$INSTDIR\..\bin\libsoup-2.4-1.dll"
	Delete "$INSTDIR\..\bin\libgthread-2.0-0.dll"
	DeleteRegKey HKCR ".abicollab"

	; AbiFreeTranslation
	Delete "$INSTDIR\AbiFreeTranslation.dll"

	; AbiURLDict
	Delete "$INSTDIR\AbiURLDict.dll"

	; AbiWikipedia
	Delete "$INSTDIR\AbiWikipedia.dll"

	; AbiGimp
;	Delete "$INSTDIR\AbiGimp.dll"

	; AbiGoogle
	Delete "$INSTDIR\AbiGoogle.dll"

	; AbiPaint
	Delete "$INSTDIR\AbiPaint.dll"

	; AbiScriptHappy
	Delete "$INSTDIR\AbiScriptHappy.dll"

	; AbiMathView
	Delete "$INSTDIR\AbiMathView.dll"
	Delete "$INSTDIR\..\bin\libmathview_frontend_libxml2-0.dll"
	Delete "$INSTDIR\..\bin\libmathview-0.dll"
	Delete "$INSTDIR\..\..\math\dictionary.xml"
	Delete "$INSTDIR\..\..\math\dictionary-local.xml"
	Delete "$INSTDIR\..\..\math\gtkmathview.conf.xml"
	RMDir "$INSTDIR\..\..\math\"

	; AbiGrammar
	Delete "$INSTDIR\AbiGrammar.dll"
	Delete "$INSTDIR\..\bin\liblink-grammar-4.dll"
	RMDir /r "$INSTDIR\..\bin\en\"

	; remove uninstaller
	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordToolsPlugins.exe"

SectionEnd

; eof
