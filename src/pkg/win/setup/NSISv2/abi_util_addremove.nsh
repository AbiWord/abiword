;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Support for add/remove functionality within installer
;               i.e. support change installed options via installer,
;               however uninstaller still works as before

; Based on work by THRaSH
; see http://nsis.sourceforge.net/archive/nsisweb.php?page=353&instances=0,64


;--- Add/Remove system macros: ---
; (You may place them to include file)
Var AR_SecFlags
Var AR_RegFlags

!macro InitSection SecName
  ;  This macro reads component installed flag from the registry and
  ;changes checked state of the section on the components page.
  ;Input: section index constant name specified in Section command.

  ; skip this for subsection, they will be correctly set based on subitems
  SectionGetFlags ${${SecName}} $AR_SecFlags          ;Reading default section flags
  IntOp $AR_SecFlags $AR_SecFlags & ${SF_SECGRP}      ;ignore all but group bit
  IntCmp $AR_SecFlags ${SF_SECGRP} "default_${SecName}" ; is a section so skip ahead

  ;Reading component status from registry
  ClearErrors
  ReadRegDWORD $AR_RegFlags HKLM \
    "${REG_UNINSTALL_KEY}\Components\${SecName}" "Installed"
  IfErrors "default_${SecName}"
    ;Status will stay default if registry value not found
    ;(component was never installed)
  IntOp $AR_RegFlags $AR_RegFlags & 0x01              ;Turn off all other bits
  SectionGetFlags ${${SecName}} $AR_SecFlags          ;Reading default section flags
  IntOp $AR_SecFlags $AR_SecFlags & ${SECTION_OFF}    ;Turn lowest (enabled) bit off
  IntOp $AR_SecFlags $AR_RegFlags | $AR_SecFlags      ;Change lowest bit

  ;Writing modified flags
  SectionSetFlags ${${SecName}} $AR_SecFlags

 "default_${SecName}:"
!macroend

!macro FinishSection SecName
  ;  This macro reads section flag set by user and removes the section
  ;if it is not selected.
  ;Then it writes component installed flag to registry
  ;Input: section index constant name specified in Section command.

  SectionGetFlags ${${SecName}} $AR_SecFlags  ;Reading section flags
  ;Checking lowest bit:
  IntOp $AR_SecFlags $AR_SecFlags & 0x0001
  IntCmp $AR_SecFlags 1 "leave_${SecName}"
    ;Section is not selected:
    ;Calling Section uninstall macro and writing zero installed flag
    !insertmacro "Remove_${${SecName}}"
    WriteRegDWORD HKLM "${REG_UNINSTALL_KEY}\Components\${SecName}" \
  "Installed" 0
    Goto "exit_${SecName}"

 "leave_${SecName}:"
    ;Section is selected:
    WriteRegDWORD HKLM "${REG_UNINSTALL_KEY}\Components\${SecName}" \
  "Installed" 1

 "exit_${SecName}:"
!macroend

!macro RemoveSection SecName
  ;  This macro is used to call section's Remove_... macro
  ;from the uninstaller.
  ;Input: section index constant name specified in Section command.

  !insertmacro "Remove_${${SecName}}"
!macroend
;--- End of Add/Remove macros ---

