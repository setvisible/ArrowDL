<img align="left" src="./src/resources/logo/icon64.png">

# NativeClient

## Install Native-Client

Optionally, *NativeClient* can be installed with Firefox or Chrome, and linked to *DownZemAll!*.

*NativeClient* allows you to run *DownZemAll!* from the Firefox or Chrome interface.

To install it:

 - Install the [native-client](https://github.com/andy-portmen/native-client "https://github.com/andy-portmen/native-client") plugin of Andy Portmen in your browser.

This will add the addon "External Application Button" to the web browser.

 - Open the addon settings 
 - Add a new *application button*
 - Configure:
	 - Display Name: `DownZemAll`
	 - Executable Name: select the path to the executable `DownZemAll.exe` 
	 - Argument: `[HREF]`
     - Icon (32x32): select the the executable `DownZemAll.exe`. It contains icons.
	 - Placement:
		 - `[X] Toolbar Button` 
		 - `[ ] Page Context`
		 - `[ ] Frame Context`
		 - `[X] Selection Context`
		 - `[X] Link Context`
		 - `[X] Image Context`
		 - `[ ] Video Context`
		 - `[ ] Audio Context`
		 - `[ ] Tab Context`
 - Click *Update Application* so it save the settings.



## Usage

When surfing, click on *DownZemAll!* Button on your Firefox or Chrome toolbar (and/or context menu). 

The `native-client` plugin will send the URL of the current page to *DownZemAll!* as a command line argument:

  	DownZemAll "https://www.mywebsite.com/2019-06/holidays.html"

After clicking OK, *DownZemAll!* starts to collect the resources.


## Acknowledge

Thanks to Andy Portmen for the `native-client` plugin.


