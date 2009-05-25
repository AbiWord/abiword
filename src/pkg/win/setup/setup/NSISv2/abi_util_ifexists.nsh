;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains macros for conditional installer compilation
;               depending on file existing during installer creation phase


; based on snippet from KiCHiK
!ifndef _ABI_UTIL_IFEXISTS_NSH_
!define _ABI_UTIL_IFEXISTS_NSH_


!macro IfExists file
!system 'echo !define file_\>temp.nsh'
!system 'if exist "${file}" echo exists >> temp.nsh'
!system 'echo # >> temp.nsh'
!include temp.nsh
!system 'del temp.nsh'
!ifndef file_exists
!undef "file_#"
!endif
!ifdef file_exists
!undef file_exists
!macroend

!define IfExists "!insertmacro IfExists"
!define IfExistsEnd "!endif"


!endif ; _ABI_UTIL_IFEXISTS_NSH_
