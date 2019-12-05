# ![logo](/src/icons/logo/icon32.png) Developers Corner

## Build with GNU or MinGW

Use **CMake** or **QMake** (with *QtCreator*).

Setup the project:

	> unzip . downzemall-src.zip
	> mkdir "build"
	> cd ./build/


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

 - [DTA!'s legacy github repository](https://github.com/downthemall/downthemall "https://github.com/downthemall/downthemall")
 - Legacy download & description page of the [Firefox Addon](https://addons.mozilla.org/en-US/firefox/addon/downthemall/ "https://addons.mozilla.org/en-US/firefox/addon/downthemall/")
 - [Wikipedia page](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") 
 - [www.downthemall.net](https://www.downthemall.net/ "https://www.downthemall.net/")

