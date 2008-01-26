;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Set options for Modern UI and set installation pages and page order


; Support 'Modern' UI and multiple languages
  ; specify where to get resources from for UI elements (default)
  !define MUI_UI "${NSISDIR}\Contrib\UIs\modern.exe"

  ; define options to MUI pages (we do this before including Mui.nsh
  ; as some do not work (e.g. MUI_COMPONENTSPAGE_SMALLDESC) if defined
  ; afterwards.
    ; see bug 710, don't require user to accept license only view it
    ; so we change the 'I Accept' button to 'Next'
    !define MUI_LICENSEPAGE_BUTTON "$(^NextBtn)"
    !define MUI_LICENSEPAGE_TEXT_BOTTOM "$(^ClickNext) - $(^NameDA)"
    ; put the component (what gets installed) description below choices (list with checkboxes)
      !define MUI_COMPONENTSPAGE_SMALLDESC
    ; set default start menu name and where we store this value (for uninstallation)
      !define MUI_STARTMENUPAGE_DEFAULTFOLDER "$(SM_PRODUCT_GROUP)"
      !define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
      !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}"
      !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
      Var STARTMENU_FOLDER     ; holds SM name chosen so we can create it and add shortcuts to it
    ; specify Finish Page settings
      ; prompt to run AbiWord (start with readme.txt)
      !define MUI_FINISHPAGE_RUN "$INSTDIR\${MAINPROGRAM}"
      !define MUI_FINISHPAGE_RUN_PARAMETERS  $\"$INSTDIR\readme.abw$\"
      ; or uncomment to allow viewing readme with default text editor
      ;!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.txt"
      ; force user to close so they can see install done & not start readme
      !define MUI_FINISHPAGE_NOAUTOCLOSE
    ; warn user if they try to quit the install before it completes
      !define MUI_ABORTWARNING

    ;Remember the installer language
    !define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
    !define MUI_LANGDLL_REGISTRY_KEY "Software\${APPSET}\${PRODUCT}\v${VERSION_MAJOR}"
    !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

    ;Custom help
    ${IfExists} "plugins\helpbutton.dll"
    Function CustomHelpFuncComponentsPage
      InitPluginsDir               ; insure directory exists and $PLUGINDIR initialized to its location
      SetOutPath $PLUGINSDIR       ; set directory to one that ensures file removed after install
      File /nonfatal insthelp.rtf  ; the custom help file displayed to describe in detail the various components
      HelpButton::show /NOUNLOAD "33,331" "?" "Components Help" "/file=$PLUGINSDIR\insthelp.rtf" wrap 33
    FunctionEnd
    Function .onGuiEnd ;customHelpFuncCleanup
      # This needs to be called otherwise the dll will not be correctly unloaded and so will stay on the hd :o(
      HelpButton::end
      ;MessageBox MB_OK "Cleanup HelpButton"
    FunctionEnd
    ${IfExistsEnd}


  ; include the Modern UI support
  !include "Mui.nsh"

  ; specify the pages and order to show to user
    ; introduce ourselves
    !insertmacro MUI_PAGE_WELCOME
    ; including the license of AbiWord  (license could be localized, but we lack translations)
    !insertmacro MUI_PAGE_LICENSE $(LicenseTXT)
    ; allow user to select what parts to install
    ${IfExists} "plugins\helpbutton.dll"
      !define MUI_PAGE_CUSTOMFUNCTION_PRE customHelpFuncComponentsPage
    ${IfExistsEnd}
    !insertmacro MUI_PAGE_COMPONENTS
    ; and where to install to
    !insertmacro MUI_PAGE_DIRECTORY
    ; get start menu entry name from user
    !insertmacro MUI_PAGE_STARTMENU "Application" $STARTMENU_FOLDER
    ; allow insertion of our custom pages
    !define MUI_CUSTOMPAGECOMMANDS
    ; query user for download mirror to use (only if dl item was selected though)
    !ifndef NODOWNLOADS
      !ifdef OPT_DICTIONARIES
        Page custom getDLMirror ; Custom page to get DL mirror
      !endif
    !endif
    ; actually install the files
    !insertmacro MUI_PAGE_INSTFILES
    ; show the Finish page, e.g. give users to read release notes, etc.
    !insertmacro MUI_PAGE_FINISH


  ; create an uninstaller and specify the pages it should have
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES


