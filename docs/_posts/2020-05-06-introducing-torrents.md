---
layout: post
title: "Introducing Torrents"
date:   2020-05-06 21:23:24 +0100
tags:
  - introducing
  - torrent
  - magnet
  - peer-to-peer
excerpt_separator:  <!--more-->
---

## Introducing Magnet Links and Torrents download

*DownZemAll!* version 2.0.0 is now able to download .torrent files.

### Why a torrent manager?

After implementing a web page content crawler, then a kind of RegExp downloader for batch file downloading, and recently a video streaming downloader, why not adding a new feature in order to download peer-to-peer things like torrents?

So there is.

Indeed, good torrent downloaders have a problem: they are mostly shareware / closed-source software. History is full of solution providers that use their torrent application to **mine bitcoins** without **user consent**, polluting the UI with **advertising banners**, or simply collect the users data to resell it to marketing agencies, crackers and maybe policing government institutions.

In the FOS (free and open-source) world, there are not tons of choices. So DZA is a new way to **enjoy torrents again**. The technology behind the peer-to-peer concept is very exciting to explore, and open so many opportunities.


### How it 's made

*DownZemAll!* uses the library *libtorrent*, a free and open-source library by Arvid Norberg to download magnet links and .torrent files.

[https://www.libtorrent.org/](https://www.libtorrent.org/)

[https://github.com/arvidn/libtorrent](https://github.com/arvidn/libtorrent)

> Remark:
>
> Libtorrent has tons of features. Everything is however customizable in DZA from the Preferences dialog.
>
> There are also 2 presets of settings for lazy people.
> 


## Screenshots

See pictures below to know how to use it:

(yes, this is the icon of transmission)

![Torrent download](../assets/images/2.0/torrent_01.png)

![Torrent download](../assets/images/2.0/torrent_02.png)

Torrent settings are customizable:

![Download torrents](../assets/images/2.0/torrent_prefs.png)

