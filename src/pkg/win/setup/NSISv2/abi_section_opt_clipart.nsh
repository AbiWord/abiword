;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for clipart


; OPTIONAL Installation of Clipart
Section "$(TITLE_section_clipart)" section_clipart
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\clipart"
SectionEnd
!macro Remove_${section_clipart}
	;Removes this component
	DetailPrint "*** Removing clipart..."

	; remove clipart
	; TODO don't removed modified/new images without asking user 1st
	Delete "$INSTDIR\clipart\*.png"

	; attempt to remove install directory
	${DeleteDirIfEmpty} "$INSTDIR\clipart"
!macroend


