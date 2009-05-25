;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
; English ${LANG_English}

; Copyright (C) 2008 AbiSource Corporation B.V.

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Primary components"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (required)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} support files (required)"
${LSTR} TITLE_ssection_shortcuts            "Shortcuts"
${LSTR} TITLE_ssection_shortcuts_cu         "Shortcuts (Current User)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Start Menu Shortcuts (Current User)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Desktop Shortcut (Current User)"
${LSTR} TITLE_ssection_shortcuts_au         "Shortcuts (All Users)"
${LSTR} TITLE_section_sm_shortcuts_au       "Start Menu Shortcuts (All Users)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Desktop Shortcut (All Users)"
${LSTR} TITLE_ssection_fa_shellupdate       "Update shell file associations"
${LSTR} TITLE_section_fa_abw                "Open .abw, .awt, .zabw with AbiWord"
${LSTR} TITLE_section_fa_awt                "Associate .awt with AbiWord"
${LSTR} TITLE_section_fa_zabw               "Associate .zabw with AbiWord"
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


; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Primary (core) set of components for AbiWord to run well."
${LSTR} DESC_section_abi              "Required.  Installs the actual ${PROGRAMEXE} program."
${LSTR} DESC_section_abi_req          "Required.  Installs the basic support files, e.g. stringsets, BMP clipboard support, etc."
${LSTR} DESC_ssection_shortcuts       "Installs shortcuts in various places to allow starting AbiWord through additional locations."
${LSTR} DESC_ssection_shortcuts_cu    "Installs shortcuts for the currently logged on user."
${LSTR} DESC_ssection_shortcuts_au    "Installs shortcuts for all users (or current user on systems without multiple users)."
${LSTR} DESC_ssection_fa_shellupdate  "Adds entries to the registry to allow the Explorer shell to use AbiWord to open various document formats."
${LSTR} DESC_section_fa_abw           "Specifies that AbiWord should be used to open documents in its native formats.  (Recommended)"
${LSTR} DESC_section_fa_awt           "Specifies that AbiWord should be used to open templates in its native format.  (Recommended)"
${LSTR} DESC_section_fa_zabw          "Specifies that AbiWord should be used to open compressed documents in its native format.  (Recommended)"
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
${LSTR} PROMPT_OVERWRITE                      "Overwrite Existing ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} does not appear installed correctly!$\r$\n\
                                               Failed to find ${MAINPROGRAM}, it will be reinstalled.$\r$\n\
                                               Continue to modify installation?"
${LSTR} MSG_ABORT                             "Quitting the install process"

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
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Uninstall ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "(English) Help for ${PRODUCT}"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "This will delete $INSTDIR and all subdirectories and files?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Catalan"
${LSTR} dict_Czech         "Czech"
${LSTR} dict_Danish        "Danish"
${LSTR} dict_Swiss         "Swiss"
${LSTR} dict_Deutsch       "German"
${LSTR} dict_Ellhnika      "Greek"
${LSTR} dict_English       "English (GB)"
${LSTR} dict_American      "English (US)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Spanish"
${LSTR} dict_Finnish       "Finnish"
${LSTR} dict_Francais      "French"
${LSTR} dict_Hungarian     "Hungarian"
${LSTR} "dict_Irish gaelic""Irish gaelic"
${LSTR} dict_Galician      "Galician"
${LSTR} dict_Italian       "Italian"
${LSTR} dict_Kurdish       "Kurdish"
${LSTR} dict_Latin         "Latin"
${LSTR} dict_Lietuviu      "Lithuanian"
${LSTR} dict_Dutch         "Dutch"
${LSTR} dict_Norsk         "Norwegian Bokmål"
${LSTR} dict_Nynorsk       "Norwegian Nynorsk"
${LSTR} dict_Polish        "Polish"
${LSTR} dict_Portugues     "Portuguese"
${LSTR} dict_Brazilian     "Brazilian"
${LSTR} dict_Russian       "Russian"
${LSTR} dict_Sardinian     "Sardinian"
${LSTR} dict_Slovensko     "Slovenian"
${LSTR} dict_Svenska       "Swedish"
${LSTR} dict_Ukrainian     "Ukrainian"

!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Plugins"
${LSTR} TITLE_ssection_toolsplugins         "Optional Tools"
${LSTR} TITLE_ssection_impexpplugins        "Additional File Format Support"

; Tools Plugins
${LSTR} TITLE_section_toolsplugins_mathview				"Equation Editor"
${LSTR} TITLE_section_toolsplugins_abicollab			"AbiCollab Real Time Collaboration"
${LSTR} TITLE_section_toolsplugins_grammar				"Grammar Checker"
${LSTR} TITLE_section_toolsplugins_urldict				"Web Dictionary"
${LSTR} TITLE_section_toolsplugins_google				"Google Search Integration"
${LSTR} TITLE_section_toolsplugins_wikipedia			"Wikipedia Search Integration"
${LSTR} TITLE_section_toolsplugins_babelfish			"Babelfish Internet Translation"
${LSTR} TITLE_section_toolsplugins_freetranslation		"Freetranslation.com Internet Translation"
${LSTR} TITLE_section_toolsplugins_scripthappy			"Command Output Scripting"

; Imp/Exp Plugins
${LSTR} TITLE_ssection_loadsaveplugins					"Load and Save Formats"
${LSTR} TITLE_ssection_loadplugins						"Load-only Formats"
${LSTR} TITLE_ssection_saveplugins						"Save-only Formats"

${LSTR} TITLE_section_impexpplugins_applix				"ApplixWare"
${LSTR} TITLE_section_impexpplugins_clarisworks			"ClarisWorks"
${LSTR} TITLE_section_impexpplugins_docbook				"DocBook"
${LSTR} TITLE_section_impexpplugins_opendocument		"OpenDocument (ODT)"
${LSTR} TITLE_section_impexpplugins_openwriter			"OpenWriter (.sxw)"
${LSTR} TITLE_section_impexpplugins_iscii_text			"Indic Script (ISCII) Text"
${LSTR} TITLE_section_impexpplugins_eml					"Saved Email (.eml) format"
${LSTR} TITLE_section_impexpplugins_palmdoc				"Palm DOC (.pdb)"
${LSTR} TITLE_section_impexpplugins_wml					"Wireless Markup Language"
${LSTR} TITLE_section_impexpplugins_xslfo				"XSL-FO"

${LSTR} TITLE_section_impexpplugins_openxml				"Microsoft Office Open XML"
${LSTR} TITLE_section_impexpplugins_mswrite				"Microsoft Write"
${LSTR} TITLE_section_impexpplugins_opml				"OPML"
${LSTR} TITLE_section_impexpplugins_sdw					"StarOffice Writer 5.1 (.sdw)"
${LSTR} TITLE_section_impexpplugins_t602				"T602"
${LSTR} TITLE_section_impexpplugins_wordperfect			"WordPerfect"

${LSTR} TITLE_section_impexpplugins_hrtext				"Newsgroup text (Hrtext)"
${LSTR} TITLE_section_impexpplugins_latex				"LaTeX"


!endif
; End Language descriptions
