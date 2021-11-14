#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_stream
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_stream.cpp

CONFIG      += c++11

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS += \
    $$PWD/../../../src/core/fileutils.h \
    $$PWD/../../../src/core/format.h \
    $$PWD/../../../src/core/stream.h \
    $$PWD/../../../test/utils/biginteger.h \
    $$PWD/../../../test/utils/dummystreamfactory.h

SOURCES += \
    $$PWD/../../../src/core/fileutils.cpp \
    $$PWD/../../../src/core/format.cpp \
    $$PWD/../../../src/core/stream.cpp \
    $$PWD/../../../test/utils/dummystreamfactory.cpp
