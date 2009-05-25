; I/E importer and exporter plugins
; This is a file for creating an installer for Abiword Plugins using NSIS 
; Based on .nsi file created by Alan Horkan <horkana@tcd.ie>
; and modified by Michael D. Pritchett <mpritchett@attglobal.net>
; modified by Kenneth J Davis <jeremyd@computer.org>

!include Sections.nsH

; Uncomment the following define to include plugins that
; either lack functionality, are unstable, or otherwise
; not for general use
;!define EXPERIMENTALPLUGINS

; Note: plugins that are not yet setup to build are
; wrapped with !ifdef 0 !endif

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
Name "AbiWord Importer/Exporter Plugins"
BrandingText "AbiWord 2.6 Series"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

OutFile "abiword-plugins-impexp-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)." "$(^NextBtn)"
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;; Importers/Exporters

SubSection /e "Importers/Exporters"

Section "ApplixWare"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiApplix.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing ApplixWare Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping ApplixWare Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiApplix.dll"
  
	End:
SectionEnd

Section "ClarisWorks"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiClarisWorks.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing ClarisWorks Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping ClarisWorks Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiClarisWorks.dll"

	End:
SectionEnd

Section "DocBook"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiDocBook.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing DocBook Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping DocBook Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiDocBook.dll"

	End:
SectionEnd

Section "Microsoft Office Open XML"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiOpenXML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Microsoft Office Open XML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Microsoft Office Open XML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiOpenXML.dll"

	End:
SectionEnd

Section "OpenDocument (.odt)"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiOpenDocument.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing OpenDocument Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping OpenDocument Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiOpenDocument.dll"

	End:
SectionEnd

Section "OpenWriter (*.sxw)"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiOpenWriter.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing OpenWriter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping OpenWriter Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiOpenWriter.dll"

	End:
SectionEnd

Section "ISCII (Indic script) Text"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiISCII_Text.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing ISCII (Indic script) Text Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping ISCII (Indic script) Text Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiISCII_Text.dll"

	End:
SectionEnd

Section "Saved email (.eml) format"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiEML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Saved email (.eml) Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Saved email (.eml) Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiEML.dll"

	End:
SectionEnd

Section "Palm .pdb DOC"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiPalmDoc.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Palm .pdb DOC Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Palm .pdb DOC Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiPalmDoc.dll"
  
	End:
SectionEnd

;Wireless Markup Language (old HTML replacement for mobile devices)
Section "WML Wireless Markup"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiWML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing WML Wireless Markup Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping WML Wireless Markup Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiWML.dll"
  
	End:
SectionEnd


;XML/XSL Formatting objects, meant to be similar in scope to LaTeX
Section "XSL-FO"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiXSLFO.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing XSL-FO Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping XSL-FO Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiXSLFO.dll"

	End:
SectionEnd

SubSectionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;; Import Only

SubSection /e "Import-Only Support"
Section "MS Write"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiMSWrite.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing MS Write Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping MS Write Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiMSWrite.dll"

	End:
SectionEnd


Section "OPML"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiOPML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing OPML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping OPML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiOPML.dll"

	End:
SectionEnd


Section "Star Office Writer 5.1"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiSDW.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Star Office Writer 5.1 Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Star Office Writer 5.1 Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiSDW.dll"

	End:
SectionEnd


Section "T602"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiT602.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing T602 Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping T602 Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiT602.dll"

	End:
SectionEnd


Section "WordPerfect"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiWordPerfect.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing WordPerfect Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping WordPerfect Plugin (already exists)!"
	Goto End

	DoInstall:
        SetOutPath $INSTDIR\AbiWord\bin
        File "libwpd-0.8.dll"

        SetOutPath $INSTDIR\AbiWord\plugins
	  File "AbiWordPerfect.dll"

	End:
SectionEnd

SubSectionEnd


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;; Export Only

SubSection /e "Export-Only Support"

Section "Newsgroup Markup (Hrtext)"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiHrtext.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing Newsgroup Markup (Hrtext) Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping Newsgroup Markup (Hrtext) Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiHrtext.dll"

	End:
SectionEnd

Section "LaTeX"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiLaTeX.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing LaTeX Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping LaTeX Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiLaTeX.dll"

	End:
SectionEnd

SubSectionEnd

!if 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Disabled Plugins
Section "Psion (*.psiword) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiPsion.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPsion Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPsion Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiPsion.dll"

	End:
SectionEnd

Section "Nroff/Unix Manpages"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiNroff.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiNroff Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiNroff Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiNroff.dll"

	End:
SectionEnd

Section "AbiMIF Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiMIF.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiMIF Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiMIF Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiMIF.dll"

	End:
SectionEnd

Section "AbiRSVG Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\Abi_IEG_RSVG.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiRSVG Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiRSVG Plugin (already exists)!"
	Goto End

	DoInstall:
	File "Abi_IEG_RSVG.dll"

	End:
SectionEnd

Section "AbiPDF (*.pdf) Exporter Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiPDF.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPDF exporter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPDF Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiPDF.dll"

	End:
SectionEnd

Section "AbiXHTML (*.xhtml) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiXHTML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiXHTML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiXHTML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "libAbiXHTML.dll"
  
	End:
SectionEnd

Section "AbiHancom Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiHancom.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiHancom Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiHancom Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiHancom.dll"
  
	End:
SectionEnd

Section "AbiKword Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiKword.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiKword Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiKword Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiKword.dll"
  
	End:
SectionEnd
!ifdef 0

; OPTIONAL
SubSection /e "Image Manipulation"

;SectionDivider

Section "AbiBitmap (*.bmp) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\Abi_IEG_BMP.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiBitmap Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiBitmap Plugin (already exists)!"
	Goto End

	DoInstall:
	File "Abi_IEG_BMP.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiJPEG (*.jpg) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\Abi_IEG_jpeg.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiJPEG Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiJPEG Plugin (already exists)!"
	Goto End

	DoInstall:
	File "Abi_IEG_jpeg.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiSVG Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\Abi_IEG_svg.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiSVG Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiSVG Plugin (already exists)!"
	Goto End

	DoInstall:
	File "Abi_IEG_svg.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiWindowsMetaFile (*.wmf) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\Abi_IEG_wmf.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWindowsMetaFile Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWindowsMetaFile Plugin (already exists)!"
	Goto End

	DoInstall:
	File "Abi_IEG_wmf.dll"
  
	End:
SectionEnd

;SectionDivider
SubSectionEnd
!endif
!endif
;;;;;;;;;;;;;;;;
; end disabled plugins


; uncomment [here and in uninstall] & change .ext if this plugin adds support for new type (with new extension)
; OPTIONAL Registry Settings
;Section "Update Registry (Add File Associations)"
;	SectionIn 1 2
;	; Write File Associations
;	WriteRegStr HKCR ".ext" "" "AbiSuite.AbiWord"
;	WriteRegStr HKCR ".ext" "Content Type" "application/abiword"
;
;SectionEnd

;SectionDivider


; OPTIONAL Create Uninstaller for Plugin
Section "-CreateUninstaller"
	SectionIn 1 2
	; Write the uninstall keys for Windows
	; N.B. This needs to include a version number or unique identifier.  
	; More than one version of Abiword but only one Control Panel.  
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" "DisplayName" "AbiWord Importer/Exporter Plugins"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" "UninstallString" '"$INSTDIR\AbiWord\plugins\UninstallAbiWordIEPlugins.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" "NoRepair" "1"
	; New Uninstaller 
	WriteUninstaller "AbiWord\plugins\UninstallAbiWordIEPlugins.exe"

SectionEnd


; uninstall stuff
UninstallText "This will uninstall AbiWord's Importer/Exporter Plugins. Hit next to continue."
;;UninstallExeName "UninstallAbiWordIEPlugins.exe"

; special uninstall section.
Section "Uninstall"

	MessageBox MB_OKCANCEL "Are you sure you want to fully remove all AbiWord Import/Export Plugins?" IDOK DoUnInstall
	
	Abort "Quitting the uninstall process"

	DoUnInstall:
	; remove registry keys
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins"
	;;DeleteRegKey HKLM SOFTWARE\Abisuite

	; remove file assoications
	;DeleteRegKey HKCR ".ext"

	; remove files used
	Delete "$INSTDIR\AbiApplix.dll"
	Delete "$INSTDIR\AbiClarisWorks.dll"
	Delete "$INSTDIR\AbiDocBook.dll"
	Delete "$INSTDIR\AbiEML.dll"
	Delete "$INSTDIR\AbiHancom.dll"
	Delete "$INSTDIR\AbiHrtext.dll"
	Delete "$INSTDIR\AbiISCII_Text.dll"
	Delete "$INSTDIR\AbiKword.dll"
	Delete "$INSTDIR\AbiLaTeX.dll"
	Delete "$INSTDIR\AbiMIF.dll"
	Delete "$INSTDIR\AbiMSWrite.dll"
	Delete "$INSTDIR\AbiNroff.dll"
	Delete "$INSTDIR\AbiOpenDocument.dll"
	Delete "$INSTDIR\AbiOpenWriter.dll"
	Delete "$INSTDIR\AbiOPML.dll"
	Delete "$INSTDIR\AbiPalmDoc.dll"
	Delete "$INSTDIR\AbiPsion.dll"
	Delete "$INSTDIR\AbiSDW.dll"
	Delete "$INSTDIR\AbiT602.dll"
	Delete "$INSTDIR\AbiWML.dll"
	Delete "$INSTDIR\AbiWordPerfect.dll"
	Delete "$INSTDIR\AbiXSLFO.dll"
	Delete "$INSTDIR\AbiOpenXML.dll"

	Delete "$INSTDIR\..\bin\libwpd-0.8.dll"

!ifdef 0
	Delete "$INSTDIR\Abi_IEG_BMP.dll"
	Delete "$INSTDIR\Abi_IEG_jpeg.dll"
	Delete "$INSTDIR\Abi_IEG_svg.dll"
	Delete "$INSTDIR\Abi_IEG_wmf.dll"
	Delete "$INSTDIR\AbiPDF.dll"
	Delete "$INSTDIR\AbiPW.dll"
	Delete "$INSTDIR\AbiXHTML.dll"
!endif

	Delete /REBOOTOK "$INSTDIR\UninstallAbiWordIEPlugins.exe"

SectionEnd

; eof
