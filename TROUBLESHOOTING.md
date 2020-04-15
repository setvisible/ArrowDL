# Troubleshooting

## Security Restriction

### Binary signature

Since the binary of *DownZemAll!* is **not signed**, *Windows SmartScreen* might scream at runtime. 

Just ignore it.


### Secure Sockets Layer (SSL)

To download resources through HTTPS (*Secured HTTP*) protocol, The Qt5's *QtNetwork* requires access to a SSL (*Secure Sockets Layer*) library.

However *Qt* doesn't publish the OpenSSL libraries due to legal restrictions in some countries. Then, at runtime, when Qt doesn't find the libraries, it generates *SSH Protocol errors* when trying to connect to HTTP**S** servers.

To solve the problem, copy your OpenSSH libraries in the same directory as your *DownZemAll!* executable. You can also add the path to your OpenSSH libraries to your system environment path variable (PATH).

- [Qt5 OpenSSL Support](https://doc.qt.io/archives/qt-5.5/opensslsupport.html "https://doc.qt.io/archives/qt-5.5/opensslsupport.html")
- [Wikipedia's OpenSSH article](https://en.wikipedia.org/wiki/OpenSSH "https://en.wikipedia.org/wiki/OpenSSH")


### Firewall ports

*DownZemAll!* downloads resources using HTTP and HTTPS protocols.

To do it, configure your Firewall to allow *DownZemAll!* to use the following **TCP ports**:

- TCP/80 (used for HTTP protocol)
- TCP/443 (used for HTTPS protocol)


## Fatal crash when starting the Wizard on MSVC builds (Windows only)

__Issue__

DZA built with MSVC 2019 and Qt5 5.13.2 (MSVC version) crashes when opening the wizard.

Rem: DZA built with MinGW doesn't have this issue.

__Explanation__

Appveyor (CI bot) deploys the wrong Qt5Core.dll version.

Indeed, Qt5Core.dll provided by Appveyor is the version built on 03/11/2019, 
whilst other DLLs in the same archive (Qt5widgets.dll, Qt5Network.dll, -etc.-) are all built on 25/10/2019.  

This difference seem to drive the application to a *FATAL ERROR*, 
especially the action of opening the Wizard to parse an URL crashes the application. 

__Solution__

Replace the faulty Qt5Core.dll (built on 03/11/2019) with the same version as the one of the other DLLs.

Rem: In particular, Qt5Core.dll built on 02/11/2019 seems to work fine, but built on 03/11/2019 crashes.


## Can't launch: "system lookup error" (Linux only)

Error occuring:

    > ./DownZemAll
    ./DownZemAll: system lookup error: ./DownZemAll undefined symbol _ZN8QSysInfo8buildAbiEv

The executable can't find some shared libraries,
or the shared libraries (used for build) are different than the shared libraries called at runtime.

For example, you build with *Qt 5.4.1* (`/opt/Qt5.4.1/5.4/gcc/lib`)
but the runtime finds *Qt 5.2.0* (`/usr/lib/i386-linux-gnu/libQt5Core.so.5`).

Look for the Runtime Search Path (RPATH):

    > readelf -d ./DownZemAll | grep NEEDED
    0x00000001 (NEEDED)   Shared Library: [libQt5Widgets.so.5]
    0x00000001 (NEEDED)   Shared Library: [libQt5Network.so.5]
    0x00000001 (NEEDED)   Shared Library: [libQt5Gui.so.5]
    0x00000001 (NEEDED)   Shared Library: [libQt5Core.so.5]
    0x00000001 (NEEDED)   Shared Library: [libstdc++.so.6]
    0x00000001 (NEEDED)   Shared Library: [libm.so.6]
    0x00000001 (NEEDED)   Shared Library: [libgcc_s.so.1]
    0x00000001 (NEEDED)   Shared Library: [libc.so.6]

    > readelf -d ./DownZemAll | grep RPATH
    0x0000000f (RPATH)    Library rpath: [$ORIGIN]

    > ldd ./DownZemAll | grep "not found"
    libQt5Widgets.so.5 => not found


__Solution 1__

Set the correct runtime path with LD_LIBRARY_PATH:

    > LD_LIBRARY_PATH=/opt/Qt5.4.1/5.4/gcc/lib ./DownZemAll

Create a `DownZemAll.launcher` to set the correct path, or a script:

    ```bash
    #!/bin/bash
    LD_LIBRARY_PATH=/opt/Qt5.4.1/5.4/gcc/lib
    export LD_LIBRARY_PATH
    ./DownZemAll
    ```bash

   
__Solution 2__

Build with the same Qt version as installed.


source:
[Understanding Dynamic Loading](https://amir.rachum.com/blog/2016/09/17/shared-libraries/ "https://amir.rachum.com/blog/2016/09/17/shared-libraries/")
