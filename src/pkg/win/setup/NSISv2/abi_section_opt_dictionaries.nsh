;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for dictionary (hash) files


SubSection /e "$(TITLE_ssection_dictionary)" ssection_dictionary

; OPTIONAL Installation of Default Dictionary
Section "$(TITLE_section_dictinary_def_English)" section_dictinary_def_English
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
SectionEnd


!ifdef OPT_DICTIONARIES
!include "abi_dict_misc.nsh"
!endif ; OPT_DICTIONARIES


SubSectionEnd ; Dictionaries
