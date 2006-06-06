;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
; Croatian ${LANG_Croatian}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Osnovne komponente"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (obavezno)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} datoteke podrkške (obavezno)"
${LSTR} TITLE_ssection_shortcuts            "Preèaci"
${LSTR} TITLE_ssection_shortcuts_cu         "Preèaci (trenutan korisnik)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Preèaci izbornika Start (trenutan korisnik)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Preèaci radne površine (trenutan korisnik)"
${LSTR} TITLE_ssection_shortcuts_au         "Preèaci (svi korisnici)"
${LSTR} TITLE_section_sm_shortcuts_au       "Preèaci izbornika Start (svi korisnici)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Preèaci radne površine (svi korisnici)"
${LSTR} TITLE_ssection_fa_shellupdate       "Ažuriraj pridruživanje datoteka"
${LSTR} TITLE_section_fa_abw                "Pridruži .abw uz AbiWord"
${LSTR} TITLE_section_fa_awt                "Pridruži .awt uz AbiWord"
${LSTR} TITLE_section_fa_zabw               "Pridruži .zabw uz AbiWord"
${LSTR} TITLE_section_fa_doc                "Pridruži .doc uz AbiWord"
${LSTR} TITLE_section_fa_rtf                "Pridruži .rtf uz AbiWord"
${LSTR} TITLE_ssection_helper_files         "Datoteke pomoæi"
${LSTR} TITLE_section_help                  "Datoteke pomoæi"
${LSTR} TITLE_section_templates             "Predlošci"
;${LSTR} TITLE_section_samples               "Uzorci"
${LSTR} TITLE_section_clipart               "Isjeèci"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Preuzmi CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Rjeènici"
${LSTR} TITLE_section_dictinary_def_English "en-US amerièki engleski (zadano)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Preuzmi opcionalne rjeènike"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Dodaci"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Osnovni (jezgreni) komplet komponenti potreban za pravilan rad programa AbiWord."
${LSTR} DESC_section_abi              "Obavezno. Instalira sam program ${PROGRAMEXE}."
${LSTR} DESC_section_abi_req          "Obavezno. Instalira osnovne datoteke podrške, npr. komplete niozova, podršku za BMP meðuspremnik, itd."
${LSTR} DESC_ssection_shortcuts       "Instalira preèace na razlièite lokacije radi lakšeg pokretanja programa AbiWord."
${LSTR} DESC_ssection_shortcuts_cu    "Instalira preèace za trenutno prijavljenog korisnika."
${LSTR} DESC_ssection_shortcuts_au    "Instalira preèace za sve korisnike (ili za trenutnog korisnika na sustavima s više korisnika)."
${LSTR} DESC_ssection_fa_shellupdate  "Dodaje unose u registar koji omoguæuju Exploreru da pomoæu AbiWorda otvara razlièito oblikovane datoteke."
${LSTR} DESC_section_fa_abw           "Odreðuje upotrebu AbiWorda za otvaranje datoteka u njihovom izvornom obliku. (Preporuèeno)"
${LSTR} DESC_section_fa_awt           "Odreðuje upotrebu AbiWorda za otvaranje predložaka u njihovom izvornom obliku. (Preporuèeno)"
${LSTR} DESC_section_fa_zabw          "Odreðuje upotrebu AbiWorda za otvaranje komprimiranih dokumenata u njihovom izvornom obliku. (Preporuèeno)"
${LSTR} DESC_section_fa_doc           "Odreðuje upotrebu AbiWorda za otvaranje Microsoft Word (R) oblikovanih dokumenata."
${LSTR} DESC_section_fa_rtf           "Odreðuje upotrebu AbiWorda za otvaranje RichText datoteka, standardnog oblika za ureðivaèe teksta."
${LSTR} DESC_ssection_helper_files    "Instalira razlièite opcionalne datoteke koje pomažu pri upotrebi programa AbiWord."
${LSTR} DESC_section_help             "Instalira dokumente pomoæi. Izostavljanjem ove opcije nikakva pomoæ neæe biti na raspolaganju."
${LSTR} DESC_section_templates        "Instalira predloške koji se mogu upotrijebiti za izradu novih dokumenata uz unaprijed definirano oblikovanje."
${LSTR} DESC_section_samples          "Uzorci su uklonjeni."
${LSTR} DESC_section_clipart          "Instalira slike (isjeèke) koji je moguæe umetati u dokumente."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Instalira biblioteku C pogona koju upotrebljava AbiWord. Korisno ako trenutno nije prisutno na vašem sustavu."
!endif
${LSTR} DESC_ssection_dictionary      "Instalira rjeènike za razne jezike radi provjeravanja pravopisa u dokumentima."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Instalira razne opcionalne dodatke."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Prepisati postojeæi ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "Izgleda kao da program ${PRODUCT} nije pravilno instaliran!$\r$\n\
                                               Pretraživanje za ${MAINPROGRAM} nije uspjelo. Bit æe ponovno instaliran.$\r$\n\
                                               Nastaviti s izmjenama instalacije?"
${LSTR} MSG_ABORT                             "Odustajanje od instalacijskog postupka"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Preuzimanje zatraženih biblioteka C pogona (DLL) nije uspjelo: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Osnovna URL adresa za preuzimanje opcionalnih komponenti"
${LSTR} TEXT_IO_SUBTITLE              "Rjeènici"
${LSTR} MSG_SELECT_DL_MIRROR          "Odaberite zrcalo preuzimanja..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Pogreška pri dohvaæanju korisnièkog odabira. Bit æe upotrebljena zadana lokacija!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "Ureðivaè teksta ${PRODUCT}"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Ukloni ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "Pomoæ za ${PRODUCT} (na engleskom)"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Želite li izbrisati $INSTDIR, kao i sve podamape i datoteke?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "katalonski"
${LSTR} dict_Czech         "èeški"
${LSTR} dict_Danish        "danski"
${LSTR} dict_Swiss         "švicarski"
${LSTR} dict_Deutsch       "njemaèki"
${LSTR} dict_Ellhnika      "grèki"
${LSTR} dict_English       "engleski (VB)"
${LSTR} dict_American      "engleski (SAD)"
${LSTR} dict_Esperanto     "esperanto"
${LSTR} dict_Espanol       "španjolski"
${LSTR} dict_Finnish       "finski"
${LSTR} dict_Francais      "francuski"
${LSTR} dict_Hungarian     "maðarski"
${LSTR} "dict_Irish gaelic""irski galski"
${LSTR} dict_Galician      "galicijski"
${LSTR} dict_Italian       "talijanski"
${LSTR} dict_Kurdish       "kurdski"
${LSTR} dict_Latin         "latinski"
${LSTR} dict_Lietuviu      "litavski"
${LSTR} dict_Dutch         "nizozemski"
${LSTR} dict_Norsk         "norveški Bokmal"
${LSTR} dict_Nynorsk       "norveški Nynorsk"
${LSTR} dict_Polish        "poljski"
${LSTR} dict_Portugues     "portugalski"
${LSTR} dict_Brazilian     "brazilski"
${LSTR} dict_Russian       "ruski"
${LSTR} dict_Sardinian     "sardinijski"
${LSTR} dict_Slovensko     "slovenski"
${LSTR} dict_Svenska       "švedski"
${LSTR} dict_Ukrainian     "ukrajinski"


; End Language descriptions
