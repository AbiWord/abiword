;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optional tools plugins

; Copyright (C) 2008 AbiSource Corporation B.V.

SubSection /e "$(TITLE_ssection_toolsplugins)" ssection_toolsplugins
Section "" section_toolsplugins_required

SectionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  AbiMathView
; re-enable this when we actually build it
;Section "$(TITLE_section_toolsplugins_mathview)" section_toolsplugins_mathview
;	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
;	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	; Unzip libmathview into same directory as AbiWord.exe
;	SetOutPath $INSTDIR\bin
;	File /r "libmathview-0.dll"
;	File /r "libmathview_frontend_libxml2-0.dll"
;	
;	;Install Configuration Files - This better work...
;	SetOutPath $INSTDIR\math
;	File /r "..\AbiSuite\math\gtkmathview.conf.xml"
;	File /r "..\AbiSuite\math\dictionary-local.xml"
;	File /r "..\AbiSuite\math\dictionary.xml"
;	
;	SetOutPath $INSTDIR\plugins
;	File "..\plugins\AbiMathView.dll"
;SectionEnd

;!macro Remove_${section_toolsplugins_mathview}
;	;Removes this component
;	DetailPrint "*** Removing or skipping install of tool plugin: Equation Editor ..."
;
;	; remove plugin and related files
;	Delete "$INSTDIR\AbiMathView.dll"
;!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Collab
Section "$(TITLE_section_toolsplugins_abicollab)" section_toolsplugins_abicollab
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\bin
	File "${ABIWORD_COMPILED_PATH}\bin\libsoup-2.4-1.dll"
	File "${ABIWORD_COMPILED_PATH}\bin\libgcrypt-11.dll"
	File "${ABIWORD_COMPILED_PATH}\bin\libgnutls-26.dll"
	File "${ABIWORD_COMPILED_PATH}\bin\libgpg-error-0.dll"
	
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginCollab.dll"
	
	SetOutPath $INSTDIR\certs
	File "${ABIWORD_COMPILED_PATH}\certs\cacert.pem"
	
	WriteRegStr HKCR ".abicollab" "" "AbiSuite.AbiWord"
	WriteRegStr HKCR ".abicollab" "Content Type" "application/abiword"
SectionEnd

!macro Remove_${section_toolsplugins_abicollab}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: AbiCollab ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\PluginCollab.dll"
	Delete "$INSTDIR\bin\libsoup-2.4-1.dll"
	Delete "$INSTDIR\bin\libgcrypt-11.dll"
	Delete "$INSTDIR\bin\libgnutls-26.dll"
	Delete "$INSTDIR\bin\libgpg-error-0.dll"
	
	Delete "$INSTDIR\certs\cacert.pem"
	${DeleteDirIfEmpty} "$INSTDIR\certs"
	IfFileExists "$INSTDIR\certs" 0 +2
	DetailPrint "Unable to remove $INSTDIR\certs"	
	
	DeleteRegKey HKCR ".abicollab"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Grammar
Section "$(TITLE_section_toolsplugins_grammar)" section_toolsplugins_grammar
	SectionIn ${FULLSECT} ${FULLASSOCSECT} ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	
	SetOutPath $INSTDIR\bin
	File "${ABIWORD_COMPILED_PATH}\bin\liblink-grammar-4.dll"
	File "${ABIWORD_COMPILED_PATH}\bin\libgnurx-0.dll"
	;SetOutPath $INSTDIR\bin\en
	;File /r "en\*"
	
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginGrammar.dll"
SectionEnd

!macro Remove_${section_toolsplugins_grammar}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: grammar ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\PluginGrammar.dll"
	Delete "$INSTDIR\bin\liblink-grammar-4.dll"
	Delete "$INSTDIR\bin\libgnurx-0.dll"
	;RMDir /r "$INSTDIR\bin\en\"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  URLDict
Section "$(TITLE_section_toolsplugins_urldict)" section_toolsplugins_urldict
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginUrldict.dll"
SectionEnd

!macro Remove_${section_toolsplugins_urldict}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: urldict ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\PluginUrldict.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  Google
Section "$(TITLE_section_toolsplugins_google)" section_toolsplugins_google
	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginGoogle.dll"
SectionEnd

!macro Remove_${section_toolsplugins_google}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: google ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\PluginGoogle.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  wikipedia
Section "$(TITLE_section_toolsplugins_wikipedia)" section_toolsplugins_wikipedia
	SectionIn ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginWikipedia.dll"
SectionEnd

!macro Remove_${section_toolsplugins_wikipedia}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: wikipedia ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\AbiWikipedia.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  babelfish
Section "$(TITLE_section_toolsplugins_babelfish)" section_toolsplugins_babelfish
	SectionIn ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginBabelfish.dll"
SectionEnd

!macro Remove_${section_toolsplugins_babelfish}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: babelfish ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\PluginBabelfish.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  freetranslation
Section "$(TITLE_section_toolsplugins_freetranslation)" section_toolsplugins_freetranslation
	SectionIn ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
	SetOutPath $INSTDIR\plugins
	File "${ABIWORD_COMPILED_PATH}\plugins\PluginFreetranslation.dll"
SectionEnd

!macro Remove_${section_toolsplugins_freetranslation}
	;Removes this component
	DetailPrint "*** Removing or skipping install of tool plugin: freetranslation ..."

	; remove plugin and related files
	Delete "$INSTDIR\plugins\PluginFreetranslation.dll"
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  scripthappy
; re-enable this when we actually build it
;Section "$(TITLE_section_toolsplugins_scripthappy)" section_toolsplugins_scripthappy
;	SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT} ; Typical, Full w/ assoc, Full, Full w/ downloads
;	SetOutPath $INSTDIR\plugins
;	File "${ABIWORD_COMPILED_PATH}\plugins\PluginScripthappy.dll"
;SectionEnd
;
;!macro Remove_${section_toolsplugins_scripthappy}
;	;Removes this component
;	DetailPrint "*** Removing or skipping install of tool plugin: scripthappy ..."
;
;	; remove plugin and related files
;	Delete "$INSTDIR\plugins\PluginScripthappy.dll"
;!macroend




SubSectionEnd ; Tools Plugins
!macro Remove_${ssection_toolsplugins}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_toolsplugins"


!macroend
