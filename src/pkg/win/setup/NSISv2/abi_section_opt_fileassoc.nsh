;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for associating files with AbiWord

; Copyright (C) 2008 AbiSource Corporation B.V.


; Associate file extension and content type with an application entry
!include "abi_util_fileassoc.nsh"

; Application entry for all types registered to AbiWord
!define appType "${APPSET}.${PRODUCT}"


; OPTIONAL File associations
SubSection /e "$(TITLE_ssection_fa_shellupdate)" ssection_fa_shellupdate

Section "" section_fa_shellupdate_inv ; invisible section that sets up general application information
  SectionIn ${TYPICALSECT} ${FULLSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${DLSECT}

  ; Write the application generic file association keys (define app and how to run it)
  ;CreateApplicationAssociation ApplicationSuite.FileType AppName AppDesc DefIcon ExeCmd
  ${CreateApplicationAssociation} "${appType}" "${PRODUCT}" "${PRODUCT} Document" \
                                  "$INSTDIR\${MAINPROGRAM},1" "$INSTDIR\${MAINPROGRAM}"
SectionEnd
!macro Remove_${section_fa_shellupdate_inv}
	;Removes this component
	DetailPrint "*** Begin: Removing file associations..."

	; ${RemoveFileAssociation} calls will remove file assoications 
	; (only removes if still registered with ${appType}), and
	; tries to restore prior one

	; actual apptype entry
	DetailPrint "*** Removing ${appType} entry..."
	DeleteRegKey HKCR "${appType}"
!macroend

; Write File Associations for each supported types

; OPTIONAL (native format, recommended)
Section "$(TITLE_section_fa_abw)" section_fa_abw
	SectionIn ${TYPICALSECT} ${FULLSECT} ${FULLASSOCSECT} ${MINIMALSECT} ${DLSECT}
	${CreateFileAssociation} ".abw"  "${appType}" "application/abiword"
	${CreateFileAssociation} ".awt"  "${appType}" "application/abiword-template"
	${CreateFileAssociation} ".zabw" "${appType}" "application/abiword-compressed"
SectionEnd
!macro Remove_${section_fa_abw}
	;Removes this component
	DetailPrint "*** Removing .abw Registry Association..."
	${RemoveFileAssociation} ".abw"  "${appType}"
	DetailPrint "*** Removing .awt Registry Association..."
	${RemoveFileAssociation} ".awt"  "${appType}"
	DetailPrint "*** Removing .zabw Registry Association..."
	${RemoveFileAssociation} ".zabw" "${appType}"
!macroend

; OPTIONAL (Word document, only recommended if lack Word itself)
Section "$(TITLE_section_fa_doc)" section_fa_doc
	SectionIn ${FULLASSOCSECT} ${DLSECT}
	${CreateFileAssociation} ".doc"  "${appType}" "application/msword"
SectionEnd
!macro Remove_${section_fa_doc}
	;Removes this component
	DetailPrint "*** Restoring original .doc Registry Association..."
	${RemoveFileAssociation} ".doc"  "${appType}"
!macroend

; OPTIONAL (Rich Text, standard Word Processor interchange format, no recommendation)
Section "$(TITLE_section_fa_rtf)" section_fa_rtf
	SectionIn ${FULLASSOCSECT} ${DLSECT}
	${CreateFileAssociation} ".rtf"  "${appType}" "text/richtext"
SectionEnd
!macro Remove_${section_fa_rtf}
	;Removes this component
	DetailPrint "*** Restoring original .rtf Registry Association..."
	${RemoveFileAssociation} ".rtf"  "${appType}"
!macroend

SubSectionEnd ; file associations
!macro Remove_${ssection_fa_shellupdate}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_fa_shellupdate"
!macroend

