;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Set basic defines for setup, default version/name/etc


; Declarations
!define PRODUCT "AbiWord"

!ifndef VERSION_MAJOR
!define VERSION_MAJOR "2"
!endif
!ifndef VERSION_MINOR
!define VERSION_MINOR "1"
!endif
!ifndef VERSION_MICRO
!define VERSION_MICRO "0"
!endif
!ifdef VERSION
!undef VERSION
!endif
!define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}"

!define INSTALLERNAME "abiword-setup-${VERSION}.exe"

!define APPSET  "AbiSuite"
!define PROGRAMEXE "AbiWord.exe"
!define MAINPROGRAM "${PRODUCT}\bin\${PROGRAMEXE}"

