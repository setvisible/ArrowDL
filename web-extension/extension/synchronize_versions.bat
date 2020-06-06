@echo off
:: Copyright 2019 Sébastien Vavassori. All rights reserved.
:: Use of this source code is governed by a LGPL license that can be
:: found in the LICENSE file.

::
:: This batch script rewrites the Firefox/Chrome manifest.json files 
:: with the current value present in <root>/version
::

if not "%1"=="--output-directory" (
    echo.Error: Bad argurment.
    echo.   Must be: synchronize_versions.bat --output-directory DIR
    exit 1
)

set currentpath=%~dp0
:: echo %currentpath:~0,-1% without the trailing '\'

set out=%2

echo.Updating version...

python %currentpath%\substitute.py "0.0.65536" %currentpath%\..\..\version %out%\chromium\manifest.json
python %currentpath%\substitute.py "0.0.65536" %currentpath%\..\..\version %out%\firefox\manifest.json

echo.Version updated. 
