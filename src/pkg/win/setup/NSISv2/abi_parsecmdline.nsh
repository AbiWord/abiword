;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Processes command line for options we care about and
;               sets internal variables accordingly


!ifndef _ABI_PARSECMDLINE_NSH_
!define _ABI_PARSECMDLINE_NSH_

!include "abi_util_getcmdlnparams.nsh"


; /OPT_ENABLE_MODIFY
; Define this to add an entry to the control panel for modify
; note: assumes installer remains at same location AbiWord originally installed
; from, so in general only useful for AbiWord from CD-ROM or similar
; as AbiWord from download can in no way guarentee constant source
; values are actual _no_modify registry entry, 0 (enable modify option), 1 (disable modify [enable nomodify])
Var v_opt_modify_reg

; /OPT_MODIFY_PATH
; Override the path stored in the registry, default to installer.
Var v_opt_modify_path


!ifdef NODOWNLOADS  ; only process download related items if downloads enabled

; /OPT_ENABLE_DOWNLOADS
; ignore checks for connection and force enabling of downloadable components & pages
; /OPT_DISABLE_DOWNLOADS
; ignore checks for connection and force disabling of downloadable comonents & pages
; Note: default action is to enable/disable depending on if internet connected detected
; values are -1 (default), 0 (disabled), 1 (enabled)
Var v_opt_enable_downloads

!endif


!ifdef OPT_CRTL_WIN95ONLY  ; change as needed if any other Win95 only options added
; /OPT_ENABLE_WIN95ONLY
; Prevent disabling of Win95 specific options on non Win95 systems
Var v_opt_enable_win95only
!endif


!macro DoHelpCmd
  MessageBox MB_OK "Support options are $\r$\n  /S silent install $\r$\n\
  /OPT_ENABLE_MODIFY add modify entry to add/remove in control panel$\r$\n\
  /OPT_MODIFY_PATH=path indicates where installer is for modify entry$\r$\n\
  /OPT_ENABLE_DOWNLOADS skip detection of connection and assume can download$\r$\n\
  /OPT_DISABLE_DOWNLOADS skip detection of connection and assume can NOT download$\r$\n"
  Quit
!macroend
!define DoHelpCmd "!insertmacro DoHelpCmd"


!macro SetOptionValue cmdarg optval
  ${Select} ${cmdarg}
    ${Case2} "/HELP" "/?"
      ${DoHelpCmd}
    ${Case} "/OPT_ENABLE_MODIFY"
      StrCpy $v_opt_modify_reg "0"             ; enable modify (nomodify = 0)
    ${Case} "/OPT_MODIFY_PATH"
      StrCpy $v_opt_modify_path "${optval}"
!ifdef NODOWNLOADS  ; only process download related items if downloads enabled
    ${Case} "/OPT_ENABLE_DOWNLOADS"
      StrCpy $v_opt_enable_downloads "1"
    ${Case} "/OPT_DISABLE_DOWNLOADS"
      StrCpy $v_opt_enable_downloads "0"
!endif
!ifdef OPT_CRTL_WIN95ONLY  ; change as needed if any other Win95 only options added
    ${Case} "/OPT_ENABLE_WIN95ONLY"
      StrCpy $v_opt_enable_win95only "1"
!endif
    ${CaseElse}
      MessageBox MB_OK "Error: Unsupported option ${cmdarg} found!"
      ${DoHelpCmd}
  ${EndSelect}
!macroend
!define SetOptionValue "!insertmacro SetOptionValue"


; call this macro during oninit or similar to actually initialize the
; above variables either with their defaults or command line overrides
!macro ProcessCmdLineArgs

  ; first initialize them all to their default values
  StrCpy $v_opt_modify_reg "1"                               ; disable modify (nomodify = 1)
  StrCpy $v_opt_modify_path '"$EXEDIR\${INSTALLERNAME}"'     ; where ever current installer is

  !ifdef NODOWNLOADS  ; only process download related items if downloads enabled
    StrCpy $v_opt_enable_downloads "-1"
  !endif

  !ifdef OPT_CRTL_WIN95ONLY  ; change as needed if any other Win95 only options added
    StrCpy $v_opt_enable_win95only "0"
  !endif

  ; now cycle through all the cmd line options and set the values
  Call GetParameters
  pop $0

  ${SetOptionValue} "/OPT_ENABLE_WIN95ONLY" ""

!macroend
!define ProcessCmdLineArgs "!insertmacro ProcessCmdLineArgs"


!endif ; _ABI_PARSECMDLINE_NSH_
