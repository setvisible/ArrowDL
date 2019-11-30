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
  var connectionStatus = 'Ok'
  var details = 'Detected path: ' + response.text
  document.getElementById('status-message').innerHTML 
    = '<p><span style="border:4px solid MediumSeaGreen; background-color:MediumSeaGreen; color:White;"> ' + connectionStatus + '</span>' 
    + '<br/><br/>'
    + details
    + '</p>';
}

function onHelloError(error) {
  console.log(`Error: ${error}`);

  var connectionStatus = 'Error: Can\'t find the launcher.'
  var details = 'Follow the Download and Install instructions below.'
  document.getElementById('status-message').innerHTML 
    = '<p><span style="border:4px solid Tomato; background-color:Tomato; color:White;"> ' + connectionStatus + '</span>' 
    + '<br/><br/>'
    + details
    + '</p>';
}

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

