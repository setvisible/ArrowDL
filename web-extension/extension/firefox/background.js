"use strict";

const application = "DownRightNow";

/* ***************************** */
/* Context Menu                  */
/* ***************************** */
browser.contextMenus.removeAll(
  function() {
    function addAction(actionId, actionTitle, actionContext) {
      browser.contextMenus.create({
        id: actionId,
        title: actionTitle,
        icons: {
            "16": "icons/icon16.png",
            "32": "icons/icon32.png"
          },
        contexts: [actionContext]
      });
    }
    addAction("save-page",        "Down Right Now",                       "page");
    addAction("save-frame",       "Save frame with Down Right Now",       "frame");
    addAction("save-selection",   "Save selection with Down Right Now",   "selection");
    addAction("save-link",        "Save link with Down Right Now",        "link");
    addAction("save-editable",    "Save editable with Down Right Now",    "editable");
    addAction("save-image",       "Save image with Down Right Now",       "image");
    addAction("save-video",       "Save video with Down Right Now",       "video");
    addAction("save-audio",       "Save audio with Down Right Now",       "audio");
    addAction("save-launcher",    "Save launcher with Down Right Now",    "launcher");  
  }
);

browser.contextMenus.onClicked.addListener(
  function(info, tab) {
           if (info.menuItemId === "save-page"        ) { save_page(info, tab);
    } else if (info.menuItemId === "save-frame"       ) { save_page(info, tab);
    } else if (info.menuItemId === "save-image"       ) { save_image(info, tab);
    } else if (info.menuItemId === "save-audio"       ) { save_page(info, tab);
    } else if (info.menuItemId === "save-launcher"    ) { save_page(info, tab);
    } else if (info.menuItemId === "save-link"        ) { save_link(info, tab);
    } else if (info.menuItemId === "save-selection"   ) { save_page(info, tab);
    } else if (info.menuItemId === "save-video"       ) { save_page(info, tab);
    } else if (info.menuItemId === "save-editable"    ) { save_page(info, tab);
    }
  }
);

function save_page(info, tab) {
  collectDOMandSendData();
}

function save_link(info, tab) {
  const safeUrl = escapeHTML(info.linkUrl);
  sendData("[DOWNLOAD_LINK] " + safeUrl);
}

function save_image(info, tab) {
  const safeUrl = escapeHTML(info.srcUrl);
  sendData("[DOWNLOAD_LINK] " + safeUrl);
}

/* ***************************** */
/* Collect links and media       */
/* ***************************** */
function collectDOMandSendData() {

  function modifyDOM() {
    var array = "";

    // Get the current URL
    const url = document.URL;
    array += "[CURRENT_URL] ";
    array += url;
    array += " ";

    // Get all elements of type <a href="..." ></a>
    array += "[LINKS] ";
    var links = document.getElementsByTagName("a");
    for(var i=0, max=links.length; i<max; i++) {
        array += links[i].href;
        array += " ";
    }

    // Get all elements of type <img src="..." />
    array += "[MEDIA] ";
    var imagess = document.getElementsByTagName("img");
    for(var i=0, max=imagess.length; i<max; i++) {
      array += imagess[i].src;
      array += " ";
    }

    return array;
  }

  //We have permission to access the activeTab, so we can call chrome.tabs.executeScript:
  browser.tabs.executeScript({
    code: '(' + modifyDOM + ')();' //argument here is a string but function.toString() returns function's code
  }, (results) => {   
    sendData(results[0]);
    // console.log('Tab script:');
  });
};


/* ***************************** */
/* Native Message                */
/* ***************************** */
function sendData(links) {
  function onResponse(message) {
    console.log(`Message from the launcher:  ${message.text}`);
  }

  function onError(error) {
    console.log(`Error: ${error}`);
  }

  var data = "launch " + links;
  console.log("Sending message to launcher:  " + data);
  var sending = browser.runtime.sendNativeMessage(application, data);
  sending.then(onResponse, onError);
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
