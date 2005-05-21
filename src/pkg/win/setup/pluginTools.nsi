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

; The name of the installer
Name "AbiWord's Tools Plugins"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

OutFile "abiword-plugins-tools-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
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

SubSection /e "Dictionary, Thesaurus, etc."

!ifdef 0
SubSection "AikSaurus (thesaurus) Plugins"
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

; OPTIONAL
Section "AbiURLDict Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiURLDict.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiURLDict Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiURLDict Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiURLDict.dll"

	End:
SectionEnd

; OPTIONAL
Section "AbiWikipedia Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiWikipedia.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWikipedia Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWikipedia Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiWikipedia.dll"

	End:  
SectionEnd

; OPTIONAL
Section "AbiGoogle Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiGoogle.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiGoogle Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiGoogle Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiGoogle.dll"

	End:  
SectionEnd

; Dictionary, thesaurus, encyclopedia, etc.
SubSectionEnd

SubSection /e "Translation Plugins"

; OPTIONAL
Section "AbiBabelfish Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiBabelfish.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiBabelfish Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiBabelfish Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiBabelfish.dll"

	End:  
SectionEnd

;SectionDivider

; OPTIONAL
Section "AbiFreeTranslation Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiFreeTranslation.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiFreeTranslation Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiFreeTranslation Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiFreeTranslation.dll"

	End:
SectionEnd

SubSectionEnd

;SectionDivider
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

SubSection /e "Script Related Plugins"

Section "AbiScriptHappy Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiScriptHappy.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiScriptHappy Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiScriptHappy Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiScriptHappy.dll"
  
	End:
SectionEnd

SubSectionEnd

; uncomment [here and in uninstall] & change .ext if this plugin adds support for new type (with new extension)
; OPTIONAL Registry Settings
;Section "Update Registry (Add File Associations)"
;	SectionIn 1 2
;	; Write File Associations
;	WriteRegStr HKCR ".ext" "" "AbiSuite.AbiWord"
;	WriteRegStr HKCR ".ext" "Content Type" "application/abiword"
;
;SectionEnd


; OPTIONAL Create Uninstaller for Plugin
Section "Create Uninstaller for Tools Plugins"
	SectionIn 1 2
	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" "DisplayName" "AbiWord's Tools Plugins (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" "UninstallString" '"$INSTDIR\AbiWord\plugins\UninstallAbiWordToolsPlugins.exe"'

	; New Uninstaller 
	WriteUninstaller "AbiWord\plugins\UninstallAbiWordToolsPlugins.exe"

SectionEnd


; uninstall stuff
UninstallText "This will uninstall AbiWord's Tools Plugins. Hit next to continue."
;;UninstallExeName "UninstallAbiWordToolsPlugins.exe"

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "This will delete all Tools plugins and associated files & registry entries?" IDOK DoUnInstall
	
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

	; remove uninstaller
	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordToolsPlugins.exe"

SectionEnd

; eof
