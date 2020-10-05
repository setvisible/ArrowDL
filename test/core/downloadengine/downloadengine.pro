#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_downloadengine
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_downloadengine.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += $$PWD/../../../include

# Resources:
HEADERS = \
    $$PWD/../../../src/core/abstractdownloaditem.h \
    $$PWD/../../../src/core/downloadengine.h \
    $$PWD/../../../src/core/idownloaditem.h \
    $$PWD/../../utils/fakedownloaditem.h

SOURCES = \
    $$PWD/../../../src/core/abstractdownloaditem.cpp \
    $$PWD/../../../src/core/downloadengine.cpp \
    $$PWD/../../utils/fakedownloaditem.cpp
