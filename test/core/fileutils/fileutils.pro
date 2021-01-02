#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_fileutils
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_fileutils.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/fileutils.h

SOURCES += \
    $$PWD/../../../src/core/fileutils.cpp
