"use strict";

const application = "com.arrowdl.extension";

/* ***************************** */
/* Native Message                */
/* ***************************** */
function checkConnection() {
  // todo  browser.runtime.sendMessage({'send-to-native-app': "areyouthere"});
  const message = {"text": "areyouthere"};
  browser.runtime.sendNativeMessage(application, message, (response) => {
    if (browser.runtime.lastError) {
      console.log(browser.runtime.lastError.message);
      console.log(`Error: ${response}`);
      showWarningMessage(true);
    }

    if (response === undefined) {
      console.log(`Error: ${response}`);
      showWarningMessage(true);
    } else {
      showWarningMessage(false);
    }
  });
}


/* ***************************** */
/* Core                          */
/* ***************************** */
function showWarningMessage(hasError) {
  const x = document.getElementById("warning-area");
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

function immediateButtonLabel(userSettings) {
  const mediaId = getSettingMediaId(userSettings);
  const startPaused = isSettingStartPaused(userSettings);
  if (mediaId === 1) {
    if (startPaused) {
      return browser.i18n.getMessage("popupDownloadLinksPaused")
    } else {
      return browser.i18n.getMessage("popupDownloadLinks")
    }
  } else if (mediaId === 2) {
    if (startPaused) {
      return browser.i18n.getMessage("popupDownloadContentPaused")
    } else {
      return browser.i18n.getMessage("popupDownloadContent")
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
/* Events                        */
/* ***************************** */
document.addEventListener('DOMContentLoaded', () => {
    checkConnection();
  
    // send a message to the background
    browser.runtime.sendMessage('get-user-settings', (userSettings) => {        
        const enabled = isSettingAskEnabled(userSettings);
        setVisible("button-immediate-download", !enabled);
  
        if (!enabled) {
          const label = immediateButtonLabel(userSettings);
          safeInnerHtmlAssignment("button-immediate-download-label", label);
        }
    });
}); 


document.getElementById("button-start").addEventListener('click', () => {
    browser.runtime.sendMessage('collect-dom-and-send-to-native-app-with-wizard');
    window.close();
});

document.getElementById("button-immediate-download").addEventListener('click', () => {
    browser.runtime.sendMessage('collect-dom-and-send-to-native-app');
    window.close();
});

document.getElementById("button-manager").addEventListener('click', () => {
    browser.runtime.sendMessage({'send-to-native-app': "[MANAGER]"});
    window.close();
});

document.getElementById("button-preference").addEventListener('click', () => {
    browser.runtime.sendMessage({'send-to-native-app': "[PREFS]"});
    window.close();
});

document.getElementById("button-options-page").addEventListener('click', () => {
    const openingPage = browser.runtime.openOptionsPage();
    window.close();
});

document.getElementById("button-website").addEventListener('click', () => {
    window.open(document.getElementById("website-link").getAttribute("href"), "_blank");
    window.close();
});

/* ***************************** */
/* Options                       */
/* ***************************** */
function isSettingAskEnabled(userSettings) {
  return userSettings === undefined || userSettings.radioApplicationId === undefined || userSettings.radioApplicationId === 1;
}

function getSettingMediaId(userSettings) {
  if (userSettings === undefined) {
    return -1;
  }
  return userSettings.radioMediaId;
}

function isSettingStartPaused(userSettings) {
  if (userSettings === undefined) {
    return false;
  }
  return userSettings.startPaused;
}

/* ***************************** */
/* Internationalization          */
/* ***************************** */
safeInnerHtmlAssignment("button-download",    browser.i18n.getMessage("popupDownload"));
safeInnerHtmlAssignment("button-open",        browser.i18n.getMessage("popupOpen"));
safeInnerHtmlAssignment("button-preferences", browser.i18n.getMessage("popupPreferences"));
safeInnerHtmlAssignment("button-options",     browser.i18n.getMessage("popupOptions"));
safeInnerHtmlAssignment("website-link",       browser.i18n.getMessage("popupVisitWebsite"));

safeInnerHtmlAssignment("msg-error",          browser.i18n.getMessage("popupError"));
safeInnerHtmlAssignment("msg-error-1",        browser.i18n.getMessage("popupError1"));
safeInnerHtmlAssignment("msg-error-2",        browser.i18n.getMessage("popupError2"));

safeInnerHtmlAssignment("msg-remark",         browser.i18n.getMessage("popupRemark"));
safeInnerHtmlAssignment("msg-remark-1",       browser.i18n.getMessage("popupRemark1"));
