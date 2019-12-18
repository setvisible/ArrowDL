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
  setDisabled("button-open-wizard", hasError);
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

/* ***************************** */
/* Events                        */
/* ***************************** */
function checkInstallation() {
  checkConnection();

  var enabled = chrome.extension.getBackgroundPage().isSettingAskEnabled();
  setVisible("button-open-wizard", !enabled);
}

document.addEventListener('DOMContentLoaded', checkInstallation); 

document.getElementById("button-start").addEventListener('click', () => {
    // Call collectDOMandSendData() from 'background.js'
    chrome.extension.getBackgroundPage().collectDOMandSendData();
    window.close();
});

document.getElementById("button-open-wizard").addEventListener('click', () => {
    chrome.extension.getBackgroundPage().collectDOMandSendDataWithWizard();
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
