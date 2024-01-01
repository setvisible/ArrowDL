# WebExtension

A WebExtension is an add-on for web browsers(*).

This add-on permits the web browser to communicate with ArrowDL.


__Organisation__

 - `/extension`
 
    Contains the WebExtension source code.

    The WebExtension creates the web browser messages.


 - `/launcher`

    Contains the application launcher source code.

    The launcher passes asynchroneous messages from the web browser to the application.


 - `/setup`

    Contains several scripts and files that install/uninstall the launcher.


(*)  for Chrome, Firefox, WaterFox, Thunderbird
