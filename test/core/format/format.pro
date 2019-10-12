#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_format
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_format.cpp

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS     += $$PWD/../../../src/core/format.h
SOURCES     += $$PWD/../../../src/core/format.cpp

