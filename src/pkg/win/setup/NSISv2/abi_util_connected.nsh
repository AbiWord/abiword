;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Function to determine if user is connected to Internet/has networking enabled


!ifndef NODOWNLOADS
!ifndef _ABI_UTIL_CONNECTED_NSH_
!define _ABI_UTIL_CONNECTED_NSH_

; ConnectInternet (uses Dialer plugin)
; Originally Written by Joost Verburg 
;
; This function attempts to make a connection to the internet if there is
; no connection available. If you are not sure that a system using the
; installer has an active internet connection, call this function before
; downloading files with NSISdl.
; 
; The function requires Internet Explorer 3, but asks to connect manually
; if IE3 is not installed.  [Assumes available on error.]
;
; On return $0 is set to "online" or error value
 
Function ConnectInternet
	ClearErrors
	Dialer::AttemptConnect
	IfErrors noie3
    
	Pop $0	; $0 is set to "online"
	StrCmp $0 "online" connected
		DetailPrint "Unable to establish Internet connection, aborting download"
		DetailPrint "Dialer::AttemptConnect returned $0"
		MessageBox MB_OK|MB_ICONSTOP "Cannot connect to the internet."
		Goto connected
     
     noie3:
   
     ; IE3 not installed
     MessageBox MB_OK|MB_ICONINFORMATION "Please connect to the internet now."
     StrCpy $0 "online"	; assume user established connection ...
     
     connected:
     
FunctionEnd

!endif ; _ABI_UTIL_CONNECTED_NSH_
!endif ; NODOWNLOADS
