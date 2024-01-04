<img align="left" src="./src/resources/logo/icon64.png">

# Developers Corner

## Prerequisites

Compiler default is C++11 (eventually C99 for C parts)

Here's the software you'll need before you start building (mandatory in bold)

- **Standard C++ Library** (>= C++11)
- **Qt6** (>= 6.0)
- **Boost** (>= 1.77)
- CMake (>= 3.1.0)
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
    sudo apt install git-gui
    
#### Install Qt and QtCreator

    sudo apt install libfontconfig1
    sudo apt install mesa-common-dev
    sudo apt install libglu1-mesa-dev
    
    wget https://download.qt.io/official_releases/qt/6.0/6.0.0/qt-opensource-linux-x64-6.0.0.run

    chmod +x qt-opensource-linux-x64-6.0.0.run
    sudo ./qt-opensource-linux-x64-6.0.0.run

It install the tools (QtCreator) and SDK on `/opt/Qt/`.

Run it:

    /opt/Qt/Tools/QtCreator/bin/qtcreator

Create a shortcut (Ubuntu, KDE...):

    mkdir ~/.local/share/applications
    cp /opt/Qt/Tools/QtCreator/share/applications/org.qt-project.qtcreator.desktop ~/.local/share/applications



#### Install Boost C++ Library

    wget https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.bz2
    tar -zxvf boost_1_77_0.tar.gz


Or download and unzip manually an older version of [Boost](https://www.boost.org/users/history/).


## Build with GCC or MinGW

Use **CMake** or **QMake** (with *QtCreator*).

Setup the project:

    unzip . arrowdl-src.zip
    mkdir "build"
    cd ./build/

Setup the *Boost* library:

- For CMake:
    Add variable `BOOST_ROOT_DIR` (type: PATH) with the path to Boost
    PS: or configure `Boost_INCLUDE_DIR`


Build the application:

    make -j8
    $ ...
    $ [ 99%] Linking CXX executable ArrowDL.exe
    $ [100%] Built target ArrowDL


Run the tests:

    ctest .
    ctest . --verbose


Finally install the binary and clean:

    make install
    make clean


## Build with MSVC

Use **CMake**.

Setup the project:

    unzip . arrowdl-src.zip
    mkdir "build"
    cd ./build/


Build the application:

    cmake --build . --parallel 8 --config Release
    $ ...
    $ [ 99%] Linking CXX executable ArrowDL.exe
    $ [100%] Built target ArrowDL


Run the tests:

    ctest . -C Release
    ctest . -C Release --verbose


Finally install the binary and clean:

    cmake --build . --target install --config Release
    cmake --build . --target clean --config Release


## Run

Launch *ArrowDL*:

    ArrowDL

Show help and version:

    ArrowDL -h (or --help)
    ArrowDL -v (or --version)

Launch and download links from an URL:

    ArrowDL "https://www.example.com/docs/2019/10/index.htm"


Launch in Interactive mode (reserved for WebExtension communication):

    ArrowDL -i (or --interactive) <urls>


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

