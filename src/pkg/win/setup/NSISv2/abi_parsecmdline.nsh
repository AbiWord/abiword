;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Processes command line for options we care about and
;               sets internal variables accordingly


!ifndef _ABI_PARSECMDLINE_NSH_
!define _ABI_PARSECMDLINE_NSH_

!include "abi_util_getcmdlnparams.nsh"


; /S
; Silent install, mostly a NSIS handled switch.
; Use IfSilent and /SD option to MessageBox
; See also /RESPONSEFILE and /D=install Directory

; /D=install Directory
; Indicates the default Directory displayed/used for
; installation.  Should be last option on command line
; as all text after equal (=) is treated as directory
; name, including any spaces.  Do not put in quotes.
; Note: /D is processed fully by NSIS so we never see it.

; /INSTALLTYPE=#
; Selects the default Install Type, normally Typical=0
; This is a 0 based number, where 0 is the 1st in the
; list presented.  Most useful with /S (silent install).

; /RESPONSEFILE=filename
; Indicates responses to use (instead of defaults) for various
; prompts.  Not yet implemented
Var v_responsefile

; /M or /MODIFYINSTALL
; This should passed on command line when installer is to work in modify mode
; If invoked with /MODIFYINSTALL, then installer will assume AbiWord has been
; installed already, and adjust its behaviour accordingly.
; Parts of the installer may not be shown or messages changed, also certain
; parts may not be removed within remove function.
; Use StrCmp "$v_modifyinstall" "1" MODIFYACTION NORMALINSTALLACTION
; Note: Do not confuse with /OPT_ENABLE_MODIFY, which adds modify entry to
;       the registry to invoke this installer with /MODIFYINSTALL
Var v_modifyinstall

; /OPT_ENABLE_MODIFY
; Define this to add an entry to the control panel for modify
; note: assumes installer remains at same location AbiWord originally installed
; from, so in general only useful for AbiWord from CD-ROM or similar
; as AbiWord from download can in no way guarentee constant source
; values are actual _no_modify registry entry, 0 (enable modify option), 1 (disable modify [enable nomodify])
Var v_opt_modify_reg

; /OPT_MODIFY_PATH=path
; Override the path stored in the registry, default to installer.
Var v_opt_modify_path


!ifndef NODOWNLOADS  ; only process download related items if downloads enabled

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


; Show command line options, not localized (ie this help only in English)
!macro DoHelpCmd
  StrCpy $R0 "\
  Support options are $\r$\n  \
  /S silent install $\r$\n  \
  /D=path sets default install dir, MUST be last option, No quotes, supports spaces $\r$\n  \
  /INSTALLTYPE=# sets default install type to nth option, e.g. 0=Typical $\r$\n  \
  /RESPONSEFILE=filename indicates choices to use instead of defaults (TODO) $\r$\n  \
  /M or /MODIFYINSTALL invokes installer in modify mode$\r$\n        \
        ( change components installed for current installation of AbiWord$\r$\n  \
  /OPT_ENABLE_MODIFY add modify entry to add/remove in control panel$\r$\n  \
  /OPT_MODIFY_PATH=path indicates where installer is for modify entry$\r$\n"
!ifndef NODOWNLOADS  ; only process download related items if downloads enabled
  StrCpy $R0 "$R0  \
  /OPT_ENABLE_DOWNLOADS skip detection of connection and assume can download$\r$\n  \
  /OPT_DISABLE_DOWNLOADS skip detection of connection and assume can NOT download$\r$\n"
!endif
!ifdef OPT_CRTL_WIN95ONLY  ; change as needed if any other Win95 only options added
  StrCpy $R0 "$R0  \
  /OPT_ENABLE_WIN95ONLY for win95 only sections enabled$\r$\n"
!endif

  MessageBox MB_OK $R0
  Quit
!macroend
!define DoHelpCmd "!insertmacro DoHelpCmd"


!macro SetOptionValue cmdarg optval
  ${Select} ${cmdarg}
    ${Case2} "/HELP" "/?"
      ${DoHelpCmd}
    ${Case2} "/S" "/D"
      ; Dummy case, these options are handled by NSIS internally but not always removed
    ${Case} "/INSTALLTYPE"
      SetCurInstType "${optval}"
    ${Case2} "/RESPONSEFILE" "/R"
      StrCpy $v_responsefile "${optval}"
    ${Case2} "/MODIFYINSTALL" "/M"
      StrCpy $v_modifyinstall "1"              ; set installer into change/modify mode
    ${Case} "/OPT_ENABLE_MODIFY"
      StrCpy $v_opt_modify_reg "0"             ; enable modify (nomodify = 0)
    ${Case} "/OPT_MODIFY_PATH"
      StrCpy $v_opt_modify_path "${optval}"
!ifndef NODOWNLOADS  ; only process download related items if downloads enabled
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


; loop through each parameter and do action on it
; based on GetONEParameter -- written by Alexis de Valence --
!macro ProcessParameters
  Push $0
  Push $R0
  Push $R1
  Push $R2
  Push $R3
  Push $R4 

  !define CMDPARAMS $0
  !define PNAME     $R0
  !define PVAL      $R1
  !define PINDEX    $R2
  !define PTEMP     $R3
  !define FLAGEND   $R4

  ; init variables
  Call GetParameters              ; get $CMDLINE without argv[0] (ie just parameters)
  pop ${CMDPARAMS}

  StrCpy ${PINDEX} -1             ; index into cmdline for current character
  StrCpy ${FLAGEND} 0             ; flag, no more parameters

  ;MessageBox MB_OK "command paramaters are [${CMDPARAMS}]"
  loop3: ; looking for a char that's not a space
     IntOp ${PINDEX} ${PINDEX} + 1             ; point to next character in cmdline string
     StrCpy ${PTEMP} ${CMDPARAMS} 1 ${PINDEX}  ; copy one character at index over into buffer
     StrCmp ${PTEMP} " " loop3                 ; is it a space, yes then try next character
     StrCmp ${PTEMP} "" end                    ; no parameters
     StrCpy ${PNAME} ""           ; clear parameter name
     StrCpy ${PVAL}  ""           ; set default option value (usually does not exist so this is used)
     ;MessageBox MB_OK "start of paramater is [${PINDEX}]"

  loop:          ; scanning for the end of the current parameter

     StrCpy ${PTEMP} ${CMDPARAMS} 1 ${PINDEX}
     StrCmp ${PTEMP} " " loop2           ; end of parameter? (a space separates parameters)
     StrCmp ${PTEMP} ""  last            ; end of parameter? ("" marks end of full parameter list)
     IntOp  ${PINDEX} ${PINDEX} + 1
     StrCmp ${PTEMP} "=" loopVal         ; parameter has a value (points to after equal sign)
     StrCpy ${PNAME} "${PNAME}${PTEMP}"  ; keep running copy of parameter name
     Goto loop

  loopVal:       ; check for an = and split into parameter name & value pair
     StrCpy ${PTEMP} ${CMDPARAMS} 1 ${PINDEX}
     StrCmp ${PTEMP} " " loop2           ; end of parameter? (a space separates parameters)
     StrCmp ${PTEMP} ""  last            ; end of parameter? ("" marks end of full parameter list)
     IntOp  ${PINDEX} ${PINDEX} + 1
     StrCpy ${PVAL} "${PVAL}${PTEMP}"    ; keep running copy of parameter value
     Goto loopVal


  last: ; there will be no other parameter to extract
   StrCpy ${FLAGEND} 1

  loop2: ; found the end of the current parameter

   ; process current option
   ;MessageBox MB_OK "setting (${PNAME})=(${PVAL})"
   ${SetOptionValue} "${PNAME}" "${PVAL}"
  
   ; check if end of parameters reached and exit if so
   IntCmp ${FLAGEND} 1 end

   ; process the next parameter
   Goto loop3

  end:

  Pop $R4  ; restore $0, R0 - R4 to their initial value
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
  Pop $0

  !undef CMDPARAMS
  !undef PNAME
  !undef PVAL
  !undef PINDEX
  !undef PTEMP
  !undef FLAGEND
!macroend
!define ProcessParameters "!insertmacro ProcessParameters"


; call this macro during oninit or similar to actually initialize the
; above variables either with their defaults or command line overrides
!macro ProcessCmdLineArgs

  ; first initialize them all to their default values
  StrCpy $v_modifyinstall "0"  ; assume user is attempting to install AbiWord, not modify current installation

  StrCpy $v_opt_modify_reg "1"                                ; disable modify (nomodify = 1)
  StrCpy $v_opt_modify_path "'$EXEDIR\${INSTALLERNAME}' /M"   ; where ever current installer is

  !ifdef NODOWNLOADS  ; only process download related items if downloads enabled
    StrCpy $v_opt_enable_downloads "-1"
  !endif

  !ifdef OPT_CRTL_WIN95ONLY  ; change as needed if any other Win95 only options added
    StrCpy $v_opt_enable_win95only "0"
  !endif

  StrCpy $v_responsefile ""

  ; force Typical as default install type
  SetCurInstType 0

  ; now cycle through all the cmd line options and set the values
  ${ProcessParameters}

!macroend
!define ProcessCmdLineArgs "!insertmacro ProcessCmdLineArgs"


!endif ; _ABI_PARSECMDLINE_NSH_
