
set(MY_TEST_TARGET tst_engine)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/jobclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/engine/tst_engine.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test Network )
