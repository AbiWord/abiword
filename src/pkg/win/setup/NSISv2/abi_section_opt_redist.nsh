;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for redistributable components

!ifdef OPT_CRTL_LOCAL
; OPTIONAL Installation of c runtime library dll
; Hidden if for Win95 only (e.g msvcrt.dll)
Section "$(TITLE_section_crtlib_local)" section_crtlib_local
	SectionIn 2 ${DLSECT}	; select if full installation choosen
	SetOutPath $INSTDIR\${PRODUCT}\bin

	; Note: since NSIS does not support / in File commands, we 
	; first copy all the redistributables to ./REDIST and then
	; use File without the base path that may contain either slash
	!system "md REDIST"
	!system "cp -r ${OPT_CRTL_LOCAL}* ./REDIST"

	; include the C Runtime Lib
	File "REDIST\${OPT_CRTL_FILENAME}"

	; optionally in the C++ Runtime support Lib
	!ifdef OPT_CPPL_FILENAME
	File "REDIST\${OPT_CPPL_FILENAME}"
	!endif

SectionEnd
!endif ; OPT_CRTL_LOCAL


; we only enable this option if a url to connect to was
; specified during installation building; this should
; only be enabled for release builds if your server (where
; the url points) can handle the load and you need
; a crtlib other than msvcrt.dll (or to support Win95)
!ifdef OPT_CRTL_URL

!include "abi_util_dl.nsh"

; OPTIONAL Installation of c runtime library dll
; Hidden if for Win95 only (e.g msvcrt.dll)
Section "$(TITLE_section_crtlib_dl)" section_crtlib_dl
	SectionIn 2	${DLSECT}	; select if full installation choosen

	${dlFile} "${OPT_CRTL_URL}${OPT_CRTL_FILENAME}" "$INSTDIR\${PRODUCT}\bin\${OPT_CRTL_FILENAME}" "$(PROMPT_CRTL_DL_FAILED)"

	!ifdef OPT_CPPL_FILENAME
	${dlFile} "${OPT_CRTL_URL}${OPT_CRTL_FILENAME}" "$INSTDIR\${PRODUCT}\bin\${OPT_CPPL_FILENAME}" "$(PROMPT_CRTL_DL_FAILED)"
	!endif

SectionEnd
!endif ; OPT_CRTL_URL

