@echo off
:: Copyright 2019-present Sébastien Vavassori. All rights reserved.
:: Use of this source code is governed by a LGPL license that can be
:: found in the LICENSE file.

set currentpath=%~dp0
:: echo %currentpath:~0,-1% without the trailing '\'

set out=.

if not "%1"=="" (
    if not "%1"=="--output-directory" (
        echo.Error: Bad argurment.
        echo.   Must be: make.bat --output-directory DIR
        exit 1
    )
    set out=%2
)

echo.Making Browser Addons...

echo.Current directory: %currentpath%
echo.Output directory: %out%


echo.Making Chromium Addon...

:: robocopy /E = copy subdirs, even if empty 

robocopy  %currentpath%\src\base %out%\chromium /E
robocopy  %currentpath%src\chromium %out%\chromium /E

echo.Making Firefox Addon...

robocopy  %currentpath%\src\base %out%\firefox /E
robocopy  %currentpath%\src\firefox %out%\firefox /E


call %currentpath%\synchronize_versions.bat --output-directory %out%


echo.Done.
