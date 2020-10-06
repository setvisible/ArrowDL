
set(MY_TEST_TARGET tst_texthandler)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/abstractdownloaditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/ifilehandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/texthandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/utils/fakedownloaditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/utils/fakedownloadmanager.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/io/texthandler/tst_texthandler.cpp
    ${MY_TEST_SOURCES}
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test )
