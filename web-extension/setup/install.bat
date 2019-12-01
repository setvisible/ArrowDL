@echo off

SET PATH=C:\Windows\System32;%PATH%

rem Remove the final backslash with the ":n,m" substring syntax, here ":~0,-1"
SET currentPath=%~dp0
SET INSTALL_PATH=%currentPath:~0,-1%

ECHO .. Install directory: %INSTALL_PATH%

ECHO .. Writting to Chrome Registry
ECHO .. Key: HKCU\Software\Google\Chrome\NativeMessagingHosts\com.setvisible.downrightnow
REG ADD "HKCU\Software\Google\Chrome\NativeMessagingHosts\com.setvisible.downrightnow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-chrome.json" /f

ECHO .. Writting to Firefox Registry
ECHO .. Key: HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow
REG ADD "HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-firefox.json" /f

ECHO .. Writting to Waterfox Registry
ECHO .. Key: HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow
REG ADD "HKCU\SOFTWARE\Waterfox\NativeMessagingHosts\DownRightNow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-firefox.json" /f

ECHO .. Writting to Thunderbird Registry
ECHO .. Key: HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow
REG ADD "HKCU\SOFTWARE\Thunderbird\NativeMessagingHosts\DownRightNow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-firefox.json" /f

PAUSE
