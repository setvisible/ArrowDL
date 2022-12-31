---
layout: post
title: "Major Upgrade Announcement"
date:   2022-12-31 16:15:12 +0100
tags:
  - qt
  - deployment
  - build
  - project
excerpt_separator:  <!--more-->
---

## Major Upgrade Announcement

During 2022, a lot of refactoring has been done. 

The final phase of this refactoring was the move of the deployment process to the Github Actions. This was done yesterday!

Thus, the *DownZemAll!* of 2023 will be better than 2022. It deserves a "version 3.x" to introduces this new era.


### Introducing Qt6

*DownZemAll!* now uses **Qt6** instead of Qt5. This change required many refactoring, to use the new Qt API and remove obsolete parts.

It permits to use the last C++ standard: **C++20**, instead of C++11.

Many other advantages of this major upgrade: simplier and more robust API, some changes with network and sockets like QWebEngine, etc. We might dream that the Gumbo Parser and other workarounds could be eventually replaced soon by native Qt6 functions.


### Introducing Libtorrent-Rasterbar v2.0

In 2022, **LibTorrent-Rasterbar** has been also upgraded from v1.2.5 to v2.0.7.

This change was only possible because the Qt6's compiler enables C++20.

Note: Some deprecated functions still remain temporarly, but should be removed in future releases.


### Introducing Github Actions

*Appveyor-CI* (historical build runner for Windows targets) and *Travis-CI* (historical build runner for Linux targets) have been replaced both by **Github Actions**.

This new build runner will allow a lot of simplification and maintenance reduction.


### Simplify project files

Historically, we had to maintain both *CMakelists.txt* and *.pro/.pri*,
to make build possible from the QtCreator's GUI and from the CMake command line.

Today, QtCreator is able to compile from *CMakeList.txt* directly, so the .pro/.pri are no longer necessary and have been removed.


### A new hope for 2023

Thanks to these upgrades in 2022, 2023 looks brighter!

C++20 offers more simplification and abstraction, Qt6 offers better widgets, and LibTorrent-Rasterbar offers better functions. *DownZemAll!* will be improved a lot, for sure :)

Happy New Year 2023! :)

