"use strict";

const application = "DownRightNow";

/* ***************************** */
/* Native Message                */
/* ***************************** */
function checkConnection() {
  function onResponse(response) {
    showWarningMessage(false);
  }
  
  function onError(error) {
    showWarningMessage(true);
  }
  var data = "areyouthere";
  var sending = browser.runtime.sendNativeMessage(application, data);
  sending.then(onResponse, onError);
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

/* ***************************** */
/* Events                        */
/* ***************************** */
function checkInstallation() {
  checkConnection();

}

document.addEventListener('DOMContentLoaded', checkInstallation); 

document.getElementById("button-start").addEventListener('click', () => {
    // Call collectDOMandSendData() from 'background.js'
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
    var openingPage = browser.runtime.openOptionsPage();
    window.close();
});
