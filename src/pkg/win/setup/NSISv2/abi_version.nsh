;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Specify a version resource for installer matching installed application


VIProductVersion "${VERSION}.0"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${PRODUCT}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Word Processor, see http://www.abisource.com/"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "AbiSource Developers"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "AbiWord, AbiSource are trademarks of SourceGear Inc."
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "© AbiSource"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Installer for AbiWord"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
