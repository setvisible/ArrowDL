#-------------------------------------------------
# libtorrent - Config
#-------------------------------------------------
# source: libtorrent-1_2_5/CMakeLists.txt

#message(STATUS "Compiler default is C++11")

VER_MAJOR = 1  # from version.hpp
VER_MINOR = 2
VER_TINY  = 5

SOVERSION = "10"

DEFINES += NDEBUG                   # eliminates debug info
DEFINES += TORRENT_DISABLE_LOGGING  # eliminates logging alerts

# build libtorrent as a static (not shared) library
DEFINES += TORRENT_BUILDING_SHARED # tagged with __declspec(dllexport)
DEFINES += TORRENT_LINKING_SHARED # tagged with __declspec(dllimport)

#CONFIG(debug, debug|release) {
#    DEFINES += TORRENT_USE_ASSERTS
#}
DEFINES += BOOST_ASIO_ENABLE_CANCELIO # enable cancel() in asio on windows

DEFINES += TORRENT_BUILDING_LIBRARY
DEFINES += _FILE_OFFSET_BITS=64 # boost/config.hpp
DEFINES += BOOST_EXCEPTION_DISABLE
DEFINES += BOOST_ASIO_HAS_STD_CHRONO

DEFINES += TORRENT_NO_DEPRECATE # eliminates a little bit of code


win32{
    LIBS += -lwsock32 -lws2_32 -lIphlpapi
    # LIBS += -ldebug -ldbghelp
    DEFINES += _WIN32_WINNT=0x0600  # target Windows Vista or later
    DEFINES += WIN32_LEAN_AND_MEAN  # prevent winsock1 to be included
}
unix{
    # todo check that dep
    # QMAKE_CFLAGS += -pthread
    # QMAKE_CXXFLAGS += -pthread
    # LIBS += -pthread
}

win32-msvc* {
    DEFINES += BOOST_ALL_NO_LIB
    DEFINES += _SCL_SECURE_NO_DEPRECATE _CRT_SECURE_NO_DEPRECATE # disable bogus deprecation warnings on msvc8

    #Note w.r.t. flags:
    #  /Zc:wchar_t /Zc:forScope   # make the compiler standard conforming
    #  /MP                        # for multi-core compilation
    #  /bigobj                    # increase the number of sections for obj files

}

#-------------------------------------------------
# PATCH
# Avoid "File too big, too many sections" compiling
# error with Qmake and MinGW-w64 on DEBUG mode:
#
# https://bugreports.qt.io/browse/QTBUG-48902
#
if(win32-g++*:if(CONFIG(debug, debug|release))) {
    QMAKE_CXXFLAGS += -Wa,-mbig-obj
}
#-------------------------------------------------


#message("QMAKE_CFLAGS   ==" $${QMAKE_CFLAGS})
#message("QMAKE_CXXFLAGS ==" $${QMAKE_CXXFLAGS})

#win32-msvc* {
#    MSVC_VER = $$(VisualStudioVersion)
#    equals(MSVC_VER, 14.0){
#        message("msvc 2015")
#    }
#    equals(MSVC_VER, 15.0){
#        message("msvc 2017")
#    }
#    INCLUDEPATH += $$PWD/0.10.1/visualc/include
#}

