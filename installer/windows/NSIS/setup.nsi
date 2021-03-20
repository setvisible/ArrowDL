;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; NSIS installer script for DZA ;
; (http://nsis.sourceforge.net) ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Output directory (where the package will be saved)
!ifndef PATH_OUT
    !define PATH_OUT "."
!endif
!system 'if not exist "${PATH_OUT}" mkdir "${PATH_OUT}"'

;Input directory (where are the binaries to pack)
!ifndef BIN_PATH
    Abort
!endif

!define PRODUCT_NAME "DownZemAll"
!ifndef VERSION
    !define VERSION "0.0.65536"
!endif
!define PRODUCT_VERSION ${VERSION}
!define PRODUCT_GROUP "Sebastien Vavassori"
!define PRODUCT_PUBLISHER "Sebastien Vavassori"
!define PRODUCT_WEB_SITE "https://setvisible.github.io/DownZemAll"

VIProductVersion "${PRODUCT_VERSION}.000"
VIFileVersion "${VERSION}.000"
VIAddVersionKey "FileDescription" "DownZemAll - Mass Download Manager"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductName" "DownZemAll"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "LegalCopyright" "(C) Sebastien Vavassori. All Rights Reserved."


ManifestDPIAware true
Unicode true

;--------------------------------
;INCLUDES
;--------------------------------
!include macros.nsh

;--------------------------------
;Verify the application is closed

;Installer
Function .onInit
    Call .quitIfRunning
FunctionEnd

;Uninstaller
Function un.onInit
    Call un.quitIfRunning
FunctionEnd

;--------------------------------
;Include NSIS Modern User Interface (MUI)

    !include "MUI2.nsh"

;--------------------------------
;General

    ;Name and output file
    Name "${PRODUCT_NAME} ${VERSION}"
    OutFile "${PATH_OUT}\DownZemAllSetup.exe"

    ;Default installation folder
    InstallDir "$LOCALAPPDATA\DownZemAll"

    ;Options
    ShowInstDetails show
    ShowUnInstDetails show
    SetOverwrite ifdiff
    CRCCheck on
    BrandingText "${PRODUCT_GROUP} ${PRODUCT_NAME}"

    ;Request application privileges for Windows Vista
    ;and no admin privileges for Windows 8/8.1/10
    RequestExecutionLevel user

;--------------------------------
;Interface Settings

    !define MUI_ICON "..\..\..\src\icons\logo\icon.ico"
    !define MUI_UNICON "..\..\..\src\icons\logo\icon.ico"

    ; Banner (welcome and finish page) for installer
    !define MUI_WELCOMEFINISHPAGE_BITMAP "branding.bmp"
    !define MUI_BGCOLOR  0xFFFFFF

    ; Banner for uninstaller
    !define MUI_UNWELCOMEFINISHPAGE_BITMAP "branding.bmp"

;--------------------------------
;Pages

    ; --- Installer pages ---
    ;Welcome page
        !define MUI_WELCOMEPAGE_TITLE_3LINES
        !insertmacro MUI_PAGE_WELCOME
    ;License page
        !insertmacro MUI_PAGE_LICENSE "COPYING.txt"
    ;Components page
        !insertmacro MUI_PAGE_COMPONENTS
    ;Directory page
        !insertmacro MUI_PAGE_DIRECTORY
    ;Instfiles page
        !insertmacro MUI_PAGE_INSTFILES
    ;Finish page
        !define MUI_FINISHPAGE_RUN DownZemAll.exe
        !define MUI_FINISHPAGE_LINK "Visit our website: ${PRODUCT_WEB_SITE}"
        !define MUI_FINISHPAGE_LINK_LOCATION "${PRODUCT_WEB_SITE}"
        !define MUI_FINISHPAGE_LINK_COLOR 0xFF8700 ; orange
        !insertmacro MUI_PAGE_FINISH

    ; --- Uninstaller pages ---
        !define MUI_WELCOMEPAGE_TITLE_3LINES
        !insertmacro MUI_UNPAGE_WELCOME
        !insertmacro MUI_UNPAGE_CONFIRM
        !insertmacro MUI_UNPAGE_INSTFILES
        !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
; reference: https://nsis.sourceforge.io/Examples/Modern%20UI/MultiLanguage.nsi

    !insertmacro MUI_LANGUAGE "English"
    !insertmacro MUI_LANGUAGE "Arabic"
    !insertmacro MUI_LANGUAGE "SimpChinese"    
    !insertmacro MUI_LANGUAGE "French"
    !insertmacro MUI_LANGUAGE "German"
    !insertmacro MUI_LANGUAGE "Korean"
    !insertmacro MUI_LANGUAGE "Italian"
    !insertmacro MUI_LANGUAGE "Portuguese"
    !insertmacro MUI_LANGUAGE "PortugueseBR"
    !insertmacro MUI_LANGUAGE "Russian"
    !insertmacro MUI_LANGUAGE "Spanish"

;--------------------------------
;Installer Sections

Section "Application" SectionMainApplication
    SectionIn RO ;Make it read-only
    SetOutPath "$INSTDIR"
    SetOverwrite try

    ;Add installation files
    File /r "${BIN_PATH}\"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Launcher" SectionLauncher
    SectionIn RO ;Make it read-only

    ;Run script that writes Regitry keys for the Launcher
    ExecWait '"$INSTDIR\install.bat" -quiet'

SectionEnd

;Shortcuts
Section "Start Menu Shortcut" SectionStartMenuShortcut
    
    ;Create shortcuts in the start menu programs directory
    CreateDirectory "$SMPROGRAMS\DownZemAll"
    CreateShortCut "$SMPROGRAMS\DownZemAll\DownZemAll.lnk" "$INSTDIR\DownZemAll.exe"
    CreateShortCut "$SMPROGRAMS\DownZemAll\$(DESC_UninstallIconDescription).lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Desktop Shortcut" SectionDesktopShortcut
    
    ;Create shortcuts in the start menu programs directory
    ;SetShellVarContext current
    CreateShortCut "$DESKTOP\DownZemAll.lnk" "$INSTDIR\DownZemAll.exe"

SectionEnd

;--------------------------------
;Descriptions

    ;Language strings

    ; Language strings (English)
    LangString DESC_SectionMainApplication ${LANG_ENGLISH} "The main application."
    LangString DESC_SectionLauncher ${LANG_ENGLISH} "The launcher, used for messaging with web browser."
    LangString DESC_SectionStartMenuShortcut ${LANG_ENGLISH} "Create Start Menu Shortcut."
    LangString DESC_SectionDesktopShortcut ${LANG_ENGLISH} "Create Desktop Shortcut."
    LangString DESC_UninstallIconDescription ${LANG_ENGLISH} "Uninstall DownZemAll"

    ; Language strings (Italian)
    LangString DESC_SectionMainApplication ${LANG_ITALIAN} "Applicazione principale."
    LangString DESC_SectionLauncher ${LANG_ITALIAN} "Il launcher, usato per i messaggi con il browser web."
    LangString DESC_SectionStartMenuShortcut ${LANG_ITALIAN} "Crea gruppo programmi nel menu Start."
    LangString DESC_SectionDesktopShortcut ${LANG_ITALIAN} "Crea collegamento programma sul desktop."
    LangString DESC_UninstallIconDescription ${LANG_ITALIAN} "Disinstalla DownZemAll"

    ;Assign language strings to sections
    !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
        !insertmacro MUI_DESCRIPTION_TEXT ${SectionMainApplication} $(DESC_SectionMainApplication)
        !insertmacro MUI_DESCRIPTION_TEXT ${SectionLauncher} $(DESC_SectionLauncher)
        !insertmacro MUI_DESCRIPTION_TEXT ${SectionStartMenuShortcut} $(DESC_SectionStartMenuShortcut)
        !insertmacro MUI_DESCRIPTION_TEXT ${SectionDesktopShortcut} $(DESC_SectionDesktopShortcut)
    !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
 
    ;First, clean the Registry 
    ExecWait '"$INSTDIR\uninstall.bat" -quiet'

    ;Second, delete the uninstaller
    RMDir /r "$INSTDIR"

    ;Third, remove the link from the start menu
    RMDir /r "$SMPROGRAMS\DownZemAll"

    ;and the link from the desktop
    Delete "$DESKTOP\DownZemAll.lnk"

SectionEnd

