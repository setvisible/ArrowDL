
set(APP_VERSION "0.0.0")

set(MY_TEST_TARGET tst_downloadmanager)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/abstractdownloaditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/abstractsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloaditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadstreamitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadtorrentitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/fileutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/networkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrentbasecontext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrentcontext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrentcontext_p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrentmessage.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/downloadmanager/tst_downloadmanager.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# LibTorrent
include(${CMAKE_CURRENT_SOURCE_DIR}/3rd/boost/CMakeLists.txt)
# include(${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/CMakeLists.txt)

target_link_libraries(${MY_TEST_TARGET}
    libtorrent_static_lib
    )

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    target_link_libraries(${MY_TEST_TARGET}
        wsock32
        ws2_32
        Iphlpapi
        # debug
        # dbghelp
        )
else() # MacOS or Unix Compilers
    target_link_libraries(${MY_TEST_TARGET}
        Threads::Threads
        )
endif()


# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test Network )
