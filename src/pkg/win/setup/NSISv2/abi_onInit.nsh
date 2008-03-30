;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Tasks done during initialization


!include "abi_util_sectdisable.nsh"


; Perform one time steps done at installer startup, e.g. get installer language, 
; check internet connection/OS/etc and enable/disable options as appropriate
Function .onInit
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

;;;;;;;;;;;;;;;;;;
; clean up old versions before install
; adapted from http://nsis.sourceforge.net/Auto-uninstall_old_before_installing_new

;;;;;;;;;;;;;;;;;;;;
; Uninstall IEPlugins if already installed
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordIEPlugins" \
  "UninstallString"
  StrCmp $R0 "" doneIEPlugins
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "Import/Export Plugins for AbiWord are installed, and must be removed before upgrading.\
  When the upgrade is complete, you can install the new version of the plugins.  $\n$\nClick 'OK' to remove the \
  plugins or 'Cancel' to cancel this upgrade." \
  IDOK uninstIEPlugins
  Abort
  
;Run the uninstaller
uninstIEPlugins:
  ClearErrors
 ExecWait '$R0 /S _?=$INSTDIR\AbiWord\plugins' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller_IEPlugins
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
	Delete '$R0'
  no_remove_uninstaller_IEPlugins:
  
doneIEPlugins:

;;;;;;;;;;;;;;;;;;;;
; Uninstall ToolsPlugins if already installed
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\AbiwordToolsPlugins" \
  "UninstallString"
  StrCmp $R0 "" doneToolsPlugins
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "Tools Plugins for AbiWord are installed, and must be removed before upgrading.\
  When the upgrade is complete, you can install the new version of the plugins.  $\n$\nClick 'OK' to remove the \
  previous version or 'Cancel' to cancel this upgrade." \
  IDOK uninstToolsPlugins
  Abort
  
;Run the uninstaller
uninstToolsPlugins:
  ClearErrors
 ExecWait '$R0 /S _?=$INSTDIR\AbiWord\plugins' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller_ToolsPlugins
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
	Delete '$R0'
  no_remove_uninstaller_ToolsPlugins:
  
doneToolsPlugins:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Uninstall AbiWord if already installed
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}${VERSION_MAJOR}" \
  "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "AbiWord is already installed. $\n$\nClick 'OK' to remove the \
  previous version or 'Cancel' to cancel this upgrade." \
  IDOK uninst
  Abort
  
;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0 /S _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
	Delete '$R0'
  no_remove_uninstaller:
  
done:

FunctionEnd
