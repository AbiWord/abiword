; ScriptHappy plugin
; This is a file for creating an installer for Abiword Plugins using NSIS 
; Based on .nsi file created by Alan Horkan <horkana@tcd.ie>
; and modified by Michael D. Pritchett <mpritchett@attglobal.net>
; modified by Kenneth J Davis <jeremyd@computer.org>

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; The name of the installer
Name "AbiWord Plugin - AbiScriptHappy"
Icon "..\..\pkg\win\setup\setup.ico"
;Icon "..\..\pkg\win\setup\setup_tm.ico"
OutFile "AbiWord_Plugin_AbiScriptHappy.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install Abiword's AbiScriptHappy Plugin on your computer."

; Different installation types (usual or with a plugin specific uninstaller)
InstType "Typical (default)"
InstType "Full"

; The text to prompt the user to enter a directory
DirText "Choose the AbiSuite directory where you previously installed Abiword:"

EnabledBitmap  ..\..\pkg\win\setup\checkbox.bmp
DisabledBitmap ..\..\pkg\win\setup\emptybox.bmp

; The stuff that must be installed
; binary, license, or whatever
Section "Abiword Plugin - AbiScriptHappy (required)"
	SectionIn 1 2

	;;;;
	; Testing clause to abort if required AbiWord.exe DLL does not exist
	IfFileExists "$INSTDIR\AbiWord\bin\AbiWord.exe" DoOverwriteTest 0

	MessageBox MB_ICONSTOP "Quitting the install process - AbiWord.exe not found"
	Quit

	DoOverwriteTest:
	;;;;
	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiScriptHappy.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiScriptHappy Plugin?" IDYES DoInstall
	
	Abort "Quitting the install process"

	DoInstall:
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\plugins
	File "libAbiScriptHappy.dll"
  
	; Write the installation path into the registry
	;;WriteRegStr HKLM SOFTWARE\Abisuite "Install_Dir" "$INSTDIR"

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


; OPTIONAL Create Uninstaller for Plugin
Section "Create Uninstaller for AbiScriptHappy Plugin"
	SectionIn 2
	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordPluginAbiScriptHappy" "DisplayName" "Abiword's AbiScriptHappy Plugin (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordPluginAbiScriptHappy" "UninstallString" '"$INSTDIR\AbiWord\plugins\UninstallAbiWordPluginAbiScriptHappy.exe"'

	; New Uninstaller 
	WriteUninstaller "AbiWord\plugins\UninstallAbiWordPluginAbiScriptHappy.exe"

SectionEnd


; uninstall stuff
UninstallText "This will uninstall Abiword's AbiScriptHappy Plugin. Hit next to continue."
;;UninstallExeName "UninstallAbiWordPluginAbiScriptHappy.exe"

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "This will delete $INSTDIR\libAbiScriptHappy.dll and associated registry entries?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordPluginAbiScriptHappy"
	;;DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove file assoications
	;DeleteRegKey HKCR ".ext"

	; remove files used
	Delete "$INSTDIR\libAbiScriptHappy.dll"
	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordPluginAbiScriptHappy.exe"

SectionEnd

; eof
