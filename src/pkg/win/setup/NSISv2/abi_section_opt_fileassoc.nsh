;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for associating files with AbiWord


; Associate file extension and content type with an application entry
!include "abi_util_fileassoc.nsh"


; OPTIONAL File associations
SubSection /e "$(TITLE_ssection_fa_shellupdate)" ssection_fa_shellupdate

Section "" ; invisible section that sets up general application information
  SectionIn 1 2 3 ${DLSECT}

  ; Write the application generic file association keys (define app and how to run it)
  ;CreateApplicationAssociation ApplicationSuite.FileType AppName AppDesc DefIcon ExeCmd
  ${CreateApplicationAssociation} "${APPSET}.${PRODUCT}" "${PRODUCT}" "${PRODUCT} Document" \
                                  "$INSTDIR\${MAINPROGRAM},2" "$INSTDIR\${MAINPROGRAM}"
SectionEnd

; Write File Associations for each supported types

; OPTIONAL (native format, recommended)
Section "$(TITLE_section_fa_abw)" section_fa_abw
	SectionIn 1 2 3 ${DLSECT}
	${CreateFileAssociation} ".abw"  "${APPSET}.${PRODUCT}" "application/abiword"
SectionEnd

; OPTIONAL (native format, recommended)
Section "$(TITLE_section_fa_awt)" section_fa_awt
	SectionIn 1 2 3 ${DLSECT}
	${CreateFileAssociation} ".awt"  "${APPSET}.${PRODUCT}" "application/abiword-template"
SectionEnd

; OPTIONAL (native format, recommended)
Section "$(TITLE_section_fa_zabw)" section_fa_zabw
	SectionIn 1 2 3 ${DLSECT}
	${CreateFileAssociation} ".zabw" "${APPSET}.${PRODUCT}" "application/abiword-compressed"
SectionEnd

; OPTIONAL (Word document, only recommended if lack Word itself)
Section "$(TITLE_section_fa_doc)" section_fa_doc
	SectionIn 2 ${DLSECT}
	${CreateFileAssociation} ".doc"  "${APPSET}.${PRODUCT}" "application/msword"
SectionEnd

; OPTIONAL (Rich Text, standard Word Processor interchange format, no recommendation)
Section "$(TITLE_section_fa_rtf)" section_fa_rtf
	SectionIn 2 ${DLSECT}
	${CreateFileAssociation} ".rtf"  "${APPSET}.${PRODUCT}" "text/richtext"
SectionEnd

SubSectionEnd ; file associations

