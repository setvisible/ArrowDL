;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; NSIS installer script for DZA ;
; (http://nsis.sourceforge.net) ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


!define PROJECT_PATH "..\..\.."

;Output directory (where the package will be saved)
!ifndef PATH_OUT
    !define PATH_OUT "."
!endif
!system 'if not exist "${PATH_OUT}" mkdir "${PATH_OUT}"'

;Input directory (where are the binaries to pack)
!ifndef BIN_PATH
    Abort
!endif

!ifndef VERSION
    !define VERSION "0.0.65536"
!endif

!ifndef PLATFORM
    !define PLATFORM ""
!endif

!define PRODUCT_NAME "DownZemAll"
!define PRODUCT_VERSION ${VERSION}
!define PRODUCT_GROUP "Sebastien Vavassori"
!define PRODUCT_PUBLISHER "Sebastien Vavassori"
!define PRODUCT_WEB_SITE "https://setvisible.github.io/DownZemAll"

; Adds info to the installer
VIProductVersion "${PRODUCT_VERSION}.000"
VIFileVersion "${VERSION}.000"
VIAddVersionKey "FileDescription" "Installation for ${PRODUCT_NAME}"
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
    !include "macros.nsh"

    ;Include NSIS Modern User Interface (MUI)
    !include "MUI2.nsh"


;--------------------------------
;General

    ;Name and output file
    Name "${PRODUCT_NAME} ${VERSION}"
    OutFile "${PATH_OUT}\DownZemAllSetup.exe"

    ;Default installation folder
    InstallDir "$LOCALAPPDATA\${PRODUCT_NAME}" ; todo local user (otherwise config files and auto-update might not work)
    ;InstallDir "$PROGRAMFILES\${PRODUCT_NAME}" ; todo PLATFORM == "x86"
    ;InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}" ; todo PLATFORM == "x64"

    ; The default installation directory
    InstallDirRegKey HKLM "Software\${REGISTRY_INSTALLER_FOLDER_NAME}\${PRODUCT_NAME}" "InstallDir"

    ;Options
    ShowInstDetails show
    ShowUnInstDetails show
    SetOverwrite ifdiff
    CRCCheck on
    BrandingText "${PRODUCT_GROUP} ${PRODUCT_NAME}"

    ;Request application privileges for Windows Vista
    ;and no admin privileges for Windows 8/8.1/10
    RequestExecutionLevel user ; todo admin ?

;--------------------------------
;Interface Settings

    !define MUI_ICON "${PROJECT_PATH}\src\resources\logo\icon.ico"
    !define MUI_UNICON "${PROJECT_PATH}\src\resources\logo\icon.ico"

    ; Make installer pretty
    !define MUI_HEADERIMAGE
    !define MUI_HEADERIMAGE_RIGHT
    !define MUI_HEADERIMAGE_BITMAP "${PROJECT_PATH}\installer\windows\NSIS\images\header.bmp" ;

    ; Banner (welcome and finish page) for installer
    !define MUI_WELCOMEFINISHPAGE_BITMAP "${PROJECT_PATH}\installer\windows\NSIS\images\branding.bmp"
    !define MUI_BGCOLOR  0xFFFFFF

    ; Banner for uninstaller
    !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${PROJECT_PATH}\installer\windows\NSIS\images\branding.bmp"

;--------------------------------
;Pages

    ; --- Installer pages ---
    ;Abort Warning
        !define MUI_ABORTWARNING
    ;Welcome page
        !define MUI_WELCOMEPAGE_TITLE_3LINES
        !insertmacro MUI_PAGE_WELCOME
    ;License page
        !insertmacro MUI_PAGE_LICENSE "${PROJECT_PATH}\installer\windows\NSIS\COPYING.txt"
    ;Components page
        !define MUI_COMPONENTSPAGE_SMALLDESC
        !insertmacro MUI_PAGE_COMPONENTS
    ;Directory page
        !insertmacro MUI_PAGE_DIRECTORY
    ;Instfiles page
        !insertmacro MUI_PAGE_INSTFILES
    ;Finish page
        !define MUI_FINISHPAGE_RUN DownZemAll.exe
        ;!define MUI_FINISHPAGE_LINK "${DESC_VisitWebSite} ${PRODUCT_WEB_SITE}" ; todo Doesn't work
        !define MUI_FINISHPAGE_LINK "${PRODUCT_WEB_SITE}"
        !define MUI_FINISHPAGE_LINK_LOCATION "${PRODUCT_WEB_SITE}"
        !define MUI_FINISHPAGE_LINK_COLOR 0xFF8700 ; orange
        !insertmacro MUI_PAGE_FINISH

    ; --- Uninstaller pages ---
        !define MUI_WELCOMEPAGE_TITLE_3LINES
        !insertmacro MUI_UNPAGE_WELCOME
        !insertmacro MUI_UNPAGE_CONFIRM
        !insertmacro MUI_UNPAGE_INSTFILES
        !define MUI_UNPAGE_FINISH_TITLE_3LINES
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

!macro LOAD_LANGUAGE LANGUAGE
    !insertmacro MUI_LANGUAGE "${LANGUAGE}"
    !include "i18n\out\${LANGUAGE}.nsh"
!macroend

    !insertmacro LOAD_LANGUAGE "English"
    !insertmacro LOAD_LANGUAGE "Arabic"
    !insertmacro LOAD_LANGUAGE "SimpChinese"
    !insertmacro LOAD_LANGUAGE "French"
    !insertmacro LOAD_LANGUAGE "German"
    !insertmacro LOAD_LANGUAGE "Korean"
    !insertmacro LOAD_LANGUAGE "Italian"
    !insertmacro LOAD_LANGUAGE "Portuguese"
    !insertmacro LOAD_LANGUAGE "PortugueseBR"
    !insertmacro LOAD_LANGUAGE "Russian"
    !insertmacro LOAD_LANGUAGE "Spanish"

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

    ;Assign language strings to section names
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

    ;Abort installation if the application is currently running
    Call .quitIfRunning
FunctionEnd

;Uninstaller
Function un.onInit
    ;Display languages
    !insertmacro MUI_LANGDLL_DISPLAY

    ;Abort installation if the application is currently running
    Call un.quitIfRunning
FunctionEnd
