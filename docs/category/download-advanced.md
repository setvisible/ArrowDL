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

    1. Download **ArrowDL** portable or installer [here](#download-application)

    2. Unzip

2. Then, install the web extension:

    1. Install **DownRightNow** add-on on the [web browser](#install-webextension)

    2. Go to the install directory of the Application (*ArrowDL*), and follow the instructions in `ReadMe.txt`

    3. Then, to verify the install, go to "*Options*" in the WebExtension interface in your Web Browser. It should tell you if it's correctly connected with the Application. 

<br/>

### Web Browser Addon<a name="install-webextension"></a>

Choose the browser:

| ![logo](/ArrowDL/assets/images/firefox.png) | ![logo](/ArrowDL/assets/images/chrome.png) | ![logo](/ArrowDL/assets/images/iexplorer.png) |
|-----------------------------------|----------------------------------|----------------------------------|
| Click here to intall [DownRightNow for Mozilla Firefox](https://addons.mozilla.org/en-US/firefox/addon/down-right-now/ ) | Click here to intall [DownRightNow for Google Chrome](https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio "https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio") | Click here to intall... Just kidding |


#### Portable Archives

- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_chromium">Addon for Chrome (Portable)</a>
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_firefox">Addon for Firefox (Portable)</a>


<br/>

## Download the latest release<a name="download-application"></a>

The latest release is available [here](https://github.com/setvisible/ArrowDL/releases/latest)

[![GitHub release](https://img.shields.io/github/v/release/setvisible/arrowdl.svg)](https://github.com/setvisible/ArrowDL/releases/latest)


### Choose your plateform<a name="download-plateform"></a>

Choose the most relevant version for your computer.
The next table explains the differences between the packages.


#### Windows<a name="download-plateform-win32"></a>[^1]

- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_windows_setup_x64">ArrowDL <span class="version-text">v0.0.0</span> for Windows (Installer, 64 bit)</a>
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_windows_mingw_x64">ArrowDL <span class="version-text">v0.0.0</span> for Windows (Portable, MinGW, 64 bit)</a>
{% comment %}
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_windows_setup_x86">ArrowDL <span class="version-text">v0.0.0</span> for Windows (Installer, 32 bit)</a>
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_windows_mingw_x86">ArrowDL <span class="version-text">v0.0.0</span> for Windows (Portable, MinGW, 32 bit)</a>
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_windows_msvc_x64" >ArrowDL <span class="version-text">v0.0.0</span> for Windows (Portable, MSVC, 64 bit)</a>
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_windows_msvc_x86" >ArrowDL <span class="version-text">v0.0.0</span> for Windows (Portable, MSVC, 32 bit)</a>
{% endcomment %}

[^1]: Prefer the MSVC version if you are on Windows.
      It uses the *Chromium* engine that parses HTML *and* Javascript.
      The MinGW version is more basic, it uses *Google Gumbo*, that parses HTML only, not Javascript.


#### Linux<a name="download-plateform-unix"></a>[^2]
{% comment %}
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_linux_x64_app"     >ArrowDL <span class="version-text">v0.0.0</span> for Linux (AppImage, 64 bit)</a>
{% endcomment %}
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_linux_x64_zip"     >ArrowDL <span class="version-text">v0.0.0</span> for Linux (Portable, GCC, 64 bit)</a>

[^2]: Prefer the Tarball if you are on Linux.
      The AppImage can only run ArrowDL, unfortunately the launcher can't be used here.
      That's why the filename is suffixed with '_no_launcher'.
      In other words, your web browser can't launch the application.   


#### MacOS<a name="download-plateform-macos"></a>

- Not available yet
{% comment %}
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_mac_x64_zip">ArrowDL <span class="version-text">v0.0.0</span> for Mac (Portable, 64 bit)</a>
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_mac_x64_dmg">ArrowDL <span class="version-text">v0.0.0</span> for Mac (64 bit)</a>
{% endcomment %}


#### Source Code<a name="download-source"></a>

- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_tarball">ArrowDL <span class="version-text">v0.0.0</span> Source Code</a> (Tarball)
- <a href="https://github.com/setvisible/ArrowDL/releases/latest/" id="id_zipball">ArrowDL <span class="version-text">v0.0.0</span> Source Code</a> (Zip)

<br/>

### Latest release build status<a name="last-build-status"></a>

| Platform                | Status   |
|-------------------------|----------|
| Windows/Linux/MacOS     | [![Built with GitHub Actions](https://github.com/setvisible/ArrowDL/actions/workflows/deployment.yml/badge.svg?branch=master)](https://github.com/setvisible/ArrowDL/actions "Go to GitHub Actions") |

<br/>

## Older releases<a name="older-releases"></a>

Click [here](https://github.com/setvisible/ArrowDL/releases) to see the previous releases


---
Footnotes:



<script type="module">

  /* Github latest release version detection */
  function doHttpGetAsync(theUrl, callback) {
    const xmlHttp = new XMLHttpRequest();
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
    const tag_name = obj['tag_name'];
    const tarball_url = obj['tarball_url'];
    const zipball_url = obj['zipball_url'];

    /* A-Z sorted Urls */
    const artifact_id_to_filenames = {
      'id_chromium':            "DownRightNow_chromium_" + tag_name + ".zip",
      'id_firefox':             "DownRightNow_firefox_" + tag_name + ".xpi",
      'id_linux_x64_app':       "ArrowDL_" + tag_name + "_x86_64_no_launcher.AppImage",
      'id_linux_x64_zip':       "ArrowDL_" + tag_name + "_x86_64.tar.gz",
      'id_mac_x64_dmg':         "ArrowDL_" + tag_name + "_x86_64.dmg",
      'id_mac_x64_zip':         "ArrowDL_" + tag_name + "_x86_64_macos.zip",
      'id_windows_mingw_x64':   "ArrowDL_" + tag_name + "_windows_mingw_x64.zip",
      'id_windows_mingw_x86':   "ArrowDL_" + tag_name + "_windows_mingw_x86.zip",
      'id_windows_msvc_x64':    "ArrowDL_" + tag_name + "_windows_msvc_x64.zip",
      'id_windows_msvc_x86':    "ArrowDL_" + tag_name + "_windows_msvc_x86.zip",
      'id_windows_setup_x64':   "ArrowDL_x64_Setup.exe",
      'id_windows_setup_x86':   "ArrowDL_x86_Setup.exe",
    };

    const url_root = "https://github.com/setvisible/ArrowDL/releases/latest/download/";
    for (let id in artifact_id_to_filenames) {
      const artifact = document.getElementById(id);
      if (artifact) {
        artifact.href = url_root + artifact_id_to_filenames[id];
      }
    }

    document.getElementById('id_tarball').href = tarball_url;
    document.getElementById('id_zipball').href = zipball_url;

    /* Text */
    const version = tag_name.replace("v", "");
    const element = document.getElementsByClassName("version-text");
    for (let i = 0; i < element.length; i++) {
      element[i].innerHTML = version;
    }
  }

  window.addEventListener("DOMContentLoaded", (event) => {
    doHttpGetAsync("https://api.github.com/repos/setvisible/ArrowDL/releases/latest", onGithubResponse);
  });

</script>
