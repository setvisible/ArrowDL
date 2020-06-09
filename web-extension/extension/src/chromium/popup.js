"use strict";

const application = "com.setvisible.downrightnow";

/* ***************************** */
/* Native Message                */
/* ***************************** */
function checkConnection() {
  function onResponse(response) {
    if (chrome.runtime.lastError) {
      console.log(chrome.runtime.lastError.message);
      onError(response);
    }
    if (response === undefined) {
      onError(response);
    } else {
      showWarningMessage(false);
    }
  }

  function onError(error) {
    console.log(`Error: ${error}`);
    showWarningMessage(true);
  }

  var data = "areyouthere";
  chrome.runtime.sendNativeMessage(application, { "text": data }, onResponse);
}


/* ***************************** */
/* Core                          */
/* ***************************** */
function showWarningMessage(hasError) {
  var x = document.getElementById("warning-area");
  if (hasError) {
    x.style.display = "block";
  } else {
    x.style.display = "none";
  }
  if (hasError) {
    setDisabled("button-start", true);
    setDisabled("button-immediate-download", true);
    setDisabled("button-manager", true);
    setDisabled("button-preference", true);
  }
}

function setDisabled(name, disabled) {
  if (disabled) {
    document.getElementById(name).classList.add("disabled");
  } else {
    document.getElementById(name).classList.remove("disabled");
  }
}

function setVisible(name, visible) {  
  if (visible) {
    document.getElementById(name).style.display = "inline";
  } else {
    document.getElementById(name).style.display = "none";
  }
}

function immediateButtonLabel() {
  var mediaId = getBackgroundPage().getSettingMediaId();
  var startPaused = getBackgroundPage().isSettingStartPaused();
  if (mediaId === 1) {
    if (startPaused) {
      return chrome.i18n.getMessage("popupDownloadLinksPaused")
    } else {
      return chrome.i18n.getMessage("popupDownloadLinks")
    }
  } else if (mediaId === 2) {
    if (startPaused) {
      return chrome.i18n.getMessage("popupDownloadContentPaused")
    } else {
      return chrome.i18n.getMessage("popupDownloadContent")
    }
  }
  return "";
}

function safeInnerHtmlAssignment(elementId, label) {
  const parser = new DOMParser();
  const parsed = parser.parseFromString(`${label}`, `text/html`);
  const tags = parsed.getElementsByTagName(`body`);
  document.getElementById(elementId).innerHTML = ``;
  for (const tag of tags) {
    document.getElementById(elementId).appendChild(tag.lastChild);
  }
}

/* ***************************** */
/* FIREFOX BUG #1329304          */
/* ***************************** */
function checkIncognitoMode() {
  showIncognitoWarningMessage(hasIncognitoModeBug());
}

function hasIncognitoModeBug() {
  // Remark: 
  // https://developer.mozilla.org/en/docs/Mozilla/Add-ons/WebExtensions/API/runtime/getBackgroundPage
  // method 'getBackgroundPage()' cannot be used in a private window in Firefox
  // it always returns null. 
  // For more info see related bug at bugzilla.
  // https://bugzilla.mozilla.org/show_bug.cgi?id=1329304
  return (chrome.extension.getBackgroundPage() === null);
}

function showIncognitoWarningMessage(hasError) {
  var x = document.getElementById("warning-area-incognito");
  if (hasError) {
    x.style.display = "block";
  } else {
    x.style.display = "none";
  }
  if (hasError) {
    setDisabled("button-start", true);
  }
}

// This function is a work-around to the Firefox Incognito Context mode Bug.
function getBackgroundPage() {
  if (hasIncognitoModeBug()) {
    return new DummyChromeExtensionForIncognitoMode();
  }
  return chrome.extension.getBackgroundPage();
}

class DummyChromeExtensionForIncognitoMode {
  constructor() {
  }

  isSettingAskEnabled() {
    return true;
  }
  getSettingMediaId() {
    return -1;
  }  
  isSettingStartPaused() {
    return false;
  }
  collectDOMandSendDataWithWizard() {
    // Nothing
  }
  collectDOMandSendData() {
    // Nothing
  }

  sendData(links) {
    function onResponse(response) {
      if (chrome.runtime.lastError) {
        console.log(chrome.runtime.lastError.message);
        onError(response);
      }
      if (response === undefined) {
        onError(response);
      } else {
        console.log("Message from the launcher:  " + response.text);
      }
    }
  
    function onError(error) {
      console.log(`Error: ${error}`);
    }
  
    var data = "launch " + links;
    console.log("Sending message to launcher:  " + data);
    chrome.runtime.sendNativeMessage(application, { "text": data }, onResponse);
  }
}

/* ***************************** */
/* Events                        */
/* ***************************** */
function onLoaded() {
  checkConnection();
  checkIncognitoMode();

  var enabled = getBackgroundPage().isSettingAskEnabled();
  setVisible("button-immediate-download", !enabled);

  if (!enabled) {
    var label = immediateButtonLabel();
    safeInnerHtmlAssignment("button-immediate-download-label", label);
  }
}

document.addEventListener('DOMContentLoaded', onLoaded); 

document.getElementById("button-start").addEventListener('click', () => {
    getBackgroundPage().collectDOMandSendDataWithWizard();
    window.close();
});

document.getElementById("button-immediate-download").addEventListener('click', () => {
    getBackgroundPage().collectDOMandSendData();
    window.close();
});

document.getElementById("button-manager").addEventListener('click', () => { 
    var command = "[MANAGER]";
    getBackgroundPage().sendData(command);
    window.close();
});

document.getElementById("button-preference").addEventListener('click', () => { 
    var command = "[PREFS]";
    getBackgroundPage().sendData(command);
    window.close();
});

document.getElementById("button-options-page").addEventListener('click', () => {
    var openingPage = chrome.runtime.openOptionsPage();
    window.close();
});

document.getElementById("button-website").addEventListener('click', () => {
    window.open(document.getElementById("website-link").getAttribute("href"), "_blank");
    window.close();
});

/* ***************************** */
/* Internationalization          */
/* ***************************** */
safeInnerHtmlAssignment("button-download",    chrome.i18n.getMessage("popupDownload"));
safeInnerHtmlAssignment("button-open",        chrome.i18n.getMessage("popupOpen"));
safeInnerHtmlAssignment("button-preferences", chrome.i18n.getMessage("popupPreferences"));
safeInnerHtmlAssignment("button-options",     chrome.i18n.getMessage("popupOptions"));
safeInnerHtmlAssignment("website-link",       chrome.i18n.getMessage("popupVisitWebsite"));

safeInnerHtmlAssignment("msg-error",          chrome.i18n.getMessage("popupError"));
safeInnerHtmlAssignment("msg-error-1",        chrome.i18n.getMessage("popupError1"));
safeInnerHtmlAssignment("msg-error-2",        chrome.i18n.getMessage("popupError2"));

safeInnerHtmlAssignment("msg-remark",         chrome.i18n.getMessage("popupRemark"));
safeInnerHtmlAssignment("msg-remark-1",       chrome.i18n.getMessage("popupRemark1"));
