# Qt Solutions Component: Single Application
# 
# The QtSingleApplication component provides support for
# applications that can be only started once per user.
#
# https://github.com/qtproject/qt-solutions
#

INCLUDEPATH += $$PWD/include

HEADERS += \
    $$PWD/src/qtlocalpeer.h \
    $$PWD/src/qtsingleapplication.h

SOURCES += \
    $$PWD/src/qtlocalpeer.cpp \
    $$PWD/src/qtsingleapplication.cpp
