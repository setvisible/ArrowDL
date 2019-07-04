#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_regex
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_regex.cpp

# Include:
INCLUDEPATH += ../../../include

# Resources:
HEADERS     += $$PWD/../../../src/core/regex.h
SOURCES     += $$PWD/../../../src/core/regex.cpp

