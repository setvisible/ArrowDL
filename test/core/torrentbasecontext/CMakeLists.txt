set(MY_TEST_TARGET tst_torrentbasecontext)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Test
    Network
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrent.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentbasecontext.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentmessage.cpp
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_torrentbasecontext.cpp
    ${MY_TEST_SOURCES}
)

target_include_directories(${MY_TEST_TARGET}
    PRIVATE
        ${Project_INCLUDE_DIRS}
    )

target_link_libraries(${MY_TEST_TARGET}
    PRIVATE
        Qt::Core
        Qt::Test
        Qt::Network
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})
