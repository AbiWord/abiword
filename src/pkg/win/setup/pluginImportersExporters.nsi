; I/E importer and exporter plugins
; This is a file for creating an installer for Abiword Plugins using NSIS 
; Based on .nsi file created by Alan Horkan <horkana@tcd.ie>
; and modified by Michael D. Pritchett <mpritchett@attglobal.net>
; modified by Kenneth J Davis <jeremyd@computer.org>


; Uncomment the following define to include plugins that
; either lack functionality, are unstable, or otherwise
; not for general use
;!define EXPERIMENTALPLUGINS

; Note: plugins that are not yet setup to build are
; wrapped with !ifdef 0 !endif


!ifndef VERSION_MAJOR
!define VERSION_MAJOR "2"
!endif

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
InstallDir $PROGRAMFILES\AbiSuite${VERSION_MAJOR}

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite\AbiWord\v${VERSION_MAJOR} "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install AbiWord's Importer/Exporter Plugins on your computer."

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
; OPTIONAL
SubSection /e "Image Manipulation"

!ifdef 0
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
!endif

Section "AbiBitmap (*.bmp) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbi_IEG_BMP.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiBitmap Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiBitmap Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbi_IEG_BMP.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiJPEG (*.jpg) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbi_IEG_jpeg.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiJPEG Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiJPEG Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbi_IEG_jpeg.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef 0
Section "AbiSVG Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbi_IEG_svg.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiSVG Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiSVG Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbi_IEG_svg.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiWindowsMetaFile (*.wmf) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbi_IEG_wmf.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWindowsMetaFile Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWindowsMetaFile Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbi_IEG_wmf.dll"
  
	End:
SectionEnd

;SectionDivider
!endif
SubSectionEnd

SubSection /e "File Format Importers/Exporters"

Section "AbiApplix Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiApplix.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiApplix Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiApplix Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiApplix.dll"
  
	End:
SectionEnd

;SectionDivider

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

Section "AbiClarisWorks Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiClarisWorks.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiClarisWorks Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiClarisWorks Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiClarisWorks.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiCoquille Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiCoquille.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiCoquille Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiCoquille Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiCoquille.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiDocbook Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiDocBook.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiDocbook Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiDocbook Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiDocBook.dll"
  
	End:
SectionEnd

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

!ifdef 0
Section "AbiHancom Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiHancom.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiHancom Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiHancom Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiHancom.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiHrtext Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiHrtext.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiHrtext Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiHrtext Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiHrtext.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiIscii-text Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiIsciitext.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiIscii-text Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiIscii-text Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiIsciitext.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiKword Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiKword.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiKword Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiKword Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiKword.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

Section "AbiLaTeX Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiLaTeX.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiLaTeX Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiLaTeX Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiLaTeX.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef EXPERIMENTALPLUGINS
Section "AbiMIF Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiMIF.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiMIF Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiMIF Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiMIF.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

Section "MSWrite (*.wri) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiMSWrite.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiMSWrite Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiMSWrite Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiMSWrite.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef EXPERIMENTALPLUGINS
Section "AbiNroff Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiNroff.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiNroff Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiNroff Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiNroff.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

Section "AbiPalmDoc (*.pdb) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiPalmDoc.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPalmDoc Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPalmDoc Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiPalmDoc.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef 0
Section "AbiPDF (*.pdf) Exporter Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiPDF.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPDF exporter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPDF Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiPDF.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

Section "AbiPsion (*.psiword) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbi_psion.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPsion Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPsion Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbi_psion.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef 0
;Saig PatheticWriter 
Section "AbiPatheticWriter Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiPW.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPatheticWriter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPatheticWriter Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiPW.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

Section "AbiT602 importer Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiT602.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiT602 Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiT602 Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiT602.dll"
  
	End:
SectionEnd

;SectionDivider

;Wireless Markup Language (old HTML replacement for mobile devices)
Section "AbiWML (*.wml) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiWML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiWML.dll"
  
	End:
SectionEnd

;SectionDivider

;XML/XSL Formatting objects, meant to be similar in scope to LaTeX
Section "AbiXSL-FO Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\libAbiXSLFO.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiXSL-FO Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiXSL-FO Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiXSLFO.dll"
  
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
	Delete "$INSTDIR\libAbiMagick.dll"

	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordIEPlugins.exe"

SectionEnd

; eof
