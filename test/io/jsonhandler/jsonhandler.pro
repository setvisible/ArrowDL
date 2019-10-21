#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_jsonhandler
CONFIG      += testcase
QT           = core testlib

CONFIG      += c++11

#-------------------------------------------------
# Dependencies
#-------------------------------------------------
HEADERS += \
    $$PWD/../../../src/core/abstractdownloaditem.h \
    $$PWD/../../../src/core/downloadengine.h \
    $$PWD/../../../src/core/idownloaditem.h \
    $$PWD/../../../src/io/ifilehandler.h \
    $$PWD/../../../src/io/jsonhandler.h

SOURCES += \
    $$PWD/../../../src/core/abstractdownloaditem.cpp \
    $$PWD/../../../src/core/downloadengine.cpp \
    $$PWD/../../../src/io/ifilehandler.cpp \
    $$PWD/../../../src/io/jsonhandler.cpp


#-------------------------------------------------
# Include
#-------------------------------------------------
INCLUDEPATH += $$PWD/../../../include/

#-------------------------------------------------
# Test Resources
#-------------------------------------------------
HEADERS += \
    $$PWD/../../utils/fakedownloaditem.h \
    $$PWD/../../utils/fakedownloadmanager.h

SOURCES += \
    $$PWD/../../utils/fakedownloaditem.cpp \
    $$PWD/../../utils/fakedownloadmanager.cpp \
    $$PWD/tst_jsonhandler.cpp
