# ![](/src/icons/logo/icon32.png) DownZemAll!

[![GitHub release](https://img.shields.io/github/v/release/setvisible/downzemall.svg)](../../releases/latest)

| Windows | Linux/OSX |
|---------|-----------|
| [![Build status](https://ci.appveyor.com/api/projects/status/github/setvisible/downzemall?branch=master&svg=true)](https://ci.appveyor.com/project/setvisible/downzemall) | [![Build status](https://api.travis-ci.org/setvisible/downzemall.svg?branch=master)](https://travis-ci.org/setvisible/downzemall) |


*DownZemAll!* is a rewrite of the legacy software [DownThemAll!](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") which was an extension for Mozilla Firefox, but whose development stopped around 2016, when Mozilla Firefox migrated to WebExtensions.


[![promotional](/screenshots/promo_330x43.png)](https://setvisible.github.io/DownZemAll "https://setvisible.github.io/DownZemAll")


*DownZemAll!* is a standalone download manager for Windows, Mac OS X and Linux. 

It aims to work with latest versions of Mozilla Firefox (powered by *WebExtensions*), and other web browsers (Chrome, Edge, Safari...). 

*DownZemAll!* is written in C++ and based on the [Qt5](https://www.qt.io/ "https://www.qt.io/") framework.

[![Built with Qt](/screenshots/built_with_qt.png)](https://www.qt.io/ "Go to Qt official site - www.qt.io")


## ![](/src/icons/logo/icon32.png) Installation

Go to
[Download](https://setvisible.github.io/DownZemAll/category/download.html)
page to install the application for your operating system.

Rem: *Native-Client* is an alternative to *DownRightNow*. Click [here](NativeClient.md "NativeClient.md") for more information.


## ![](/src/icons/logo/icon32.png) Usage

Go to
[Tutorial](https://setvisible.github.io/DownZemAll/category/tutorial.html)
page to get started.


## ![](/src/icons/logo/icon32.png) Screenshots

Screenshots of version 1.0

[![anim_01](/screenshots/anim_01.gif)](https://setvisible.github.io/DownZemAll/category/screenshots.html "Go to Screenshots page")

[![anim_02](/screenshots/anim_02.gif)](https://setvisible.github.io/DownZemAll/category/screenshots.html "Go to Screenshots page")


## ![](/src/icons/logo/icon32.png) Under the hood

*DownZemAll!* is a standalone application, embedding its own web engine. That is, it aims to be free and independent, and not rely on any third-party Web Browser technology.

The internal web engine is currently:

* [Google Gumbo Parser](https://github.com/google/gumbo-parser "https://github.com/google/gumbo-parser"), for the **MinGW** and **GNU** versions. 

     Gumbo is a small pure-C HTML5 parser (but doesn't parse Javascript)

* [Chromium](https://fr.wikipedia.org/wiki/Chromium "https://fr.wikipedia.org/wiki/Chromium"), for the **MSVC** version.

     Chromium is a powerful web engine (parses HTML+Javascript)

When we give an URL address to *DownZemAll!*, *DownZemAll!* downloads the page, parses the HTML page and collects the links.

Due to rapid evolution of web technology, *DownZemAll!* is designed to implement new parsers or add existing ones if required.


## ![](/src/icons/logo/icon32.png) Coding

Click [here](DEVELOPERS.md "DEVELOPERS.md") for build instructions.


## ![](/src/icons/logo/icon32.png) License and Disclaimer

The code is released under the GNU [Lesser General Public License (LGPL)](LICENSE "LICENSE").

Use it at your own risk. None of the authors, contributors, or anyone else connected with this DownZemAll software and the DownRightNow web-extension, in any way whatsoever, can be responsible for your use of the application. 

Please be aware that this site contains copyrighted material the use of which has not always been specifically authorized by the copyright owner.
