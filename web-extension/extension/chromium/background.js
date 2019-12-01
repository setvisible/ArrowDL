"use strict";

const application = 'com.setvisible.downrightnow';

/* ***************************** */
/* Context Menu                  */
/* ***************************** */
function contextMenusCreateCallback() {
    if (chrome.runtime.lastError) {
        console.log(chrome.runtime.lastError.message);
    }
};

chrome.contextMenus.removeAll();

chrome.contextMenus.create({
  id: "save-page",
  title: "Down Right Now",
  contexts: ["page"]
}, contextMenusCreateCallback);

chrome.contextMenus.create({
  id: "save-frame",
  title: "Save frame with Down Right Now",
  contexts: ["frame"]
}, contextMenusCreateCallback);

chrome.contextMenus.create({
  id: "save-selection",
  title: "Save selection with Down Right Now",
  contexts: ["selection"]
}, contextMenusCreateCallback);

chrome.contextMenus.create({
  id: "save-link",
  title: "Save link with Down Right Now",
  contexts: ["link"]
}, contextMenusCreateCallback);

chrome.contextMenus.create({
  id: "save-editable",
  title: "Save editable with Down Right Now",
  contexts: ["editable"]
}, contextMenusCreateCallback);

chrome.contextMenus.create({
  id: "save-image",
  title: "Save image with Down Right Now",
  contexts: ["image"]
}, contextMenusCreateCallback);


chrome.contextMenus.create({
  id: "save-video",
  title: "Save video with Down Right Now",
  contexts: ["video"]
}, contextMenusCreateCallback);

chrome.contextMenus.create({
  id: "save-audio",
  title: "Save audio with Down Right Now",
  contexts: ["audio"]
}, contextMenusCreateCallback);


chrome.contextMenus.onClicked.addListener((info, tab) => {

  if (info.menuItemId === "save-page") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-frame") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-image") {

    save_page(info, tab);

  } else if (info.menuItemId === "save-audio") {

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
  sendData("[CURRENT_URL] " + safeUrl);
};

/* ***************************** */
/* Collect links and media       */
/* ***************************** */
function collectDOMandSendData() {

  function modifyDOM() {
    var array = '';

    // Get the current URL
    const url = document.URL;
    array += '[CURRENT_URL] ';
    array +=  url;
    array += ' ';

    // Get all elements of type <a href='...' ></a>
    array += '[LINKS] ';
    var links = document.getElementsByTagName('a');
    for(var i=0, max=links.length; i<max; i++) {
        array += links[i].href;
        array += ' ';
    }

    // Get all elements of type <img src='...' />
    array += '[MEDIA] ';
    var imagess = document.getElementsByTagName('img');
    for(var i=0, max=imagess.length; i<max; i++) {
      array += imagess[i].src;
      array += ' ';
    }
    return array;
  }

  // argument here is a string but function.toString() returns function's code
  var codeToExecute = '(' + modifyDOM + ')();'; 

  //We have permission to access the activeTab, so we can call chrome.tabs.executeScript:
  chrome.tabs.executeScript({
    "code": codeToExecute 
  }, (results) => {   
    if (chrome.runtime.lastError) {
      console.log(chrome.runtime.lastError.message);
    };
    sendData(results[0]);
  });
};


/* ***************************** */
/* Native Message                */
/* ***************************** */
function sendData(links) {
  var data = "launch " + links;
  chrome.runtime.sendNativeMessage(application,
    { text: data },
    onResponse);
}

function onResponse(response) {
  if (chrome.runtime.lastError) {
    console.log(chrome.runtime.lastError.message);
  }

  if (response === undefined) {
    onError(response);
  } else {
    console.log("Received " + response);
    console.log("Received Message == " + response.text);
  }
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
