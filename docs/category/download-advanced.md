---
layout: page
title: Advanced Setup
---

[Home](../index.html) >> [Download](../category/download.html) >> **[Advanced Setup](../category/download-advanced.html)**

Table of Content:

- [Manual Instructions](#install-instructions)
- [Web Browser Addon](#install-webextension)
- [Application](#download-application)
    - [Plateform](#download-plateform)
        - [Windows](#download-plateform-win32)
        - [Linux](#download-plateform-unix)
        - [MacOS](#download-plateform-macos)
        - [Source Code](#download-source)
    - [Latest release build status](#last-build-status)
- [Older releases](#older-releases)

---

## Manual Instructions<a name="install-instructions"></a>

1. First, install the application:

    1. Download **DownZemAll** portable or installer [here](#download-application)

    2. Unzip

2. Then, install the web extension:

    1. Install **DownRightNow** add-on on the [web browser](#install-webextension)

    2. Go to the install directory of the Application (*DownZemAll!*), and follow the instructions in `ReadMe.txt`

    3. Then, to verify the install, go to "*Options*" in the WebExtension interface in your Web Browser. It should tell you if it's correctly connected with the Application. 

<br/>

### Web Browser Addon<a name="install-webextension"></a>

Choose the browser:

| ![logo](/DownZemAll/assets/images/firefox.png) | ![logo](/DownZemAll/assets/images/chrome.png) |
|-----------------------------------|----------------------------------|
| Click here to intall [DownRightNow for Mozilla Firefox](https://addons.mozilla.org/en-US/firefox/addon/down-right-now/ ) | Click here to intall [DownRightNow for Google Chrome](https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio "https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio") |


#### Portable Archives

- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-chromium">Addon for Chrome (Portable)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-firefox">Addon for Firefox (Portable)</a>


<br/>

## Download the latest release<a name="download-application"></a>

The latest release is available [here](https://github.com/setvisible/DownZemAll/releases/latest)

[![GitHub release](https://img.shields.io/github/v/release/setvisible/downzemall.svg)](https://github.com/setvisible/DownZemAll/releases/latest)


### Choose your plateform<a name="download-plateform"></a>

Choose the most relevant version for your computer.
The next table explains the differences between the packages.


#### Windows<a name="download-plateform-win32"></a>[^1]

- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-windows-setup-x64">DownZemAll <span class="version-text">v0.0.0</span> for Windows (Installer, 64 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-windows-setup-x86">DownZemAll <span class="version-text">v0.0.0</span> for Windows (Installer, 32 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-windows-mingw-x64">DownZemAll <span class="version-text">v0.0.0</span> for Windows (Portable, MinGW, 64 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-windows-mingw-x86">DownZemAll <span class="version-text">v0.0.0</span> for Windows (Portable, MinGW, 32 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-windows-msvc-x64" >DownZemAll <span class="version-text">v0.0.0</span> for Windows (Portable, MSVC, 64 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-windows-msvc-x86" >DownZemAll <span class="version-text">v0.0.0</span> for Windows (Portable, MSVC, 32 bit)</a>


[^1]: Prefer the MSVC version if you are on Windows.
      It uses the *Chromium* engine that parses HTML *and* Javascript.
      The MinGW version is more basic, it uses *Google Gumbo*, that parses HTML only, not Javascript.


#### Linux<a name="download-plateform-unix"></a>[^2]

- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-linux-x64-app"     >DownZemAll <span class="version-text">v0.0.0</span> for Linux (AppImage, 64 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-linux-x64-zip"     >DownZemAll <span class="version-text">v0.0.0</span> for Linux (Portable, GCC, 64 bit)</a>

[^2]: Prefer the Tarball if you are on Linux.
      The AppImage can only run DownZemAll, unfortunately the launcher can't be used here.
      That's why the filename is suffixed with '_no_launcher'.
      In other words, your web browser can't launch the application.   


#### MacOS<a name="download-plateform-macos"></a>

- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-mac-x64-zip">DownZemAll <span class="version-text">v0.0.0</span> for Mac (Portable, 64 bit)</a>
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-mac-x64-dmg">DownZemAll <span class="version-text">v0.0.0</span> for Mac (64 bit)</a>



#### Source Code<a name="download-source"></a>

- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-tarball">DownZemAll <span class="version-text">v0.0.0</span> Source Code</a> (Tarball)
- <a href="https://github.com/setvisible/DownZemAll/releases/latest/" id="url-zipball">DownZemAll <span class="version-text">v0.0.0</span> Source Code</a> (Zip)

<br/>

### Latest release build status<a name="last-build-status"></a>

| Platform    | Status   |
|-------------|----------|
| Windows     | [![Build status](https://ci.appveyor.com/api/projects/status/github/setvisible/downzemall?branch=master&svg=true)](https://ci.appveyor.com/project/setvisible/downzemall)  |
| Linux/MacOS | [![Build status](https://api.travis-ci.com/setvisible/downzemall.svg?branch=master)](https://travis-ci.com/setvisible/downzemall) |

<br/>

## Older releases<a name="older-releases"></a>

Click [here](https://github.com/setvisible/DownZemAll/releases) to see the previous releases


---
Footnotes:




<script>

  /* Github latest release version detection */
  function doHttpGetAsync(theUrl, callback) {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
      if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
            callback(xmlHttp.responseText);
      }
    }
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
  }

  function onGithubResponse(json) {
    const obj = JSON.parse(json);
    tag_name = obj.tag_name;
    tarball_url = obj.tarball_url;
    zipball_url = obj.zipball_url;

    /* A-Z sorted Urls */
    document.getElementById('url-chromium').href            = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownRightNow_chromium_" + tag_name + ".zip";
    document.getElementById('url-firefox').href             = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownRightNow_firefox_" + tag_name + ".xpi";
    document.getElementById('url-windows-mingw-x64').href   = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_windows-mingw-x64.zip";
    document.getElementById('url-windows-mingw-x86').href   = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_windows-mingw-x86.zip";
    document.getElementById('url-windows-msvc-x64').href    = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_windows-msvc-x64.zip";
    document.getElementById('url-windows-msvc-x86').href    = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_windows-msvc-x86.zip";
    document.getElementById('url-linux-x64-zip').href       = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_x86_64.tar.gz";
    document.getElementById('url-linux-x64-app').href       = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_x86_64_no_launcher.AppImage";
    document.getElementById('url-mac-x64-zip').href         = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_x86_64_macos.zip";
    document.getElementById('url-mac-x64-dmg').href         = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_" + tag_name + "_x86_64.dmg";
    document.getElementById('url-windows-setup-x64').href   = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_x64_Setup.exe";
    document.getElementById('url-windows-setup-x86').href   = "https://github.com/setvisible/DownZemAll/releases/latest/download/DownZemAll_x86_Setup.exe";
    document.getElementById('url-tarball').href             = tarball_url;
    document.getElementById('url-zipball').href             = zipball_url;

    /* Text */
    var version = tag_name.replace("v", "");
    var element = document.getElementsByClassName("version-text");
    for (var i = 0; i < element.length; i++) {
      element[i].innerHTML = version;
    }
  }

  window.addEventListener("DOMContentLoaded", (event) => {
    doHttpGetAsync("https://api.github.com/repos/setvisible/DownZemAll/releases/latest", onGithubResponse);
  });

</script>
