# ![logo](/src/icons/logo/dza_32x32.png) DownZemAll!

*DownZemAll!* is a rewrite of the legacy software [DownThemAll!](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") which was an extension for Mozilla Firefox, but whose development stopped around 2016, when Mozilla Firefox migrated to WebExtensions.

![logo](/src/icons/menu/logo.png)

*DownZemAll!* is a free download manager for Windows, Mac OS X and Linux. 

It aims to work with latest versions of Mozilla Firefox (powered by *WebExtensions*), and other web browsers (Chrome, Edge, Safari...). 

*DownZemAll!* is written in C++ and based on the [Qt5](https://www.qt.io/ "https://www.qt.io/") framework.


## ![logo](/src/icons/logo/dza_32x32.png) Screenshots

todo


## ![logo](/src/icons/logo/dza_32x32.png) Installation and Usage

### ![logo](/src/icons/logo/dza_32x32.png) Installation

*DownZemAll!* is currently shipped as a binary (no installer present). Just click on one of the release numbers above (probably the latest), download and uncompress the archive and run `DownZemAll.exe`.

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



### ![logo](/src/icons/logo/dza_32x32.png) Usage

When web surfing, click the *DownZemAll!* Button on the web browser toolbar or context menu. 

The `native-client` plugin sends the URL of the current page to *DownZemAll!* as a command line argument:

	.\DownZemAll.exe "https://www.mywebsite.com/2019-06/holidays.html"

After approval, *DownZemAll!* collects the resources at the page address.




## ![logo](/src/icons/logo/dza_32x32.png) Build

todo


## ![logo](/src/icons/logo/dza_32x32.png) Credits and Resources

Thanks to Andy Portmen for the `native-client` plugin.

Special thanks to the developers of the legacy plugin *DownThemAll!*, especially for keeping the resources free and open-source. 

 - [DTA!'s legacy github repository](https://github.com/downthemall/downthemall "https://github.com/downthemall/downthemall")
 - Legacy download & description page of the [Firefox Addon](https://addons.mozilla.org/en-US/firefox/addon/downthemall/ "https://addons.mozilla.org/en-US/firefox/addon/downthemall/")
 - [Wikipedia page](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") 
 - [www.downthemall.net](https://www.downthemall.net/ "https://www.downthemall.net/")


## ![logo](/src/icons/logo/dza_32x32.png) License

The code is released under the GNU [Lesser General Public License (LGPL)](LICENSE "LICENSE").
