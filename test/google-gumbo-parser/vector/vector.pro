#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_vector
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_vector.cpp

# Dependencies
include($$PWD/../test_utils.pri)
include($$PWD/../../../3rd/google-gumbo-parser/google-gumbo-parser.pri)
