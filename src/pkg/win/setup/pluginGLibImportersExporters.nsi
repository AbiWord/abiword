; Glib/GSF based I/E importer and exporter plugins
; This is a file for creating an installer for Abiword Plugins using NSIS 
; Based on .nsi file created by Alan Horkan <horkana@tcd.ie>
; and modified by Michael D. Pritchett <mpritchett@attglobal.net>
; modified by Kenneth J Davis <jeremyd@computer.org>

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; The name of the installer
Name "AbiWord's Glib/GSF Importer/Exporter Plugins"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

OutFile "AbiWord_GIE_Plugins.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install AbiWord's Glib/GSF Importer/Exporter Plugins on your computer."

; Different installation types (usual or with a plugin specific uninstaller)
InstType "Typical (default)"
InstType "Full"

; The text to prompt the user to enter a directory
DirText "Choose the AbiSuite directory where you previously installed Abiword:"

; For NSIS 2.xx
CheckBitmap ..\..\pkg\win\setup\modern.bmp

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

; Not required if Glib is already available, otherwise is required
Section "Glib 2.2"
	SectionIn 1 2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\bin

	; Glib 2.2
	File "libglib-2.0-0.dll"
	File "libgobject-2.0-0.dll"
	File "iconv.dll"
	File "intl.dll"
	
	; libGSF
	;File "libgsf-1.8.dll"

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\plugins
SectionEnd

;SectionDivider
!ifdef 0
; OPTIONAL
SubSection /e "Image Manipulation"

Section "AbiRSVG Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbi_IEG_RSVG.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiRSVG Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiRSVG Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbi_IEG_RSVG.dll"
  
	End:
SectionEnd

;SectionDivider
SubSectionEnd
!endif

SubSection /e "Additional File Format Importers/Exporters"

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

!ifdef 0
Section "AbiWordPerfect (*.wpd) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiWordPerfect.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWordPerfect Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWordPerfect Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiWordPerfect.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiXHTML (*.xhtml) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiXHTML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiXHTML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiXHTML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiXHTML.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

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
Section "Create Uninstaller for Glib/GSF I/E Plugins"
	SectionIn 2
	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordGIEPlugins" "DisplayName" "AbiWord's Glib/GSF Importer/Exporter Plugins (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordGIEPlugins" "UninstallString" '"$INSTDIR\AbiWord\plugins\UninstallAbiWordGIEPlugins.exe"'

	; New Uninstaller 
	WriteUninstaller "AbiWord\plugins\UninstallAbiWordGIEPlugins.exe"

SectionEnd


; uninstall stuff
UninstallText "This will uninstall AbiWord's Glib/GSF Importer/Exporter Plugins. Hit next to continue."
;;UninstallExeName "UninstallAbiWordIEPlugins.exe"

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "This will delete all Glib/GSF Importer/Exporter plugins and associated registry entries?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordGIEPlugins"
	;;DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove file assoications
	;DeleteRegKey HKCR ".ext"

	; remove files used
	Delete "$INSTDIR\libOpenWriter.dll"
	Delete "$INSTDIR\libAbiSDW.dll"
	Delete "$INSTDIR\libAbiWordPerfect.dll"
	Delete "$INSTDIR\libAbiXHTML.dll"

	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordGIEPlugins.exe"

SectionEnd

; eof
