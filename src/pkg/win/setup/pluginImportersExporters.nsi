; I/E importer and exporter plugins
; This is a file for creating an installer for Abiword Plugins using NSIS 
; Based on .nsi file created by Alan Horkan <horkana@tcd.ie>
; and modified by Michael D. Pritchett <mpritchett@attglobal.net>
; modified by Kenneth J Davis <jeremyd@computer.org>

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; The name of the installer
Name "AbiWord's Importer/Exporter Plugins"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

OutFile "AbiWord_IE_Plugins.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install AbiWord's Importer/Exporter Plugins on your computer."

; Different installation types (usual or with a plugin specific uninstaller)
InstType "Typical (default)"
InstType "Full"

; The text to prompt the user to enter a directory
DirText "Choose the AbiSuite directory where you previously installed Abiword:"

; For NSIS 2.xx
CheckBitmap ..\..\pkg\win\setup\modern.bmp
; For NSIS 1.xx
;EnabledBitmap  ..\..\pkg\win\setup\checkbox.bmp
;DisabledBitmap ..\..\pkg\win\setup\emptybox.bmp

; The stuff that must be installed
; binary, license, or whatever
;Section "Importer/Exporter Plugins (required)"
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

;SectionDivider
; OPTIONAL
SubSection /e "Image Manipulation"

Section "ImageMagick Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiMagick.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing ImageMagick Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping ImageMagick Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiMagick.dll"
  
	End:
SectionEnd

;SectionDivider
SubSectionEnd
SubSection /e "Additional File Format Importers/Exporters"

SubSection "BZ2ABW (*.bzabw) Plugin"
Section "The .bzabw Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libBZ2Abw.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing BZ2ABW Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping BZ2ABW Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libBZ2Abw.dll"

	End:
SectionEnd


; OPTIONAL Registry Settings
Section "Add .bzabw Association (to Registry)"
	SectionIn 1 2

	; Skip adding registry entries if plugin doesn't exist
	IfFileExists "$INSTDIR\AbiWord\plugins\libBZ2Abw.dll" 0 End

	; Write File Associations
	WriteRegStr HKCR ".bzabw" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".bzabw" "Content Type" "application/abiword-compressed"

	End:
SectionEnd
SubSectionEnd

;SectionDivider

Section "AbiEML Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiEML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiEML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiEML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiEML.dll"
  
	End:
SectionEnd

;SectionDivider

Section "OpenWriter (*.sxw) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libOpenWriter.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing OpenWriter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping OpenWriter Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libOpenWriter.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiSDW (*.sdw) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiSDW.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiSDW Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiSDW Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiSDW.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiHTML (*.html) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiHTML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiHTML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiHTML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiHTML.dll"
  
	End:
SectionEnd

;SectionDivider

; uncomment [here and in uninstall] & change .ext if this plugin adds support for new type (with new extension)
; OPTIONAL Registry Settings
;Section "Update Registry (Add File Associations)"
;	SectionIn 1 2
;	; Write File Associations
;	WriteRegStr HKCR ".ext" "" "AbiSuite.AbiWord"
;	WriteRegStr HKCR ".ext" "Content Type" "application/abiword"
;
;SectionEnd

; Additional File Format importer/exporters
SubSectionEnd


; OPTIONAL Create Uninstaller for Plugin
Section "Create Uninstaller for I/E Plugins"
	SectionIn 2
	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" "DisplayName" "AbiWord's Importer/Exporter Plugins (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" "UninstallString" '"$INSTDIR\AbiWord\plugins\UninstallAbiWordIEPlugins.exe"'

	; New Uninstaller 
	WriteUninstaller "AbiWord\plugins\UninstallAbiWordIEPlugins.exe"

SectionEnd


; uninstall stuff
UninstallText "This will uninstall AbiWord's Importer/Exporter Plugins. Hit next to continue."
;;UninstallExeName "UninstallAbiWordIEPlugins.exe"

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "This will delete all Importer/Exporter plugins and associated registry entries?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins"
	;;DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove file assoications
	;DeleteRegKey HKCR ".ext"
	DeleteRegKey HKCR ".bzabw"

	; remove files used
	Delete "$INSTDIR\libBZ2Abw.dll"
	Delete "$INSTDIR\libAbiEML.dll"
	Delete "$INSTDIR\libOpenWriter.dll"
	Delete "$INSTDIR\libAbiSDW.dll"
	Delete "$INSTDIR\libAbiHTML.dll"
	Delete "$INSTDIR\libAbiMagick.dll"

	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordIEPlugins.exe"

SectionEnd

; eof
