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
