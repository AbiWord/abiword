;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for clipart


; OPTIONAL Installation of Clipart
Section "$(TITLE_section_clipart)" section_clipart
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\clipart"
SectionEnd

