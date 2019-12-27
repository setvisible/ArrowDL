---
layout: page
title: Advanced Setup
---

[Home](../index.html) >> [Download](../category/download.html) >> **[Advanced Setup](../category/download-advanced.html)**

Table of Content:

- [Manual Instructions](#install-instructions)
- [Web Browser Add-on](#install-webextension)
- [Latest release](#download-latest-release)
    - [Plateform](#download-plateform)
        - [Windows](#download-plateform-win32)
        - [Linux](#download-plateform-unix)
        - [MacOS](#download-plateform-macos)
    - [Last build status](#last-build-status)
- [Older releases](#older-releases)

---

## Manual Instructions<a name="install-instructions"></a>

1. First, install the application:

    1. Download **DownZemAll** portable or installer [here](#download-latest-release)

    2. Unzip

2. Then, install the web extension:

    1. Install **DownRightNow** add-on on the [web browser](#install-webextension)

    2. Go to the install directory of the Application (*DownZemAll!*), and follow the instructions in `ReadMe.txt`

    3. Then, to verify the install, go to "*Options*" in the WebExtension interface in your Web Browser. It should tell you if it's correctly connected with the Application. 

### Web Browser Add-on<a name="install-webextension"></a>

Choose the browser:

| ![logo](/DownZemAll/assets/images/firefox.png) | ![logo](/DownZemAll/assets/images/chrome.png) |
|-----------------------------------|----------------------------------|
| Click here to intall [DownRightNow for Mozilla Firefox](https://addons.mozilla.org/en-US/firefox/addon/down-right-now/ "https://addons.mozilla.org/en-US/firefox/addon/down-right-now/") | Click here to intall [DownRightNow for Google Chrome](https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio "https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio") |


## Download the latest release<a name="download-latest-release"></a>

The latest release is available [here](https://github.com/setvisible/DownZemAll/releases/latest)

[![GitHub release](https://img.shields.io/github/v/release/setvisible/downzemall.svg)](https://github.com/setvisible/DownZemAll/releases/latest)

### Choose your plateform<a name="download-plateform"></a>

Choose the most relevant version for your computer.
The next table explains the differences between the packages.

#### Windows<a name="download-plateform-win32"></a>

- [Installer 64-bit](https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_x64_Setup.exe)    
- [Installer 32-bit](https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_x86_Setup.exe)
- [Portable MSVC 64-bit](https://github.com/setvisible/DownZemAll/releases/latest/) [^1]
- [Portable MSVC 32-bit](https://github.com/setvisible/DownZemAll/releases/latest/)
- [Portable MinGW 64-bit](https://github.com/setvisible/DownZemAll/releases/latest/) [^2]
- [Portable MinGW 32-bit](https://github.com/setvisible/DownZemAll/releases/latest/)

[^1]: MSVC means it's built with the *Microsoft Visual C++* compiler. 
    Prefer this version if you are on Windows.
    It uses the *Chromium* engine and *Google Gumbo*.

[^2]: MinGW means it's built with the *MinGW GCC* compiler. 
    Basic version.
    It does't use *Chromium*, so parses HTML but not Javascript.


#### Linux<a name="download-plateform-unix"></a>

- [Portable GCC 64-bit](https://github.com/setvisible/DownZemAll/releases/latest/) [^3]
- [Portable GCC 32-bit](https://github.com/setvisible/DownZemAll/releases/latest/)

[^3]: Currently untested. Built with GCC.

#### MacOS<a name="download-plateform-macos"></a>

- [Portable GCC 64-bit](https://github.com/setvisible/DownZemAll/releases/latest/) [^4]
- [Portable GCC 32-bit](https://github.com/setvisible/DownZemAll/releases/latest/)


[^4]: Currently untested. Built with GCC.


### Last build status<a name="last-build-status"></a>

| Platform | Status |
|---------|-----------|
| Windows |  [![Build status](https://ci.appveyor.com/api/projects/status/github/setvisible/downzemall?branch=master&svg=true)](https://ci.appveyor.com/project/setvisible/downzemall)  |
| Linux/OSX | [![Build status](https://api.travis-ci.org/setvisible/downzemall.svg?branch=master)](https://travis-ci.org/setvisible/downzemall) |

<br/>

## Older releases<a name="older-releases"></a>

Click [here](https://github.com/setvisible/DownZemAll/releases) to see the previous releases


---
Footnotes:
