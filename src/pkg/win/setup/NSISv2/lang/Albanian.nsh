;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
; English ${LANG_English}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "P�rb�r�rit par�sor�"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (i nevojsh�m)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} kartela suporti (t� nevojshme)"
${LSTR} TITLE_ssection_shortcuts            "Shkurtprerje"
${LSTR} TITLE_ssection_shortcuts_cu         "Shkurtprerje (P�rdoruesi i �astit)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Shkurtprerje Menuje Start (P�rdoruesi i �astit)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Shkurtprerje Desktopi (P�rdoruesi i �astit)"
${LSTR} TITLE_ssection_shortcuts_au         "Shkurtprerje (T�r� P�rdorues�t)"
${LSTR} TITLE_section_sm_shortcuts_au       "Shkurtprerje Menuje Start (T�r� P�rdorues�t)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Shkurtprerje Desktopi (T�r� P�rdorues�t)"
${LSTR} TITLE_ssection_fa_shellupdate       "P�rdit�so zgjatime kartelash shelli"
${LSTR} TITLE_section_fa_abw                "Shoq�ro .abw me AbiWord"
${LSTR} TITLE_section_fa_awt                "Shoq�ro .awt me AbiWord"
${LSTR} TITLE_section_fa_zabw               "Shoq�ro .zabw me AbiWord"
${LSTR} TITLE_section_fa_doc                "Shoq�ro .doc me AbiWord"
${LSTR} TITLE_section_fa_rtf                "Shoq�ro .rtf me AbiWord"
${LSTR} TITLE_ssection_helper_files         "Kartela Ndihm�si"
${LSTR} TITLE_section_help                  "Kartela Ndihme"
${LSTR} TITLE_section_templates             "Stampa"
;${LSTR} TITLE_section_samples              "Shembuj"
${LSTR} TITLE_section_clipart               "Vizatime"
!ifdef OPT_CRTL_LOCAL
${LSTR} TITLE_section_crtlib_local          "CRTlib ${OPT_CRTL_FILENAME}"
!endif
!ifdef OPT_CRTL_URL
${LSTR} TITLE_section_crtlib_dl             "Shkarko CRTlib ${OPT_CRTL_FILENAME}"
!endif
${LSTR} TITLE_ssection_dictionary           "Fjalor�"
${LSTR} TITLE_section_dictinary_def_English "en-US  US English (parazgjedhje)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Shkarko fjalor� t� tjer�"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Shtojca"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Grupi par�sor (b�rthama) i p�rb�r�sve q� AbiWord t� xhiroj� mir�."
${LSTR} DESC_section_abi              "I nevojsh�m.  Instalon programin aktual ${PROGRAMEXE}."
${LSTR} DESC_section_abi_req          "I nevojsh�m.  Instalon kartela baz� suporti, p.sh. vargje, BMP clipboard support, etj."
${LSTR} DESC_ssection_shortcuts       "Instalon shkurtprerje n� vende t� ndrysh�m p�r t� mund�suar nisjen e AbiWord-it nga vendndodhje t� ndryshme."
${LSTR} DESC_ssection_shortcuts_cu    "Instalon shkurtprerje p�r p�rdoruesin e �astit."
${LSTR} DESC_ssection_shortcuts_au    "Instalon shkurtprerje p�r t�r� p�rdoruesit (ose p�rdoruesin e �astit n� sisteme pa p�rdoruesa t� shum�fisht�)."
${LSTR} DESC_ssection_fa_shellupdate  "Shton z�ra n� regjist�r p�r t'i lejuar Explorer-it t� p�rdor� AbiWord-in t� hap� formate t� ndrysh�m dokumentesh."
${LSTR} DESC_section_fa_abw           "P�rcakton q� AbiWord do t� duhej t� p�rdorej p�r t� hapur dokumentet n� formatin e tyre origjinal.  (E k�shillueshme)"
${LSTR} DESC_section_fa_awt           "P�rcakton q� AbiWord do t� duhej t� p�rdorej p�r t� hapur stampa n� formatin e tyre origjinal.  (E k�shillueshme)"
${LSTR} DESC_section_fa_zabw          "P�rcakton q� AbiWord do t� duhej t� p�rdorej p�r t� hapur dokumente t� ngjeshur n� formatin e tyre origjinal.  (E k�shillueshme)"
${LSTR} DESC_section_fa_doc           "P�rcakton q� AbiWord do t� duhej t� p�rdorej p�r t� hapur dokumente me format origjinal Microsoft Word (R)."
${LSTR} DESC_section_fa_rtf           "P�rcakton q� AbiWord do t� duhej t� p�rdorej p�r t� hapur Kartela Rich Text, nj� format 'standard' p�r Fjal�p�rpunuesa."
${LSTR} DESC_ssection_helper_files    "Instalon kartela t� ndryshme t� mundshme p�r t� ndihmuar p�rdorimin e AbiWord-it."
${LSTR} DESC_section_help             "Instalon dokumentet e ndihm�s, nuk ka ndihm� t� mundshme po u lan� jasht� k�ta."
${LSTR} DESC_section_templates        "Instalon stampa t� cilat mund t� p�rdoren p�r t� ndihmuar n� krijim dokumentesh t� rinj me formatime t� paracaktuar."
${LSTR} DESC_section_samples          "Shembujt jan� hequr."
${LSTR} DESC_section_clipart          "Instalon piktura (vizatime) t� cilat mund t� futen n� dokumente."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Instalon C Runtime Library t� p�rdorur nga AbiWord, e dobishme n�se sistemit tuaj i mungon."
!endif
${LSTR} DESC_ssection_dictionary      "Instalon fjalor� p�r gjuh� t� ndryshme t� cil�t p�rdoren p�r kontroll drejtshkrimi t� dokumentit tuaj."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Instalon shtojca t� ndryshme t� mundshme."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "T� mbishkruaj ${PRODUCT} ekzistues?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} nuk duket t� jet� instaluar si duhet!$\r$\n\
                                               D�shtova n� gjetjen e ${MAINPROGRAM}, do t� riinstalohet.$\r$\n\
                                               T� vazhdohet me ndryshimin e instalimit?"
${LSTR} MSG_ABORT                             "Po l� procesin e instalimit"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "D�shtova n� shkarkimin e c runtime library (DLL) t� nevojshme: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "URL Baz� P�rb�r�sish t� Shkarkuesh�m t� Mundsh�m"
${LSTR} TEXT_IO_SUBTITLE              "Fjalor�"
${LSTR} MSG_SELECT_DL_MIRROR          "Zgjidh pasqyr� shkarkimi..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Gabim n� marrjen e zgjedhjes s� p�rdoruesit, po p�rdor site-in parazgjedhje!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} Fjal� P�rpunues"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "�instalo ${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_HELP            "(Shqip) Ndihm� p�r ${PRODUCT}"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Kjo do t� fshij� $INSTDIR me t�r� n�ndrejtorit� dhe kartelat!?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Katalane"
${LSTR} dict_Czech         "�ekisht"
${LSTR} dict_Danish        "Danisht"
${LSTR} dict_Swiss         "Svicerane"
${LSTR} dict_Deutsch       "Gjermanisht"
${LSTR} dict_Ellhnika      "Greqisht"
${LSTR} dict_English       "Anglisht (GB)"
${LSTR} dict_American      "Anglisht (US)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Spanjisht"
${LSTR} dict_Finnish       "Finlandisht"
${LSTR} dict_Francais      "Fr�ngjisht"
${LSTR} dict_Hungarian     "Hungarisht"
${LSTR} "dict_Irish gaelic""Irlandisht galike"
${LSTR} dict_Galician      "Galike"
${LSTR} dict_Italian       "Italisht"
${LSTR} dict_Latin         "Latinisht"
${LSTR} dict_Lietuviu      "Lituanisht"
${LSTR} dict_Dutch         "Holandisht"
${LSTR} dict_Norsk         "Norvegjisht Bokm�l"
${LSTR} dict_Nynorsk       "Norvegjisht Nynorsk"
${LSTR} dict_Polish        "Polonisht"
${LSTR} dict_Portugues     "Portugalisht"
${LSTR} dict_Brazilian     "Brazilian"
${LSTR} dict_Russian       "Rusisht"
${LSTR} dict_Slovensko     "Slovenisht"
${LSTR} dict_Svenska       "Suedisht"
${LSTR} dict_Ukrainian     "Ukrainisht"


; End Language descriptions
