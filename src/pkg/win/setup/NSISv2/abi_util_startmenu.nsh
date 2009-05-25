;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Utility functions for start menu entries


!ifndef _ABI_UTIL_STARTMENU_NSH_
!define _ABI_UTIL_STARTMENU_NSH_


; Create a language localized Start Menu group
!macro lngCreateSMGroup group
	push $0
	StrCpy $0 "${group}"
	CreateDirectory "$SMPROGRAMS\$0"
	pop $0
!macroend
!define lngCreateSMGroup "!insertmacro lngcreateSMGroup"

; Create a language localized Start Menu ShortCut
; we split the link.lnk up so we can use localized components without confusing NSIS
!macro lngCreateShortCut basedir group linkname target.file parameters icon.file icon_index_number
	push $0
	push $1
	push $2
	StrCpy $0 "${basedir}"
	StrCpy $0 "$0\"
	; if group is empty skip past it
	StrCpy $1 "${group}"
	StrLen $2 "$1"
	IntCmp $2 0 +3
	StrCpy $0 "$0$1"
	StrCpy $0 "$0\"
	;skipGroup:
	StrCpy $1 ${linkname}
	StrCpy $0 "$0$1"
	StrCpy $0 "$0.lnk"
	CreateShortCut $0 "${target.file}" "${parameters}" "${icon.file}" ${icon_index_number}
	pop $2
	pop $1
	pop $0
!macroend
!define lngCreateShortCut "!insertmacro lngCreateShortCut"


!endif ; _ABI_UTIL_STARTMENU_NSH_
