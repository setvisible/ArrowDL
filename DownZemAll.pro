TEMPLATE = subdirs
CONFIG  += ordered

# Note:
# libtorrent is built apart as a static library, -O2 without debug info
# (otherwise the binary is more than 200 MB)
# This allows to build 'src' with -O0 -g debug info
# and keep a reasonable size (i.e. 25 MB)
#SUBDIRS += $$PWD/3rd/libtorrent

SUBDIRS += $$PWD/src/src.pro
SUBDIRS += $$PWD/test/test.pro

SUBDIRS += $$PWD/web-extension/launcher/launcher.pro
