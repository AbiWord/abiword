;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Function to delete a directory if it has no files within it


; Based on work by camillo
; see http://nsis.sourceforge.net/archive/nsisweb.php?page=220&instances=0,211
; Usage:
;       ${DeleteDirIfEmpty} "\somepath\directory to remove if empty"

!ifndef _ABI_UTIL_DELDIR_NSH_
!define _ABI_UTIL_DELDIR_NSH_


; performs action for each file in directory found matching filespec
!macro DoDirForEach filespec dir actionToDo
  !define DoActionLbl "DoAction_${__LINE__}"
  !define CleanupLbl  "Cleanup_${__LINE__}"

  push $R0
  push $R1
  ClearErrors
  FindFirst $R0 $R1 "${dir}\${filespec}"

  ${DoActionLbl}:
  IfErrors ${CleanupLbl}          ; assume failed to find file matching filespec
  ${actionToDo} "${dir}" "$R1"    ; do action on file (action can combine if need full path)
  FindNext $R0 $R1                ; any match now means filespec was found
  Goto ${DoActionLbl}

  ${CleanupLbl}:
  FindClose $R0                   ; free used resources (close open search handle)
  pop $R1
  pop $R0
  !undef DoActionLbl
  !undef CleanupLbl
!macroend
!define DoDirForEach "!insertmacro DoDirForEach"


; performs action only if none of specified files in directory found (ignores . & ..)
!macro DoIfDirLacks filespec dir actionToDo
  !define NoDeleteLbl "NoDelete_${__LINE__}"
  !define DoActionLbl "DoAction_${__LINE__}"
  !define CleanupLbl  "Cleanup_${__LINE__}"

  ;DetailPrint "Searching in [${dir}] for [${filespec}]"
  push $R0
  push $R1
  ClearErrors
  FindFirst $R0 $R1 "${dir}\${filespec}"
  IfErrors ${DoActionLbl}           ; assume failed to find any files matching filespec
  ;DetailPrint "Found $R1"
  strcmp $R1 "." 0 ${NoDeleteLbl}   ; some file found matching filespec, ignore if current dir entry (.)
   FindNext $R0 $R1                 ; check again (could still be .. or file matching filespec)
   IfErrors ${DoActionLbl}          ; assume failed to find any files matching filespec
   ;DetailPrint "Found next $R1"
   strcmp $R1 ".." 0 ${NoDeleteLbl} ; ignore only if it was the parent directory (..)
    FindNext $R0 $R1                ; any match now means filespec was found
    ;DetailPrint "Found last $R1"
    IfErrors 0 ${NoDeleteLbl}       ; so if an error then we can assume no files matching filespec found
     ${DoActionLbl}:
     ;DetailPrint "Doing Action"
     FindClose $R0                  ; close handle on directory in case we want to delete it or something
     Sleep 1000                     ; give close handle time to process and propogate
     ${actionToDo}                  ; do action since no files matching filespec found in dir
     Goto ${CleanupLbl}

  ${NoDeleteLbl}:
   FindClose $R0                    ; free used resources (close open search handle)

  ${CleanupLbl}:
  pop $R1
  pop $R0
  !undef NoDeleteLbl
  !undef DoActionLbl
  !undef CleanupLbl
!macroend
!define DoIfDirLacks "!insertmacro DoIfDirLacks"


; performs action only if no files (other than current(.) & parent (..)) in it
!define DoIfDirEmpty "${DoIfDirLacks} '*.*'"


; removes directory if no more files in it
!macro DeleteDirIfEmpty dirToDelete
${DoIfDirEmpty} ${dirToDelete} 'RMDir "${dirToDelete}"'
!macroend
!define DeleteDirIfEmpty "!insertmacro DeleteDirIfEmpty"

!endif ; _ABI_UTIL_DELDIR_NSH_
