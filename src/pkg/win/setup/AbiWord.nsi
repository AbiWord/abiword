; This is a file for creating an installer for Abiword using NSIS v1 series
; by Alan Horkan <horkana@tcd.ie>
; modified by Michael D. Pritchett <mpritchett@attglobal.net>
; Kenneth J. Davis <jeremyd@computer.org> (2002)
; [add your name here]

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

; Install types 
InstType "Typical (default)"			; Section 1
InstType "Full (with File Associations)" 	; Section 2
InstType "Minimal"				; Section 3
; any other combination is "Custom"

; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

EnabledBitmap  ..\..\pkg\win\setup\checkbox.bmp
DisabledBitmap ..\..\pkg\win\setup\emptybox.bmp

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

	!ifdef MINGW32
		SetOutPath $INSTDIR\AbiWord\bin
		File "libAbiWord.dll"
	!endif

	; Image plugin for importers & cut-n-paste of 
      ; various standard image formats (BMP, WMF, JPEG) on Windows
	SetOutPath $INSTDIR\AbiWord\plugins
	File "..\plugins\libAbi_IEG_Win32Native.dll"

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
	WriteRegStr HKLM SOFTWARE\Abisuite "Install_Dir" "$INSTDIR"

	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Abiword" "DisplayName" "Abiword (remove only)"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Abiword" "UninstallString" '"$INSTDIR\UninstallAbiWord.exe"'

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\AbiWord.exe" "" '"$INSTDIR\AbiWord\bin\AbiWord.exe"'

	; New Uninstaller 
	WriteUninstaller "UninstallAbiWord.exe"

SectionEnd

; OPTIONAL Registry Settings
Section "Update Registry Settings"
	SectionIn 1 2 3
	; Write the AbiSuite.AbiWord Keys
	WriteRegStr HKCR "AbiSuite.AbiWord" "" "AbiWord Document"
	WriteRegStr HKCR "AbiSuite.AbiWord\DefaultIcon" "" "$INSTDIR\AbiWord\bin\AbiWord.exe,2"
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

; OPTIONAL Start Menu Shortcut for the current user profile
Section "Start Menu Shortcuts (Current User)"
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	CreateDirectory "$SMPROGRAMS\Abiword"
	CreateShortCut "$SMPROGRAMS\Abiword\Uninstall Abiword.lnk" "$INSTDIR\UninstallAbiWord.exe" "" "$INSTDIR\UninstallAbiWord.exe" 0
	CreateShortCut "$SMPROGRAMS\Abiword\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd

; OPTIONAL Desktop Shortcut for the current user profile
Section "Desktop Shortcut (Current User)"
	SectionIn 1 2 3
	SetShellVarContext current  	; This is probably overkill, but playing it safe
	CreateShortCut "$DESKTOP\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
SectionEnd


; OPTIONAL Start Menu Shortcut for the special All User profile (not used in win9x) 
Section "Start Menu Shortcuts (All Users)"
	SectionIn 2		; off by default, included in 2 Full Install
        SetShellVarContext all  	; set to all, reset at end of section
	CreateDirectory "$SMPROGRAMS\Abiword"
	CreateShortCut "$SMPROGRAMS\Abiword\Uninstall Abiword.lnk" "$INSTDIR\UninstallAbiWord.exe" "" "$INSTDIR\UninstallAbiWord.exe" 0
	CreateShortCut "$SMPROGRAMS\Abiword\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
	SetShellVarContext current  	; This is pro'ly overkill
SectionEnd


; OPTIONAL Desktop Shortcut for All Users
Section "Desktop Shortcut (All Users)"
	SectionIn 2	; not in default, included in 2 Full Install
        SetShellVarContext all  	;  All users 
	CreateShortCut "$DESKTOP\Abiword.lnk" "$INSTDIR\Abiword\bin\Abiword.exe" "" "$INSTDIR\Abiword\bin\Abiword.exe" 0
        SetShellVarContext current  	; reset to current user
SectionEnd


; MORE OPTIONS
; language packs, clipart, help docs, templates etc.   
;Section "Help Files"
;SectionEnd

SectionDivider " helper files "

; OPTIONAL Installation of Default Dictionary
Section "Dictionary - US-English"
	SectionIn 1 2
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
SectionEnd

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


SectionDivider " general file associations "

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

; uninstall stuff
UninstallText "This will uninstall Abiword. Hit next to continue."
;UninstallExeName "UninstallAbiWord.exe"

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

	ReadRegStr $0 HKCR ".doc" ""
	StrCmp $0 "AbiSuite.AbiWord" Del_Word_Assoc Skip_Del_Word
	Del_Word_Assoc:
	DeleteRegKey HKCR ".doc"
	Skip_Del_Word:
	
	ReadRegStr $0 HKCR ".rtf" ""
	StrCmp $0 "AbiSuite.AbiWord" Del_RTF_Assoc Skip_Del_RTF
	Del_RTF_Assoc:
	DeleteRegKey HKCR ".rtf"
	Skip_Del_RTF:
	
	; remove start menu shortcuts.
	Delete "$SMPROGRAMS\Abiword\*.*"

	; remove desktop shortcut.
	Delete "$DESKTOP\Abiword.lnk"

	; remove directories used
	RMDir "$SMPROGRAMS\Abiword"
	RMDir /r "$INSTDIR"
	 
SectionEnd

; eof
