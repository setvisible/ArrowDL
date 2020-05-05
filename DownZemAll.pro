TEMPLATE = subdirs
CONFIG  += ordered

# General Note:
# *************
# Each Qt projects (*.pro) here can be built separately.
# Just comment-out the projects you don't want to build.


# Note w.r.t. LibTorrent:
# ***********************
# libtorrent is built apart in a separate Qt project,
# in order to build it as a static library in '-O2' mode (without debug info).
#
# (otherwise the .a can have a big size, more than 200 MB!)
#
# Considering libtorrent a perfect piece of code, we don't want to debug it
# so we don't need 200MB of debug info.
#
# Once built 'make' and 'make install', comment-out the 'libtorrent.pro' line.
#
# Then build 'src' and others in '-O0 -g' debug mode
# that keep a reasonable size for the final executable (i.e. 25 MB)
SUBDIRS += $$PWD/3rd/libtorrent/libtorrent.pro

# src/ contains the main project
SUBDIRS += $$PWD/src/src.pro
src.depends = libtorrent

# test/ contains the tests
SUBDIRS += $$PWD/test/test.pro
test.depends = libtorrent

# web-extension/ contains the web-extension add-on
SUBDIRS += $$PWD/web-extension/launcher/launcher.pro

