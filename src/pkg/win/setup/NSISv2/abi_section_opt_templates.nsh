;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for templates


; OPTIONAL Installation of Templates
Section "$(TITLE_section_templates)" section_templates
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\templates"
SectionEnd
