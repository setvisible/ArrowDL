
set(MY_TEST_TARGET tst_downloadmanager)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/abstractsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloaditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/core/downloadmanager/tst_downloadmanager.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test Network )
