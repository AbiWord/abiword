;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;SimpChinese ${LANG_SimpChinese}

!undef LANG_X
!define LANG_X ${LANG_SimpChinese}


; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "SimpChinese:Primary components"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (required)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} support files (required)"
${LSTR} TITLE_section_shellupdate           "Update Registry Settings"
${LSTR} TITLE_ssection_shortcuts            "Shortcuts"
${LSTR} TITLE_ssection_shortcuts_cu         "Shortcuts (Current User)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Start Menu Shortcuts (Current User)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Desktop Shortcut (Current User)"
${LSTR} TITLE_ssection_shortcuts_au         "Shortcuts (All Users)"
${LSTR} TITLE_section_sm_shortcuts_au       "Start Menu Shortcuts (All Users)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Desktop Shortcut (All Users)"
${LSTR} TITLE_ssection_gen_file_assoc       "General file associations"
${LSTR} TITLE_section_fa_doc                "Associate .doc with AbiWord"
${LSTR} TITLE_section_fa_rtf                "Associate .rtf with AbiWord"
${LSTR} TITLE_ssection_helper_files         "Helper files"
${LSTR} TITLE_section_help                  "Help Files"
${LSTR} TITLE_section_templates             "Templates"
;${LSTR} TITLE_section_samples               "Samples"
${LSTR} TITLE_section_clipart               "Clipart"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Download CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Dictionaries"
${LSTR} TITLE_section_dictinary_def_English "en-US  US English (default)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Download optional dictionaries"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Plugins"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Primary (core) set of components for AbiWord to run well."
${LSTR} DESC_section_abi              "Required.  Installs the actual ${PROGRAMEXE} program."
${LSTR} DESC_section_abi_req          "Required.  Installs the basic support files, e.g. stringsets, BMP clipboard support, etc."
${LSTR} DESC_section_shellupdate      "Adds entries to the Windows registry to allow the shell (Explorer) to handle supported file formats."
${LSTR} DESC_ssection_shortcuts       "Installs shortcuts in various places to allow starting AbiWord through additional locations."
${LSTR} DESC_ssection_shortcuts_cu    "Installs shortcuts for the currently logged on user."
${LSTR} DESC_ssection_shortcuts_au    "Installs shortcuts for all users (or current user on systems without multiple users)."
${LSTR} DESC_ssection_gen_file_assoc  "Associates various documents with AbiWord, so AbiWord will be used to open them."
${LSTR} DESC_section_fa_doc           "Specifies that AbiWord should be used to open Microsoft Word (R) native format documents."
${LSTR} DESC_section_fa_rtf           "Specifies that AbiWord should be used to open Rich Text Files, a 'standard' format for WordProcessors."
${LSTR} DESC_ssection_helper_files    "Installs various optional files to aid in using AbiWord."
${LSTR} DESC_section_help             "Installs the help documents, no help is available if this is omitted."
${LSTR} DESC_section_templates        "Installs templates that can be used to aid in creation of new documents with predefined formatting."
${LSTR} DESC_section_samples          "Samples have been removed."
${LSTR} DESC_section_clipart          "Installs pictures (clipart) that can be inserted into documents."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Installs the C Runtime Library used by AbiWord, useful if your system lacks this already."
!endif
${LSTR} DESC_ssection_dictionary      "Installs dictionaries for various languages that are used to spell check your document."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Installs various optional plugins."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE              "Overwrite Existing ${PRODUCT}?"
${LSTR} MSG_ABORT                     "Quitting the install process"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Failed to download requested c runtime library (DLL): ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Optional Downloadable Components Base URL"
${LSTR} TEXT_IO_SUBTITLE              "Dictionaries"
${LSTR} MSG_SELECT_DL_MIRROR          "Select download mirror..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Error obtaining user choice, using default site!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} Word Processor"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Uninstall ${PRODUCT} v${VERSION_MAJOR}"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "This will delete $INSTDIR and all subdirectories and files?"


; End Language descriptions
