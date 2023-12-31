set(MY_TEST_TARGET tst_torrentcontext)

#set(APP_VERSION "0.0.0")

find_package(LibtorrentRasterbar REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Test
    Network
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/abstractsettings.cpp
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/networkmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/settings.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrent.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentbasecontext.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentcontext.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentcontext_p.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentmessage.cpp
)

set(MY_TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/core/abstractsettings.h
    ${CMAKE_SOURCE_DIR}/src/core/format.h
    ${CMAKE_SOURCE_DIR}/src/core/networkmanager.h
    ${CMAKE_SOURCE_DIR}/src/core/settings.h
    ${CMAKE_SOURCE_DIR}/src/core/torrent.h
    ${CMAKE_SOURCE_DIR}/src/core/torrentbasecontext.h
    ${CMAKE_SOURCE_DIR}/src/core/torrentcontext.h
    ${CMAKE_SOURCE_DIR}/src/core/torrentcontext_p.h
    ${CMAKE_SOURCE_DIR}/src/core/torrentmessage.h
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_torrentcontext.cpp
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
            Qt::Test
            Qt::Network
    )

else() # MacOS or Unix Compilers

    target_link_libraries(${MY_TEST_TARGET}
        PRIVATE
            ${LibtorrentRasterbar_LIBRARIES}
            Threads::Threads

            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}

            Qt::Core
            Qt::Test
            Qt::Network
    )

endif()

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# copy test files after build
add_custom_command(
    TARGET
        ${MY_TEST_TARGET}
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)
