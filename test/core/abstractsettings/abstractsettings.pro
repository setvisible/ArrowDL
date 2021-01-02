#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_abstractsettings
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_abstractsettings.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/abstractsettings.h

SOURCES += \
    $$PWD/../../../src/core/abstractsettings.cpp
