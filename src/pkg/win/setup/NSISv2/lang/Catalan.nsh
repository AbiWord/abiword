;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;Catalan ${LANG_Catalan}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Components principals"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (necessari)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} fitxers de suport (necessari)"
${LSTR} TITLE_ssection_shortcuts            "Dreceres"
${LSTR} TITLE_ssection_shortcuts_cu         "Dreceres (usuari actual)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Dreceres al Menu d'Inici (usuari actual)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Drecera a l'escriptori (usuari actual)"
${LSTR} TITLE_ssection_shortcuts_au         "Shortcuts (All Users)"
${LSTR} TITLE_section_sm_shortcuts_au       "Start Menu Shortcuts (tots els usuaris)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Drecera a l'escriptori (tots els usuaris)"
${LSTR} TITLE_ssection_fa_shellupdate       "Associacions de fitxers"
${LSTR} TITLE_section_fa_abw                "Associa fitxers .abw amb l'AbiWord"
${LSTR} TITLE_section_fa_awt                "Associa fitxers .awt amb l'AbiWord"
${LSTR} TITLE_section_fa_zabw               "Associa fitxers .zabw amb l'AbiWord"
${LSTR} TITLE_section_fa_doc                "Associa fitxers .doc amb l'AbiWord"
${LSTR} TITLE_section_fa_rtf                "Associa fitxers .rtf amb l'AbiWord"
${LSTR} TITLE_ssection_helper_files         "Fitxers complementaris"
${LSTR} TITLE_section_help                  "Fitxers d'ajuda"
${LSTR} TITLE_section_templates             "Plantilles"
;${LSTR} TITLE_section_samples              "Exemples"
${LSTR} TITLE_section_clipart               "Fitxers d'imatges"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Download CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Diccionaris"
${LSTR} TITLE_section_dictinary_def_English "Anglès, català, francès i espanyol"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Baixada opcional de diccionaris"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Plugins"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Conjunt primari (central) d'elements perquè l'AbiWord funcioni bé."
${LSTR} DESC_section_abi              "Necessari.  Instal·la el programa ${PROGRAMEXE} actual."
${LSTR} DESC_section_abi_req          "Necessari.  Instal·la els fitxers de suport bàsics, p.ex. conjunts de cadenes, suport de porta-retalls BMP, etc."
${LSTR} DESC_ssection_shortcuts       "Instal·la dreceres a varis emplaçaments per permetre iniciar l'AbiWord a través d'ubicacions adicionals."
${LSTR} DESC_ssection_shortcuts_cu    "Instal·la dreceres per a l'usuari connectat en aquest moment."
${LSTR} DESC_ssection_shortcuts_au    "Instal·la dreceres per a tots els usuaris (o l'usuari actual en sistemes sense usuaris múltiples)."
${LSTR} DESC_ssection_fa_shellupdate  "Afegeix entrades al registre per permetre a Windows utilitzar l'AbiWord per obrir varis formats de document."
${LSTR} DESC_section_fa_abw           "Especifica que l'Abiword s'hauria d'utilitzar per obrir documents en el seu format originari.  (Recomanat)"
${LSTR} DESC_section_fa_awt           "Especifica que l'Abiword s'hauria d'utilitzar per obrir plantilles en el seu format originari.  (Recomanat)"
${LSTR} DESC_section_fa_zabw          "Especifica que l'Abiword s'hauria d'utilitzar per obrir documents comprimits en el seu format originari.  (Recomanat)"
${LSTR} DESC_section_fa_doc           "Especifica que l'Abiword s'hauria d'utilitzar per obrir documents de Microsoft Word (R) en format originari."
${LSTR} DESC_section_fa_rtf           "Especifica que l'Abiword s'hauria d'utilitzar per obrir fitxers en format de text enriquit."
${LSTR} DESC_ssection_helper_files    "Instal·la varis fitxers opcionals per ajudar a utilitzar l'AbiWord."
${LSTR} DESC_section_help             "Instal·la els documents d'ajuda, si s'omet, no hi ha ajuda disponible."
${LSTR} DESC_section_templates        "Instal·la plantilles que es poden utilitzar per ajudar a crear nous documents amb format predefinit."
${LSTR} DESC_section_samples          "S'han extret les mostres."
${LSTR} DESC_section_clipart          "Instal·la imatges (fitxer d'imatges) que es poden inserir en documents."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Instal·la la biblioteca de temps d'execució en C (C Runtime Library) utilitzada per l'AbiWord, útil si el sistema ja no ho té."
!endif
${LSTR} DESC_ssection_dictionary      "Instal·la diccionaris per a vàries llengües que s'utilitzen per verificar l'ortografia del document."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Instal·la varis connectors opcionals."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Voleu sobreescriure ${PRODUCT} existent?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "Sembla que ${PRODUCT} no s'ha instal·lat correctament!$\r$\n\
                                               És impossible trobar ${MAINPROGRAM}, es tornarà a instal·lar.$\r$\n\
                                               Voleu continuar per modificar la instal·lació?"
${LSTR} MSG_ABORT                             "S'està sortint del procés d'instal·lació"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "És impossible baixar la c runtime library (DLL) sol·licitada: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Adreça web per la baixada de components opcionals"
${LSTR} TEXT_IO_SUBTITLE              "Diccionaris"
${LSTR} MSG_SELECT_DL_MIRROR          "Selecciona la rèplica de baixada..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "S'ha produït un error en obtenir l'opció de l'usuari, en utilitzar el lloc per defecte!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} processador de textos"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Desinstal·la ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "(English) Ajuda per a ${PRODUCT}"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Això suprimirà $INSTDIR i tots els subdirectoris i fitxers?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Català"
${LSTR} dict_Czech         "Txec"
${LSTR} dict_Danish        "Danès"
${LSTR} dict_Swiss         "Suís"
${LSTR} dict_Deutsch       "Alemany"
${LSTR} dict_Ellhnika      "Grec"
${LSTR} dict_English       "Anglès (GB)"
${LSTR} dict_American      "Anglès (US)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Espanyol"
${LSTR} dict_Finnish       "Finès"
${LSTR} dict_Francais      "Francès"
${LSTR} dict_Hungarian     "Hongarès"
${LSTR} "dict_Irish gaelic""Irlandès"
${LSTR} dict_Galician      "Gallec"
${LSTR} dict_Italian       "Italià"
${LSTR} dict_Latin         "Llatí"
${LSTR} dict_Lietuviu      "Lituà"
${LSTR} dict_Dutch         "Holandès"
${LSTR} dict_Norsk         "Bokmål noruec"
${LSTR} dict_Nynorsk       "Nynorsk noruec"
${LSTR} dict_Polish        "Polonès"
${LSTR} dict_Portugues     "Portuguès"
${LSTR} dict_Brazilian     "Brasiler"
${LSTR} dict_Russian       "Rus"
${LSTR} dict_Slovensko     "Eslovè"
${LSTR} dict_Svenska       "Suec"
${LSTR} dict_Ukrainian     "Ucraïnès"


; End Language descriptions

