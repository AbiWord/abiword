;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Function to disable a given section or subsection and make invisible


!ifndef _ABI_UTIL_SECTDISABLE_NSH_
!define _ABI_UTIL_SECTDISABLE_NSH_

!macro SectionDisable SECTIONID
  !insertmacro UnselectSection ${SECTIONID}	; mark section as unselected
  SectionSetText ${SECTIONID} ""	; and make invisible so user doesn't see it
!macroend
!define SectionDisable "!insertmacro SectionDisable"


!endif ; _ABI_UTIL_SECTDISABLE_NSH_
