;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains macros for simplifying downloads with retries
;               Use of ${dlFile} adds NSISdl and Dialer plugins to installer.


!ifndef NODOWNLOADS
!ifndef dlFile

!include "abi_util_connected.nsh"

; Attempts to download the remote file (URL given by ${remoteFname})
; and save locally as file (path and name given by ${localFname})
; on error (NSISdl does not return 'success') it will display a 
; message box and ask the user to retry or not.  If yes retry then
; will restart the download and repeat until successfully downloads
; or user says not to repeat.
; Modifies $0, on return should equal "success" if was able to
; connect to the Internet and successfully download the file.
; Usage:
; ${dlFile} "http://www.somehost.com/somepath/somefile.ext" "${INSTDIR}\somedir\f.ext" "ERROR: failed to download"
; StrCmp $0 "success" dlSuccessful dlFailed

!macro dlFileMacro remoteFname localFname errMsg
	!define retryDLlbl retryDL_${__FILE__}${__LINE__}
	!define dlDonelbl dlDoneDL_${__FILE__}${__LINE__}

	Call ConnectInternet	; try to establish connection if not connected
	StrCmp $0 "online" 0 ${dlDonelbl}

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

!endif ; dlFile
!endif ; NODOWNLOADS
