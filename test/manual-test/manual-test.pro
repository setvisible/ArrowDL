TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += $$PWD/demo
SUBDIRS += $$PWD/streamwidget

# Hack
win32-msvc* {
    webengine_DIR=$$PWD/webengine
} else {
    webengine_DIR=$$PWD/demo
}

SUBDIRS += $$webengine_DIR
