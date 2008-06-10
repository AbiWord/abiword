;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optional plugins for import/export support

; Copyright 2008 AbiSource Corporation B.V.

SubSection /e "$(TITLE_ssection_impexpplugins)" ssection_impexpplugins
Section "" section_impexpplugins_required

SectionEnd

SubSection /e "$(TITLE_ssection_loadsaveplugins)" ssection_loadsaveplugins

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  applix
Section "$(TITLE_section_impexpplugins_applix)" section_impexpplugins_applix
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiapplix.dll"
SectionEnd

!macro Remove_${section_impexpplugins_applix}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: applix ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiapplix.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  clarisworks
Section "$(TITLE_section_impexpplugins_clarisworks)" section_impexpplugins_clarisworks
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiclarisworks.dll"
SectionEnd

!macro Remove_${section_impexpplugins_clarisworks}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: clarisworks ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiclarisworks.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  docbook
Section "$(TITLE_section_impexpplugins_docbook)" section_impexpplugins_docbook
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abidocbook.dll"
SectionEnd

!macro Remove_${section_impexpplugins_docbook}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: docbook ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abidocbook.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  opendocument
Section "$(TITLE_section_impexpplugins_opendocument)" section_impexpplugins_opendocument
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiopendocument.dll"
SectionEnd

!macro Remove_${section_impexpplugins_opendocument}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: opendocument ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiopendocument.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  openwriter
Section "$(TITLE_section_impexpplugins_openwriter)" section_impexpplugins_openwriter
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiopenwriter.dll"
SectionEnd

!macro Remove_${section_impexpplugins_openwriter}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: openwriter ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiopenwriter.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  iscii_text
Section "$(TITLE_section_impexpplugins_iscii_text)" section_impexpplugins_iscii_text
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiiscii_text.dll"
SectionEnd

!macro Remove_${section_impexpplugins_iscii_text}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: iscii_text ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiiscii_text.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  eml
Section "$(TITLE_section_impexpplugins_eml)" section_impexpplugins_eml
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abieml.dll"
SectionEnd

!macro Remove_${section_impexpplugins_eml}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: eml ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abieml.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  palmdoc
Section "$(TITLE_section_impexpplugins_palmdoc)" section_impexpplugins_palmdoc
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abipalmdoc.dll"
SectionEnd

!macro Remove_${section_impexpplugins_palmdoc}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: palmdoc ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abipalmdoc.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  wml
Section "$(TITLE_section_impexpplugins_wml)" section_impexpplugins_wml
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiwml.dll"
SectionEnd

!macro Remove_${section_impexpplugins_wml}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: wml ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiwml.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  xslfo
Section "$(TITLE_section_impexpplugins_xslfo)" section_impexpplugins_xslfo
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abixslfo.dll"
SectionEnd

!macro Remove_${section_impexpplugins_xslfo}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: xslfo ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abixslfo.dll"
!macroend

SubSectionEnd ; both import/export


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Import Only
SubSection /e "$(TITLE_ssection_loadplugins)" ssection_loadplugins

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  openxml
Section "$(TITLE_section_impexpplugins_openxml)" section_impexpplugins_openxml
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiopenxml.dll"
SectionEnd

!macro Remove_${section_impexpplugins_openxml}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: openxml ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiopenxml.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  mswrite
Section "$(TITLE_section_impexpplugins_mswrite)" section_impexpplugins_mswrite
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abimswrite.dll"
SectionEnd

!macro Remove_${section_impexpplugins_mswrite}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: mswrite ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abimswrite.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  opml
Section "$(TITLE_section_impexpplugins_opml)" section_impexpplugins_opml
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiopml.dll"
SectionEnd

!macro Remove_${section_impexpplugins_opml}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: opml ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiopml.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  sdw
Section "$(TITLE_section_impexpplugins_sdw)" section_impexpplugins_sdw
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abisdw.dll"
SectionEnd

!macro Remove_${section_impexpplugins_sdw}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: sdw ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abisdw.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  t602
Section "$(TITLE_section_impexpplugins_t602)" section_impexpplugins_t602
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abit602.dll"
SectionEnd

!macro Remove_${section_impexpplugins_t602}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: t602 ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abit602.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  wordperfect
Section "$(TITLE_section_impexpplugins_wordperfect)" section_impexpplugins_wordperfect
	SectionIn 1 2 4 ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abiwordperfect.dll"
SectionEnd

!macro Remove_${section_impexpplugins_wordperfect}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: wordperfect ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abiwordperfect.dll"
!macroend

SubSectionEnd ; import only

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Export Only
SubSection /e "$(TITLE_ssection_saveplugins)" ssection_saveplugins

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  hrtext
Section "$(TITLE_section_impexpplugins_hrtext)" section_impexpplugins_hrtext
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abihrtext.dll"
SectionEnd

!macro Remove_${section_impexpplugins_hrtext}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: hrtext ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abihrtext.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  latex
Section "$(TITLE_section_impexpplugins_latex)" section_impexpplugins_latex
	SectionIn 2 4 ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "..\plugins\abilatex.dll"
SectionEnd

!macro Remove_${section_impexpplugins_latex}
	;Removes this component
	DetailPrint "*** Removing or skipping install of import/export plugin: latex ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\abilatex.dll"
!macroend


SubSectionEnd ; export only


SubSectionEnd ; Tools Plugins
!macro Remove_${ssection_impexpplugins}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_impexpplugins"


!macroend