;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       User defined compile time options.


; Define this for the older (non 'Modern') user interface, English only
;!define CLASSIC_UI

; Define this to not include the optional downloadable components (dictionaries, crtlib, ...)
;!define NODOWNLOADS

; To include the C Runtime Library for the compiler used
; Define either the URL for downloadable one or LOCAL to include within setup (include final slash)
; then define the actual crt library filename (both file and (URL or LOCAL) must be defined)
; [If OPT_CRTL_URL and OPT_CRTL_LOCAL are defined, the local one will be used!]
; Optionally add a C++ [STL] library
; 1a) the base URL where the crt library can be downloaded from
;!define OPT_CRTL_URL "http://abiword.pchasm.org/microsoft/"
; 1b) alternately where the file may be found on the local filesystem for inclusion
;!define OPT_CRTL_LOCAL "\Program Files\Microsoft Visual Studio\REDIST\"
; 2) the actual filename of the crt library
;!define OPT_CRTL_FILENAME "msvcrt.dll"  ; MSVC 5 and 6 (not for Windows Me, NT 2000, or newer)
;!define OPT_CRTL_FILENAME "msvcr70.dll" ; MSVC 7
;!define OPT_CPPL_FILENAME "msvcp70.dll" ; MSVC 7
; 3) Optional, indicate if crt library is only needed for Windows 95 (e.g. msvcrt.dll)
;!define OPT_CRTL_WIN95ONLY

; Define this to include the dictionaries in the installer
; if NODOWNLOADS is defined then the files must be locally available
!define OPT_DICTIONARIES


; Define this to include the standard set of plugins
; if NODOWNLOADS is defined then the files must be locally available
; if you enable this, you must build make toolsplugins, make impexpplugins, then finally make distribution
;!define OPT_PLUGINS

; If you have upx available in your PATH, enable this for a smaller setup file
;!define HAVE_UPX

; Specify this one only if you are the TradeMark holder!!!
;!define TRADEMARKED_BUILD

; Initially selected/default download location for dictionary files
; we attempt to let user pick, but this is our fallback/default entry
!define DICTIONARY_BASE_DEFAULT "http://www.abisource.com/downloads/dictionaries/Windows/archives"

; The other/mirror locations for downloading dictionary files
!define DICTIONARY_BASE_MIRRORS "http://unc.dl.sourceforge.net/abiword|http://telia.dl.sourceforge.net/abiword|http://umn.dl.sourceforge.net/abiword|http://twtelecom.dl.sourceforge.net/abiword|http://easynews.dl.sourceforge.net/abiword|http://belnet.dl.sourceforge.net/abiword|http://cesnet.dl.sourceforge.net/abiword|http://switch.dl.sourceforge.net/abiword"

; reserve room at start of compressed archive for plugins [leave defined]
!define RESERVE_PLUGINS
