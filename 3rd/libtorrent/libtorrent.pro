TEMPLATE = lib
TARGET   = libtorrent

CONFIG += staticlib
CONFIG += console
CONFIG += c++11

gcc|clang{
    QMAKE_CFLAGS += -std=c99
    QMAKE_CXXFLAGS += -std=c++11
}


#-------------------------------------------------
# Dependencies
#-------------------------------------------------
include($$PWD/../boost/boost.pri)


#-------------------------------------------------
# LIBTORRENT CONFIG
#-------------------------------------------------
include($$PWD/libtorrent.pri)


#-------------------------------------------------
# SOURCES
#-------------------------------------------------
HEADERS = $$LIBTORRENT_HEADERS
SOURCES = $$LIBTORRENT_SOURCES


#-------------------------------------------------
# BUILD PATH
#-------------------------------------------------
# Rem: On Ubuntu, directories starting with '.' are hidden by default
win32{
    MOC_DIR =      ./.moc
    OBJECTS_DIR =  ./.obj
    UI_DIR =       ./.ui
}else{
    MOC_DIR =      ./_moc
    OBJECTS_DIR =  ./_obj
    UI_DIR =       ./_ui
}


#-------------------------------------------------
# BUILD OPTIONS
#-------------------------------------------------
# Strip debug infos to reduce the obj sizes of libtorrent
# -g -O0 : 200 Mo !!!
# -O2    : 25 Mo
CONFIG(debug, debug|release) {
    # message("Strip all code of LibTorrent")
    # QMAKE_CXXFLAGS_DEBUG += -DNDEBUG

    QMAKE_CXXFLAGS_DEBUG -= -g
    QMAKE_CXXFLAGS_DEBUG += -O2
}


#-------------------------------------------------
# OUTPUT
#-------------------------------------------------
#build_pass:CONFIG(debug, debug|release) {
#    unix: TARGET = $$join(TARGET,,,_debug)
#    else: TARGET = $$join(TARGET,,,d)
#}


#-------------------------------------------------
# TARGET DIRECTORY
#-------------------------------------------------
DESTDIR = $${OUT_PWD}/../../libtorrent_install/lib


#-------------------------------------------------
# INSTALL
#-------------------------------------------------
make_install_files_to_copy.files += $$PWD/1.2.5/include/*
make_install_files_to_copy.path = $${DESTDIR}/../include
INSTALLS += make_install_files_to_copy

isEmpty(INSTALLS){
    error(INSTALLS is empty)
}


#-------------------------------------------------
# QMAKE DEBUGGING
#-------------------------------------------------
# message("*********************************************")
# message("")
# message(CONFIG: $$CONFIG)
# message("")
# message(DEFINES: $$DEFINES)
# message("")
# message(QMAKE_CFLAGS: $$QMAKE_CFLAGS)
# message(QMAKE_CFLAGS_DEBUG: $$QMAKE_CFLAGS_DEBUG)
# message(QMAKE_CFLAGS_RELEASE: $$QMAKE_CFLAGS_RELEASE)
# message("")
# message(QMAKE_CXXFLAGS: $$QMAKE_CXXFLAGS)
# message(QMAKE_CXXFLAGS_DEBUG: $$QMAKE_CXXFLAGS_DEBUG)
# message(QMAKE_CXXFLAGS_RELEASE: $$QMAKE_CXXFLAGS_RELEASE)
# message("")
# message(QMAKE_LFLAGS: $$QMAKE_LFLAGS)
# message(QMAKE_LFLAGS_DEBUG: $$QMAKE_LFLAGS_DEBUG)
# message(QMAKE_LFLAGS_RELEASE: $$QMAKE_LFLAGS_RELEASE)

