;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       functions/macros/etc for supporting multi-language text within installer


!ifndef _ABI_LNG_MISC_NSH_
!define _ABI_LNG_MISC_NSH_

  ; Languages, include MUI & NSIS language support
  ; then include app install specific language support

    ; indicate default language definitions to use if a translation is missing a string
    !define DEF_LANG "ENGLISH"

    ; actually sets the LangString
    !macro SETLSTR NAME VALUE	; e.g. English sectID sectDesc
	!echo "${LANG} ( ${LANG_${LANG}} )"
      !define "STRING_ISSET_${LANG}_${NAME}"
      LangString "${NAME}" "${LANG_${LANG}}" "${VALUE}"
    !macroend
    !define SETLSTR "!insertmacro SETLSTR"

    ; macro to set string, assumes LANG already defined (call within context of LANG_LOAD)
    !macro LSTR NAME VALUE	; e.g. sectID sectDesc
      !ifdef SETDEFLANG
        ; if string is already set, we do nothing, otherwise we set to default value and warn user
        !ifndef "STRING_ISSET_${LANG}_${NAME}"
          !ifndef APPSET_LANGUAGEFILE_DEFAULT_USED     ; flag default value must be used
            !define APPSET_LANGUAGEFILE_DEFAULT_USED
          !endif
          ${SETLSTR} "${NAME}" "${VALUE}"  ; set to default value
        !endif
      !else ; just set the value
        ${SETLSTR} "${NAME}" "${VALUE}"
      !endif
    !macroend
    !define LSTR "!insertmacro LSTR"

    ; macro to include necessary language files
    ; Usage: 
    ;       ${LANG_LOAD} "<nsis language name>"
    ;  e.g. ${LANG_LOAD} "English"
    ;
    !macro LANG_LOAD LANG
      !insertmacro MUI_LANGUAGE "${LANG}"
      !echo "Loading language ${LANG} ( ${LANG_${LANG}} )"
      ; Specify the license text to use (for multilang support, must come after MUI_LANGUAGE)
      LicenseLangString LicenseTXT "${LANG_${LANG}}" "..\AbiSuite\Copying"
      !verbose push
      !verbose 3
      !include "lang\${LANG}.nsh"   ; Localized Installer Messages (Language Strings)
      !define SETDEFLANG
      !include "lang\${DEF_LANG}.nsh"
      !verbose pop
      !ifdef APPSET_LANGUAGEFILE_DEFAULT_USED
        !undef APPSET_LANGUAGEFILE_DEFAULT_USED
        !warning "${LANG} Installation language file incomplete.  Using default texts for missing strings."
      !endif
      !undef SETDEFLANG
      !echo "End loading language ${LANG}"
      !undef LANG
    !macroend
    !define LANG_LOAD "!insertmacro LANG_LOAD"

!endif ; _ABI_LNG_MISC_NSH_
; End of file
