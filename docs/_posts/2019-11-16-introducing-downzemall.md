---
layout: post
title: Introducing DownZemAll!
date:   2019-11-16 19:06:30 +0100
tags:
  - update
  - introducing
excerpt_separator:  <!--more-->
---

*DownZemAll!* is a rewrite of the legacy software [DownThemAll!](https://en.wikipedia.org/wiki/DownThemAll! "https://en.wikipedia.org/wiki/DownThemAll!") which was an extension for Mozilla Firefox, but whose development stopped around 2016, when Mozilla Firefox migrated to WebExtensions.

*DownZemAll!* is a standalone download manager for Windows, Mac OS X and Linux. 

It aims to work with latest versions of Mozilla Firefox (powered by *WebExtensions*), and other web browsers (Chrome, Edge, Safari...). 

*DownZemAll!* is written in C++ and based on the [Qt5](https://www.qt.io/ "https://www.qt.io/") framework.


## Goals

*DownZemAll!* is a standalone application, embedding its own web engine. That is, it aims to be free and independent, and not rely on any third-party Web Browser technology.

The internal web engine is currently [Google Gumbo Parser](https://github.com/google/gumbo-parser "https://github.com/google/gumbo-parser"). It's a small pure-C HTML5 parser.

When we give an URL address to *DownZemAll!*, *DownZemAll!* downloads the page, parses the HTML code and collects the links contained in the page.

Due to rapid evolution of web technology, *DownZemAll!* is designed to implement new parsers or add existing ones if required.




## Download

*DownZemAll!* is developed on and hosted with GitHub. Head to the <a href="https://github.com/setvisible/DownZemAll">GitHub repository</a> for downloads, bug reports, and features requests.

Thanks!
