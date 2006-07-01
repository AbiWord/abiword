;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
; Croatian ${LANG_Croatian}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Osnovne komponente"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (obavezno)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} datoteke podrk�ke (obavezno)"
${LSTR} TITLE_ssection_shortcuts            "Pre�aci"
${LSTR} TITLE_ssection_shortcuts_cu         "Pre�aci (trenutan korisnik)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Pre�aci izbornika Start (trenutan korisnik)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Pre�aci radne povr�ine (trenutan korisnik)"
${LSTR} TITLE_ssection_shortcuts_au         "Pre�aci (svi korisnici)"
${LSTR} TITLE_section_sm_shortcuts_au       "Pre�aci izbornika Start (svi korisnici)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Pre�aci radne povr�ine (svi korisnici)"
${LSTR} TITLE_ssection_fa_shellupdate       "A�uriraj pridru�ivanje datoteka"
${LSTR} TITLE_section_fa_abw                "Pridru�i .abw uz AbiWord"
${LSTR} TITLE_section_fa_awt                "Pridru�i .awt uz AbiWord"
${LSTR} TITLE_section_fa_zabw               "Pridru�i .zabw uz AbiWord"
${LSTR} TITLE_section_fa_doc                "Pridru�i .doc uz AbiWord"
${LSTR} TITLE_section_fa_rtf                "Pridru�i .rtf uz AbiWord"
${LSTR} TITLE_ssection_helper_files         "Datoteke pomo�i"
${LSTR} TITLE_section_help                  "Datoteke pomo�i"
${LSTR} TITLE_section_templates             "Predlo�ci"
;${LSTR} TITLE_section_samples               "Uzorci"
${LSTR} TITLE_section_clipart               "Isje�ci"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Preuzmi CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Rje�nici"
${LSTR} TITLE_section_dictinary_def_English "en-US ameri�ki engleski (zadano)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Preuzmi opcionalne rje�nike"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Dodaci"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Osnovni (jezgreni) komplet komponenti potreban za pravilan rad programa AbiWord."
${LSTR} DESC_section_abi              "Obavezno. Instalira sam program ${PROGRAMEXE}."
${LSTR} DESC_section_abi_req          "Obavezno. Instalira osnovne datoteke podr�ke, npr. komplete niozova, podr�ku za BMP me�uspremnik, itd."
${LSTR} DESC_ssection_shortcuts       "Instalira pre�ace na razli�ite lokacije radi lak�eg pokretanja programa AbiWord."
${LSTR} DESC_ssection_shortcuts_cu    "Instalira pre�ace za trenutno prijavljenog korisnika."
${LSTR} DESC_ssection_shortcuts_au    "Instalira pre�ace za sve korisnike (ili za trenutnog korisnika na sustavima s vi�e korisnika)."
${LSTR} DESC_ssection_fa_shellupdate  "Dodaje unose u registar koji omogu�uju Exploreru da pomo�u AbiWorda otvara razli�ito oblikovane datoteke."
${LSTR} DESC_section_fa_abw           "Odre�uje upotrebu AbiWorda za otvaranje datoteka u njihovom izvornom obliku. (Preporu�eno)"
${LSTR} DESC_section_fa_awt           "Odre�uje upotrebu AbiWorda za otvaranje predlo�aka u njihovom izvornom obliku. (Preporu�eno)"
${LSTR} DESC_section_fa_zabw          "Odre�uje upotrebu AbiWorda za otvaranje komprimiranih dokumenata u njihovom izvornom obliku. (Preporu�eno)"
${LSTR} DESC_section_fa_doc           "Odre�uje upotrebu AbiWorda za otvaranje Microsoft Word (R) oblikovanih dokumenata."
${LSTR} DESC_section_fa_rtf           "Odre�uje upotrebu AbiWorda za otvaranje RichText datoteka, standardnog oblika za ure�iva�e teksta."
${LSTR} DESC_ssection_helper_files    "Instalira razli�ite opcionalne datoteke koje poma�u pri upotrebi programa AbiWord."
${LSTR} DESC_section_help             "Instalira dokumente pomo�i. Izostavljanjem ove opcije nikakva pomo� ne�e biti na raspolaganju."
${LSTR} DESC_section_templates        "Instalira predlo�ke koji se mogu upotrijebiti za izradu novih dokumenata uz unaprijed definirano oblikovanje."
${LSTR} DESC_section_samples          "Uzorci su uklonjeni."
${LSTR} DESC_section_clipart          "Instalira slike (isje�ke) koji je mogu�e umetati u dokumente."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Instalira biblioteku C pogona koju upotrebljava AbiWord. Korisno ako trenutno nije prisutno na va�em sustavu."
!endif
${LSTR} DESC_ssection_dictionary      "Instalira rje�nike za razne jezike radi provjeravanja pravopisa u dokumentima."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Instalira razne opcionalne dodatke."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Prepisati postoje�i ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "Izgleda kao da program ${PRODUCT} nije pravilno instaliran!$\r$\n\
                                               Pretra�ivanje za ${MAINPROGRAM} nije uspjelo. Bit �e ponovno instaliran.$\r$\n\
                                               Nastaviti s izmjenama instalacije?"
${LSTR} MSG_ABORT                             "Odustajanje od instalacijskog postupka"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Preuzimanje zatra�enih biblioteka C pogona (DLL) nije uspjelo: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Osnovna URL adresa za preuzimanje opcionalnih komponenti"
${LSTR} TEXT_IO_SUBTITLE              "Rje�nici"
${LSTR} MSG_SELECT_DL_MIRROR          "Odaberite zrcalo preuzimanja..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Pogre�ka pri dohva�anju korisni�kog odabira. Bit �e upotrebljena zadana lokacija!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "Ure�iva� teksta ${PRODUCT}"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Ukloni ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "Pomo� za ${PRODUCT} (na engleskom)"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "�elite li izbrisati $INSTDIR, kao i sve podamape i datoteke?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "katalonski"
${LSTR} dict_Czech         "�e�ki"
${LSTR} dict_Danish        "danski"
${LSTR} dict_Swiss         "�vicarski"
${LSTR} dict_Deutsch       "njema�ki"
${LSTR} dict_Ellhnika      "gr�ki"
${LSTR} dict_English       "engleski (VB)"
${LSTR} dict_American      "engleski (SAD)"
${LSTR} dict_Esperanto     "esperanto"
${LSTR} dict_Espanol       "�panjolski"
${LSTR} dict_Finnish       "finski"
${LSTR} dict_Francais      "francuski"
${LSTR} dict_Hungarian     "ma�arski"
${LSTR} "dict_Irish gaelic""irski galski"
${LSTR} dict_Galician      "galicijski"
${LSTR} dict_Italian       "talijanski"
${LSTR} dict_Kurdish       "kurdski"
${LSTR} dict_Latin         "latinski"
${LSTR} dict_Lietuviu      "litavski"
${LSTR} dict_Dutch         "nizozemski"
${LSTR} dict_Norsk         "norve�ki Bokmal"
${LSTR} dict_Nynorsk       "norve�ki Nynorsk"
${LSTR} dict_Polish        "poljski"
${LSTR} dict_Portugues     "portugalski"
${LSTR} dict_Brazilian     "brazilski"
${LSTR} dict_Russian       "ruski"
${LSTR} dict_Sardinian     "sardinijski"
${LSTR} dict_Slovensko     "slovenski"
${LSTR} dict_Svenska       "�vedski"
${LSTR} dict_Ukrainian     "ukrajinski"


; End Language descriptions
