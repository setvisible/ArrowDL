#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_interprocesscommunication
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_interprocesscommunication.cpp

CONFIG      += c++11

# Include
INCLUDEPATH += $$PWD/../../../include/

# Resources:
HEADERS += \
    $$PWD/../../../src/core/checkabletablemodel.h \
    $$PWD/../../../src/core/fileutils.h \
    $$PWD/../../../src/core/mask.h \
    $$PWD/../../../src/core/model.h \
    $$PWD/../../../src/core/resourceitem.h \
    $$PWD/../../../src/core/resourcemodel.h \
    $$PWD/../../../src/ipc/interprocesscommunication.h

SOURCES += \
    $$PWD/../../../src/core/checkabletablemodel.cpp \
    $$PWD/../../../src/core/fileutils.cpp \
    $$PWD/../../../src/core/mask.cpp \
    $$PWD/../../../src/core/model.cpp \
    $$PWD/../../../src/core/resourceitem.cpp \
    $$PWD/../../../src/core/resourcemodel.cpp \
    $$PWD/../../../src/ipc/interprocesscommunication.cpp
