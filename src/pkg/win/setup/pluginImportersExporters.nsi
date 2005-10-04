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
Name "AbiWord's Importer/Exporter Plugins"

; Personal build
Icon "..\..\pkg\win\setup\setup.ico"
UninstallIcon "..\..\pkg\win\setup\setup.ico"
; Trademarked build
;Icon "..\..\pkg\win\setup\setup_tm.ico"
;UninstallIcon "..\..\pkg\win\setup\setup_tm.ico"

OutFile "abiword-plugins-impexp-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.exe"

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

; Remember if we already enabled Glib or not
var GLIB_ENABLED 

Function .onInit
	strcpy $GLIB_ENABLED "no"
FunctionEnd

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
!ifdef 0

; OPTIONAL
SubSection /e "Image Manipulation"

Section "ImageMagick Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiMagick.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing ImageMagick Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping ImageMagick Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiMagick.dll"
  
	End:
SectionEnd

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

SubSection /e "File Format Importers/Exporters"

Section "AbiApplix Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiApplix.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiApplix Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiApplix Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiApplix.dll"
  
	End:
SectionEnd

;SectionDivider

SubSection "AbiBZ2ABW (*.bzabw) Plugin"
Section "The .bzabw Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiBZ2Abw.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing BZ2ABW Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiBZ2ABW Plugin (already exists)!"
	Goto End

	DoInstall:
	;SetOutPath $INSTDIR\AbiWord\bin
	;File "libbz2.dll" 
	SetOutPath $INSTDIR\AbiWord\plugins
	File "AbiBZ2Abw.dll"

	End:
SectionEnd

; OPTIONAL Registry Settings
Section "Add .bzabw Association (to Registry)"
	SectionIn 1 2

	; Skip adding registry entries if plugin doesn't exist
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiBZ2Abw.dll" 0 End

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
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiClarisWorks.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiClarisWorks Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiClarisWorks Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiClarisWorks.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiDocbook Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiDocBook.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiDocbook/Coquille Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiDocbook/Coquille Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiDocBook.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiEML Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiEML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiEML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiEML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiEML.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef 0
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
!endif

;SectionDivider

Section "AbiHrtext Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiHrtext.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiHrtext Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiHrtext Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiHrtext.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiISCII-Text Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiISCII_Text.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiIscii-text Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiIscii-text Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiISCII_Text.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef EXPERIMENTALPLUGINS
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
!endif

;SectionDivider

Section "AbiLaTeX Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiLaTeX.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiLaTeX Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiLaTeX Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiLaTeX.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef EXPERIMENTALPLUGINS
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

;SectionDivider
!endif

Section "MSWrite (*.wri) Plugin"
	SectionIn 1 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiMSWrite.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiMSWrite Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiMSWrite Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiMSWrite.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef EXPERIMENTALPLUGINS
Section "AbiNroff Plugin"
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

;SectionDivider
!endif

Section "AbiPalmDoc (*.pdb) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiPalmDoc.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPalmDoc Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPalmDoc Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiPalmDoc.dll"
  
	End:
SectionEnd

;SectionDivider

!ifdef 0
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

;SectionDivider
!endif

Section "AbiPsion (*.psiword) Plugin"
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
;SectionDivider

!ifdef 0
;Saig PatheticWriter 
Section "AbiPatheticWriter Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiPW.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiPatheticWriter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiPatheticWriter Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiPW.dll"
  
	End:
SectionEnd

;SectionDivider
!endif

Section "AbiT602 importer Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiT602.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiT602 Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiT602 Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiT602.dll"
  
	End:
SectionEnd

;SectionDivider

;Wireless Markup Language (old HTML replacement for mobile devices)
Section "AbiWML (*.wml) Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiWML.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWML Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWML Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiWML.dll"
  
	End:
SectionEnd

;SectionDivider

;XML/XSL Formatting objects, meant to be similar in scope to LaTeX
Section "AbiXSL-FO Plugin"
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiXSLFO.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiXSL-FO Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiXSL-FO Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiXSLFO.dll"
  
	End:
SectionEnd

; Additional File Format importer/exporters
SubSectionEnd


;SectionDivider


SubSection /e "Glib based Importers/Exporters"

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

; Not required if Glib is already available, otherwise is required
Section "Download glib 2.4" GLIB_IDX
	SectionIn 2

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Unzip glib and its dependencies into same directory as AbiWord.exe
	SetOutPath $INSTDIR\AbiWord

	;;;;;;;
	; iconv
	${dlFile} "http://www.abisource.com/downloads/dependencies/libiconv/libiconv-1.9.1.bin.win32.zip" "$TEMP\libiconv-1.9.1.bin.win32.zip" "ERROR: failed to download http://www.abisource.com/downloads/dependencies/libiconv/libiconv-1.9.1.bin.win32.zip"
	StrCmp $0 "success" 0 doCleanup
	${unzipFile} "$TEMP\libiconv-1.9.1.bin.win32.zip" "$INSTDIR\AbiWord" "bin\iconv.dll" "ERROR: failed to extract iconv.dll from libiconv-1.9.1.bin.win32.zip"
	StrCmp $0 "success" 0 doCleanup

	;;;;;;
	; intl
	${dlFile} "http://www.abisource.com/downloads/dependencies/gettext/gettext-runtime-0.13.1.zip" "$TEMP\gettext-runtime-0.13.1.zip" "ERROR: failed to download http://www.abisource.com/downloads/dependencies/gettext/gettext-runtime-0.13.1.zip"
	StrCmp $0 "success" 0 doCleanup
	${unzipFile} "$TEMP\gettext-runtime-0.13.1.zip" "$INSTDIR\AbiWord" "bin\intl.dll" "ERROR: failed to extract intl.dll from gettext-runtime-0.13.1.zip"
	StrCmp $0 "success" 0 doCleanup

	;;;;;;;;;;;;;;;;;;
	; glib and gobject
	${dlFile} "http://www.abisource.com/downloads/dependencies/glib/glib-2.4.5.zip" "$TEMP\glib-2.4.5.zip" "ERROR: failed to download http://www.abisource.com/downloads/dependencies/glib/glib-2.4.5.zip"
	StrCmp $0 "success" 0 doCleanup
	${unzipFile} "$TEMP\glib-2.4.5.zip" "$INSTDIR\AbiWord" "bin\libglib-2.0-0.dll" "ERROR: failed to extract libglib-2.0-0.dll from glib-2.4.5.zip"
	StrCmp $0 "success" 0 doCleanup
	${unzipFile} "$TEMP\glib-2.4.5.zip" "$INSTDIR\AbiWord" "bin\libgobject-2.0-0.dll" "ERROR: failed to extract libgobject-2.0-0.dll from glib-2.4.5.zip"
	StrCmp $0 "success" 0 doCleanup

	;;;;;;;;
	; libgsf
	${dlFile} "http://www.abisource.com/downloads/dependencies/libgsf/libgsf-1.8.2-20040121.zip" "$TEMP\libgsf-1.8.2-20040121.zip" "ERROR: failed to download http://www.abisource.com/downloads/dependencies/libgsf/libgsf-1.8.2-20040121.zip"
	StrCmp $0 "success" 0 doCleanup
	${unzipFile} "$TEMP\libgsf-1.8.2-20040121.zip" "$INSTDIR\AbiWord" "bin\libgsf-1-1.dll" "ERROR: failed to extract libgsf-1-1.dll from libgsf-1.8.2-20040121.zip"
	StrCmp $0 "success" 0 doCleanup

	;;;;;;;;;
	; libxml2
	${dlFile} "http://www.abisource.com/downloads/dependencies/libxml2/libxml2-2.4.12-bin.zip" "$TEMP\libxml2-2.4.12-bin.zip" "ERROR: failed to download http://www.abisource.com/downloads/dependencies/libxml2/libxml2-2.4.12-bin.zip"
	StrCmp $0 "success" 0 doCleanup
	${unzipFile} "$TEMP\libxml2-2.4.12-bin.zip" "$INSTDIR\AbiWord" "bin\libxml2.dll" "ERROR: failed to extract libxml2.dll from libxml2-2.4.12-bin.zip"
	StrCmp $0 "success" 0 doCleanup


	doCleanup:
		; Delete temporary files
		Delete "$TEMP\libiconv-1.9.1.bin.woe32.zip"
		Delete "$TEMP\gettext-runtime-0.13.1.zip"
		Delete "$TEMP\glib-2.4.5.zip"
		Delete "$TEMP\libgsf-1.8.2-20040121.zip"
		Delete "$TEMP\libxml2-2.4.12-bin.zip"

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path back to the plugins directory.
	SetOutPath $INSTDIR\AbiWord\plugins
SectionEnd

;SectionDivider
!ifdef 0
; OPTIONAL
SubSection /e "Image Manipulation"

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

;SectionDivider
SubSectionEnd
!endif

SubSection /e "File Format Importers/Exporters"

Section "OpenWriter (*.sxw) Plugin" SXW_IDX
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiOpenWriter.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiOpenWriter Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiOpenWriter Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiOpenWriter.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiSDW (*.sdw) Plugin" SDW_IDX
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiSDW.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiSDW Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiSDW Plugin (already exists)!"
	Goto End

	DoInstall:
	File "AbiSDW.dll"
  
	End:
SectionEnd

;SectionDivider

Section "AbiWordPerfect (*.wpd) Plugin" WP_IDX
	SectionIn 2

	; Testing clause to Overwrite Existing Version - if exists
	IfFileExists "$INSTDIR\AbiWord\plugins\AbiWordPerfect.dll" 0 DoInstall
	
	MessageBox MB_YESNO "Overwrite Existing AbiWordPerfect Plugin?" IDYES DoInstall
	
	DetailPrint "Skipping AbiWordPerfect Plugin (already exists)!"
	Goto End

	DoInstall:
        SetOutPath $INSTDIR\AbiWord\bin
        File "libwpd-0.8.dll"

        SetOutPath $INSTDIR\AbiWord\bin
        File "libwpd-stream-0.8.dll"

        SetOutPath $INSTDIR\AbiWord\plugins
	  File "AbiWordPerfect.dll"

  
	End:
SectionEnd

;SectionDivider

!ifdef 0
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

;SectionDivider
!endif

SubSectionEnd  ; Additional File Format importer/exporters

SubSectionEnd  ; glib based plugins

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
Section "Create Uninstaller for I/E Plugins"
	SectionIn 1 2
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
	Delete "$INSTDIR\AbiApplix.dll"
	Delete "$INSTDIR\AbiBZ2Abw.dll"
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
	Delete "$INSTDIR\AbiOpenWriter.dll"
	Delete "$INSTDIR\AbiPalmDoc.dll"
	Delete "$INSTDIR\AbiPsion.dll"
	Delete "$INSTDIR\AbiSDW.dll"
	Delete "$INSTDIR\AbiT602.dll"
	Delete "$INSTDIR\AbiWML.dll"
	Delete "$INSTDIR\AbiWordPerfect.dll"
	Delete "$INSTDIR\AbiXSLFO.dll"

	Delete "$INSTDIR\..\bin\iconv.dll"
	Delete "$INSTDIR\..\bin\intl.dll"
	Delete "$INSTDIR\..\bin\libbz2.dll"
	Delete "$INSTDIR\..\bin\libglib-2.0-0.dll"
	Delete "$INSTDIR\..\bin\libgobject-2.0-0.dll"
	Delete "$INSTDIR\..\bin\libgsf-1-1.dll"
	Delete "$INSTDIR\..\bin\libwpd-0.8.dll"
	Delete "$INSTDIR\..\bin\libwpd-stream-0.8.dll"
	Delete "$INSTDIR\..\bin\libxml2.dll"

!ifdef 0
	Delete "$INSTDIR\AbiMagick.dll"
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

; Selection Change Handler 
Function .onSelChange
	; Make sure glib is selected when seleting one of the plugins that need it
	strcmp $GLIB_ENABLED "yes" end_l
		  
	!insertmacro SectionFlagIsSet ${SXW_IDX} ${SF_SELECTED} "" sdw_l 
		!insertmacro SelectSection ${GLIB_IDX}
		strcpy $GLIB_ENABLED "yes"
		goto end_l 
sdw_l:
	!insertmacro SectionFlagIsSet ${SDW_IDX} ${SF_SELECTED} "" wpd_l 
		!insertmacro SelectSection ${GLIB_IDX}
		strcpy $GLIB_ENABLED "yes"
		goto end_l
wpd_l: 
	!insertmacro SectionFlagIsSet ${WP_IDX} ${SF_SELECTED} "" end_l
		!insertmacro SelectSection ${GLIB_IDX}
		strcpy $GLIB_ENABLED "yes"
end_l:
FunctionEnd

; eof
