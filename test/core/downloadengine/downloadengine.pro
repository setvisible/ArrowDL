#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_downloadengine
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_downloadengine.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += $$PWD/../../../include

# Test Resources:
HEADERS     += $$PWD/../../utils/fakedownloaditem.h
SOURCES     += $$PWD/../../utils/fakedownloaditem.cpp

# Resources:
HEADERS     += $$PWD/../../../src/core/idownloaditem.h

HEADERS     += $$PWD/../../../src/core/abstractdownloaditem.h
SOURCES     += $$PWD/../../../src/core/abstractdownloaditem.cpp

HEADERS     += $$PWD/../../../src/core/downloadengine.h
SOURCES     += $$PWD/../../../src/core/downloadengine.cpp

