;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;Lithuanian ${LANG_Lithuanian}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Pagrindiniai komponentai"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (b�tinas)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} palaikymo bylos (b�tinas)"
${LSTR} TITLE_ssection_shortcuts            "Nuorodos"
${LSTR} TITLE_ssection_shortcuts_cu         "Nuorodos (Dabartiniam Vartotojui)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Start Meniu Nuorodos (Dabartiniam Vartotojui)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Nuoroda Darbastalyje (Dabartiniam Vartotojui)"
${LSTR} TITLE_ssection_shortcuts_au         "Nuorodos (Visiems Vartotojams)"
${LSTR} TITLE_section_sm_shortcuts_au       "Start Meniu Nuorodos (Visiems Vartotojams)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Nuoroda Darbastalyje (Visiems Vartotojams)"
${LSTR} TITLE_ssection_fa_shellupdate       "Atnaujinti Aplinkos byl� s�sajas"
${LSTR} TITLE_section_fa_abw                "Susieti .abw su AbiWord'u"
${LSTR} TITLE_section_fa_awt                "Susieti .awt su AbiWord'u"
${LSTR} TITLE_section_fa_zabw               "Susieti .zabw su AbiWord'u"
${LSTR} TITLE_section_fa_doc                "Susieti .doc su AbiWord'u"
${LSTR} TITLE_section_fa_rtf                "Susieti .rtf su AbiWord'u"
${LSTR} TITLE_ssection_helper_files         "Pagalbin�s bylos"
${LSTR} TITLE_section_help                  "�inyno bylos"
${LSTR} TITLE_section_templates             "�ablonai"
;${LSTR} TITLE_section_samples               "Pavyzd�iai"
${LSTR} TITLE_section_clipart               "Pie�iniai"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Parsi�sdinti CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "�odynai"
${LSTR} TITLE_section_dictinary_def_English "en-US  Angl� k. (JAV) (�prastas)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Parsi�sdinti papildomus �odynus"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "�skiepiai"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Pagrindinis (�erdinis) AbiWord komponent� rinkinys, reikalingas, kas Abiword gerai veikt�."
${LSTR} DESC_section_abi              "B�tinas.  �diegia pa�i� ${PROGRAMEXE} program�."
${LSTR} DESC_section_abi_req          "B�tinas.  �diegia bazines paramos bylas, pvz., vertim� rinkinius, BMP d�kl�s palaikym� ir kita."
${LSTR} DESC_ssection_shortcuts       "�vairiose vietose sukuria nuorodas � Abiword'�, sudarydamas galimybes paleisti program� i� papildom� viet�."
${LSTR} DESC_ssection_shortcuts_cu    "Sukuria nuorodas dabar �siregistravusiam vartotojui."
${LSTR} DESC_ssection_shortcuts_au    "Sukuria nuorodas visiems vartotojams (arba �siregistravusiam vartotojui, jeigu sistemoje t�ra vienas vartotojas)."
${LSTR} DESC_ssection_fa_shellupdate  "Sistemos registre sukuria �ra�us, �galinan�ius Explorer'io aplink� atvertin�ti �vairius dokument� formatus AbiWord'u."
${LSTR} DESC_section_fa_abw           "Nurodo, kad AbiWord'as turi b�ti naudojamas atverti savo paties formato dokumentus.  (Rekomenduojama)"
${LSTR} DESC_section_fa_awt           "Nurodo, kad AbiWord'as turi b�ti naudojamas atverti savo paties formato �ablonus.      (Rekomenduojama)"
${LSTR} DESC_section_fa_zabw          "Nurodo, kad AbiWord'as turi b�ti naudojamas atverti savo paties formato suspaustus dokumentus.  (Rekomenduojama)"
${LSTR} DESC_section_fa_doc           "Nurodo, kad AbiWord'u turi b�ti atver�iami Microsoft Word (R) formato dokumentai."
${LSTR} DESC_section_fa_rtf           "Nurodo, kad AbiWord'u turi b�ti atver�iamos 'Turtingo Teksto' bylos, kurios yra 'standartinio' teksto dorokli� formatas."
${LSTR} DESC_ssection_helper_files    "�diegia �vairias papildomas bylas, padedan�ias naudotis AbiWord'u."
${LSTR} DESC_section_help             "�diegia �inynas ir kitus 'Pagalbos' dokumentus. Jei �� punkt� praleisite, negal�site naudotis 'Pagalba'"
${LSTR} DESC_section_templates        "�diegia �ablonus, kurie gali b�ti naudojami kurti naujus dokumentus su nustatytu i�d�stymu."
${LSTR} DESC_section_samples          "Pavyzd�iai buvo pa�alinti."
${LSTR} DESC_section_clipart          "�diegia pie�in�lius, kuriais galite praturtinti dokumentus."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "�diegia Abiword'o naudojam� C vykdymo bibliotek� - naudinga tada, jeigu jos (bibliotekos) J�s� sistemoje tr�ksta."
!endif
${LSTR} DESC_ssection_dictionary      "�diegia �vairi� kalb� �odynus, skirtus tikrinti ra�yb� J�s� dokumentuose."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "�diegia �vairius papildomus �skiepius."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Pekeisti esam� ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} n�ra taisyklingai �diegtas!$\r$\n\
                                               Nepavyko rasti ${MAINPROGRAM}, tod�l ji bus diegiama i� naujo.$\r$\n\
                                               T�sti diegimo pakeitim�?"
${LSTR} MSG_ABORT                             "Diegimas nutraukiamas"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Nepavyko parsi�sti C vykdomosios bibliotekos (DLL): ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Papildomai parsiun�iam� komponent� bazin� nuoroda (URL)"
${LSTR} TEXT_IO_SUBTITLE              "�odynai"
${LSTR} MSG_SELECT_DL_MIRROR          "Pasirinkite parsiuntimo saugykl�..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Nepavyko pasinaudoti vartotojo pasirinkimu. Naudosime �prast� saugykl�!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} Teksto Doroklis"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Pa�alinti ${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_HELP            "${PRODUCT} �inynas (Angl� kalba)"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Ar trinti katalog� $INSTDIR ir visus jo pakatalogius?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Katalon�"
${LSTR} dict_Czech         "�ek�"
${LSTR} dict_Danish        "Dan�"
${LSTR} dict_Swiss         "�veicar�"
${LSTR} dict_Deutsch       "Vokie�i�"
${LSTR} dict_Ellhnika      "Graik�"
${LSTR} dict_English       "Angl� (Jungtin�  Karalyst�)"
${LSTR} dict_American      "Angl� (Jungtin�s Valstijos)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Ispan�"
${LSTR} dict_Finnish       "Suomi�"
${LSTR} dict_Francais      "Pranc�z�"
${LSTR} dict_Hungarian     "Vengr�"
${LSTR} "dict_Irish gaelic""Airi� Gal�"
${LSTR} dict_Galician      "Galici�kasis"
${LSTR} dict_Italian       "Ital�"
${LSTR} dict_Latin         "Lotyn�"
${LSTR} dict_Lietuviu      "Lietuvi�"
${LSTR} dict_Dutch         "Oland�"
${LSTR} dict_Norsk         "Norveg� Bokm�l"
${LSTR} dict_Nynorsk       "Norveg� Nynorsk"
${LSTR} dict_Polish        "Lenk�"
${LSTR} dict_Portugues     "Portugal�"
${LSTR} dict_Brazilian     "Brazil�"
${LSTR} dict_Russian       "Rus�"
${LSTR} dict_Slovensko     "Slov�n�"
${LSTR} dict_Svenska       "�ved�"
${LSTR} dict_Ukrainian     "Ukraine�i�"


; End Language descriptions
