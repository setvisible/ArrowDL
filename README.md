# ![logo](/src/icons/logo/icon32.png) DownZemAll!

[![GitHub release](https://img.shields.io/github/v/release/setvisible/downzemall.svg)](../../releases/latest)

| Windows | Linux/OSX |
|---------|-----------|
| [![Build status](https://ci.appveyor.com/api/projects/status/github/setvisible/downzemall?branch=master&svg=true)](https://ci.appveyor.com/project/setvisible/downzemall) | [![Build status](https://api.travis-ci.org/setvisible/downzemall.svg?branch=master)](https://travis-ci.org/setvisible/downzemall) |


*DownZemAll!* is a rewrite of the legacy software [DownThemAll!](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") which was an extension for Mozilla Firefox, but whose development stopped around 2016, when Mozilla Firefox migrated to WebExtensions.

![promotional](/screenshots/promo_330x43.png)

*DownZemAll!* is a standalone download manager for Windows, Mac OS X and Linux. 

It aims to work with latest versions of Mozilla Firefox (powered by *WebExtensions*), and other web browsers (Chrome, Edge, Safari...). 

*DownZemAll!* is written in C++ and based on the [Qt5](https://www.qt.io/ "https://www.qt.io/") framework.

![Built with Qt](/screenshots/built_with_qt.png)


## ![logo](/src/icons/logo/icon32.png) Installation and Usage

### Install the standalone application

1. [Download here](https://github.com/setvisible/DownZemAll/releases "https://github.com/setvisible/DownZemAll/releases") the latest version.

2. Unzip the archive, and run the executable.

Rem1: *DownZemAll!* is currently shipped as a **portable application** (no installer present).

Rem2: Configuration files will be created in the same directory as the executable.


### Install the Web Browser Add-on (WebExtensions)


| ![logo](/screenshots/firefox.png) | ![logo](/screenshots/chrome.png) |
|-----------------------------------|----------------------------------|
| Click here to intall [DownRightNow for Mozilla Firefox](https://addons.mozilla.org/en-US/firefox/addon/down-right-now/ "https://addons.mozilla.org/en-US/firefox/addon/down-right-now/") | Click here to intall [DownRightNow for Google Chrome](https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio "https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio") |


1. Install **DownRightNow** add-on

2. Go to the install directory of the Application (*DownZemAll!*), and follow the instructions in `ReadMe.txt`

3. Then, go to "*Options*" in the WebExtension interface in your Web Browser. It should tell you if it's correctly connected with the Application. 


Rem: *Native-Client* is an alternative of DownRightNow. Click [here](NativeClient.md "NativeClient.md") for more information.


## ![logo](/src/icons/logo/icon32.png) Goals

*DownZemAll!* is a standalone application, embedding its own web engine. That is, it aims to be free and independent, and not rely on any third-party Web Browser technology.

The internal web engine is currently:

* [Google Gumbo Parser](https://github.com/google/gumbo-parser "https://github.com/google/gumbo-parser"), for the **MinGW** and **GNU** versions. 

     Gumbo is a small pure-C HTML5 parser (but doesn't parse Javascript)

* [Chromium](https://fr.wikipedia.org/wiki/Chromium "https://fr.wikipedia.org/wiki/Chromium"), for the **MSVC** version.

     Chromium is a powerful web engine (parses HTML+Javascript)

When we give an URL address to *DownZemAll!*, *DownZemAll!* downloads the page, parses the HTML page and collects the links.

Due to rapid evolution of web technology, *DownZemAll!* is designed to implement new parsers or add existing ones if required.


## ![logo](/src/icons/logo/icon32.png) Usage

When surfing, click on *DownZemAll!* Button on your Firefox or Chrome toolbar (and/or context menu). 

The web browser sends the data to *DownZemAll!*.

*DownZemAll!* can start to collect the resources.


## ![logo](/src/icons/logo/icon32.png) Screenshots

Screenshots of version 1.

![anim_01](/screenshots/anim_01.gif)

![anim_02](/screenshots/anim_02.gif)


## ![logo](/src/icons/logo/icon32.png) Developer

Click [here](DEVELOPERS.md "DEVELOPERS.md") for the advanced information.


## ![logo](/src/icons/logo/icon32.png) License and Disclaimer

The code is released under the GNU [Lesser General Public License (LGPL)](LICENSE "LICENSE").

Use it at your own risk. None of the authors, contributors, or anyone else connected with this DownZemAll software and the DownRightNow web-extension, in any way whatsoever, can be responsible for your use of the application. 

Please be aware that this site contains copyrighted material the use of which has not always been specifically authorized by the copyright owner.
