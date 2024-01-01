@echo off
:: Copyright 2019 Sébastien Vavassori. All rights reserved.
:: Use of this source code is governed by a LGPL license that can be
:: found in the LICENSE file.

:: USAGE
:: Replace <INSTALL_PATH> with the absolute path to your install directory.
::
:: (This is the path generally defined as "CMAKE_INSTALL_PREFIX" in CMake)
:: Ex: 
::      /DBIN_PATH="<INSTALL_PATH>" 
:: becomes 
::      /DBIN_PATH="C:\Program Files\ArrowDL"
::

makensis  ^
    /DPATH_OUT=".."  ^
    /DVERSION="0.0.0"  ^
    /DPLATFORM="x64"  ^
    /DBIN_PATH="<INSTALL_PATH>"  ^
    .\NSIS\setup.nsi

rename "ArrowDLSetup.exe" "ArrowDL_x64_Setup.exe"

pause
