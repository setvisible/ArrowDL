<img align="left" src="./src/resources/logo/icon64.png">

# NativeClient

## Install Native-Client

Optionally, *NativeClient* can be installed with Firefox or Chrome, and linked to *ArrowDL*.

*NativeClient* allows you to run *ArrowDL* from the Firefox or Chrome interface.

To install it:

 - Install the [native-client](https://github.com/andy-portmen/native-client "https://github.com/andy-portmen/native-client") plugin of Andy Portmen in your browser.

This will add the addon "External Application Button" to the web browser.

 - Open the addon settings 
 - Add a new *application button*
 - Configure:
	 - Display Name: `ArrowDL`
	 - Executable Name: select the path to the executable `ArrowDL.exe` 
	 - Argument: `[HREF]`
     - Icon (32x32): select the the executable `ArrowDL.exe`. It contains icons.
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

When surfing, click on *ArrowDL* Button on your Firefox or Chrome toolbar (and/or context menu). 

The `native-client` plugin will send the URL of the current page to *ArrowDL* as a command line argument:

  	ArrowDL "https://www.mywebsite.com/2019-06/holidays.html"

After clicking OK, *ArrowDL* starts to collect the resources.


## Acknowledge

Thanks to Andy Portmen for the `native-client` plugin.


