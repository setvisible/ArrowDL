"use strict";

const application = "com.arrowdl.extension";

// let us know we're running
console.log("Background service worker has loaded via Manifest V3.");

/* ***************************** */
/* Context Menu                  */
/* ***************************** */
function contextMenusCreateCallback() {
    if (chrome.runtime.lastError) {
        console.log(chrome.runtime.lastError.message);
    }
};

chrome.contextMenus.removeAll(
  function() {
    function addAction(actionId, actionTitle, actionContext) {
      chrome.contextMenus.create({
        id: actionId,
        title: actionTitle,
        contexts: [actionContext]
        }, contextMenusCreateCallback);
    }
    addAction("save-page",      chrome.i18n.getMessage("contextMenuSavePage"),      "page");
    addAction("save-frame",     chrome.i18n.getMessage("contextMenuSaveFrame"),     "frame");
    addAction("save-selection", chrome.i18n.getMessage("contextMenuSaveSelection"), "selection");
    addAction("save-link",      chrome.i18n.getMessage("contextMenuSaveLink"),      "link");
    addAction("save-editable",  chrome.i18n.getMessage("contextMenuSaveEditable"),  "editable");
    addAction("save-image",     chrome.i18n.getMessage("contextMenuSaveImage"),     "image");
    addAction("save-video",     chrome.i18n.getMessage("contextMenuSaveVideo"),     "video");
    addAction("save-audio",     chrome.i18n.getMessage("contextMenuSaveAudio"),     "audio");
  }
);

chrome.contextMenus.onClicked.addListener(
  function(info, tab) {
           if (info.menuItemId === "save-page"        ) { save_page(info, tab);
    } else if (info.menuItemId === "save-frame"       ) { save_page(info, tab);
    } else if (info.menuItemId === "save-image"       ) { save_image(info, tab);
    } else if (info.menuItemId === "save-audio"       ) { save_image(info, tab);
    } else if (info.menuItemId === "save-link"        ) { save_link(info, tab);
    } else if (info.menuItemId === "save-selection"   ) { save_page(info, tab);
    } else if (info.menuItemId === "save-video"       ) { save_image(info, tab);
    } else if (info.menuItemId === "save-editable"    ) { save_page(info, tab);
    }
  }
);

function save_page(info, tab) {
  chrome.storage.local.get((userSettings) => {
    collectDOMandSendData(userSettings);
  });
}

function save_link(info, tab) {
  let safeUrl = escapeHTML(info.linkUrl);
  sendDataToArrowDL("[DOWNLOAD_LINK] " + safeUrl);
}

function save_image(info, tab) {
  const safeUrl = escapeHTML(info.srcUrl);
  sendDataToArrowDL("[DOWNLOAD_LINK] " + safeUrl);
}


/* ***************************** */
/* Collect links and media       */
/* ***************************** */
function collectDOMandSendData(userSettings) {

    chrome.tabs.query({"active": true, "lastFocusedWindow": true}, (tabs) => {

        const tab = tabs[0];
        const tabId = tab.id;
        const tabUrl = tab.url;
       
        if (tabUrl.startsWith("chrome")) {
            console.log("Nothing to download from 'chrome://' or 'chrome-extension://'");
            console.log("ArrowDL is disabled when the current tab is Chrome settings tab.");
           return;
        }
       
        function injectableScriptFunction(args) {
          const restoredSettings = JSON.parse(args); 
          let hasLinks = true;
          let hasMedia = true;
          let array = "";
       
          console.log(restoredSettings);
       
          // Options
          if (restoredSettings.radioApplicationId === 1) {
            array += "";
       
          } else if (restoredSettings.radioApplicationId === 2) {
       
            if (restoredSettings.radioMediaId === 1) {
              hasMedia = false;
              array += "[QUICK_LINKS]";
              array += " ";
       
            } else if (restoredSettings.radioMediaId === 2) {
              hasLinks = false;
              array += "[QUICK_MEDIA]";
              array += " ";
            }
       
            if (restoredSettings.startPaused === true) {
              array += "[STARTED_PAUSED]";
              array += " ";
            }
          }
       
          // Get the current URL
          const url = document.URL;
          array += "[CURRENT_URL] ";
          array += url;
          array += " ";
       
          if (hasLinks) {
            // Get all elements of type <a href="..." ></a>
            array += "[LINKS] ";
            const links = document.getElementsByTagName("a");
            for (let i = 0; i < links.length; i++) {
                array += links[i].href;
                array += " ";
            }
          }
       
          if (hasMedia) {
            // Get all elements of type <img src="..." />
            array += "[MEDIA] ";
            const pictures = document.getElementsByTagName("img");
            for (let i = 0; i < pictures.length; i++) {
              array += pictures[i].src;
              array += " ";
            }
          }

          return array;
        }

        /* Remark:
         * code: "<some code here>"
         * The value of "<some code here>" is actually the function's code of myFunction.
         * myFunction is interpreted as a string. 
         * Indeed, "(" + myFunction + ")()" is a string, because function.toString() returns function's code.
         */
        // const codeToExecute = "(" + myFunction + ")(" + myArgument + ");";
        const injectableScriptFunctionArgumentList = [ JSON.stringify(userSettings) ];

        chrome.scripting.executeScript({
            "target": {"tabId": tabId, "allFrames": true},
            "func": injectableScriptFunction,
            "args": injectableScriptFunctionArgumentList
          }).then(results => {       
            if (chrome.runtime.lastError) {
              console.log(chrome.runtime.lastError.message);
            }
            if (results.length > 0 && results[0] !== undefined) {
              const data = results[0].result;
              sendDataToArrowDL(data);
            }
          });
    });  
}

/* ***************************** */
/* Worker Message                */
/* ***************************** */
chrome.runtime.onMessage.addListener((message , sender, sendResponse) => {
    console.log("Message received!", message);

    if (message === 'get-user-settings') {

        chrome.storage.local.get((userSettings) => {
            if (chrome.runtime.lastError) {
                console.log(chrome.runtime.lastError.message);
                console.log(`Error: ${userSettings}`);
            }

            if (userSettings === undefined) {
                console.log(`Error: ${userSettings}`);
            } else {
                sendResponse(userSettings);
                // console.log("Settings changed: " + JSON.stringify(userSettings));  
            }
        });

    } else if (message === 'collect-dom-and-send-to-native-app') {

        chrome.storage.local.get((userSettings) => {
            collectDOMandSendData(userSettings);
        });

    } else if (message === 'collect-dom-and-send-to-native-app-with-wizard') {

        chrome.storage.local.get((userSettings) => {
            userSettings.radioApplicationId = 1; // Force the Wizard to appear
            collectDOMandSendData(userSettings);
        });

    } else if ('send-to-native-app' in message) {

        const data = message['send-to-native-app'];
        console.log("Sending message to native app:  " + data);
        sendDataToArrowDL(data);

    }

    sendResponse(true);
});

/* ***************************** */
/* Native Message                */
/* ***************************** */
function sendDataToArrowDL(data) {
  const message = {"text": "launch " + data};
  chrome.runtime.sendNativeMessage(application, message, (response) => {
      if (chrome.runtime.lastError) {
        console.log(chrome.runtime.lastError.message);
        console.log(`Error: ${response}`);
      }

      if (response === undefined) {
        console.log(`Error: ${response}`);
      } else {
        console.log("Received response from native app:  " + response.text);    
      }
    });
}

/* ***************************** */
/* Misc                          */
/* ***************************** */
// Always HTML-escape external input to avoid XSS
function escapeHTML(str) {
  // https://gist.github.com/Rob--W/ec23b9d6db9e56b7e4563f1544e0d546
  // Note: string cast using String; may throw if `str` is non-serializable, e.g. a Symbol.
  // Most often this is not the case though.
  return String(str)
    .replace(/&/g, "&amp;")
    .replace(/"/g, "&quot;").replace(/'/g, "&#39;")
    .replace(/</g, "&lt;").replace(/>/g, "&gt;");
}
