#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_utf8
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_utf8.cpp

# Dependencies
include($$PWD/../test_utils.pri)
include($$PWD/../../../3rd/google-gumbo-parser/google-gumbo-parser.pri)
