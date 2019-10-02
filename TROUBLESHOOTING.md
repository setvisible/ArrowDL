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
