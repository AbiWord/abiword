;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Set descriptions for components when using Modern UI


!define MUI_DESCRIPTION_TEXT "!insertmacro MUI_DESCRIPTION_TEXT"

; add the section and subsection descriptions (scroll over text)
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		${MUI_DESCRIPTION_TEXT} ${section_abi} $(DESC_section_abi)
		${MUI_DESCRIPTION_TEXT} ${section_abi_req} $(DESC_section_abi_req)

		!ifdef CLASSIC_UI
		${MUI_DESCRIPTION_TEXT} ${ssection_shortcuts} $(DESC_ssection_shortcuts)
		${MUI_DESCRIPTION_TEXT} ${ssection_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		${MUI_DESCRIPTION_TEXT} ${section_sm_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		${MUI_DESCRIPTION_TEXT} ${section_desktop_shortcuts_cu} $(DESC_ssection_shortcuts_cu)
		${MUI_DESCRIPTION_TEXT} ${ssection_shortcuts_au} $(DESC_ssection_shortcuts_au)
		${MUI_DESCRIPTION_TEXT} ${section_sm_shortcuts_au} $(DESC_ssection_shortcuts_au)
		${MUI_DESCRIPTION_TEXT} ${section_desktop_shortcuts_au} $(DESC_ssection_shortcuts_au)
		!endif

		${MUI_DESCRIPTION_TEXT} ${ssection_core} $(DESC_ssection_core)
		${MUI_DESCRIPTION_TEXT} ${ssection_fa_shellupdate} $(DESC_ssection_fa_shellupdate)
		${MUI_DESCRIPTION_TEXT} ${section_fa_abw} $(DESC_section_fa_abw)
		;${MUI_DESCRIPTION_TEXT} ${section_fa_awt} $(DESC_section_fa_awt)
		;${MUI_DESCRIPTION_TEXT} ${section_fa_zabw} $(DESC_section_fa_zabw)
		${MUI_DESCRIPTION_TEXT} ${section_fa_doc} $(DESC_section_fa_doc)
		${MUI_DESCRIPTION_TEXT} ${section_fa_rtf} $(DESC_section_fa_rtf)
		${MUI_DESCRIPTION_TEXT} ${ssection_helper_files} $(DESC_ssection_helper_files)
		${MUI_DESCRIPTION_TEXT} ${section_help} $(DESC_section_help)
		${MUI_DESCRIPTION_TEXT} ${section_templates} $(DESC_section_templates)
		${MUI_DESCRIPTION_TEXT} ${section_clipart} $(DESC_section_clipart)

!ifdef OPT_CRTL_LOCAL
		${MUI_DESCRIPTION_TEXT} ${section_crtlib_local} $(DESC_section_crtlib)
!endif

!ifndef NODOWNLOADS
!ifdef OPT_CRTL_URL
		${MUI_DESCRIPTION_TEXT} ${section_crtlib_dl} $(DESC_section_crtlib)
!endif

		${MUI_DESCRIPTION_TEXT} ${ssection_dictionary} $(DESC_ssection_dictionary)
		${MUI_DESCRIPTION_TEXT} ${section_dictinary_def_English} $(DESC_ssection_dictionary)
!ifdef OPT_DICTIONARIES
		${MUI_DESCRIPTION_TEXT} ${ssection_dl_opt_dict} $(DESC_ssection_dictionary)
!endif ; OPT_DICTIONARIES
!endif ; NODOWNLOADS

!ifdef OPT_DICTIONARIES
		!insertmacro cycle_over_dictionary_sections "${MUI_DESCRIPTION_TEXT} $R1 $(DESC_ssection_dictionary)"
!endif

!ifdef OPT_PLUGINS
		${MUI_DESCRIPTION_TEXT} ${ssection_plugins} $(DESC_ssection_plugins)
!macro PLUGIN_FUNC id
		${MUI_DESCRIPTION_TEXT} ${${id}} $(DESC_${id})
!macroend
		;include "plugins\plugin_list.nsh"
!endif


!insertmacro MUI_FUNCTION_DESCRIPTION_END

