
set(MY_TEST_TARGET tst_torrentbasecontext)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrentbasecontext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/torrentmessage.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/torrentbasecontext/tst_torrentbasecontext.cpp
    ${MY_TEST_SOURCES}
    )


add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test Network )
