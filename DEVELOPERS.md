# ![logo](/src/icons/logo/icon32.png) Developers Corner

## Prerequisites

Compiler default is C++11 (eventually C99 for C parts)

Here's the software you'll need before you start building (mandatory in bold)

- **Standard C++ Library** (>= C++11)
- **Qt5** (>= 5.5)
- **Boost** (>= 1.49)
- CMake (>= 3.1.0) or QMake (>= 5.5)
- GNU C++ Compiler (>= 5.3.0) 
  or Microsoft Visual Studio (>= 2017)

Other 3rd-party libraries are included within the project, so built and linked
automatically.


## Build with GCC or MinGW

Use **CMake** or **QMake** (with *QtCreator*).

Setup the project:

	> unzip . downzemall-src.zip
	> mkdir "build"
	> cd ./build/

Setup the *Boost* library:

- For CMake:
    Add variable `BOOST_ROOT_DIR` with the path to Boost
    
- For QMake:
    Open `./3rd/boost/boost.pri`- 
    and add line `INCLUDEPATH += <here-absolute-path-to-boost>/Boost/boost_1_49_0`



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
	> make clean


## Build with MSVC

Use **CMake**.

Setup the project:

	> unzip . downzemall-src.zip
	> mkdir "build"
	> cd ./build/


Build the application:

	> cmake --build . --config Release 
	...
	[ 99%] Linking CXX executable DownZemAll.exe
	[100%] Built target DownZemAll


Run the tests:

	> ctest .
	or
	> ctest . --verbose


Finally install the binary:

	> cmake --build . --target install


## Run

Launch *DownZemAll*:

	> ./DownZemAll.exe


Launch and download links from an URL:

	> ./DownZemAll.exe "https://www.example.com/docs/2019/10/index.htm"


Show more information:

	> ./DownZemAll.exe -h
	> ./DownZemAll.exe -v


Launch in Interactive mode (reserved for WebExtension communication):

	> ./DownZemAll.exe -i [...]
	or
	> ./DownZemAll.exe --interactive [...]


## Troubleshooting

[Troubleshooting page](TROUBLESHOOTING.md "TROUBLESHOOTING.md")


## Acknowledge

Thanks to Andy Portmen for the `native-client` plugin.

Special thanks to the developers of the legacy plugin *DownThemAll!*, especially for keeping the resources free and open-source.

Thanks to Arvid Norberg and contributors for the `libtorrent` library.

 - [DTA!'s legacy github repository](https://github.com/downthemall/downthemall "https://github.com/downthemall/downthemall")
 - Legacy download & description page of the [Firefox Addon](https://addons.mozilla.org/en-US/firefox/addon/downthemall/ "https://addons.mozilla.org/en-US/firefox/addon/downthemall/")
 - [Wikipedia page](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") 
 - [www.downthemall.net](https://www.downthemall.net/ "https://www.downthemall.net/")
 - [www.libtorrent.org](https://www.libtorrent.org/ "https://www.libtorrent.org/")

