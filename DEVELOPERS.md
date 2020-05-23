# ![logo](/src/icons/logo/icon32.png) Developers Corner

## Prerequisites

Compiler default is C++11 (eventually C99 for C parts)

Here's the software you'll need before you start building (mandatory in bold)

- **Standard C++ Library** (>= C++11)
- **Qt5** (>= 5.5)
- **Boost** (>= 1.58)
- CMake (>= 3.1.0) or QMake (>= 5.5)
- GNU C++ Compiler (gcc/g++ >= 5.3.0)
  or Microsoft Visual Studio (>= 2017)

Other 3rd-party libraries are included within the project, so built and linked
automatically.


### Setup Development Tools for Windows

Follow official instructions of each tool.


### Setup Development Tools for Unix

(tested on Ubuntu 20.04 LTS 64bits)

#### Install GCC, CMake, Git...

    sudo apt install build-essential
    sudo apt install cmake-qt-gui
    sudo apt install git
    
#### Install Qt5

    sudo apt install qt5-default
    whereis qt5
    $ qt5: /usr/lib/x86_64-linux-gnu/qt5 /usr/lib/qt5 /usr/share/qt5
    
#### Install Qt5 IDE (QtCreator) and SDK

    sudo apt install qtcreator
    
Or follow [instructions](https://wiki.qt.io/Install_Qt_5_on_Ubuntu
"https://wiki.qt.io/Install_Qt_5_on_Ubuntu").

    sudo apt-get install libfontconfig1
    sudo apt-get install mesa-common-dev
    sudo apt-get install libglu1-mesa-dev -y
    
    wget https://download.qt.io/official_releases/qt/5.14/5.14.2/qt-opensource-linux-x64-5.14.2.run

    chmod +x qt-opensource-linux-x64-5.14.2.run
    sudo ./qt-opensource-linux-x64-5.14.2.run

It install the tools and SDK on `/opt/Qt5.14.2/`.


## Build with GCC or MinGW

Use **CMake** or **QMake** (with *QtCreator*).

Setup the project:

    unzip . downzemall-src.zip
    mkdir "build"
    cd ./build/

Setup the *Boost* library:

- For CMake:
    Add variable `BOOST_ROOT_DIR` (type: PATH) with the path to Boost
    PS: or configure `Boost_INCLUDE_DIR`
    
- For QMake:
    Open `./3rd/boost/boost.pri`- 
    and add path manually to variable `BOOST_ROOT_DIR`,
    that gives the following line: 
    `BOOST_ROOT_DIR = <here-absolute-path-to-boost>/Boost/boost_1_58_0`



Build the application:

    make -j8
    $ ...
    $ [ 99%] Linking CXX executable DownZemAll.exe
    $ [100%] Built target DownZemAll


Run the tests:

    ctest .
    ctest . --verbose


Finally install the binary and clean:

    make install
    make clean


## Build with MSVC

Use **CMake**.

Setup the project:

    unzip . downzemall-src.zip
    mkdir "build"
    cd ./build/


Build the application:

    cmake --build . --parallel 8 --config Release
    $ ...
    $ [ 99%] Linking CXX executable DownZemAll.exe
    $ [100%] Built target DownZemAll


Run the tests:

    ctest . -C Release
    ctest . -C Release --verbose


Finally install the binary and clean:

    cmake --build . --target install --config Release
    cmake --build . --target clean --config Release


## Run

Launch *DownZemAll*:

    DownZemAll

Show help and version:

    DownZemAll -h (or --help)
    DownZemAll -v (or --version)

Launch and download links from an URL:

    DownZemAll "https://www.example.com/docs/2019/10/index.htm"


Launch in Interactive mode (reserved for WebExtension communication):

    DownZemAll -i (or --interactive) <urls>


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

