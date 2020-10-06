
set(MY_TEST_TARGET tst_stream)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/utils/dummystreamfactory.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/stream/tst_stream.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test )
