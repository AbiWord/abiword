; This is a file for creating an installer for Abiword Dictionaries using NSIS 
; by Alan Horkan <horkana@tcd.ie>
; modified by Michael D. Pritchett <mpritchett@attglobal.net>

; Do a Cyclic Redundancy Check to make sure the installer 
; was not corrupted by the download.  
CRCCheck on

; The name of the installer
Name "AbiWord Plugin - ImageMagick"
Icon "..\..\pkg\win\setup\setup.ico"
OutFile "AbiWord_Plugin_ImageMagick.exe"

; License Information
LicenseText "This program is Licensed under the GNU General Public License (GPL)."
LicenseData "..\AbiSuite\Copying"

; The default installation directory
InstallDir $PROGRAMFILES\AbiSuite

; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\Abisuite "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install Abiword's ImageMagic Plugin on your computer."

; The text to prompt the user to enter a directory
DirText "Choose the AbiSuite directory where you previously installed Abiword:"

EnabledBitmap  ..\..\pkg\win\setup\checkbox.bmp
DisabledBitmap ..\..\pkg\win\setup\emptybox.bmp

; The stuff that must be installed
; binary, license, and American dictionary
Section "Abiword Plugin - ImageMagick (required)"

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Set output path to the installation directory.
	SetOutPath $INSTDIR\AbiWord\plugins
	File "..\plugins\libAbiMagick.dll"
  
SectionEnd

; eof
