;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Tasks done during initialization


!include "abi_util_sectdisable.nsh"


; Perform one time steps done at installer startup, e.g. get installer language, 
; check internet connection/OS/etc and enable/disable options as appropriate
Function .onInit
MessageBox MB_OK "TESTING: .onInit"

  ; Load default values and parse command line
  ${ProcessCmdLineArgs}

  ; Select language the installation is displayed in
  !insertmacro MUI_LANGDLL_DISPLAY

  ;Reads components status from registry (in case of change or re-install)
  !insertmacro SectionList "InitSection"

!ifndef NODOWNLOADS
  ; Disable all downloads if not connected
  Call ConnectInternet	; try to establish connection if not connected
  StrCmp $0 "online" connected
  !ifdef OPT_DICTIONARIES
	${SectionDisable} ${ssection_dl_opt_dict}
	!insertmacro cycle_over_dictionary_sections "${SectionDisable} $R1"
  !endif
  !ifdef OPT_CRTL_URL
  	${SectionDisable} ${section_crtlib_dl}
  !endif
  connected:
!endif ;NODOWNLOADS

; Disable Windows 95 specific sections
!ifdef OPT_CRTL_WIN95ONLY
StrCmp $v_opt_enable_win95only "1" skipDisableW95dl  ; cmd line opt to skip OS check and leave enabled
Call GetWindowsVersion
Pop $R0
StrCmp $R0 '95' skipDisableW95dl 0	; disable for all but Windows 95
  !ifdef OPT_CRTL_URL
     ${SectionDisable} ${section_crtlib_dl}
  !endif
  !ifdef OPT_CRTL_LOCAL
     ${SectionDisable} ${section_crtlib_local}
  !endif
skipDisableW95dl:
!endif ;OPT_CRTL_WIN95ONLY


FunctionEnd
