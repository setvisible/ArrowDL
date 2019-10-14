#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_texthandler
CONFIG      += testcase
QT           = core testlib


#-------------------------------------------------
# Dependencies
#-------------------------------------------------
HEADERS += \
    $$PWD/../../../src/core/abstractdownloaditem.h \
    $$PWD/../../../src/core/downloadengine.h \
    $$PWD/../../../src/core/idownloaditem.h \
    $$PWD/../../../src/io/ifilehandler.h \
    $$PWD/../../../src/io/texthandler.h

SOURCES += \
    $$PWD/../../../src/core/abstractdownloaditem.cpp \
    $$PWD/../../../src/core/downloadengine.cpp \
    $$PWD/../../../src/io/ifilehandler.cpp \
    $$PWD/../../../src/io/texthandler.cpp


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
    $$PWD/tst_texthandler.cpp
