#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_engine
CONFIG      += testcase
QT           = core testlib network

SOURCES     += tst_engine.cpp

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS     += $$PWD/../../../src/core/engine.h
SOURCES     += $$PWD/../../../src/core/engine.cpp

HEADERS     += $$PWD/../../../src/core/jobclient.h
HEADERS     += $$PWD/../../../src/core/jobclient_p.h
SOURCES     += $$PWD/../../../src/core/jobclient.cpp

HEADERS     += $$PWD/../../../src/core/resourceitem.h
SOURCES     += $$PWD/../../../src/core/resourceitem.cpp

HEADERS     += $$PWD/../../../src/core/mask.h
SOURCES     += $$PWD/../../../src/core/mask.cpp
