TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += $$PWD/demo
SUBDIRS += $$PWD/streamwidget
SUBDIRS += $$PWD/texteditorwidget
SUBDIRS += $$PWD/torrentwidget

# Hack
win32-msvc* {
    webengine_DIR=$$PWD/webengine
} else {
    webengine_DIR=$$PWD/demo
}

SUBDIRS += $$webengine_DIR
