; This is a file for creating an installer for Abiword using NSIS 
; by Alan Horkan <horkana@tcd.ie>
; modified by Michael D. Pritchett <mpritchett@attglobal.net>

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; The name of the installer
Name "AbiWord"
Icon "..\..\pkg\win\setup\setup.ico"
OutFile "setup_abiword.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install Abiword on your computer. Select which optional components you want installed."

; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

EnabledBitmap  ..\..\pkg\win\setup\checkbox.bmp
DisabledBitmap ..\..\pkg\win\setup\emptybox.bmp

; The stuff that must be installed
; binary, license, and American dictionary
Section "Abiword.exe (required)"

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\bin
	File "AbiWord.exe"

	SetOutPath $INSTDIR\AbiWord
	File /r "..\AbiSuite\AbiWord\help"
	File /r "..\AbiSuite\AbiWord\sample"
	File /r "..\AbiSuite\AbiWord\strings"

	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
	File /oname=copying.txt "..\AbiSuite\Copying"
	File "..\AbiSuite\readme.txt"
  
	; Write the installation path into the registry
	WriteRegStr HKLM SOFTWARE\Abisuite "Install_Dir" "$INSTDIR"

	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Abiword" "DisplayName" "Abiword (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Abiword" "UninstallString" '"$INSTDIR\UninstallAbiWord.exe"'
SectionEnd

; OPTIONAL Start Menu Shortcut
Section "Start Menu Shortcuts"
	CreateDirectory "$SMPROGRAMS\Abiword"
	CreateShortCut "$SMPROGRAMS\Abiword\Uninstall.lnk" "$INSTDIR\UninstallAbiWord.exe" "" "$INSTDIR\UninstallAbiWord.exe" 0
	CreateShortCut "$SMPROGRAMS\Abiword\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd

; OPTIONAL Desktop Shortcut 
Section "Desktop Shortcut"
	CreateShortCut "$DESKTOP\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd

; OPTIONAL Installation of Clipart
Section "Clipart"
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\clipart"
SectionEnd

; MORE OPTIONS
; language packs, clipart, help docs, templates etc.   
;Section "Help Files"
;SectionEnd

; uninstall stuff
UninstallText "This will uninstall Abiword. Hit next to continue."
UninstallExeName "UninstallAbiWord.exe"

; special uninstall section.
Section "Uninstall"

	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Abiword"
	DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove start menu shortcuts.
	Delete "$SMPROGRAMS\Abiword\*.*"

	; remove desktop shortcut.
	Delete "$DESKTOP\Abiword.lnk"

	; remove directories used
	RMDir "$SMPROGRAMS\Abiword"
	RMDir /r "$INSTDIR"

SectionEnd

; eof
