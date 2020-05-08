#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_interprocesscommunication
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_interprocesscommunication.cpp

CONFIG      += c++11

# Include
INCLUDEPATH += $$PWD/../../../include/

# Resources:
HEADERS     += $$PWD/../../../src/core/mask.h
SOURCES     += $$PWD/../../../src/core/mask.cpp

HEADERS     += $$PWD/../../../src/core/model.h
SOURCES     += $$PWD/../../../src/core/model.cpp

HEADERS     += $$PWD/../../../src/core/resourceitem.h
SOURCES     += $$PWD/../../../src/core/resourceitem.cpp

HEADERS     += $$PWD/../../../src/core/resourcemodel.h
SOURCES     += $$PWD/../../../src/core/resourcemodel.cpp

HEADERS     += $$PWD/../../../src/ipc/interprocesscommunication.h
SOURCES     += $$PWD/../../../src/ipc/interprocesscommunication.cpp
