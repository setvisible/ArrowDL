"use strict";

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
    var openingPage = chrome.runtime.openOptionsPage();
    window.close();
});
