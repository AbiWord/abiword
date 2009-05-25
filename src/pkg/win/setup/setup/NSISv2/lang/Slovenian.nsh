;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;Slovenian ${LANG_Slovenian}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Osnovne komponente"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (zahtevano)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} - podporne datoteke (zahtevano)"
${LSTR} TITLE_ssection_shortcuts            "Bližnjice"
${LSTR} TITLE_ssection_shortcuts_cu         "Bližnjice (za trenutnega uporabnika)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Bližnjice v meniju Start (za trenutnega uporabnika)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Bližnjica na namizju (za trenutnega uporabnika)"
${LSTR} TITLE_ssection_shortcuts_au         "Bližnjice (za vse uporabnike)"
${LSTR} TITLE_section_sm_shortcuts_au       "Bližnjice v meniju Start (za vse uporabnike)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Bližnjica na namizju (za vse uporabnike)"
${LSTR} TITLE_ssection_fa_shellupdate       "Posodobi povezave s konènicami datotek"
${LSTR} TITLE_section_fa_abw                "Poveži .abw z AbiWordom"
${LSTR} TITLE_section_fa_awt                "Poveži .awt z AbiWordom"
${LSTR} TITLE_section_fa_zabw               "Poveži .zabw z AbiWordom"
${LSTR} TITLE_section_fa_doc                "Poveži .doc z AbiWordom"
${LSTR} TITLE_section_fa_rtf                "Poveži .rtf z AbiWordom"
${LSTR} TITLE_ssection_helper_files         "Pomožne datoteke"
${LSTR} TITLE_section_help                  "Datoteke pomoèi"
${LSTR} TITLE_section_templates             "Predloge"
;${LSTR} TITLE_section_samples               "Vzorci"
${LSTR} TITLE_section_clipart               "Izrezki"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Prenesi CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Slovarji"
${LSTR} TITLE_section_dictinary_def_English "en-US  angleški (ZDA) (privzeto)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Prenesi dodatne slovarje"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Vtièniki"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Osnovni nabor komponent, da bo program AbiWord lahko deloval."
${LSTR} DESC_section_abi              "Zahtevano.  Namesti sam program ${PROGRAMEXE}."
${LSTR} DESC_section_abi_req          "Zahtevano.  Namesti osnovne pomožne datoteke, npr. nabore nizov, podporo za BMP odložišèa itn."
${LSTR} DESC_ssection_shortcuts       "Namesti bližnjice na razliènih mestih, ki omogoèajo poganjanje AbiWorda iz dodatnih mest."
${LSTR} DESC_ssection_shortcuts_cu    "Namesti bližnjice za trenutno prijavljenega uporabnika."
${LSTR} DESC_ssection_shortcuts_au    "Namesti bližnjice za vse uporabnike (ali trenutnega uporabnika na enouporabniških sistemih)."
${LSTR} DESC_ssection_fa_shellupdate  "Doda vnose v register, kar omogoèa Raziskovalcu z uporabo AbiWorda odpirati razliène vrste dokumentov."
${LSTR} DESC_section_fa_abw           "Doloèa, da bo AbiWord uporabljen za odpiranje dokumentov v njegovem lastnem zapisu.  (Priporoèeno)"
${LSTR} DESC_section_fa_awt           "Doloèa, da bo AbiWord uporabljen za odpiranje predlog v njegovem lastnem zapisu.  (Priporoèeno)"
${LSTR} DESC_section_fa_zabw          "Doloèa, da bo AbiWord uporabljen za odpiranje stisnjenih dokumentov v njegovem lastnem zapisu.  (Priporoèeno)"
${LSTR} DESC_section_fa_doc           "Doloèa, da bo AbiWord uporabljen za odpiranje dokumentov v zapisu Microsoft Word (R)."
${LSTR} DESC_section_fa_rtf           "Doloèa, da bo AbiWord uporabljen za odpiranje datotek z obogatenim besedilom (Rich Text File), 'standardnim' zapisom urejevalnikov besedil."
${LSTR} DESC_ssection_helper_files    "Namesti razliène druge dodatne datoteke, ki pomagajo pri uporabi AbiWorda."
${LSTR} DESC_section_help             "Namesti dokumente pomoèi; èe to izpustite, pomoè ne bo na voljo."
${LSTR} DESC_section_templates        "Namesti predloge, s katerimi si lahko pomagate pri ustvarjanju novih dokumentov z vnaprej pripravljenim oblikovanjem."
${LSTR} DESC_section_samples          "Vzorci so bili odstranjeni."
${LSTR} DESC_section_clipart          "Namesti slike (izrezke), ki jih lahko vstavite v dokumente."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Namesti izvajalno knjižnico C, ki jo potrebuje AbiWord; uporabno, èe vaš operacijski sistem le-tega nima."
!endif
${LSTR} DESC_ssection_dictionary      "Namesti slovarje za razliène jezike, s katerimi lahko preverjate èrkovanje dokumentov."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Namesti razliène dodatne vtiènike."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Želite prepisati obstojeèo namestitev ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} najverjetneje ni pravilno namešèen!$\r$\n\
                                               Datoteke ${MAINPROGRAM} ni mogoèe najti, zato bo ponovno namešèena.$\r$\n\
                                               Želite nadaljevati s spreminjanjem namestitve?"
${LSTR} MSG_ABORT                             "Zapušèanje namestitvenega postopka"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Zahtevane knjižnice (DLL) ni mogoèe namestiti: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Osnovni URL za prenos dodatnih komponent"
${LSTR} TEXT_IO_SUBTITLE              "Slovarji"
${LSTR} MSG_SELECT_DL_MIRROR          "Izberite zrcalni strežnik za prenos ..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Napaka pri pridobivanju zahtevanega, uporabljen bo privzeti strežnik!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "Urejevalnik besedil ${PRODUCT}"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Odstrani ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "Pomoè za ${PRODUCT} (v angl.)"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Želite izbrisati $INSTDIR in vse podmape in datoteke?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "katalonski"
${LSTR} dict_Czech         "èeški"
${LSTR} dict_Danish        "danski"
${LSTR} dict_Swiss         "švicarski"
${LSTR} dict_Deutsch       "nemški"
${LSTR} dict_Ellhnika      "grški"
${LSTR} dict_English       "angleški (VB)"
${LSTR} dict_American      "angleški (ZDA)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "španski"
${LSTR} dict_Finnish       "finski"
${LSTR} dict_Francais      "francoski"
${LSTR} dict_Hungarian     "madžarski"
${LSTR} "dict_Irish gaelic""irski"
${LSTR} dict_Galician      "galicijski"
${LSTR} dict_Italian       "italijanski"
${LSTR} dict_Kurdish       "kurdski"
${LSTR} dict_Latin         "latinski"
${LSTR} dict_Lietuviu      "litovski"
${LSTR} dict_Dutch         "nizozemski"
${LSTR} dict_Norsk         "norveški (Bokmal)"
${LSTR} dict_Nynorsk       "norveški (Nynorsk)"
${LSTR} dict_Polish        "poljski"
${LSTR} dict_Portugues     "portugalski"
${LSTR} dict_Brazilian     "brazilski"
${LSTR} dict_Russian       "ruski"
${LSTR} dict_Sardinian     "sardinijski"
${LSTR} dict_Slovensko     "slovenski"
${LSTR} dict_Svenska       "švedski"
${LSTR} dict_Ukrainian     "ukrajinski"


; End Language descriptions
