;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       List of dictionaries that may be included or optionally downloaded


; this is the count of dictionaries available for download [count of sections defined]
!define DICTIONARY_COUNT 27	; used to query sections & set description text


; These are listed alphabetically based on English LANG-LOCALE
; NOTE: if the dictinaries are updated so to should these sizes (KB)
; Be sure to update DICTIONARY_COUNT above (so description & selection query work correctly)
;${SectionDict} "Language name" "Language Abbr" "Locale/country code" "endianess [i386|ppc]" "filesize"

${SectionDict} "Catalan"      "ca" "ES" "i386"  4324
${SectionDict} "Czech"        "cs" "DZ" "i386"  2558
${SectionDict} "Danish"       "da" "DK" "i386"  1580
${SectionDict} "Swiss"        "de" "CH" "i386"  8501
${SectionDict} "Deutsch"      "de" "DE" "i386"  2277
${SectionDict} "Ellhnika"     "el" "GR" "i386"  2049  ;Greek
${SectionDict} "English"      "en" "GB" "i386"  2109
${SectionDict} "Esperanto"    "eo" "Xx" "i386"   942  ;no locale...
${SectionDict} "Espa�ol"      "es" "ES" "i386"  2632
${SectionDict} "Finnish"      "fi" "FI" "i386" 10053
${SectionDict} "Fran�ais"     "fr" "FR" "i386"  1451
${SectionDict} "Hungarian"    "hu" "HU" "i386"  8086
${SectionDict} "Irish gaelic" "ga" "IE" "i386"   587
${SectionDict} "Galician"     "gl" "ES" "i386"   807
${SectionDict} "Italian"      "it" "IT" "i386"  1638
${SectionDict} "Latin"        "la" "IT" "i386"  2254  ;mlatin
${SectionDict} "Lietuviu"     "lt" "LT" "i386"  1907  ;Lithuanian
${SectionDict} "Dutch"        "nl" "NL" "i386"  1079  ;nederlands
${SectionDict} "Norsk"        "nb" "NO" "i386"  2460  ;Norwegian
${SectionDict} "Nynorsk"      "nn" "NO" "i386"  3001  ;Norwegian(nynorsk)
${SectionDict} "Polish"       "pl" "PL" "i386"  4143
${SectionDict} "Portugues"    "pt" "PT" "i386"  1117  ;Portuguese
${SectionDict} "Brazilian"    "pt" "BR" "i386"  1244  ;Portuguese
${SectionDict} "Russian"      "ru" "RU" "i386"  8307
${SectionDict} "Slovensko"    "sl" "SI" "i386"   857  ;Slovenian
${SectionDict} "Svenska"      "sv" "SE" "i386"   753  ;Swedish
${SectionDict} "Ukrainian"    "uk" "UA" "i386"  3490

