;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for help files


; OPTIONAL Installation of Help Files
Section "$(TITLE_section_help)" section_help
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR\AbiWord
	; help documents may not be created if peer abiword-docs not found
	File /nonfatal /r "..\abisuite\abiword\help"

	SetOutPath $INSTDIR\AbiWord\help\en-US
	File "..\..\..\credits.txt"
SectionEnd

