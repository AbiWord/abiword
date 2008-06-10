;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for templates
; Copyright (C) 2008 AbiSource Corporation B.V.

!macro StoreMD5 dir fname
	md5dll::GetFileMD5 "${dir}\${fname}"
 	Pop $0
	DetailPrint "MD5 for ${fname} is $0"
	;TODO store in registry
!macroend
!define StoreMD5 "!insertmacro StoreMD5"

; OPTIONAL Installation of Templates
Section "$(TITLE_section_templates)" section_templates
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\templates"

	; generate and store md5 of each template so we can determine if user updated or not
	${DoDirForEach} *.awt "$INSTDIR\templates" "${StoreMD5}"
SectionEnd
!macro Remove_${section_templates}
	;Removes this component
	DetailPrint "*** Removing templates..."

	; remove templates
	; TODO: don't delete modified templates without asking user 1st
	Delete "$INSTDIR\templates\*.awt"

	; attempt to remove install directory
	${DeleteDirIfEmpty} "$INSTDIR\templates"
!macroend
