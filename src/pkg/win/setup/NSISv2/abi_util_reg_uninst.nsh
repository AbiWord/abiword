;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Adds all the registry keys for use by Windows add/remove control panel


; filename of uninstaller
!define UninstallerFname "Uninstall${PRODUCT}${VERSION_MAJOR}.exe"

; base uninstaller key used by control panel add/remove util to get info about Abi's uninstaller
!define UninstallerKeyName "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}${VERSION_MAJOR}"

; simply control panel reg writing
!macro WRSU name value
  WriteRegStr HKLM  "${UninstallerKeyName}" "${name}" '"${value}"'
!macroend
!define WRSU "!insertmacro WRSU"

!macro WRDU name value
  WriteRegDWORD HKLM "${UninstallerKeyName}" "${name}" "${value}"
!macroend
!define WRDU "!insertmacro WRDU"

; Write the uninstall keys for Windows
; See http://nsis.sourceforge.net/archive/nsisweb.php?page=100&instances=0,44
; for information about registry keys from Konrad (and KiCHiK)

; Required values
; DisplayName (string) - Name of the application
; UninstallString (string) - Path and filename of the uninstaller. You should always quote the path to make sure spaces in the path will not disrupt Windows to find the uninstaller.


; N.B. This needs to include a version number or unique identifier.  
; More than one version of Abiword but only one Control Panel.  
${WRSU} "DisplayName" "${PRODUCT} ${VERSION} (remove only)"
${WRSU} "UninstallString" "$INSTDIR\${UninstallerFname}"


; Optional values
; Some of the values will not be used by older Windows versions.

; InstallLocation (string) - Installation directory ($INSTDIR)

${WRSU} "InstallLocation" "$INSTDIR"

; DisplayIcon (string) - Path, filename and index of of the icon that will be displayed next to your application name
; Publisher (string) - (Company) name of the publisher

${WRSU} "DisplayIcon" "$INSTDIR\${MAINPROGRAM},2"
${WRSU} "Publisher" "AbiSource Developers"

; ModifyPath (string) - Path and filename of the application modify program
; InstallSource (string) - Location where the application was installed from

;${WRSU} "ModifyPath" "$INSTDIR\${UninstallerFname}"
;${WRSU} "InstallSource" ""

; ProductID (string) - Product ID of the application
; RegOwner (string) - Registered owner of the application
; RegCompany (string) - Registered company of the application

;${WRSU} "ProductID" ""
;${WRSU} "RegOwner" ""
;${WRSU} "RegCompany" ""

; HelpLink (string) - Link to the support website
; HelpTelephone (string) - Telephone number for support

${WRSU} "HelpLink" "http://www.abisource.com/support/"
;${WRSU} "HelpTelephone" "(704) 555-1234"

; URLUpdateInfo (string) - Link to the website for application updates
; URLInfoAbout (string) - Link to the application home page

${WRSU} "URLUpdateInfo" "http://www.abisource.com/"
${WRSU} "URLInfoAbout" "http://www.abisource.com/information/about/"

; DisplayVersion (string) - Displayed version of the application
; VersionMajor (DWORD) - Major version number of the application
; VersionMinor (DWORD) - Minor version number of the application

${WRSU} "DisplayVersion" "${VERSION}"
${WRDU} "VersionMajor" ${VERSION_MAJOR}
${WRDU} "VersionMinor" ${VERSION_MINOR}

; NoModify (DWORD) - 1 if uninstaller has no option to modify the installed application
; NoRepair (DWORD) - 1 if the uninstaller has no option to repair the installation
; If both NoModify and NoRapair are set to 1, the button displays "Remove" instead of "Modify/Remove".

${WRDU} "NoModify" 1
${WRDU} "NoRepair" 1

; Comments (string) - misc info (1 line, no wrap or newlines)

${WRSU} "Comments" "AbiWord is a free word processing program suitable for typing papers, letters, reports, memos, and so forth."


; Create the Uninstaller
WriteUninstaller "${UninstallerFname}"
