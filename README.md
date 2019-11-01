# ![logo](/src/icons/logo/dza_32x32.png) DownZemAll!

[![GitHub release](https://img.shields.io/github/v/release/setvisible/downzemall.svg)](../../releases/latest)

| Windows | Linux/OSX |
|---------|-----------|
| [![Build status](https://ci.appveyor.com/api/projects/status/github/setvisible/downzemall?branch=master&svg=true)](https://ci.appveyor.com/project/setvisible/downzemall) | [![Build status](https://api.travis-ci.org/setvisible/downzemall.svg?branch=master)](https://travis-ci.org/setvisible/downzemall) |


*DownZemAll!* is a rewrite of the legacy software [DownThemAll!](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") which was an extension for Mozilla Firefox, but whose development stopped around 2016, when Mozilla Firefox migrated to WebExtensions.

![logo](/src/icons/menu/logo.png)

*DownZemAll!* is a standalone download manager for Windows, Mac OS X and Linux. 

It aims to work with latest versions of Mozilla Firefox (powered by *WebExtensions*), and other web browsers (Chrome, Edge, Safari...). 

*DownZemAll!* is written in C++ and based on the [Qt5](https://www.qt.io/ "https://www.qt.io/") framework.


## ![logo](/src/icons/logo/dza_32x32.png) Goals

*DownZemAll!* is a standalone application, embedding its own web engine. That is, it aims to be free and independent, and not rely on any third-party Web Browser technology.

The internal web engine is currently [Google Gumbo Parser](https://github.com/google/gumbo-parser "https://github.com/google/gumbo-parser"). It's a small pure-C HTML5 parser.

When we give an URL address to *DownZemAll!*, *DownZemAll!* downloads the page, parses the HTML code and collects the links contained in the page.

Due to rapid evolution of web technology, *DownZemAll!* is designed to implement new parsers or add existing ones if required.



## ![logo](/src/icons/logo/dza_32x32.png) Screenshots

Screenshots of version 1.

![anim_01](/screenshots/anim_01.gif)

![anim_02](/screenshots/anim_02.gif)


## ![logo](/src/icons/logo/dza_32x32.png) Installation and Usage

### ![logo](/src/icons/logo/dza_32x32.png) Install the standalone application

[Download here](https://github.com/setvisible/DownZemAll/releases "https://github.com/setvisible/DownZemAll/releases") the latest version.

Unzip the archive, and run the executable.

Rem1: *DownZemAll!* is currently shipped as a **portable application** (no installer present).

Rem2: Configuration files will be created in the same directory as the executable.


### ![logo](/src/icons/logo/dza_32x32.png) Install Native-Client

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



### ![logo](/src/icons/logo/dza_32x32.png) Usage

When surfing, click on *DownZemAll!* Button on your Firefox or Chrome toolbar (and/or context menu). 

The `native-client` plugin will send the URL of the current page to *DownZemAll!* as a command line argument:

	.\DownZemAll.exe "https://www.mywebsite.com/2019-06/holidays.html"

After clicking OK, *DownZemAll!* starts to collect the resources.


## ![logo](/src/icons/logo/dza_32x32.png) Build

### Build

Use **CMake** or **QMake** (with *QtCreator*).

Setup the project:

	> unzip . downzemall-src.zip
	> mkdir "build"
	> cd ./build/
	> make clean


Build the application:

	> make -j8
	...
	[ 99%] Linking CXX executable DownZemAll.exe
	[100%] Built target DownZemAll


Run the tests:

	> ctest .
	or
	> ctest . --verbose


Finally install the binary:

	> make install


### Run

Launch *DownZemAll*:

	> ./DownZemAll.exe


Launch and download links from an URL:

	> ./DownZemAll.exe "https://www.example.com/docs/2019/10/index.htm"


Show more information:

	> ./DownZemAll.exe -h
	> ./DownZemAll.exe -v


## ![logo](/src/icons/logo/dza_32x32.png) Troubleshooting

[Troubleshooting page](TROUBLESHOOTING.md "TROUBLESHOOTING.md")


## ![logo](/src/icons/logo/dza_32x32.png) Credits and Resources

Thanks to Andy Portmen for the `native-client` plugin.

Special thanks to the developers of the legacy plugin *DownThemAll!*, especially for keeping the resources free and open-source. 

 - [DTA!'s legacy github repository](https://github.com/downthemall/downthemall "https://github.com/downthemall/downthemall")
 - Legacy download & description page of the [Firefox Addon](https://addons.mozilla.org/en-US/firefox/addon/downthemall/ "https://addons.mozilla.org/en-US/firefox/addon/downthemall/")
 - [Wikipedia page](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") 
 - [www.downthemall.net](https://www.downthemall.net/ "https://www.downthemall.net/")


## ![logo](/src/icons/logo/dza_32x32.png) License

The code is released under the GNU [Lesser General Public License (LGPL)](LICENSE "LICENSE").
