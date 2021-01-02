#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_downloadmanager
CONFIG      += testcase
QT           = core testlib network
SOURCES     += tst_downloadmanager.cpp

CONFIG      += c++11


#-------------------------------------------------
# VERSION
#-------------------------------------------------
DEFINES += APP_VERSION=\\\"0.0.0.0\\\"

#-------------------------------------------------
# Dependencies
#-------------------------------------------------
include($$PWD/../../../3rd/boost/boost.pri)


#-------------------------------------------------
# LibTorrent
#-------------------------------------------------
LIBTORRENT_INSTALL = $${OUT_PWD}/../../../libtorrent_install

!exists($$LIBTORRENT_INSTALL) {
    error("libtorrent's install directory doesn't exist: $$LIBTORRENT_INSTALL")
}

# Note: order of declared libs is very important
LIBS += -L$${LIBTORRENT_INSTALL}/lib -llibtorrent
# LIBS += -lwsock32 -lws2_32 -lIphlpapi --> see libtorrent-config.pri

INCLUDEPATH += $${LIBTORRENT_INSTALL}/include/

include($$PWD/../../../3rd/libtorrent/libtorrent-config.pri)


#-------------------------------------------------
# SOURCES
#-------------------------------------------------
# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/abstractdownloaditem.h \
    $$PWD/../../../src/core/abstractsettings.h \
    $$PWD/../../../src/core/downloadengine.h \
    $$PWD/../../../src/core/downloaditem.h \
    $$PWD/../../../src/core/downloaditem_p.h \
    $$PWD/../../../src/core/downloadmanager.h \
    $$PWD/../../../src/core/downloadstreamitem.h \
    $$PWD/../../../src/core/downloadtorrentitem.h \
    $$PWD/../../../src/core/file.h \
    $$PWD/../../../src/core/fileutils.h \
    $$PWD/../../../src/core/format.h \
    $$PWD/../../../src/core/idownloaditem.h \
    $$PWD/../../../src/core/ifileaccessmanager.h \
    $$PWD/../../../src/core/mask.h \
    $$PWD/../../../src/core/networkmanager.h \
    $$PWD/../../../src/core/resourceitem.h \
    $$PWD/../../../src/core/session.h \
    $$PWD/../../../src/core/settings.h \
    $$PWD/../../../src/core/stream.h \
    $$PWD/../../../src/core/torrent.h \
    $$PWD/../../../src/core/torrentbasecontext.h \
    $$PWD/../../../src/core/torrentcontext.h \
    $$PWD/../../../src/core/torrentcontext_p.h \
    $$PWD/../../../src/core/torrentmessage.h       

SOURCES += \      
    $$PWD/../../../src/core/abstractdownloaditem.cpp \
    $$PWD/../../../src/core/abstractsettings.cpp \
    $$PWD/../../../src/core/downloadengine.cpp \
    $$PWD/../../../src/core/downloaditem.cpp \
    $$PWD/../../../src/core/downloadmanager.cpp \
    $$PWD/../../../src/core/downloadstreamitem.cpp \
    $$PWD/../../../src/core/downloadtorrentitem.cpp \
    $$PWD/../../../src/core/file.cpp \
    $$PWD/../../../src/core/fileutils.cpp \
    $$PWD/../../../src/core/format.cpp \
    $$PWD/../../../src/core/mask.cpp \
    $$PWD/../../../src/core/networkmanager.cpp \
    $$PWD/../../../src/core/resourceitem.cpp \
    $$PWD/../../../src/core/session.cpp \
    $$PWD/../../../src/core/settings.cpp \
    $$PWD/../../../src/core/stream.cpp \
    $$PWD/../../../src/core/torrent.cpp \
    $$PWD/../../../src/core/torrentbasecontext.cpp \
    $$PWD/../../../src/core/torrentcontext.cpp \
    $$PWD/../../../src/core/torrentcontext_p.cpp \
    $$PWD/../../../src/core/torrentmessage.cpp
