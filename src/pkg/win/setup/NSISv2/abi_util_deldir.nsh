;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Function to delete a directory if it has no files within it


; Based on work by camillo
; see http://nsis.sourceforge.net/archive/nsisweb.php?page=220&instances=0,211
; Usage:
;       ${DeleteDirIfEmpty} "\somepath\directory to remove if empty"

!ifndef _ABI_UTIL_DELDIR_NSH_
!define _ABI_UTIL_DELDIR_NSH_

!macro DeleteDirIfEmpty dirToDelete
  !define NoDeleteLbl "NoDelete_${__LINE__}"
  ; push $R0
  ; push $R1
  FindFirst $R0 $R1 "${dirToDelete}\*.*"
  strcmp $R1 "." 0 ${NoDeleteLbl}
   FindNext $R0 $R1
   strcmp $R1 ".." 0 ${NoDeleteLbl}
    ClearErrors
    FindNext $R0 $R1
    IfErrors 0 ${NoDeleteLbl}
     FindClose $R0
     Sleep 1000
     RMDir "$0"
  ${NoDeleteLbl}:
   FindClose $R0
  ; pop $R1
  ; pop $R0
  !undef NoDeleteLbl
!macroend
!define DeleteDirIfEmpty "!insertmacro DeleteDirIfEmpty"

!endif ; _ABI_UTIL_DELDIR_NSH_
