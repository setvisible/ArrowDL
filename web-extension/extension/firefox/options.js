"use strict";

const application = 'DownRightNow';

/* ***************************** */
/* Native Message                */
/* ***************************** */
function checkConnection() {
  var data = "areyouthere";
  var sending = browser.runtime.sendNativeMessage(application, data);
  sending.then(onHelloResponse, onHelloError);
};

function onHelloResponse(response) {
  var connectionStatus = 'Ok';
  var details = 'Detected path: ' + response.text;
  safeInnerHtmlAssignment(connectionStatus, details, 'MediumSeaGreen');
};

function onHelloError(error) {
  console.log(`Error: ${error}`);
  var connectionStatus = 'Error: Can\'t find the launcher.';
  var details = 'Follow the Download and Install instructions below.';
  safeInnerHtmlAssignment(connectionStatus, details, 'Tomato');
};

function safeInnerHtmlAssignment(connectionStatus, details, color) {
  const statusTag = `<p><span style="border:4px solid ${color}; background-color:${color}; color:White;">${connectionStatus}</span><br/><br/>${details}</p>`;

  const parser = new DOMParser()
  const parsed = parser.parseFromString(statusTag, `text/html`)
  const tags = parsed.getElementsByTagName(`body`)
   
  document.getElementById('status-message').innerHTML = ``
  for (const tag of tags) {
    document.getElementById('status-message').appendChild(tag)
  }
};

/* ***************************** */
/* GUI Event                     */
/* ***************************** */
function buttonClicked() {;
  checkInstallation();
};

function checkInstallation() {
  checkConnection();
};

document.addEventListener('DOMContentLoaded', checkInstallation); 

document.querySelector("form").addEventListener("submit", buttonClicked);
