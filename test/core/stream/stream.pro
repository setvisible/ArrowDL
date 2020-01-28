#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_stream
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_stream.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS     += $$PWD/../../../src/core/format.h
SOURCES     += $$PWD/../../../src/core/format.cpp

HEADERS     += $$PWD/../../../src/core/stream.h
SOURCES     += $$PWD/../../../src/core/stream.cpp
