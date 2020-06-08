# Web-Extensions 

## Directory organization

    ./src/                          Source Directory
    ./src/base                      Contains common files for Firefox and Chromium (translations, icons...)
    ./src/chromium                  Contains specific files for Chromium
    ./src/firefox                   Contains specific files for Firefox

    ./make.bat                      Main script to make the addons


## Build

(Windows only)

To build:

    .\make.bat

eq. to:

    .\make.bat --output-directory .


See `.appveyor` script for packaging to .ZIP and .XPI files.
