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
HEADERS     += $$PWD/../../../src/core/abstractdownloaditem.h
SOURCES     += $$PWD/../../../src/core/abstractdownloaditem.cpp

HEADERS     += $$PWD/../../../src/core/abstractsettings.h
SOURCES     += $$PWD/../../../src/core/abstractsettings.cpp

HEADERS     += $$PWD/../../../src/core/downloadengine.h
SOURCES     += $$PWD/../../../src/core/downloadengine.cpp

HEADERS     += $$PWD/../../../src/core/downloaditem.h
HEADERS     += $$PWD/../../../src/core/downloaditem_p.h
SOURCES     += $$PWD/../../../src/core/downloaditem.cpp

HEADERS     += $$PWD/../../../src/core/downloadmanager.h
SOURCES     += $$PWD/../../../src/core/downloadmanager.cpp

HEADERS     += $$PWD/../../../src/core/downloadstreamitem.h
SOURCES     += $$PWD/../../../src/core/downloadstreamitem.cpp

HEADERS     += $$PWD/../../../src/core/downloadtorrentitem.h
SOURCES     += $$PWD/../../../src/core/downloadtorrentitem.cpp

HEADERS     += $$PWD/../../../src/core/downloadtorrentitem_p.h
SOURCES     += $$PWD/../../../src/core/downloadtorrentitem_p.cpp

HEADERS     += $$PWD/../../../src/core/file.h
SOURCES     += $$PWD/../../../src/core/file.cpp

HEADERS     += $$PWD/../../../src/core/format.h
SOURCES     += $$PWD/../../../src/core/format.cpp

HEADERS     += $$PWD/../../../src/core/ifileaccessmanager.h

HEADERS     += $$PWD/../../../src/core/idownloaditem.h

HEADERS     += $$PWD/../../../src/core/resourceitem.h
SOURCES     += $$PWD/../../../src/core/resourceitem.cpp

HEADERS     += $$PWD/../../../src/core/mask.h
SOURCES     += $$PWD/../../../src/core/mask.cpp

HEADERS     += $$PWD/../../../src/core/session.h
SOURCES     += $$PWD/../../../src/core/session.cpp

HEADERS     += $$PWD/../../../src/core/settings.h
SOURCES     += $$PWD/../../../src/core/settings.cpp

HEADERS     += $$PWD/../../../src/core/stream.h
SOURCES     += $$PWD/../../../src/core/stream.cpp

HEADERS     += $$PWD/../../../src/core/torrentcontext.h
SOURCES     += $$PWD/../../../src/core/torrentcontext.cpp

HEADERS     += $$PWD/../../../src/core/torrentcontext_p.h
SOURCES     += $$PWD/../../../src/core/torrentcontext_p.cpp

HEADERS     += $$PWD/../../../src/core/torrentmessage.h
SOURCES     += $$PWD/../../../src/core/torrentmessage.cpp
