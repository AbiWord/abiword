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

	;;;;
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
	File /r "..\AbiSuite\AbiWord\help"
	File /r "..\AbiSuite\AbiWord\sample"
	File /r "..\AbiSuite\AbiWord\strings"

	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
	File /r "..\AbiSuite\templates"
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

; OPTIONAL Registry Settings
Section "Update Registry Settings"

	; Write the AbiSuite.AbiWord Keys
	WriteRegStr HKCR "AbiSuite.AbiWord" "" "AbiWord Document"
	WriteRegStr HKCR "AbiSuite.AbiWord\DefaultIcon" "" "$INSTDIR\AbiWord\bin\AbiWord.exe,3"
	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\command" "" "$INSTDIR\AbiWord\bin\AbiWord.exe"
	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\ddeexec" "" '[Open("%1")]'
	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\ddeexec\application" "" "AbiWord"
	WriteRegStr HKCR "AbiSuite.AbiWord\shell\open\ddeexec\topic" "" "System"

	; Write File Associations
	WriteRegStr HKCR ".abw" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".abw" "Content Type" "application/abiword"

	WriteRegStr HKCR ".awt" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".awt" "Content Type" "application/abiword-template"

	WriteRegStr HKCR ".zabw" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".zabw" "Content Type" "application/abiword-compressed"

SectionEnd

; OPTIONAL Start Menu Shortcut
Section "Start Menu Shortcuts"
	CreateDirectory "$SMPROGRAMS\Abiword"
	CreateShortCut "$SMPROGRAMS\Abiword\Uninstall Abiword.lnk" "$INSTDIR\UninstallAbiWord.exe" "" "$INSTDIR\UninstallAbiWord.exe" 0
	CreateShortCut "$SMPROGRAMS\Abiword\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd

; OPTIONAL Desktop Shortcut 
Section "Desktop Shortcut"
	CreateShortCut "$DESKTOP\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd

; OPTIONAL 
Section "Associate .doc & .rtf with AbiWord"

	; Write File Associations
	WriteRegStr HKCR ".doc" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".doc" "Content Type" "application/abiword"

	WriteRegStr HKCR ".rtf" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".rtf" "Content Type" "application/abiword"

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

	MessageBox MB_OKCANCEL "This will delete $INSTDIR and all subdirectories and files?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Abiword"
	DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove file assoications
	DeleteRegKey HKCR "AbiSuite.AbiWord"
	DeleteRegKey HKCR ".abw"
	DeleteRegKey HKCR ".awt"
	DeleteRegKey HKCR ".zabw"

	ReadRegStr $0 HKCR ".doc" "(Default)"
	StrCmp $0 "AbiSuite.AbiWord" Del_Word_Assoc Skip_Del_Word
	Del_Word_Assoc:
	DeleteRegKey HKCR ".doc"
	DeleteRegKey HKCR ".rtf"
	Skip_Del_Word:
	
	; remove start menu shortcuts.
	Delete "$SMPROGRAMS\Abiword\*.*"

	; remove desktop shortcut.
	Delete "$DESKTOP\Abiword.lnk"

	; remove directories used
	RMDir "$SMPROGRAMS\Abiword"
	RMDir /r "$INSTDIR"

SectionEnd

; eof
