;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       User defined compile time options.


; Define this for the older (non 'Modern') user interface, English only
;!define CLASSIC_UI

; Define this to not include the optional downloadable components (dictionaries, crtlib, ...)
;!define NODOWNLOADS

; To include the C Runtime Library for the compiler used
; Define either the URL for downloadable one or LOCAL to include within setup (include final slash)
; then define the actual crt library filename (both file and (URL or LOCAL) must be defined)
; 1a) the base URL where the crt library can be downloaded from
!define OPT_CRTL_URL "http://abiword.pchasm.org/microsoft/"
; 1b) alternately where the file may be found on the local filesystem for inclusion
;!define OPT_CRTL_LOCAL "\Program Files\Microsoft Visual Studio\REDIST\"
; 2) the actual filename of the crt library
!define OPT_CRTL_FILENAME "msvcrt.dll"  ; MSVC 5 and 6 (not for Windows Me, NT 2000, or newer)
;!define OPT_CRTL_FILENAME "msvcr70.dll"  ; MSVC 7

; Define this to include the dictionaries in the installer
; if NODOWNLOADS is defined then the files must be locally available
!define OPT_DICTIONARIES

; NOT YET AVAILABLE
; Define this to include the standard set of plugins
; if NODOWNLOADS is defined then the files must be locally available
;!define OPT_PLUGINS

; If you have upx available in your PATH, enable this for a smaller setup file
;!define HAVE_UPX

; Specify this one only if you are the TradeMark holder!!!
;!define TRADEMARKED_BUILD
