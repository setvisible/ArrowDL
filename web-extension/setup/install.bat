@echo off
:: Copyright 2019 Sébastien Vavassori. All rights reserved.
:: Use of this source code is governed by a LGPL license that can be
:: found in the LICENSE file.

SET PATH=C:\Windows\System32;%PATH%

:: Remove the final backslash with the ":n,m" substring syntax, here ":~0,-1"
SET currentPath=%~dp0
SET INSTALL_PATH=%currentPath:~0,-1%

ECHO *************************************************
ECHO                  Down Right Now
ECHO *************************************************
ECHO.
ECHO Install to: %INSTALL_PATH%
ECHO.

ECHO.
ECHO Writting to Chrome Registry...
ECHO ---------------------------------
ECHO Key: HKCU\Software\Google\Chrome\NativeMessagingHosts\com.setvisible.downrightnow
REG ADD "HKCU\Software\Google\Chrome\NativeMessagingHosts\com.setvisible.downrightnow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-chrome.json" /f

ECHO. 
ECHO Writting to Firefox Registry...
ECHO ---------------------------------
ECHO Key: HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow
REG ADD "HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-firefox.json" /f

ECHO.
ECHO Writting to Waterfox Registry...
ECHO ---------------------------------
ECHO Key: HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow
REG ADD "HKCU\SOFTWARE\Waterfox\NativeMessagingHosts\DownRightNow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-firefox.json" /f

ECHO.
ECHO Writting to Thunderbird Registry...
ECHO ---------------------------------
ECHO Key: HKCU\SOFTWARE\Mozilla\NativeMessagingHosts\DownRightNow
REG ADD "HKCU\SOFTWARE\Thunderbird\NativeMessagingHosts\DownRightNow" /ve /t REG_SZ /d "%INSTALL_PATH%\launcher-manifest-firefox.json" /f

ECHO.
ECHO ^>^>^> Done! ^<^<^<
ECHO.

TIMEOUT /T 10
