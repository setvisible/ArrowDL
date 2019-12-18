"use strict";

const application = "com.setvisible.downrightnow";

/* ***************************** */
/* Options                       */
/* ***************************** */
function restoreOptions() {
  function onOptionResponse(response) {
    if (chrome.runtime.lastError) {
      console.log(chrome.runtime.lastError.message);
      onOptionError(response);
    }
    if (response === undefined) {
      onOptionError(response);
    } else {
      setApplicationRadio( response.radioApplicationId );
      setMediaRadio( response.radioMediaId );
      setStartPaused( response.startPaused );    
    }
  }

  function onOptionError(error) {
    console.log(`Error: ${error}`);
  }

  chrome.storage.local.get(onOptionResponse);
}

function saveOptions() {
  var options = getOptions();
  chrome.storage.local.set(options);
}

function getOptions() {
  var options = {};
  options["radioApplicationId"] = getApplicationRadio();
  options["radioMediaId"] = getMediaRadio();
  options["startPaused"] = isStartPaused();
  return options;
}

/* ***************************** */
/* GUI Elements                  */
/* ***************************** */
function unreachable() {
  console.error("This line should not be reachable");  
}

function getApplicationRadio() {
  if (document.getElementById("ask_what_to_do").checked === true) {
    return 1;
  } else if (document.getElementById("download_immediately").checked === true) {
    return 2;
  } else {
    unreachable();
  }
}

function setApplicationRadio(value) {
  if (value === undefined || value === 1) {
    document.getElementById("ask_what_to_do").checked = true;
  } else if (value === 2) {
    document.getElementById("download_immediately").checked = true;
  } else {
    unreachable();
  }
  refreshButtons();
}

function getMediaRadio() {
  if (document.getElementById("get_links").checked === true) {
    return 1;
  } else if (document.getElementById("get_content").checked === true) {
    return 2;
  } else {
    unreachable();
  }
}

function setMediaRadio(value) {
  if (value === undefined || value === 1) {
    document.getElementById("get_links").checked = true;
  } else if (value === 2) {
    document.getElementById("get_content").checked = true;
  } else {
    unreachable();
  }
}

function isStartPaused() {
  return document.getElementById("start_paused").checked;
}

function setStartPaused(value) {
  document.getElementById("start_paused").checked = value;
}

function refreshButtons(){
  var isChecked = document.getElementById("download_immediately").checked;
  setButtonEnabled("get_links", isChecked);
  setButtonEnabled("get_content", isChecked);
  setButtonEnabled("start_paused", isChecked);
}

function setButtonEnabled(name, enabled) {
  var inputButton = document.getElementById(name);
  inputButton.disabled = !enabled;
  for (var i = 0; i < inputButton.labels.length; i++) {
    var lbl = inputButton.labels[i];
    if (enabled) {
      lbl.classList.remove("disabled");
    } else {
      lbl.classList.add("disabled");
    }
  }
}

/* ***************************** */
/* Native Message                */
/* ***************************** */
function checkConnection() {
  function onHelloResponse(response) {
    if (chrome.runtime.lastError) {
      console.log(chrome.runtime.lastError.message);
      onHelloError(response);
    }
    if (response === undefined) {
      onHelloError(response);
    } else {
      console.log(`Message from the launcher:  ${response.text}`);
      var connectionStatus = "✓ Ok";
      var details = "<br><br>Detected path:<br><code>" + response.text + "</code>";
      safeInnerHtmlAssignment(connectionStatus, details, "MediumSeaGreen");     
    }
  }

  function onHelloError(error) {
    console.log(`Launcher didn't send any message. ${error}.`);
    var connectionStatus = "⚠ Error: Can't find the launcher";
    var details = "Follow the instructions below.";
    safeInnerHtmlAssignment(connectionStatus, details, "Tomato");
  }

  var data = "areyouthere";
  chrome.runtime.sendNativeMessage(application, { "text": data }, onHelloResponse);
}

function safeInnerHtmlAssignment(connectionStatus, details, color) {
  const statusTag = `<span>Status:&nbsp;&nbsp;<span style="padding: 5px 10px 5px 10px; solid ${color}; background-color:${color}; color:White;">${connectionStatus}</span>&nbsp;&nbsp;&nbsp;&nbsp;${details}</span>`;

  const parser = new DOMParser()
  const parsed = parser.parseFromString(statusTag, `text/html`)
  const tags = parsed.getElementsByTagName(`body`)

  document.getElementById("status-message").innerHTML = ``
  for (const tag of tags) {
    document.getElementById("status-message").appendChild(tag)
  }
}

/* ***************************** */
/* Message                       */
/* ***************************** */
function notifyBackgroundPage() {
  /*
   * The options.js sends an "optionsUpdated" message to the background.js
   * that the options have been updated.
   */
  function onResponse(message) {
    if (chrome.runtime.lastError) {
      console.log(chrome.runtime.lastError.message);
      onError(message);
    }
    if (message === undefined) {
      onError(message);
    } else {
      // console.log(`Message from the background.js:  ${message.response}`);
    }
  }
  function onError(error) {
    console.log(`Error: ${error}`);
  }
  var options = getOptions();
  chrome.runtime.sendMessage(options, onResponse);
}

/* ***************************** */
/* GUI Event                     */
/* ***************************** */
function loadPage() {
  restoreOptions();
  checkInstallation();

  var input = document.querySelectorAll("input");
  for(var i = 0; i < input.length; i++) {
    input[i].addEventListener("change", onInputChanged);
  }
}

function checkInstallation() {
  checkConnection();
}

function onInputChanged(){
  refreshButtons();
  saveOptions();
  notifyBackgroundPage();
}

document.addEventListener("DOMContentLoaded", loadPage);

document.getElementById("button-check").addEventListener("click", () => {
    checkInstallation();
})
