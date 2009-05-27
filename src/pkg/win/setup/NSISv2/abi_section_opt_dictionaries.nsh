;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for dictionary (hash) files

; Copyright (C) 2008 AbiSource Corporation B.V.

SubSection /e "$(TITLE_ssection_dictionary)" ssection_dictionary
Section "" section_dictionary_required
	SetOutPath $INSTDIR\lib\enchant
	File "${ABIWORD_COMPILED_PATH}\lib\enchant\libenchant_ispell.dll"
	File "${ABIWORD_COMPILED_PATH}\lib\enchant\libenchant_myspell.dll"
SectionEnd

; OPTIONAL Installation of Default Dictionary
Section "$(TITLE_section_dictinary_def_English)" section_dictinary_def_English
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT}
	SetOutPath $INSTDIR\dictionary\ispell
	File "${ABIWORD_COMPILED_PATH}\dictionary\ispell\american.hash"
SectionEnd
!macro Remove_${section_dictinary_def_English}
	;Removes this component
	DetailPrint "*** Removing en-US dictionary..."

	; remove dictionary
	Delete "$INSTDIR\dictionary\ispell\american.hash"
	;Delete "$INSTDIR\dictionary\*.hash"
!macroend


!ifdef OPT_DICTIONARIES
!include "abi_dict_misc.nsh"
!endif ; OPT_DICTIONARIES


SubSectionEnd ; Dictionaries
!macro Remove_${ssection_dictionary}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_dictionary"

	Delete "$INSTDIR\lib\enchant\libenchant_ispell.dll"
	Delete "$INSTDIR\lib\enchant\libenchant_myspell.dll"

	; attempt to remove dictionary directory, if no more are present
	${DeleteDirIfEmpty} "$INSTDIR\dictionary\ispell"
	IfFileExists "$INSTDIR\dictionary\ispell" 0 +2
	DetailPrint "Unable to remove $INSTDIR\dictionary\ispell"
	${DeleteDirIfEmpty} "$INSTDIR\dictionary"
!macroend
