#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_format
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_format.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/format.h \
    $$PWD/../../../test/utils/biginteger.h

SOURCES += \
    $$PWD/../../../src/core/format.cpp
