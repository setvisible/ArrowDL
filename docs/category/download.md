---
layout: category
title: Download
---

[Home](../index.html) >> **[Download](../category/download.html)** >> [Advanced Setup](../category/download-advanced.html)

<noscript>    
<p style="color:red;">/!\ Please enable JavaScript to view the links</p>
</noscript>

## Installation


1. Install the application for your operating system
    
    <div class="simple-instruction" style="display: none;">
        <a href="xxx" id="application-url">
            <img src="/DownZemAll/assets/images/installer.png" />
            DownZemAll for <b><span id="application-platform-arch">xxxxxx</span></b>
        </a>
        <div>
            Version: 
            <span id="application-version">x.x.x</span>
        </div>
        <br/>
    </div>


2. Then, install the extension for your browser
    
    <div class="simple-instruction" style="display: none;">
        <div class="specific-instruction" style="display: block;">
            <a href="xxx" id="webextension-image-url">
                <img src="" id="webextension-image-png"/>
            </a>
            <a href="xxx" id="webextension-url">
                 Add-on for <b><span id="webextension-browser-name">xxxxxx</span></b>
            </a>
        </div>
        <div class="general-instruction" style="display: none;">
            <table>
                <tbody>
                    <tr>
                        <td align="center">
                            <a href="https://addons.mozilla.org/en-US/firefox/addon/down-right-now/">
                                <img src="/DownZemAll/assets/images/firefox.png" alt="firefox logo">
                            </a>
                        </td>
                        <td align="center">
                            <a href="https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio">
                                <img src="/DownZemAll/assets/images/chrome.png" alt="chrome logo">
                            </a>
                        </td>
                    </tr>
                    <tr>
                        <td>
                            Click here to intall 
                            <a href="https://addons.mozilla.org/en-US/firefox/addon/down-right-now/">
                                <b>DownRightNow for Mozilla Firefox</b>
                            </a>
                        </td>
                        <td>
                            Click here to intall 
                            <a href="https://chrome.google.com/webstore/detail/down-right-now/modofbhnhlagjmejdbalnijgncppjeio">
                                <b>DownRightNow for Google Chrome</b>
                            </a>
                        </td>
                    </tr>
                </tbody>
            </table>
        </div>
        <div>
            Version: 
            <span id="webextension-version">x.x.x</span>
        </div>
    </div>
    
    <br/>

---

## Manual Setup Instructions

For a manual installation, or an install for a different operating system platform,
CPU architecture, browser, or release version, have a look at
[Advanced Setup](../category/download-advanced.html).


<script>
  /* Show instructions if javascript is enabled */
  var simpleInstructions = document.getElementsByClassName("simple-instruction");
  for (var i = 0; i < simpleInstructions.length; i ++) {
    simpleInstructions[i].style.display = "block";
  }

  /* Browser detection */
  var browserType = "";
  if(navigator.userAgent.indexOf("Chrome") != -1 ) {
    browserType = "Chrome";
  } else if(navigator.userAgent.indexOf("Firefox") != -1 )  {
    browserType = "Firefox";
  } else {
    browserType = "unknown";
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
  if (navigator.userAgent.indexOf("WOW64") != -1
      || navigator.userAgent.indexOf("Win64") != -1
      || navigator.userAgent.indexOf("IA64") != -1
      || navigator.userAgent.indexOf("x64") != -1
      || navigator.userAgent.indexOf("x86_64") != -1){
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

  /* Show specific instructions */
  function showSpecificInstructions(isSpecific) {
    var specificInstructions = document.getElementsByClassName("specific-instruction");
    for (var i = 0; i < specificInstructions.length; i++) {
      specificInstructions[i].style.display = isSpecific ? "block" : "none";
    }
    var generalInstructions = document.getElementsByClassName("general-instruction");
    for (var j = 0; j < generalInstructions.length; j++) {
      generalInstructions[j].style.display = isSpecific ? "none" : "block";
    }
  }

  /* Build the names */
  const githubUrl = "https://github.com/setvisible/DownZemAll/releases/latest/";

  var applicationUrl = "";
  var userSystemName = "";
  if (platform === "Windows") {
    if (arch === "x64") {
      applicationUrl = githubUrl + "download/DownZemAll_x64_Setup.exe";
      userSystemName += "Windows 64-bit";
    } else {
      applicationUrl = githubUrl + "download/DownZemAll_x86_Setup.exe";
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
    showSpecificInstructions(true);
  
  } else if (browserType === "Firefox") {
    webExtensionBrowserName = "Mozilla Firefox";
    webExtensionUrl="https://addons.mozilla.org/en-US/firefox/addon/down-right-now/";
    webExtensionImageSrc = "/DownZemAll/assets/images/firefox.png";
    showSpecificInstructions(true);

  } else {
    webExtensionBrowserName = "your browser";
    webExtensionUrl = "";
    webExtensionImageSrc = "";
    showSpecificInstructions(false);
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
