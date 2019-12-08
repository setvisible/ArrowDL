"use strict";

const application = 'DownRightNow';

/* ***************************** */
/* Context Menu                  */
/* ***************************** */
browser.contextMenus.create({
  id: "save-page",
  title: "Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["page"]
});

browser.contextMenus.create({
  id: "save-frame",
  title: "Save frame with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["frame"]
});

browser.contextMenus.create({
  id: "save-selection",
  title: "Save selection with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["selection"]
});

browser.contextMenus.create({
  id: "save-link",
  title: "Save link with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["link"]
});

browser.contextMenus.create({
  id: "save-editable",
  title: "Save editable with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["editable"]
});

browser.contextMenus.create({
  id: "save-image",
  title: "Save image with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["image"]
});


browser.contextMenus.create({
  id: "save-video",
  title: "Save video with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["video"]
});

browser.contextMenus.create({
  id: "save-audio",
  title: "Save audio with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["audio"]
});

browser.contextMenus.create({
  id: "save-launcher",
  title: "Save launcher with Down Right Now",
  icons: {
      "16": "icons/icon16.png",
      "32": "icons/icon32.png"
    },
  contexts: ["launcher"]
});

browser.contextMenus.onClicked.addListener((info, tab) => {

  if (info.menuItemId === "save-page") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-frame") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-image") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-audio") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-launcher") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-link") {

    save_link(info, tab);

  } else if (info.menuItemId === "save-selection") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-video") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-editable") {

    save_page(info, tab);

  }
});

function save_page(info, tab) {
  collectDOMandSendData();
}

function save_link(info, tab) {
  const safeUrl = escapeHTML(info.linkUrl);
  sendData("[DOWNLOAD_LINK] " + safeUrl);
};
};

/* ***************************** */
/* Collect links and media       */
/* ***************************** */
function collectDOMandSendData() {

  function modifyDOM() {
    var array = "";

    // Get the current URL
    const url = document.URL;
    array += "[CURRENT_URL] ";
    array +=  url;
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
  var data = "launch " + links;
  // console.log("Sending:  " + data);
  var sending = browser.runtime.sendNativeMessage(application, data);
  sending.then(onResponse, onError);
}

function onResponse(response) {
  // console.log("Received " + response);
  // console.log("Received Message == " + response.text);
}

function onError(error) {
  console.log(`Error: ${error}`);
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
