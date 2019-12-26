---
layout: category
title: Download
---

[Home](../index.html) >> **[Download](../category/download.html)** >> [Advanced Setup](../category/download-advanced.html)

<noscript>
  Please enable JavaScript to view the page.
</noscript>

## Install Instructions

1. First, install the standalone application:
    
    <a href="xxx" id="application-url">
        <img src="/DownZemAll/assets/images/installer.png" />
        DownZemAll for **<span id="application-platform-arch">xxxxxx</span>**
    </a>
    
    Version: 
    <span id="application-version">x.x.x</span>
    
    <br/>

2. Then, install the web extension:

    <a href="xxx" id="webextension-image-url">
        <img src="" id="webextension-image-png"/>
    </a>
    <a href="xxx" id="webextension-url">
         Add-on for **<span id="webextension-browser-name">xxxxxx</span>**
    </a>
    
    Version: 
    <span id="webextension-version">x.x.x</span>
    
    <br/>

<noscript>

Choose the browser:

| ![logo](/DownZemAll/assets/images/firefox.png) | ![logo](/DownZemAll/assets/images/chrome.png) |
|-----------------------------------|----------------------------------|
| Click here to intall [DownRightNow for Mozilla Firefox](https://addons.mozilla.org/en-US/firefox/addon/down-right-now/) | Click here to intall [DownRightNow for Google Chrome](https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio ) |

</noscript>

---

## Manual Instructions

For a manual install, or an install for a different operating system platform,
CPU architecture, browser, or release version, go to
[Advanced Setup](../category/download-advanced.html).


<script>
  /* Browser detection */
  var browserType = "";
  if(navigator.userAgent.indexOf("Chrome") != -1 ) {
    browserType = "Chrome";
  } else if(navigator.userAgent.indexOf("Firefox") != -1 )  {
    browserType = "Firefox";
  } else {
    browserType = "Chrome";
  }

  /* Platform detection */
  var platform = "";  
  if (navigator.appVersion.indexOf("Win") != -1) {
    platform = "Windows";
  } else if (navigator.appVersion.indexOf("Mac") != -1) {
    platform = "MacOS";
  } else if (navigator.appVersion.indexOf("X11") != -1) {
    platform = "Unix";
  } else if (navigator.appVersion.indexOf("Linux") != -1) {
    platform = "Linux";
  } else {
    platform = "Windows";
  }

  /* CPU architecture detection */
  var arch = "";
  if (navigator.userAgent.indexOf("WOW64") != -1 || navigator.userAgent.indexOf("Win64") != -1 ){
    arch = "x64";
  } else {
    arch = "x86";
  }

  /* Github latest release version detection */
  var version = "";

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
    version = obj.tag_name;
    document.getElementById('application-version').innerHTML = version;
    document.getElementById('webextension-version').innerHTML = version;
  }

  window.addEventListener("DOMContentLoaded", (event) => {
    doHttpGetAsync("https://api.github.com/repos/setvisible/DownZemAll/releases/latest", onGithubResponse);
  });

  /* Build the names */
  const githubUrl = "https://github.com/setvisible/DownZemAll/releases/latest/download/";

  var applicationUrl = "";
  var userSystemName = "";
  if (platform === "Windows") {
    if (arch === "x64") {
      applicationUrl = githubUrl + "DownZemAll_x64_Setup.exe";
      userSystemName += "Windows 64-bit";
    } else {
      applicationUrl = githubUrl + "DownZemAll_x86_Setup.exe";
      userSystemName += "Windows 32-bit";
    }

  } else if (platform === "MacOS") {
    if (arch === "x64") {
      applicationUrl = githubUrl;
      userSystemName += "MacOS 64-bit";
    } else {
      applicationUrl = githubUrl;
      userSystemName += "MacOS 32-bit";
    }

  } else {
    if (arch === "x64") {
      applicationUrl = githubUrl;
      userSystemName += "Linux 64-bit";
    } else {
      applicationUrl = githubUrl;
      userSystemName += "Linux 32-bit";
    }
  }

  var webExtensionBrowserName = "";
  var webExtensionUrl = "";
  var webExtensionImageSrc = "";

  if (browserType === "Chrome") {
    webExtensionBrowserName = "Google Chrome";
    webExtensionUrl="https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio";
    webExtensionImageSrc = "/DownZemAll/assets/images/chrome.png";
  
  } else if (browserType === "Firefox") {
    webExtensionBrowserName = "Mozilla Firefox";
    webExtensionUrl="https://addons.mozilla.org/en-US/firefox/addon/down-right-now/";
    webExtensionImageSrc = "/DownZemAll/assets/images/firefox.png";

  } else {
    webExtensionBrowserName = "your browser";
    webExtensionUrl = "";
    webExtensionImageSrc = "";
  }


  /* Apply the names */
  document.getElementById('application-url').href = applicationUrl;
  document.getElementById('application-version').innerHTML = version;
  document.getElementById('application-platform-arch').innerHTML = userSystemName;

  document.getElementById('webextension-image-url').href = webExtensionUrl;
  document.getElementById('webextension-image-png').src = webExtensionImageSrc;
  document.getElementById('webextension-url').href = webExtensionUrl;
  document.getElementById('webextension-version').innerHTML = version;
  document.getElementById('webextension-browser-name').innerHTML = webExtensionBrowserName;

</script>
