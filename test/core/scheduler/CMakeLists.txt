set(MY_TEST_TARGET tst_scheduler)

set(APP_VERSION "0.0.0")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Test
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/abstractjob.cpp
    ${CMAKE_SOURCE_DIR}/src/core/abstractsettings.cpp
    ${CMAKE_SOURCE_DIR}/src/core/file.cpp
    ${CMAKE_SOURCE_DIR}/src/core/fileaccessmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/fileutils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/jobfile.cpp
    ${CMAKE_SOURCE_DIR}/src/core/jobstream.cpp
    ${CMAKE_SOURCE_DIR}/src/core/jobtorrent.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_SOURCE_DIR}/src/core/networkmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/queuemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/core/session.cpp
    ${CMAKE_SOURCE_DIR}/src/core/settings.cpp
    ${CMAKE_SOURCE_DIR}/src/core/snapshot.cpp
    ${CMAKE_SOURCE_DIR}/src/core/stream.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrent.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentcontext.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentmessage.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/workerthread.cpp

    ${CMAKE_SOURCE_DIR}/test/utils/fakejob.cpp
)

set(MY_TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/core/abstractjob.h
    ${CMAKE_SOURCE_DIR}/src/core/abstractsettings.h
    ${CMAKE_SOURCE_DIR}/src/core/file.h
    ${CMAKE_SOURCE_DIR}/src/core/fileaccessmanager.h
    ${CMAKE_SOURCE_DIR}/src/core/fileutils.h
    ${CMAKE_SOURCE_DIR}/src/core/format.h
    ${CMAKE_SOURCE_DIR}/src/core/jobfile.h
    ${CMAKE_SOURCE_DIR}/src/core/jobstream.h
    ${CMAKE_SOURCE_DIR}/src/core/jobtorrent.h
    ${CMAKE_SOURCE_DIR}/src/core/mask.h
    ${CMAKE_SOURCE_DIR}/src/core/networkmanager.h
    ${CMAKE_SOURCE_DIR}/src/core/queuemodel.h
    ${CMAKE_SOURCE_DIR}/src/core/resourceitem.h
    ${CMAKE_SOURCE_DIR}/src/core/scheduler.h
    ${CMAKE_SOURCE_DIR}/src/core/session.h
    ${CMAKE_SOURCE_DIR}/src/core/settings.h
    ${CMAKE_SOURCE_DIR}/src/core/snapshot.h
    ${CMAKE_SOURCE_DIR}/src/core/stream.h
    ${CMAKE_SOURCE_DIR}/src/torrent/torrent.h
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentcontext.h
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentmessage.h
    ${CMAKE_SOURCE_DIR}/src/torrent/utils.h
    ${CMAKE_SOURCE_DIR}/src/torrent/workerthread.h
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_scheduler.cpp
    ${MY_TEST_SOURCES}
    ${MY_TEST_HEADERS} # only to see headers in IDE-generated project.
)

target_include_directories(${MY_TEST_TARGET}
    PRIVATE
        ${Boost_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIRS}
        ${LibtorrentRasterbar_INCLUDE_DIRS}
        ${Project_INCLUDE_DIRS}
    )

target_compile_definitions(${MY_TEST_TARGET}
    PRIVATE
        WIN32_LEAN_AND_MEAN # prevent winsock1 to be included
    )

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    target_link_libraries(${MY_TEST_TARGET}
        PRIVATE
            ${LibtorrentRasterbar_LIBRARIES}
            wsock32
            ws2_32
            Iphlpapi
            # debug
            # dbghelp

            crypt32  # required by openssl
            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}

            Qt::Core
            Qt::Widgets
            Qt::Network
            Qt::Test
    )

else() # MacOS or Unix Compilers

    target_link_libraries(${MY_TEST_TARGET}
        PRIVATE
            ${LibtorrentRasterbar_LIBRARIES}
            Threads::Threads

            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}

            Qt::Core
            Qt::Widgets
            Qt::Network
            Qt::Test
    )

endif()

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})
