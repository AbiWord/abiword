;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;Italian ${LANG_Italian}

!undef LANG_X
!define LANG_X ${LANG_Italian}


; Section titles, what user sees to select components for installation
LangString TITLE_ssection_core                 ${LANG_X} "Italian:Primary components"
LangString TITLE_section_abi                   ${LANG_X} "${PROGRAMEXE} (required)"
Langstring TITLE_section_shellupdate           ${LANG_X} "Update Registry Settings"
LangString TITLE_ssection_shortcuts            ${LANG_X} "Shortcuts"
LangString TITLE_ssection_shortcuts_cu         ${LANG_X} "Shortcuts (Current User)"
LangString TITLE_section_sm_shortcuts_cu       ${LANG_X} "Start Menu Shortcuts (Current User)"
LangString TITLE_section_desktop_shortcuts_cu  ${LANG_X} "Desktop Shortcut (Current User)"
LangString TITLE_ssection_shortcuts_au         ${LANG_X} "Shortcuts (All Users)"
LangString TITLE_section_sm_shortcuts_au       ${LANG_X} "Start Menu Shortcuts (All Users)"
LangString TITLE_section_desktop_shortcuts_au  ${LANG_X} "Desktop Shortcut (All Users)"
LangString TITLE_ssection_gen_file_assoc       ${LANG_X} "General file associations"
LangString TITLE_section_fa_doc                ${LANG_X} "Associate .doc with AbiWord"
LangString TITLE_section_fa_rtf                ${LANG_X} "Associate .rtf with AbiWord"
LangString TITLE_ssection_helper_files         ${LANG_X} "Helper files"
LangString TITLE_section_help                  ${LANG_X} "Help Files"
LangString TITLE_section_templates             ${LANG_X} "Templates"
;LangString TITLE_section_samples               ${LANG_X} "Samples"
LangString TITLE_section_clipart               ${LANG_X} "Clipart"
!ifdef OPT_CRTL_LOCAL
LangString TITLE_section_crtlib_local          ${LANG_X} "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
LangString TITLE_section_crtlib_dl             ${LANG_X} "Download CRTlib ${OPT_CRTL_FILENAME}"
!endif
LangString TITLE_ssection_dictionary           ${LANG_X} "Dictionaries"
LangString TITLE_section_dictinary_def_English ${LANG_X} "en-US  US English (default)"
!ifdef OPT_DICTIONARIES
LangString TITLE_ssection_dl_opt_dict          ${LANG_X} "Download optional dictionaries"
!endif
!ifdef OPT_PLUGINS
LangString TITLE_ssection_plugins              ${LANG_X} "Plugins"
!endif

; Section descriptions displayed to user when mouse hovers over a section
LangString DESC_ssection_core            ${LANG_X} "Primary (core) set of components for AbiWord to run well."
LangString DESC_section_abi              ${LANG_X} "Required.  Installs the actual ${PROGRAMEXE} program."
Langstring DESC_section_shellupdate      ${LANG_X} "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
LangString DESC_ssection_shortcuts       ${LANG_X} "Installs shortcuts in various places to allow starting AbiWord through additional locations."
LangString DESC_ssection_shortcuts_cu    ${LANG_X} "Installs shortcuts for the currently logged on user."
LangString DESC_ssection_shortcuts_au    ${LANG_X} "Installs shortcuts for all users (or current user on systems without multiple users)."
LangString DESC_ssection_gen_file_assoc  ${LANG_X} "Associates various documents with AbiWord, so AbiWord will be used to open them."
LangString DESC_section_fa_doc           ${LANG_X} "Specifies that AbiWord should be used to open Microsoft Word (R) native format documents."
LangString DESC_section_fa_rtf           ${LANG_X} "Specifies that AbiWord should be used to open Rich Text Files, a 'standard' format for WordProcessors."
LangString DESC_ssection_helper_files    ${LANG_X} "Installs various optional files to aid in using AbiWord."
LangString DESC_section_help             ${LANG_X} "Installs the help documents, no help is available if this is omitted."
LangString DESC_section_templates        ${LANG_X} "Installs templates that can be used to aid in creation of new documents with predefined formatting."
;LangString DESC_section_samples          ${LANG_X} "Samples have been removed."
LangString DESC_section_clipart          ${LANG_X} "Installs pictures (clipart) that can be inserted into documents."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
LangString DESC_section_crtlib           ${LANG_X} "Installs the C Runtime Library used by AbiWord, useful if your system lacks this already."
!endif
LangString DESC_ssection_dictionary      ${LANG_X} "Installs dictionaries for various languages that are used to spell check your document."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
LangString DESC_ssection_plugins         ${LANG_X} "Installs various optional plugins."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
LangString PROMPT_OVERWRITE ${LANG_X} "Overwrite Existing ${PRODUCT}?"
LangString MSG_ABORT        ${LANG_X} "Quitting the install process"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
LangString PROMPT_CRTL_DL_FAILED   ${LANG_X} "Failed to download requested c runtime library (DLL): ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
LangString TEXT_IO_TITLE                 ${LANG_X} "Optional Downloadable Components Base URL"
LangString TEXT_IO_SUBTITLE              ${LANG_X} "Dictionaries"
LangString MSG_SELECT_DL_MIRROR          ${LANG_X} "Select download mirror..."
LangString MSG_ERROR_SELECTING_DL_MIRROR ${LANG_X} "Error obtaining user choice, using default site!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
LangString SM_PRODUCT_GROUP        ${LANG_X} "${PRODUCT} Word Processor"
LangString SHORTCUT_NAME           ${LANG_X} "${PRODUCT} v${VERSION_MAJOR}"
LangString SHORTCUT_NAME_UNINSTALL ${LANG_X} "Uninstall ${PRODUCT} v${VERSION_MAJOR}"

; Uninstall Strings
LangString UNINSTALL_WARNING ${LANG_X} "This will delete $INSTDIR and all subdirectories and files?"


; End Language descriptions
