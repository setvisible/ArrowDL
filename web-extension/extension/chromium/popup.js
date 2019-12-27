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
  setDisabled("button-start", hasError);
  setDisabled("button-immediate-download", hasError);
  setDisabled("button-manager", hasError);
  setDisabled("button-preference", hasError);
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
  var label = "Download ";
  var mediaId = chrome.extension.getBackgroundPage().getSettingMediaId();
  if (mediaId === 1) {
    label += " links";
  } else if (mediaId === 2) {
    label += " content";
  }
  var startPaused = chrome.extension.getBackgroundPage().isSettingStartPaused();
  if (startPaused) {
    label += " (paused)";
  }
  return label;
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
/* Events                        */
/* ***************************** */
function onLoaded() {
  checkConnection();

  var enabled = chrome.extension.getBackgroundPage().isSettingAskEnabled();
  setVisible("button-immediate-download", !enabled);

  if (!enabled) {
    var label = immediateButtonLabel();
    safeInnerHtmlAssignment("button-immediate-download-label", label);
  }
}

document.addEventListener('DOMContentLoaded', onLoaded); 

document.getElementById("button-start").addEventListener('click', () => {
    chrome.extension.getBackgroundPage().collectDOMandSendDataWithWizard();
    window.close();
});

document.getElementById("button-immediate-download").addEventListener('click', () => {
    chrome.extension.getBackgroundPage().collectDOMandSendData();
    window.close();
});

document.getElementById("button-manager").addEventListener('click', () => { 
    var command = "[MANAGER]";
    chrome.extension.getBackgroundPage().sendData(command);
    window.close();
});

document.getElementById("button-preference").addEventListener('click', () => { 
    var command = "[PREFS]";
    chrome.extension.getBackgroundPage().sendData(command);
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
