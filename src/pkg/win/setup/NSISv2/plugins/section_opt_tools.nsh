;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optional tools plugins

; Copyright (C) 2008 AbiSource Corporation B.V.

SubSection /e "$(TITLE_ssection_toolsplugins)" ssection_toolsplugins
Section "" section_toolsplugins_required

SectionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  AbiMathView
Section "$(TITLE_section_toolsplugins_mathview)" section_toolsplugins_mathview
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; Unzip libmathview into same directory as AbiWord.exe
	SetOutPath $INSTDIR\${PRODUCT}\bin
	File /r "libmathview-0.dll"
	File /r "libmathview_frontend_libxml2-0.dll"
	
	;Install Configuration Files - This better work...
	SetOutPath $INSTDIR\math
	File /r "..\AbiSuite\math\gtkmathview.conf.xml"
	File /r "..\AbiSuite\math\dictionary-local.xml"
	File /r "..\AbiSuite\math\dictionary.xml"
	
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiMathView.dll"
SectionEnd

!macro Remove_${section_toolsplugins_mathview}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: Equation Editor ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\AbiMathView.dll"

!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  
Section "$(TITLE_section_toolsplugins_abicollab)" section_toolsplugins_abicollab
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\bin
	File "libsoup-2.4-1.dll"
	
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiCollab.dll"
	
	WriteRegStr HKCR ".abicollab" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".abicollab" "Content Type" "application/abiword"
SectionEnd

!macro Remove_${section_toolsplugins_abicollab}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: AbiCollab ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\AbiCollab.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\libsoup-2.4-1.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\libgthread-2.0-0.dll"
	DeleteRegKey HKCR ".abicollab"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Grammar
Section "$(TITLE_section_toolsplugins_grammar)" section_toolsplugins_grammar
	SectionIn ${FULLSECT} ${FULLASSOCSECT} ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	
	SetOutPath $INSTDIR\${PRODUCT}\bin
	File /r "liblink-grammar-4.dll"
	SetOutPath $INSTDIR\${PRODUCT}\bin\en
	File /r "en\*"
	
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiGrammar.dll"
SectionEnd

!macro Remove_${section_toolsplugins_grammar}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: grammar ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\AbiGrammar.dll"
	Delete "$INSTDIR\${PRODUCT}\bin\liblink-grammar-4.dll"
	RMDir /r "$INSTDIR\${PRODUCT}\bin\en\"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  URLDict
Section "$(TITLE_section_toolsplugins_urldict)" section_toolsplugins_urldict
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiURLDict.dll"
SectionEnd

!macro Remove_${section_toolsplugins_urldict}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: urldict ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\AbiURLDict.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Google
Section "$(TITLE_section_toolsplugins_google)" section_toolsplugins_google
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiGoogle.dll"
SectionEnd

!macro Remove_${section_toolsplugins_google}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: google ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\AbiGoogle.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  wikipedia
Section "$(TITLE_section_toolsplugins_wikipedia)" section_toolsplugins_wikipedia
	SectionIn ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiWikipedia.dll"
SectionEnd

!macro Remove_${section_toolsplugins_wikipedia}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: wikipedia ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\AbiWikipedia.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  babelfish
Section "$(TITLE_section_toolsplugins_babelfish)" section_toolsplugins_babelfish
	SectionIn ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\AbiBabelfish.dll"
SectionEnd

!macro Remove_${section_toolsplugins_babelfish}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: babelfish ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\AbiBabelfish.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  freetranslation
Section "$(TITLE_section_toolsplugins_freetranslation)" section_toolsplugins_freetranslation
	SectionIn ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\Abifreetranslation.dll"
SectionEnd

!macro Remove_${section_toolsplugins_freetranslation}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: freetranslation ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\Abifreetranslation.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  scripthappy
Section "$(TITLE_section_toolsplugins_scripthappy)" section_toolsplugins_scripthappy
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\${PRODUCT}\plugins
	File "..\plugins\Abiscripthappy.dll"
SectionEnd

!macro Remove_${section_toolsplugins_scripthappy}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: scripthappy ..."

	; remove plugin and related files
	Delete "$INSTDIR\${PRODUCT}\plugins\Abiscripthappy.dll"
!macroend




SubSectionEnd ; Tools Plugins
!macro Remove_${ssection_toolsplugins}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_toolsplugins"


!macroend
