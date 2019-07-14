#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_parser
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_parser.cpp

# Dependencies
include($$PWD/../test_utils.pri)
include($$PWD/../../../3rd/google-gumbo-parser/google-gumbo-parser.pri)
