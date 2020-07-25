"use strict";

const application = "com.setvisible.downrightnow";
const website_download_link = "https://setvisible.github.io/DownZemAll/category/download.html";

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
  setDivEnabled("full_menu_label", isChecked);
  setButtonEnabled("get_links", isChecked);
  setButtonEnabled("get_content", isChecked);
  setButtonEnabled("start_paused", isChecked);
}

function setDivEnabled(name, enabled) {
  document.getElementById(name).disabled = !enabled;
  if (enabled) {
    document.getElementById(name).style.color = "#000";
  } else {
    document.getElementById(name).style.color = "#aaa";
  }
}

function setButtonEnabled(name, enabled) {
  var inputButton = document.getElementById(name);
  inputButton.disabled = !enabled;
  for (var i = 0; i < inputButton.labels.length; i++) {
    var label = inputButton.labels[i];
    if (enabled) {
      label.classList.remove("disabled");
    } else {
      label.classList.add("disabled");
    }
  }
}

function showOptions(visible) {
  var instructions = document.getElementsByClassName("show-instruction");
  for (var i = 0; i < instructions.length; i ++) {
    instructions[i].style.display = visible ? "none" : "block";
  }
  var options = document.getElementsByClassName("options");
  for (var i = 0; i < options.length; i ++) {
    options[i].style.display = visible ? "block" : "none";
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
      var messageOk = chrome.i18n.getMessage("optionsOk");
      var messageDetectedPath = chrome.i18n.getMessage("optionsDetectedPath");
      var connectionStatus = "✓ " + messageOk;
      var details = "<br><br>" + messageDetectedPath + "<br><code>" + response.text + "</code>";
      setConnectionStatus(connectionStatus, details, "MediumSeaGreen");
      showOptions(true);
    }
  }

  function onHelloError(error) {
    console.log(`Launcher didn't send any message. ${error}.`);
    var messageError = chrome.i18n.getMessage("optionsError");
    var messageInstructions = chrome.i18n.getMessage("optionsInstructions");
    var connectionStatus = "⚠ " + messageError;
    var details = "<br><br>" + messageInstructions;
    setConnectionStatus(connectionStatus, details, "Tomato");
    showOptions(false);
  }

  var data = "areyouthere";
  chrome.runtime.sendNativeMessage(application, { "text": data }, onHelloResponse);
}

function setConnectionStatus(connectionStatus, details, color) {
  var messageStatus = chrome.i18n.getMessage("optionsStatus");
  const statusTag = `<span>${messageStatus}&nbsp;&nbsp;<span style="padding: 5px 10px 5px 10px; solid ${color}; background-color:${color}; color:White;">${connectionStatus}</span>&nbsp;&nbsp;&nbsp;&nbsp;${details}</span>`;

  safeInnerHtmlAssignment2("status-message", statusTag);
}

function safeInnerHtmlAssignment2(elementId, htmlText) {
  const parser = new DOMParser()
  const parsed = parser.parseFromString(htmlText, `text/html`)
  const tags = parsed.getElementsByTagName(`body`)
  document.getElementById(elementId).innerHTML = ``
  for (const tag of tags) {
    document.getElementById(elementId).appendChild(tag)
  }
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

/* ***************************** */
/* Internationalization          */
/* ***************************** */
safeInnerHtmlAssignment("main-title"          , chrome.i18n.getMessage("optionsMainTitle"));
safeInnerHtmlAssignment("toolbar-menu"        , chrome.i18n.getMessage("optionsToolbarMenu"));
safeInnerHtmlAssignment("show-simple-menu"    , chrome.i18n.getMessage("optionsShowSimpleMenu"));
safeInnerHtmlAssignment("show-full-menu"      , chrome.i18n.getMessage("optionsShowFullMenu"));
safeInnerHtmlAssignment("full_menu_label"     , chrome.i18n.getMessage("optionsShowFullMenuDescription"));
safeInnerHtmlAssignment("choice-get-links"    , chrome.i18n.getMessage("optionsChoiceGetLinks"));
safeInnerHtmlAssignment("choice-get-content"  , chrome.i18n.getMessage("optionsChoiceGetContent"));
safeInnerHtmlAssignment("choice-start-paused" , chrome.i18n.getMessage("optionsChoiceStartPaused"));
safeInnerHtmlAssignment("about"               , chrome.i18n.getMessage("optionsAbout"));
safeInnerHtmlAssignment("install"             , chrome.i18n.getMessage("optionsInstall"));
safeInnerHtmlAssignment("install-message"     , chrome.i18n.getMessage("optionsInstallMessage", website_download_link));
safeInnerHtmlAssignment("button-check"        , chrome.i18n.getMessage("optionsRefresh", "»"));
