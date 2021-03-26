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

!define REGISTRY_INSTALLER_FOLDER_NAME "DownZemAll"

ManifestDPIAware true
Unicode true

;--------------------------------
;Includes

    ;Include custom macros
    !include macros.nsh

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
        !define MUI_FINISHPAGE_LINK "${DESC_VisitWebSite} ${PRODUCT_WEB_SITE}"
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
; Languages
; reference: https://nsis.sourceforge.io/Examples/Modern%20UI/MultiLanguage.nsi

; Remember the installer language
    !define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
    !define MUI_LANGDLL_REGISTRY_KEY "Software\${REGISTRY_INSTALLER_FOLDER_NAME}"
    !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

; always show language selection dialog
    !define MUI_LANGDLL_ALWAYSSHOW

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

Section "$(DESC_ApplicationSession)" SectionMainApplication
    SectionIn RO ;Make it read-only
    SetOutPath "$INSTDIR"
    SetOverwrite try

    ;Add installation files
    File /r "${BIN_PATH}\"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "$(DESC_LauncherSession)" SectionLauncher
    SectionIn RO ;Make it read-only

    ;Run script that writes Regitry keys for the Launcher
    ExecWait '"$INSTDIR\install.bat" -quiet'

SectionEnd

;Shortcuts
Section "$(DESC_StartMenuGroupSession)" SectionStartMenuShortcut
    
    ;Create shortcuts in the start menu programs directory
    CreateDirectory "$SMPROGRAMS\DownZemAll"
    CreateShortCut "$SMPROGRAMS\DownZemAll\DownZemAll.lnk" "$INSTDIR\DownZemAll.exe"
    CreateShortCut "$SMPROGRAMS\DownZemAll\$(DESC_UninstallIconDescription).lnk" "$INSTDIR\Uninstall.exe"

SectionEnd

Section "$(DESC_DesktopShortcutSession)" SectionDesktopShortcut
    
    ;Create shortcuts in the start menu programs directory
    ;SetShellVarContext current
    CreateShortCut "$DESKTOP\DownZemAll.lnk" "$INSTDIR\DownZemAll.exe"

SectionEnd

;--------------------------------
;Descriptions

    ;Language strings

    ; Macro Default Language strings
    !macro SetDefaultEnglishTranslation missing_lang
        LangString DESC_SectionMainApplication ${missing_lang} "The main application."
        LangString DESC_SectionLauncher ${missing_lang} "The launcher, used for messaging with web browser."
        LangString DESC_SectionStartMenuShortcut ${missing_lang} "Create Start Menu Shortcut."
        LangString DESC_SectionDesktopShortcut ${missing_lang} "Create Desktop Shortcut."
        LangString DESC_UninstallIconDescription ${missing_lang} "Uninstall DownZemAll"
        LangString DESC_ApplicationSession ${missing_lang} "Application (required)"
        LangString DESC_LauncherSession ${missing_lang} "Launcher (required)"
        LangString DESC_StartMenuGroupSession ${missing_lang} "Start Menu Shortcut"
        LangString DESC_DesktopShortcutSession ${missing_lang} "Desktop Shortcut"
        LangString DESC_VisitWebSite ${missing_lang} "Visit our website:"
    !macroend

    ; Language strings (English)
    !insertmacro SetDefaultEnglishTranslation ${LANG_ENGLISH}

    ; Language strings (French)
    LangString DESC_SectionMainApplication ${LANG_FRENCH} "L'application principale."
    LangString DESC_SectionLauncher ${LANG_FRENCH} "Le launcher pour les messages avec le navigateur web."
    LangString DESC_SectionStartMenuShortcut ${LANG_FRENCH} "Ajoute des icônes au menu Démarrer pour un accès facile."
    LangString DESC_SectionDesktopShortcut ${LANG_FRENCH} "Ajoute une icône sur votre bureau pour un accès facile."
    LangString DESC_UninstallIconDescription ${LANG_FRENCH} "Désinstaller DownZemAll"
    LangString DESC_ApplicationSession ${LANG_FRENCH} "Application (requis)"
    LangString DESC_LauncherSession ${LANG_FRENCH} "Launcher(requis)"
    LangString DESC_StartMenuGroupSession ${LANG_FRENCH} "Raccourci dans le menu Démarrer"
    LangString DESC_DesktopShortcutSession ${LANG_FRENCH} "Raccourci sur le bureau"
    LangString DESC_VisitWebSite ${LANG_FRENCH} "Visitez notre site Internet :"

    ; Language strings (German)
    LangString DESC_SectionMainApplication ${LANG_GERMAN} "Die Hauptapplikation."
    LangString DESC_SectionLauncher ${LANG_GERMAN} "Der launcher, das für Messaging mit Webbrowser verwendet wird."
    LangString DESC_SectionStartMenuShortcut ${LANG_GERMAN} "Fügt Symbole im Startmenü für leichten Zugang hinzu."
    LangString DESC_SectionDesktopShortcut ${LANG_GERMAN} "Fügt ein Desktopsymbol für leichten Zugang ein."
    LangString DESC_UninstallIconDescription ${LANG_GERMAN} "Deinstallieren DownZemAll"
    LangString DESC_ApplicationSession ${LANG_GERMAN} "Applikation (benötigt)"
    LangString DESC_LauncherSession ${LANG_GERMAN} "Launcher (benötigt)"
    LangString DESC_StartMenuGroupSession ${LANG_GERMAN} "Symbol im Startmenü"
    LangString DESC_DesktopShortcutSession ${LANG_GERMAN} "Desktopsymbol"
    LangString DESC_VisitWebSite ${LANG_GERMAN} "Besuche unsere Webseite:"

    ; Language strings (Italian)
    LangString DESC_SectionMainApplication ${LANG_ITALIAN} "Applicazione principale."
    LangString DESC_SectionLauncher ${LANG_ITALIAN} "Il launcher, usato per i messaggi con il browser web."
    LangString DESC_SectionStartMenuShortcut ${LANG_ITALIAN} "Crea gruppo programmi nel menu Start."
    LangString DESC_SectionDesktopShortcut ${LANG_ITALIAN} "Crea collegamento programma sul desktop."
    LangString DESC_UninstallIconDescription ${LANG_ITALIAN} "Disinstalla DownZemAll"
    LangString DESC_ApplicationSession ${LANG_ITALIAN} "Applicazion"
    LangString DESC_LauncherSession ${LANG_ITALIAN} "Launcher"
    LangString DESC_StartMenuGroupSession ${LANG_ITALIAN} "Gruppo programmi Menu Shortcut"
    LangString DESC_DesktopShortcutSession ${LANG_ITALIAN} "Collegamento sul desktop"
    LangString DESC_VisitWebSite ${LANG_ITALIAN} "Visita il nostro sito web:"

    ; Other (missing) language strings
    !insertmacro SetDefaultEnglishTranslation ${LANG_ARABIC}
    !insertmacro SetDefaultEnglishTranslation ${LANG_KOREAN}
    !insertmacro SetDefaultEnglishTranslation ${LANG_PORTUGUESE}
    !insertmacro SetDefaultEnglishTranslation ${LANG_PORTUGUESEBR}
    !insertmacro SetDefaultEnglishTranslation ${LANG_RUSSIAN}
    !insertmacro SetDefaultEnglishTranslation ${LANG_SPANISH}
    !insertmacro SetDefaultEnglishTranslation ${LANG_SIMPCHINESE}


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

;--------------------------------

;Installer
Function .onInit
    ;Display languages
    !insertmacro MUI_LANGDLL_DISPLAY

    ;Verify the application is closed
    Call .quitIfRunning
FunctionEnd

;Uninstaller
Function un.onInit
    ;Display languages
    !insertmacro MUI_LANGDLL_DISPLAY

    ;Verify the application is closed
    Call un.quitIfRunning
FunctionEnd
