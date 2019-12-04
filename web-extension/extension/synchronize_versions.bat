@echo off
:: Copyright 2019 Sébastien Vavassori. All rights reserved.
:: Use of this source code is governed by a LGPL license that can be
:: found in the LICENSE file.

::
:: This batch script rewrites the Firefox/Chrome manifest.json files 
:: with the current value present in <root>/version
::

echo.Updating version...

python substitute.py "1.4.0" .\..\..\version .\chromium\manifest.json
python substitute.py "1.4.0" .\..\..\version  .\firefox\manifest.json

echo.Version updated. 
