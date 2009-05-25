;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Utility functions to set and save/restore file extension to application associations


!ifndef _ABI_UTIL_FILEASSOC_NSH_
!define _ABI_UTIL_FILEASSOC_NSH_


!ifdef HAVE_SYSTEM_PLUGIN
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; RefreshShellIcons based on 
;; http://nsis.sourceforge.net/archive/nsisweb.php?page=236&instances=0
;; by jerome tremblay - april 2003

!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0

Function RefreshShellIcons
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd

!define RefreshShellIcons "call RefreshShellIcons"
!else
!define RefreshShellIcons
!endif ; HAVE_SYSTEM_PLUGIN


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; parts from http://nsis.sourceforge.net/archive/viewpage.php?pageid=282 by Vytautas
;; Will add the registry entries to associate the given file extension with the
;; previously set (see CreateApplicationAssociation) appType.  I.e. indicate to
;; open documents with this extension using the application specified by appType
;; registry entry.  If the extension is currently associated with a different
;; appType, it will store the current association in the "prior_appType" key.

!macro CreateFileAssociation extension appType contentType
  !define skipBackupLbl "skipBackup_${__LINE__}"
  push $0

  ; back up old value of extension (.ext) if it exists
  ReadRegStr $0 HKCR "${extension}" ""                     ; read current value
  StrCmp $0 "" "${skipBackupLbl}"                          ; nothing, then skip storing old value
    StrCmp $0 "${appType}" "${skipBackupLbl}"              ; only store if old is different than current
      WriteRegStr HKCR "${extension}" "prior_value" "$0"   ; actually store the old association

  "${skipBackupLbl}:"
    ; Write File Associations
    WriteRegStr HKCR "${extension}" "" "${appType}"
    WriteRegStr HKCR "${extension}" "Content Type" "${contentType}"
    ; Force shell refresh (so icons updated as needed)
    ${RefreshShellIcons}

  pop $0
  !undef skipBackupLbl
!macroend
!define CreateFileAssociation "!insertmacro CreateFileAssociation"


!macro CreateApplicationAssociation appType appName appDesc defIcon exeCmd
  WriteRegStr HKCR "${appType}" "" "${appDesc}"
  WriteRegStr HKCR "${appType}\shell" "" "open"
  WriteRegStr HKCR "${appType}\DefaultIcon" "" "${defIcon}"

  ; Basic command to open the file (pass filename as argv[1] to program executable)
  WriteRegStr HKCR "${appType}\shell\open\command" "" '"${exeCmd}" "%1"'

  ; To open file via DDE (OLE, ie via already active instance) instead of in a new process
  ; Here for those who want to locally enable, not normally used as having each document
  ; open in a new process while more resource intensive means a crash with one document
  ; won't cause loss of work with other open documents.
;  WriteRegStr HKCR "${appType}\shell\open\command" "" "${exeCmd}"
;  WriteRegStr HKCR "${appType}\shell\open\ddeexec" "" '[Open("%1")]'
;  WriteRegStr HKCR "${appType}\shell\open\ddeexec\application" "" "${appName}"
;  WriteRegStr HKCR "${appType}\shell\open\ddeexec\topic" "" "System"

  ; If editing file is a different action than simply opening file
;  WriteRegStr HKCR "${appType}\shell\edit" "" "Edit Options File"
;  WriteRegStr HKCR "${appType}\shell\edit\command" "" '"${exeCmd}" "%1"'

!macroend
!define CreateApplicationAssociation "!insertmacro CreateApplicationAssociation"


; check if a file extension is associated with us and if so delete it
!macro RemoveFileAssociation extension appType
	push $0
	push $1

	ReadRegStr $0 HKCR "${extension}" ""
	StrCmp "$0" "${appType}" 0 Skip_Del_File_Assoc.${extension}
		ReadRegStr $0 HKCR "${extension}" "prior_value"
		StrCmp "$0" "" "DeleteFA.${extension}" 0     ; if "prior_value" is not empty
			ReadRegStr $1 HKCR "$0" ""             ; restore previous association
			StrCmp "$1" "" DeleteFA.${extension}   ; only if it is still valid (has something defined)
			WriteRegStr HKCR "${extension}" "" $0             ; actually restore prior association
			DeleteRegValue HKCR "${extension}" "prior_value"  ; and remove stored value
			Goto Skip_Del_File_Assoc.${extension}
		DeleteFA.${extension}:                       ; else delete file association key
			DeleteRegKey HKCR "${extension}"       ; actually remove file assoications

	Skip_Del_File_Assoc.${extension}:
	pop $1
	pop $0
!macroend
!define RemoveFileAssociation "!insertmacro RemoveFileAssociation"


!endif ; _ABI_UTIL_FILEASSOC_NSH_
