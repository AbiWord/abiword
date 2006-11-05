;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
;Slovenian ${LANG_Slovenian}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Osnovne komponente"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (zahtevano)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} - podporne datoteke (zahtevano)"
${LSTR} TITLE_ssection_shortcuts            "Bli�njice"
${LSTR} TITLE_ssection_shortcuts_cu         "Bli�njice (za trenutnega uporabnika)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Bli�njice v meniju Start (za trenutnega uporabnika)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Bli�njica na namizju (za trenutnega uporabnika)"
${LSTR} TITLE_ssection_shortcuts_au         "Bli�njice (za vse uporabnike)"
${LSTR} TITLE_section_sm_shortcuts_au       "Bli�njice v meniju Start (za vse uporabnike)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Bli�njica na namizju (za vse uporabnike)"
${LSTR} TITLE_ssection_fa_shellupdate       "Posodobi povezave s kon�nicami datotek"
${LSTR} TITLE_section_fa_abw                "Pove�i .abw z AbiWordom"
${LSTR} TITLE_section_fa_awt                "Pove�i .awt z AbiWordom"
${LSTR} TITLE_section_fa_zabw               "Pove�i .zabw z AbiWordom"
${LSTR} TITLE_section_fa_doc                "Pove�i .doc z AbiWordom"
${LSTR} TITLE_section_fa_rtf                "Pove�i .rtf z AbiWordom"
${LSTR} TITLE_ssection_helper_files         "Pomo�ne datoteke"
${LSTR} TITLE_section_help                  "Datoteke pomo�i"
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
${LSTR} TITLE_section_dictinary_def_English "en-US  angle�ki (ZDA) (privzeto)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Prenesi dodatne slovarje"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Vti�niki"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Osnovni nabor komponent, da bo program AbiWord lahko deloval."
${LSTR} DESC_section_abi              "Zahtevano.  Namesti sam program ${PROGRAMEXE}."
${LSTR} DESC_section_abi_req          "Zahtevano.  Namesti osnovne pomo�ne datoteke, npr. nabore nizov, podporo za BMP odlo�i��a itn."
${LSTR} DESC_ssection_shortcuts       "Namesti bli�njice na razli�nih mestih, ki omogo�ajo poganjanje AbiWorda iz dodatnih mest."
${LSTR} DESC_ssection_shortcuts_cu    "Namesti bli�njice za trenutno prijavljenega uporabnika."
${LSTR} DESC_ssection_shortcuts_au    "Namesti bli�njice za vse uporabnike (ali trenutnega uporabnika na enouporabni�kih sistemih)."
${LSTR} DESC_ssection_fa_shellupdate  "Doda vnose v register, kar omogo�a Raziskovalcu z uporabo AbiWorda odpirati razli�ne vrste dokumentov."
${LSTR} DESC_section_fa_abw           "Dolo�a, da bo AbiWord uporabljen za odpiranje dokumentov v njegovem lastnem zapisu.  (Priporo�eno)"
${LSTR} DESC_section_fa_awt           "Dolo�a, da bo AbiWord uporabljen za odpiranje predlog v njegovem lastnem zapisu.  (Priporo�eno)"
${LSTR} DESC_section_fa_zabw          "Dolo�a, da bo AbiWord uporabljen za odpiranje stisnjenih dokumentov v njegovem lastnem zapisu.  (Priporo�eno)"
${LSTR} DESC_section_fa_doc           "Dolo�a, da bo AbiWord uporabljen za odpiranje dokumentov v zapisu Microsoft Word (R)."
${LSTR} DESC_section_fa_rtf           "Dolo�a, da bo AbiWord uporabljen za odpiranje datotek z obogatenim besedilom (Rich Text File), 'standardnim' zapisom urejevalnikov besedil."
${LSTR} DESC_ssection_helper_files    "Namesti razli�ne druge dodatne datoteke, ki pomagajo pri uporabi AbiWorda."
${LSTR} DESC_section_help             "Namesti dokumente pomo�i; �e to izpustite, pomo� ne bo na voljo."
${LSTR} DESC_section_templates        "Namesti predloge, s katerimi si lahko pomagate pri ustvarjanju novih dokumentov z vnaprej pripravljenim oblikovanjem."
${LSTR} DESC_section_samples          "Vzorci so bili odstranjeni."
${LSTR} DESC_section_clipart          "Namesti slike (izrezke), ki jih lahko vstavite v dokumente."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Namesti izvajalno knji�nico C, ki jo potrebuje AbiWord; uporabno, �e va� operacijski sistem le-tega nima."
!endif
${LSTR} DESC_ssection_dictionary      "Namesti slovarje za razli�ne jezike, s katerimi lahko preverjate �rkovanje dokumentov."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Namesti razli�ne dodatne vti�nike."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "�elite prepisati obstoje�o namestitev ${PRODUCT}?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} najverjetneje ni pravilno name��en!$\r$\n\
                                               Datoteke ${MAINPROGRAM} ni mogo�e najti, zato bo ponovno name��ena.$\r$\n\
                                               �elite nadaljevati s spreminjanjem namestitve?"
${LSTR} MSG_ABORT                             "Zapu��anje namestitvenega postopka"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Zahtevane knji�nice (DLL) ni mogo�e namestiti: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "Osnovni URL za prenos dodatnih komponent"
${LSTR} TEXT_IO_SUBTITLE              "Slovarji"
${LSTR} MSG_SELECT_DL_MIRROR          "Izberite zrcalni stre�nik za prenos ..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Napaka pri pridobivanju zahtevanega, uporabljen bo privzeti stre�nik!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "Urejevalnik besedil ${PRODUCT}"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Odstrani ${PRODUCT} ${VERSION_MAJOR}.${VERSION_MINOR}"
${LSTR} SHORTCUT_NAME_HELP            "Pomo� za ${PRODUCT} (v angl.)"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "�elite izbrisati $INSTDIR in vse podmape in datoteke?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "katalonski"
${LSTR} dict_Czech         "�e�ki"
${LSTR} dict_Danish        "danski"
${LSTR} dict_Swiss         "�vicarski"
${LSTR} dict_Deutsch       "nem�ki"
${LSTR} dict_Ellhnika      "gr�ki"
${LSTR} dict_English       "angle�ki (VB)"
${LSTR} dict_American      "angle�ki (ZDA)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "�panski"
${LSTR} dict_Finnish       "finski"
${LSTR} dict_Francais      "francoski"
${LSTR} dict_Hungarian     "mad�arski"
${LSTR} "dict_Irish gaelic""irski"
${LSTR} dict_Galician      "galicijski"
${LSTR} dict_Italian       "italijanski"
${LSTR} dict_Kurdish       "kurdski"
${LSTR} dict_Latin         "latinski"
${LSTR} dict_Lietuviu      "litovski"
${LSTR} dict_Dutch         "nizozemski"
${LSTR} dict_Norsk         "norve�ki (Bokmal)"
${LSTR} dict_Nynorsk       "norve�ki (Nynorsk)"
${LSTR} dict_Polish        "poljski"
${LSTR} dict_Portugues     "portugalski"
${LSTR} dict_Brazilian     "brazilski"
${LSTR} dict_Russian       "ruski"
${LSTR} dict_Sardinian     "sardinijski"
${LSTR} dict_Slovensko     "slovenski"
${LSTR} dict_Svenska       "�vedski"
${LSTR} dict_Ukrainian     "ukrajinski"


; End Language descriptions
