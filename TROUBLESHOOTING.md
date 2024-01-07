# Troubleshooting

**This page is frozen. Please use Github > Issues instead.**

## Security Restriction

### Binary signature

Since the binary of *ArrowDL* is **not signed**, *Windows SmartScreen*
might scream at runtime. 

Just ignore it.


### Secure Sockets Layer (SSL)

To download resources through HTTPS (*Secured HTTP*) protocol,
The Qt5's *QtNetwork* requires access to a SSL (*Secure Sockets Layer*) library.

However *Qt* doesn't publish the OpenSSL libraries due to legal restrictions
in some countries. Then, at runtime, when Qt doesn't find the libraries,
it generates *SSH Protocol errors* when trying to connect to HTTP**S** servers.

To solve the problem, copy your OpenSSH libraries in the same directory as your
*ArrowDL* executable. You can also add the path to your OpenSSH libraries to
your system environment path variable (PATH).

- [Qt5 OpenSSL Support](https://doc.qt.io/archives/qt-5.5/opensslsupport.html "https://doc.qt.io/archives/qt-5.5/opensslsupport.html")
- [Wikipedia's OpenSSH article](https://en.wikipedia.org/wiki/OpenSSH "https://en.wikipedia.org/wiki/OpenSSH")


### Firewall ports

*ArrowDL* downloads resources using HTTP and HTTPS protocols.

To do it, configure your Firewall to allow *ArrowDL* to use the following **TCP ports**:

- TCP/80 (used for HTTP protocol)
- TCP/443 (used for HTTPS protocol)


## Fatal crash when starting the Wizard on MSVC builds (Windows only)

__Issue__

ArrowDL built with MSVC 2019 and Qt 5.13 (MSVC version) crashes when opening the web site downloader wizard.

Rem: ArrowDL built with MinGW doesn't have this issue.

__Explanation__

Appveyor-CI doesn't deploy all the required DLLs for *Chromium*.

__Solution 1__

In `./qt.conf`, set `Prefix` with the full path to your local Qt5 directory:
 
    [Paths]
    Prefix=./

Example:

    [Paths]
    Prefix=C:/Qt/Qt5.13.2/5.13.2/msvc2017_64/

__Solution 2__

Add the missing DLLs, EXE and qt.conf, in the app directory, as explained here:

https://doc.qt.io/qt-5/qtwebengine-deploying.html


## Can't launch: "system lookup error" (Linux only)

Error occuring:

    ./ArrowDL
    $ ./ArrowDL: system lookup error: ./ArrowDL undefined symbol _ZN8QSysInfo8buildAbiEv

The executable can't find some shared libraries, or the shared libraries 
(used for build) are different than the shared libraries called at runtime.

For example, you build with *Qt 5.4.1* (`/opt/Qt5.4.1/5.4/gcc/lib`)
but the runtime finds *Qt 5.2.0* (`/usr/lib/i386-linux-gnu/libQt5Core.so.5`).

Look for the Runtime Search Path (RPATH):

    readelf -d ./ArrowDL | grep NEEDED
    $ 0x00000001 (NEEDED)   Shared Library: [libQt5Widgets.so.5]
    $ 0x00000001 (NEEDED)   Shared Library: [libQt5Network.so.5]
    $ 0x00000001 (NEEDED)   Shared Library: [libQt5Gui.so.5]
    $ 0x00000001 (NEEDED)   Shared Library: [libQt5Core.so.5]
    $ 0x00000001 (NEEDED)   Shared Library: [libstdc++.so.6]
    $ 0x00000001 (NEEDED)   Shared Library: [libm.so.6]
    $ 0x00000001 (NEEDED)   Shared Library: [libgcc_s.so.1]
    $ 0x00000001 (NEEDED)   Shared Library: [libc.so.6]

    readelf -d ./ArrowDL | grep RPATH
    $ 0x0000000f (RPATH)    Library rpath: [$ORIGIN]

    ldd ./ArrowDL | grep "not found"
    $ libQt5Widgets.so.5 => not found


__Solution 1__

Set the correct runtime path with LD_LIBRARY_PATH:

    LD_LIBRARY_PATH=/opt/Qt5.4.1/5.4/gcc/lib ./ArrowDL

Create a `ArrowDL.launcher` to set the correct path, or a script:

    ```bash
    #!/bin/bash
    LD_LIBRARY_PATH=/opt/Qt5.4.1/5.4/gcc/lib
    export LD_LIBRARY_PATH
    ./ArrowDL
    ```bash

   
__Solution 2__

Build with the same Qt version as installed.

source:
[Understanding Dynamic Loading](https://amir.rachum.com/blog/2016/09/17/shared-libraries/)


## Can't download streams (Linux only)

__Issue__

When clicking "Help" > "About YT-DLP...", the box shows:

    Error:
    The process has encountered an unknown error.

Open a terminal, go to the application directory and run:

    ./yt-dlp --version
    $ /usr/bin/env: 'python': No such file or directory


__Explanation__

On Linux, `yt-dlp` requires the Python libraries to run.

Check that:

    which python
    $ /usr/bin/python
    
If the command returns an empty line, it means the package `python` (Python2)
is not installed, or Python3 only is installed.


__Solution 1__

Install Python 2.


__Solution 2__

If you want to use Python 3 instead of Python 2, and have Python3 installed:

    which python3
    $ /usr/bin/python3

Open `yt-dlp` with a text editor and replace the first line
`#!/usr/bin/env python` by `#!/usr/bin/env python3` (note the '3'):

    head -n 1 ./yt-dlp
    $ #!/usr/bin/env python
                     ^^^^^^
    
    cp ./yt-dlp ./yt-dlp-OLD
    sed -i 's/python/python3/' ./yt-dlp
    
    head -n 1 ./yt-dlp
    $ #!/usr/bin/env python3
                     ^^^^^^^
