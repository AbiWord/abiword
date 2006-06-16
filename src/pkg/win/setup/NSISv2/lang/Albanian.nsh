;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Language Strings, descriptions for Sections and SubSections
; English ${LANG_English}

; Section titles, what user sees to select components for installation
${LSTR} TITLE_ssection_core                 "Përbërërit parësorë"
${LSTR} TITLE_section_abi                   "${PROGRAMEXE} (i nevojshëm)"
${LSTR} TITLE_section_abi_req               "${PRODUCT} kartela suporti (të nevojshme)"
${LSTR} TITLE_ssection_shortcuts            "Shkurtprerje"
${LSTR} TITLE_ssection_shortcuts_cu         "Shkurtprerje (Përdoruesi i Çastit)"
${LSTR} TITLE_section_sm_shortcuts_cu       "Shkurtprerje Menuje Start (Përdoruesi i Çastit)"
${LSTR} TITLE_section_desktop_shortcuts_cu  "Shkurtprerje Desktopi (Përdoruesi i Çastit)"
${LSTR} TITLE_ssection_shortcuts_au         "Shkurtprerje (Tërë Përdoruesët)"
${LSTR} TITLE_section_sm_shortcuts_au       "Shkurtprerje Menuje Start (Tërë Përdoruesët)"
${LSTR} TITLE_section_desktop_shortcuts_au  "Shkurtprerje Desktopi (Tërë Përdoruesët)"
${LSTR} TITLE_ssection_fa_shellupdate       "Përditëso zgjatime kartelash shelli"
${LSTR} TITLE_section_fa_abw                "Shoqëro .abw me AbiWord"
${LSTR} TITLE_section_fa_awt                "Shoqëro .awt me AbiWord"
${LSTR} TITLE_section_fa_zabw               "Shoqëro .zabw me AbiWord"
${LSTR} TITLE_section_fa_doc                "Shoqëro .doc me AbiWord"
${LSTR} TITLE_section_fa_rtf                "Shoqëro .rtf me AbiWord"
${LSTR} TITLE_ssection_helper_files         "Kartela Ndihmësi"
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
${LSTR} TITLE_ssection_dictionary           "Fjalorë"
${LSTR} TITLE_section_dictinary_def_English "en-US  US English (parazgjedhje)"
!ifdef OPT_DICTIONARIES
${LSTR} TITLE_ssection_dl_opt_dict          "Shkarko fjalorë të tjerë"
!endif
!ifdef OPT_PLUGINS
${LSTR} TITLE_ssection_plugins              "Shtojca"
!endif

; Section descriptions displayed to user when mouse hovers over a section
${LSTR} DESC_ssection_core            "Grupi parësor (bërthama) i përbërësve që AbiWord të xhirojë mirë."
${LSTR} DESC_section_abi              "I nevojshëm.  Instalon programin aktual ${PROGRAMEXE}."
${LSTR} DESC_section_abi_req          "I nevojshëm.  Instalon kartela bazë suporti, p.sh. vargje, BMP clipboard support, etj."
${LSTR} DESC_ssection_shortcuts       "Instalon shkurtprerje në vende të ndryshëm për të mundësuar nisjen e AbiWord-it nga vendndodhje të ndryshme."
${LSTR} DESC_ssection_shortcuts_cu    "Instalon shkurtprerje për përdoruesin e çastit."
${LSTR} DESC_ssection_shortcuts_au    "Instalon shkurtprerje për tërë përdoruesit (ose përdoruesin e çastit në sisteme pa përdoruesa të shumëfishtë)."
${LSTR} DESC_ssection_fa_shellupdate  "Shton zëra në regjistër për t'i lejuar Explorer-it të përdorë AbiWord-in të hapë formate të ndryshëm dokumentesh."
${LSTR} DESC_section_fa_abw           "Përcakton që AbiWord do të duhej të përdorej për të hapur dokumentet në formatin e tyre origjinal.  (E këshillueshme)"
${LSTR} DESC_section_fa_awt           "Përcakton që AbiWord do të duhej të përdorej për të hapur stampa në formatin e tyre origjinal.  (E këshillueshme)"
${LSTR} DESC_section_fa_zabw          "Përcakton që AbiWord do të duhej të përdorej për të hapur dokumente të ngjeshur në formatin e tyre origjinal.  (E këshillueshme)"
${LSTR} DESC_section_fa_doc           "Përcakton që AbiWord do të duhej të përdorej për të hapur dokumente me format origjinal Microsoft Word (R)."
${LSTR} DESC_section_fa_rtf           "Përcakton që AbiWord do të duhej të përdorej për të hapur Kartela Rich Text, një format 'standard' për Fjalëpërpunuesa."
${LSTR} DESC_ssection_helper_files    "Instalon kartela të ndryshme të mundshme për të ndihmuar përdorimin e AbiWord-it."
${LSTR} DESC_section_help             "Instalon dokumentet e ndihmës, nuk ka ndihmë të mundshme po u lanë jashtë këta."
${LSTR} DESC_section_templates        "Instalon stampa të cilat mund të përdoren për të ndihmuar në krijim dokumentesh të rinj me formatime të paracaktuar."
${LSTR} DESC_section_samples          "Shembujt janë hequr."
${LSTR} DESC_section_clipart          "Instalon piktura (vizatime) të cilat mund të futen në dokumente."
!ifdef OPT_CRTL_URL | OPT_CRTL_LOCAL
${LSTR} DESC_section_crtlib           "Instalon C Runtime Library të përdorur nga AbiWord, e dobishme nëse sistemit tuaj i mungon."
!endif
${LSTR} DESC_ssection_dictionary      "Instalon fjalorë për gjuhë të ndryshme të cilët përdoren për kontroll drejtshkrimi të dokumentit tuaj."
!ifdef OPT_DICTIONARIES
!endif
!ifdef OPT_PLUGINS
${LSTR} DESC_ssection_plugins         "Instalon shtojca të ndryshme të mundshme."
!endif

; Error messages and other text displayed in Detail Window or in MessageBoxes

; in the main section
${LSTR} PROMPT_OVERWRITE                      "Të mbishkruaj ${PRODUCT} ekzistues?"
${LSTR} PROMPT_NOMAINPROGRAM_CONTINUEANYWAY   "${PRODUCT} nuk duket të jetë instaluar si duhet!$\r$\n\
                                               Dështova në gjetjen e ${MAINPROGRAM}, do të riinstalohet.$\r$\n\
                                               Të vazhdohet me ndryshimin e instalimit?"
${LSTR} MSG_ABORT                             "Po lë procesin e instalimit"

; sections involving additional downloads
!ifndef NODOWNLOADS

; C Runtime Library
!ifdef OPT_CRTL_URL
; CRTLError downloading
${LSTR} PROMPT_CRTL_DL_FAILED         "Dështova në shkarkimin e c runtime library (DLL) të nevojshme: ${OPT_CRTL_URL}${OPT_CRTL_FILENAME}"
!endif ; OPT_CRTL_URL

; for dictionary stuff
!ifdef OPT_DICTIONARIES
; Custom Download page
${LSTR} TEXT_IO_TITLE                 "URL Bazë Përbërësish të Shkarkueshëm të Mundshëm"
${LSTR} TEXT_IO_SUBTITLE              "Fjalorë"
${LSTR} MSG_SELECT_DL_MIRROR          "Zgjidh pasqyrë shkarkimi..."
${LSTR} MSG_ERROR_SELECTING_DL_MIRROR "Gabim në marrjen e zgjedhjes së përdoruesit, po përdor site-in parazgjedhje!"
!endif ; OPT_DICTIONARIES

!endif ; NODOWNLOADS

; Start menu & desktop
${LSTR} SM_PRODUCT_GROUP              "${PRODUCT} Fjalë Përpunues"
${LSTR} SHORTCUT_NAME                 "${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_UNINSTALL       "Çinstalo ${PRODUCT} v${VERSION_MAJOR}"
${LSTR} SHORTCUT_NAME_HELP            "(Shqip) Ndihmë për ${PRODUCT}"

; Uninstall Strings
${LSTR} UNINSTALL_WARNING       "Kjo do të fshijë $INSTDIR me tërë nëndrejtoritë dhe kartelat!?"


; Localized Dictionary names (language supported by dictionary, not dictionary filename)
${LSTR} dict_Catalan       "Katalane"
${LSTR} dict_Czech         "Çekisht"
${LSTR} dict_Danish        "Danisht"
${LSTR} dict_Swiss         "Svicerane"
${LSTR} dict_Deutsch       "Gjermanisht"
${LSTR} dict_Ellhnika      "Greqisht"
${LSTR} dict_English       "Anglisht (GB)"
${LSTR} dict_American      "Anglisht (US)"
${LSTR} dict_Esperanto     "Esperanto"
${LSTR} dict_Espanol       "Spanjisht"
${LSTR} dict_Finnish       "Finlandisht"
${LSTR} dict_Francais      "Frëngjisht"
${LSTR} dict_Hungarian     "Hungarisht"
${LSTR} "dict_Irish gaelic""Irlandisht galike"
${LSTR} dict_Galician      "Galike"
${LSTR} dict_Italian       "Italisht"
${LSTR} dict_Latin         "Latinisht"
${LSTR} dict_Lietuviu      "Lituanisht"
${LSTR} dict_Dutch         "Holandisht"
${LSTR} dict_Norsk         "Norvegjisht Bokmål"
${LSTR} dict_Nynorsk       "Norvegjisht Nynorsk"
${LSTR} dict_Polish        "Polonisht"
${LSTR} dict_Portugues     "Portugalisht"
${LSTR} dict_Brazilian     "Brazilian"
${LSTR} dict_Russian       "Rusisht"
${LSTR} dict_Slovensko     "Slovenisht"
${LSTR} dict_Svenska       "Suedisht"
${LSTR} dict_Ukrainian     "Ukrainisht"


; End Language descriptions
