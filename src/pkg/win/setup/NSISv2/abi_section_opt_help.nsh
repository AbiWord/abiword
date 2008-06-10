;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for help files

; Copyright (C) 2008 AbiSource Corporation B.V.

; OPTIONAL Installation of Help Files
Section "$(TITLE_section_help)" section_help
  SectionIn ${TYPICALSECT} ${FULLASSOCSECT} ${FULLSECT} ${DLSECT}
  SetOutPath $INSTDIR\AbiWord

  ; help documents may not be created if peer abiword-docs not found
  File /nonfatal /r "..\abisuite\abiword\help"

  SetOutPath $INSTDIR\AbiWord\help\en-US
  File "..\..\..\credits.txt"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    !define HelpFileIndex "$INSTDIR\AbiWord\help\en-US\index.html"
    IfFileExists ${HelpFileIndex} 0 SkipHelpFileSM	; only add if index file installed on User's computer
      ${lngCreateShortCut} "$SMPROGRAMS" "$STARTMENU_FOLDER" "$(SHORTCUT_NAME_HELP)" "${HelpFileIndex}" "" "" 0
    SkipHelpFileSM:
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd
!macro Remove_${section_help}
	;Removes this component
	DetailPrint "*** Removing help files..."

	; remove help files
      RMDir /r "$INSTDIR\AbiWord\help"

	; remove help start menu entry
	Delete "$SMPROGRAMS\$STARTMENU_FOLDER\$(SHORTCUT_NAME_HELP).lnk"
!macroend

