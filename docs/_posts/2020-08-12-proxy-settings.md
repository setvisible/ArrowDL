---
layout: post
title: "Proxy Server"
date:   2020-08-11 21:21:21 +0100
categories:
  - Tips
tags:
  - proxy
  - privacy
excerpt_separator:  <!--more-->
---

**ArrowDL** can use a proxy server now.

A **proxy server** sits between your computer and the internet. Acting as a bridge between **ArrowDL** or your web browser and internet services like websites and software-as-a-service (SaaS) platforms, a proxy server can potentially make your internet more private and secure. Here's how to set up **ArrowDL** so you can use a proxy server to your advantage. 


## What is a proxy server 

A proxy server is a gateway between you and the internet. When you visit a website, the proxy server communicates with it on behalf of ArrowDL. Then, when the website answers, the proxy forwards the data to you. 

Proxy servers can do many jobs. These include scanning for viruses, acting as a firewall, speeding up your connection by caching, and hiding your public IP address. 


## How to set up your proxy server

Here's how to set up your ArrowDL to use a proxy server :

### Network Proxy

1. Go to **Options** > **Preferences** > **Network**

2. Enable proxying by changing the *Type* to **SOCKS5** or **HTTP**

3. Fill the proxy address, port, and user authentification. Note that ArrowDL stores the password in clear text


| Type   | Description                   |
|--------|-------------------------------|
| None   | No proxying is used. |
| SOCKS5 | based on [RFC 1928](https://www.rfc-editor.org/rfc/rfc1928.txt) and [RFC 1929](https://www.rfc-editor.org/rfc/rfc1929.txt). Generic proxy for any kind of connection. Supports TCP, UDP, binding to a port (incoming connections) and authentication. |
| HTTP   | A HTTP transparent proxying is used. Implemented using the "CONNECT" command, supports only outgoing TCP connections; supports authentication. |


![Network Proxy](/ArrowDL/assets/images/2.0/proxy_settings_01.png)



### Torrent Proxy

LibTorrent uses a separate settings storage, then to enable proxying with torrent, you should :

1. Go to **Options** > **Preferences** > **Advanced**

2. Filter with "*proxy*"

3. Fill the proxy address, port, and user authentification 

![Torrent Proxy](/ArrowDL/assets/images/2.0/proxy_settings_02.png)


### Further reading

- [Proxy server](https://en.wikipedia.org/wiki/Proxy_server) definition on Wikipedia
- Qt5 [Proxy Documentation](https://doc.qt.io/qt-5/qnetworkproxy.html)
- LibTorrent [Proxy Documentation](https://www.libtorrent.org/features.html)


---

The introduction of this article is largely inspired by [a post from the Avast blog](https://www.avast.com/c-how-to-set-up-a-proxy)


