;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Optional [download/included] Dictionary sections/functions/macros

; Copyright (C) 2008 AbiSource Corporation B.V.

!ifndef NODOWNLOADS
; NOTE: If downloads are enabled (NODOWNLOADS undefined) then the dictionary
; list just references files for download, once downloaded we install(extract) them;
; otherwise the dictionary files are included within the installer (makes it huge!).
; The dictionaries are included within the same section as the always included
; English dictionary if included within the installer, but placed in their own
; subsection when referenced as downloads, hence the SubSection within the !ifndef chunk.

!include "abi_util_dl.nsh"

; WARNING: ${ssection_dl_opt_dict}+1 is assumed to be 1st section of downloadable dictionaries
SubSection /e "$(TITLE_ssection_dl_opt_dict)" ssection_dl_opt_dict

; creates an .ini file for use by custom download InstallOption dialog
; $R0 is set to temp file used
; $R1 is default/fallback entry
; $R2 is URL list except for default entry
Function createDLIni
  ; create .ini file used for custom dialog
  GetTempFileName $R0

  ; write out our fields  
  WriteINIStr $R0 "Settings" NumFields "2"
  WriteINIStr $R0 "Settings" CancelEnabled "1"
  WriteINIStr $R0 "Settings" CancelShow "1"
  WriteINIStr $R0 "Settings" BackEnabled "0"

  WriteINIStr $R0 "Field 1" Type "Label"
  WriteINIStr $R0 "Field 1" Text "$(MSG_SELECT_DL_MIRROR)"
  WriteINIStr $R0 "Field 1" Left   "0"
  WriteINIStr $R0 "Field 1" Right  "-1"
  WriteINIStr $R0 "Field 1" Top    "15"
  WriteINIStr $R0 "Field 1" Bottom "35"

  WriteINIStr $R0 "Field 2" Type "combobox"
  WriteINIStr $R0 "Field 2" Text "sel"
  WriteINIStr $R0 "Field 2" Left   "0"
  WriteINIStr $R0 "Field 2" Right  "-1"
  WriteINIStr $R0 "Field 2" Top    "39"
  WriteINIStr $R0 "Field 2" Bottom "-1"
  WriteINIStr $R0 "Field 2" minLen "8"
  WriteINIStr $R0 "Field 2" listItems "$R1|$R2"
  WriteINIStr $R0 "Field 2" state "$R1"
FunctionEnd

; determines where to download from
; $R9 is set to base URL
Function getDLMirror
  ; save callees registers
  Push $R0
  Push $R1
  Push $R2

  ; see if any of the DL dictionaries are selected (subsection partially selected)
  !insertmacro SectionFlagIsSet ${ssection_dl_opt_dict} ${SF_PSELECTED} isSel checkAll
  checkAll: !insertmacro SectionFlagIsSet ${ssection_dl_opt_dict} ${SF_SELECTED} isSel noUpDate
  isSel:

!ifndef CLASSIC_UI
  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"
!else
  ;Set text on the white rectangle
  Push $R0

    GetDlgItem $R0 $HWNDPARENT 1037
    SendMessage $R0 ${WM_SETTEXT} 0 "STR:$(TEXT_IO_TITLE)"
    GetDlgItem $R0 $HWNDPARENT 1038
    SendMessage $R0 ${WM_SETTEXT} 0 "STR:$(TEXT_IO_SUBTITLE)"

  Pop $R0
!endif


  ; "Selecting download mirror ..."
  ; use install options to allow custom entry or select from list

  ; create the inifile used to specify our custom dialog
  StrCpy $R1 "${DICTIONARY_BASE_DEFAULT}"
  StrCpy $R2 "${DICTIONARY_BASE_MIRRORS}"
  Call createDLIni ; sets $R0 to inifilename

  ; create the dialog and wait for user's response
  ; for now manually call, as the macro doesn't work as expected
  ;  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "$R0"
  InstallOptions::dialog $R0

  ; pop return status and use default value on anything other than success
  ; else read back user's choice
  Pop $R9
  StrCmp $R9 "success" 0 useDefaultURL
    ReadINIStr $R9 $R0 "Field 2" State ; $R9 = URL field's state
    Goto Next1
  useDefaultURL:
    DetailPrint "$(MSG_ERROR_SELECTING_DL_MIRROR)"
    StrCpy $R9 "${DICTIONARY_BASE_DEFAULT}"
  Next1:

  ; remove .ini file
  Delete $R0

  ; $R9 is the base URL chosen

  noUpDate:

  ; restore callees registers
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd


!endif  ; NODOWNLOADS

; $R3 is set to the filename used
Function getDictionary
      !define DICTIONARY_BASE $R9

	!define DICT_LANG $R0
	!define DICT_LOCALE $R1
	!define DICT_ARCH $R2

!ifndef NODOWNLOADS
	; set filename, handle files without locale portion (represented by Xx)
	StrCmp ${DICT_LOCALE} "Xx" noLocale 0
		StrCpy $R3 "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
      Goto Skip1
	noLocale:
		StrCpy $R3 "abispell-${DICT_LANG}.${DICT_ARCH}.tar.gz"
	Skip1:
!else ; for files included in setup we just leave the -Xx tacked on
		StrCpy $R3 "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
!endif
	!define DICT_FILENAME $R3


!ifndef NODOWNLOADS
	; download the file
	${dlFile} "${DICTIONARY_BASE}/${DICT_FILENAME}" "$TEMP\${DICT_FILENAME}" "Failed to download requested dictionary ${DICTIONARY_BASE}/${DICT_FILENAME}"
	StrCmp $0 "success" doDictInst Finish
!endif

	doDictInst:
		; Unzip dictionary into dictionary subdirecotry
		untgz::extract "-j" "-d" "$INSTDIR\dictionary" "$TEMP\${DICT_FILENAME}"
		StrCmp $0 "success" doCleanup
			DetailPrint "  Failed to extract ${DICT_FILENAME}"
			MessageBox MB_OK|MB_ICONEXCLAMATION|MB_DEFBUTTON1 "  Failed to extract ${DICT_FILENAME}"
		
		doCleanup:
		; Delete temporary files
		Delete "$TEMP\${DICT_FILENAME}"

	Finish:
		!undef DICT_LANG
		!undef DICT_LOCALE
		!undef DICT_ARCH
FunctionEnd


; used to define a section containing an optional dictionary for downloading/installation
!macro SectionDict DICT_NAME DICT_LANG DICT_LOCALE DICT_ARCH DICT_SIZE
Section '${DICT_LANG}-${DICT_LOCALE}  $(dict_${DICT_NAME})' section_dl_opt_dict_${DICT_LANG}_${DICT_LOCALE}
!ifdef NODOWNLOADS
	SectionIn ${FULLSECT} ${FULLASSOCSECT} ${DLSECT}	; Full [and Full with downloads] only
	SetOutPath $TEMP
	File "abispell-${DICT_LANG}-${DICT_LOCALE}.${DICT_ARCH}.tar.gz"
!else
	SectionIn ${DLSECT}	; Full with downloads only
	AddSize ${DICT_SIZE}
!endif

	DetailPrint "Installing dictionary for: '${DICT_LANG}-${DICT_LOCALE}  $(dict_${DICT_NAME})'"

	StrCpy $R0 ${DICT_LANG}
	StrCpy $R1 ${DICT_LOCALE}
	StrCpy $R2 ${DICT_ARCH}
	Call getDictionary
SectionEnd
!macroend
!define SectionDict "!insertmacro SectionDict"


!include "abi_dict_list.nsh"


!ifndef NODOWNLOADS
SubSectionEnd ; DL Optional downloads
!endif


!ifndef NODOWNLOADS
; used later to set description or disable
!macro cycle_over_dictionary_sections CMD
		; we !!!assume!!! that section# of 1st dictionary section is # of dictionary subsection + 1
		push $R1	; the 1st section
		push $R2	; the last section
		StrCpy $R1 ${ssection_dl_opt_dict}
		IntOp $R1 $R1 + 1                   ; order here
		IntOp $R2 $R1 + ${DICTIONARY_COUNT}	; matters  $R2 = ${ssection_dl_opt_dict} + 1 + ${DICTIONARY_COUNT}
		; $R1=section of 1st downloadable dictionary
		"loop_start:"
			${CMD}
			IntOp $R1 $R1 + 1  
			IntCmpU $R1 $R2 "loop_end"
		Goto "loop_start"
		"loop_end:"
		pop $R2
		pop $R1
!macroend
!endif


; End of file
