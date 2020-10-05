#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_torrentbasecontext
CONFIG      += testcase
QT           = core testlib network
SOURCES     += tst_torrentbasecontext.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/format.h \
    $$PWD/../../../src/core/torrent.h \
    $$PWD/../../../src/core/torrentbasecontext.h \
    $$PWD/../../../src/core/torrentmessage.h

SOURCES += \
    $$PWD/../../../src/core/format.cpp \
    $$PWD/../../../src/core/torrent.cpp \
    $$PWD/../../../src/core/torrentbasecontext.cpp \
    $$PWD/../../../src/core/torrentmessage.cpp
