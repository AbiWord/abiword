;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;Lithuanian ${LANG_Lithuanian}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Pagrindiniai komponentai"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (bûtinas)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} palaikymo bylos (bûtinas)"
${LSTR} TITLE_ssection_shortcuts            "Nuorodos"
${LSTR} TITLE_ssection_shortcuts_cu         "Nuorodos (Dabartiniam Vartotojui)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Start Meniu Nuorodos (Dabartiniam Vartotojui)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Nuoroda Darbastalyje (Dabartiniam Vartotojui)"
${LSTR} TITLE_ssection_shortcuts_au         "Nuorodos (Visiems Vartotojams)"
${LSTR} TITLE_section_sm_shortcuts_au       "Start Meniu Nuorodos (Visiems Vartotojams)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Nuoroda Darbastalyje (Visiems Vartotojams)"
${LSTR} TITLE_ssection_fa_shellupdate       "Atnaujinti Aplinkos bylø sàsajas"
${LSTR} TITLE_section_fa_abw                "Susieti .abw su AbiWord'u"
${LSTR} TITLE_section_fa_awt                "Susieti .awt su AbiWord'u"
${LSTR} TITLE_section_fa_zabw               "Susieti .zabw su AbiWord'u"
${LSTR} TITLE_section_fa_doc                "Susieti .doc su AbiWord'u"
${LSTR} TITLE_section_fa_rtf                "Susieti .rtf su AbiWord'u"
${LSTR} TITLE_ssection_helper_files         "Pagalbinës bylos"
${LSTR} TITLE_section_help                  "Þinyno bylos"
${LSTR} TITLE_section_templates             "Ðablonai"
;${LSTR} TITLE_section_samples               "Pavyzdþiai"
${LSTR} TITLE_section_clipart               "Pieðiniai"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Parsiøsdinti CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Þodynai"
${LSTR} TITLE_section_dictinary_def_English "en-US  Anglø k. (JAV) (áprastas)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Parsiøsdinti papildomus þodynus"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Áskiepiai"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Pagrindinis (ðerdinis) AbiWord komponentø rinkinys, reikalingas, kas Abiword gerai veiktø."
${LSTR} DESC_section_abi              "Bûtinas.  Ádiegia paèià ${PROGRAMEXE} programà."
${LSTR} DESC_section_abi_req          "Bûtinas.  Ádiegia bazines paramos bylas, pvz., vertimø rinkinius, BMP dëklës palaikymà ir kita."
${LSTR} DESC_ssection_shortcuts       "Ávairiose vietose sukuria nuorodas á Abiword'à, sudarydamas galimybes paleisti programà ið papildomø vietø."
${LSTR} DESC_ssection_shortcuts_cu    "Sukuria nuorodas dabar ásiregistravusiam vartotojui."
${LSTR} DESC_ssection_shortcuts_au    "Sukuria nuorodas visiems vartotojams (arba ásiregistravusiam vartotojui, jeigu sistemoje tëra vienas vartotojas)."
${LSTR} DESC_ssection_fa_shellupdate  "Sistemos registre sukuria áraðus, ágalinanèius Explorer'io aplinkà atvertinëti ávairius dokumentø formatus AbiWord'u."
${LSTR} DESC_section_fa_abw           "Nurodo, kad AbiWord'as turi bûti naudojamas atverti savo paties formato dokumentus.  (Rekomenduojama)"
${LSTR} DESC_section_fa_awt           "Nurodo, kad AbiWord'as turi bûti naudojamas atverti savo paties formato ðablonus.      (Rekomenduojama)"
${LSTR} DESC_section_fa_zabw          "Nurodo, kad AbiWord'as turi bûti naudojamas atverti savo paties formato suspaustus dokumentus.  (Rekomenduojama)"
${LSTR} DESC_section_fa_doc           "Nurodo, kad AbiWord'u turi bûti atverèiami Microsoft Word (R) formato dokumentai."
${LSTR} DESC_section_fa_rtf           "Nurodo, kad AbiWord'u turi bûti atverèiamos 'Turtingo Teksto' bylos, kurios yra 'standartinio' teksto dorokliø formatas."
${LSTR} DESC_ssection_helper_files    "Ádiegia ávairias papildomas bylas, padedanèias naudotis AbiWord'u."
${LSTR} DESC_section_help             "Ádiegia Þinynas ir kitus 'Pagalbos' dokumentus. Jei ðá punktà praleisite, negalësite naudotis 'Pagalba'"
${LSTR} DESC_section_templates        "Ádiegia ðablonus, kurie gali bûti naudojami kurti naujus dokumentus su nustatytu iðdëstymu."
${LSTR} DESC_section_samples          "Pavyzdþiai buvo paðalinti."
${LSTR} DESC_section_clipart          "Ádiegia pieðinëlius, kuriais galite praturtinti dokumentus."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Ádiegia Abiword'o naudojamà C vykdymo bibliotekà - naudinga tada, jeigu jos (bibliotekos) Jûsø sistemoje trûksta."
!endif
${LSTR} DESC_ssection_dictionary      "Ádiegia ávairiø kalbø þodynus, skirtus tikrinti raðybà Jûsø dokumentuose."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Ádiegia ávairius papildomus áskiepius."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Pekeisti esamà ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} nëra taisyklingai ádiegtas!$\r$\n\
                                               Nepavyko rasti ${MAINPROGRAM}, todël ji bus diegiama ið naujo.$\r$\n\
                                               Tæsti diegimo pakeitimà?"
${LSTR} MSG_ABORT                             "Diegimas nutraukiamas"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Nepavyko parsiøsti C vykdomosios bibliotekos (DLL): ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Papildomai parsiunèiamø komponentø bazinë nuoroda (URL)"
${LSTR} TEXT_IO_SUBTITLE              "Þodynai"
${LSTR} MSG_SELECT_DL_MIRROR          "Pasirinkite parsiuntimo saugyklà..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Nepavyko pasinaudoti vartotojo pasirinkimu. Naudosime áprastà saugyklà!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} Teksto Doroklis"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Paðalinti ${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_HELP            "${PRODUCT} Þinynas (Anglø kalba)"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Ar trinti katalogà $INSTDIR ir visus jo pakatalogius?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Katalonø"
${LSTR} dict_Czech         "Èekø"
${LSTR} dict_Danish        "Danø"
${LSTR} dict_Swiss         "Ðveicarø"
${LSTR} dict_Deutsch       "Vokieèiø"
${LSTR} dict_Ellhnika      "Graikø"
${LSTR} dict_English       "Anglø (Jungtinë  Karalystë)"
${LSTR} dict_American      "Anglø (Jungtinës Valstijos)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Ispanø"
${LSTR} dict_Finnish       "Suomiø"
${LSTR} dict_Francais      "Prancûzø"
${LSTR} dict_Hungarian     "Vengrø"
${LSTR} "dict_Irish gaelic""Airiø Galø"
${LSTR} dict_Galician      "Galiciðkasis"
${LSTR} dict_Italian       "Italø"
${LSTR} dict_Latin         "Lotynø"
${LSTR} dict_Lietuviu      "Lietuviø"
${LSTR} dict_Dutch         "Olandø"
${LSTR} dict_Norsk         "Norvegø Bokmål"
${LSTR} dict_Nynorsk       "Norvegø Nynorsk"
${LSTR} dict_Polish        "Lenkø"
${LSTR} dict_Portugues     "Portugalø"
${LSTR} dict_Brazilian     "Brazilø"
${LSTR} dict_Russian       "Rusø"
${LSTR} dict_Slovensko     "Slovënø"
${LSTR} dict_Svenska       "Ðvedø"
${LSTR} dict_Ukrainian     "Ukraineèiø"


; End Language descriptions
