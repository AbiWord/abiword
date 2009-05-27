;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Set basic defines for setup, default version/name/etc


; Declarations
!define PRODUCT "AbiWord"

!ifndef VERSION_MAJOR
!define VERSION_MAJOR "2"
!endif
!ifndef VERSION_MINOR
!define VERSION_MINOR "7"
!endif
!ifndef VERSION_MICRO
!define VERSION_MICRO "2"
!endif
!ifdef VERSION
!undef VERSION
!endif
!define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"

!define INSTALLERNAME "abiword-setup-${VERSION}.exe"

!define PROGRAMEXE "AbiWord.exe"
!define MAINPROGRAM ${PROGRAMEXE}

; make some variables to be able to adapt to changing directory locations easier
!define NSIS_SCRIPT_PATH ".."
!define ABIWORD_MODULE_PATH "..\..\..\..\.."
!define ABIWORD_COMPILED_PATH "..\..\..\..\..\msvc2008\Release"