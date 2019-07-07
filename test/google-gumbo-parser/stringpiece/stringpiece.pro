#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_stringpiece
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_stringpiece.cpp

# Dependencies
include($$PWD/../test_utils.pri)
include($$PWD/../../../3rd/google-gumbo-parser/google-gumbo-parser.pri)
