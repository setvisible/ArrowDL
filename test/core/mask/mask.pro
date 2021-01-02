#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_mask
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_mask.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/mask.h

SOURCES += \
    $$PWD/../../../src/core/mask.cpp
