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
${LSTR} TITLE_section_dictinary_def_English "Angl�s, catal�, franc�s i espanyol"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Baixada opcional de diccionaris"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Plugins"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Conjunt primari (central) d'elements perqu� l'AbiWord funcioni b�."
${LSTR} DESC_section_abi              "Necessari.  Instal�la el programa ${PROGRAMEXE} actual."
${LSTR} DESC_section_abi_req          "Necessari.  Instal�la els fitxers de suport b�sics, p.ex. conjunts de cadenes, suport de porta-retalls BMP, etc."
${LSTR} DESC_ssection_shortcuts       "Instal�la dreceres a varis empla�aments per permetre iniciar l'AbiWord a trav�s d'ubicacions adicionals."
${LSTR} DESC_ssection_shortcuts_cu    "Instal�la dreceres per a l'usuari connectat en aquest moment."
${LSTR} DESC_ssection_shortcuts_au    "Instal�la dreceres per a tots els usuaris (o l'usuari actual en sistemes sense usuaris m�ltiples)."
${LSTR} DESC_ssection_fa_shellupdate  "Afegeix entrades al registre per permetre a Windows utilitzar l'AbiWord per obrir varis formats de document."
${LSTR} DESC_section_fa_abw           "Especifica que l'Abiword s'hauria d'utilitzar per obrir documents en el seu format originari.  (Recomanat)"
${LSTR} DESC_section_fa_awt           "Especifica que l'Abiword s'hauria d'utilitzar per obrir plantilles en el seu format originari.  (Recomanat)"
${LSTR} DESC_section_fa_zabw          "Especifica que l'Abiword s'hauria d'utilitzar per obrir documents comprimits en el seu format originari.  (Recomanat)"
${LSTR} DESC_section_fa_doc           "Especifica que l'Abiword s'hauria d'utilitzar per obrir documents de Microsoft Word (R) en format originari."
${LSTR} DESC_section_fa_rtf           "Especifica que l'Abiword s'hauria d'utilitzar per obrir fitxers en format de text enriquit."
${LSTR} DESC_ssection_helper_files    "Instal�la varis fitxers opcionals per ajudar a utilitzar l'AbiWord."
${LSTR} DESC_section_help             "Instal�la els documents d'ajuda, si s'omet, no hi ha ajuda disponible."
${LSTR} DESC_section_templates        "Instal�la plantilles que es poden utilitzar per ajudar a crear nous documents amb format predefinit."
${LSTR} DESC_section_samples          "S'han extret les mostres."
${LSTR} DESC_section_clipart          "Instal�la imatges (fitxer d'imatges) que es poden inserir en documents."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Instal�la la biblioteca de temps d'execuci� en C (C Runtime Library) utilitzada per l'AbiWord, �til si el sistema ja no ho t�."
!endif
${LSTR} DESC_ssection_dictionary      "Instal�la diccionaris per a v�ries lleng�es que s'utilitzen per verificar l'ortografia del document."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Instal�la varis connectors opcionals."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Voleu sobreescriure ${PRODUCT} existent?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "Sembla que ${PRODUCT} no s'ha instal�lat correctament!$\r$\n\
                                               �s impossible trobar ${MAINPROGRAM}, es tornar� a instal�lar.$\r$\n\
                                               Voleu continuar per modificar la instal�laci�?"
${LSTR} MSG_ABORT                             "S'est� sortint del proc�s d'instal�laci�"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "�s impossible baixar la c runtime library (DLL) sol�licitada: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Adre�a web per la baixada de components opcionals"
${LSTR} TEXT_IO_SUBTITLE              "Diccionaris"
${LSTR} MSG_SELECT_DL_MIRROR          "Selecciona la r�plica de baixada..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "S'ha produ�t un error en obtenir l'opci� de l'usuari, en utilitzar el lloc per defecte!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} processador de textos"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Desinstal�la ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "(English) Ajuda per a ${PRODUCT}"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Aix� suprimir� $INSTDIR i tots els subdirectoris i fitxers?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Catal�"
${LSTR} dict_Czech         "Txec"
${LSTR} dict_Danish        "Dan�s"
${LSTR} dict_Swiss         "Su�s"
${LSTR} dict_Deutsch       "Alemany"
${LSTR} dict_Ellhnika      "Grec"
${LSTR} dict_English       "Angl�s (GB)"
${LSTR} dict_American      "Angl�s (US)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Espanyol"
${LSTR} dict_Finnish       "Fin�s"
${LSTR} dict_Francais      "Franc�s"
${LSTR} dict_Hungarian     "Hongar�s"
${LSTR} "dict_Irish gaelic""Irland�s"
${LSTR} dict_Galician      "Gallec"
${LSTR} dict_Italian       "Itali�"
${LSTR} dict_Latin         "Llat�"
${LSTR} dict_Lietuviu      "Litu�"
${LSTR} dict_Dutch         "Holand�s"
${LSTR} dict_Norsk         "Bokm�l noruec"
${LSTR} dict_Nynorsk       "Nynorsk noruec"
${LSTR} dict_Polish        "Polon�s"
${LSTR} dict_Portugues     "Portugu�s"
${LSTR} dict_Brazilian     "Brasiler"
${LSTR} dict_Russian       "Rus"
${LSTR} dict_Slovensko     "Eslov�"
${LSTR} dict_Svenska       "Suec"
${LSTR} dict_Ukrainian     "Ucra�n�s"


; End Language descriptions

