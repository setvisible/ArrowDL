#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_downloadmanager
CONFIG      += testcase
QT           = core testlib network
SOURCES     += tst_downloadmanager.cpp

CONFIG      += c++11

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

HEADERS     += $$PWD/../../../src/core/file.h
SOURCES     += $$PWD/../../../src/core/file.cpp

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
