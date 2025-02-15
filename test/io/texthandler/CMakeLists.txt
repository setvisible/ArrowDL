set(MY_TEST_TARGET tst_texthandler)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Test
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/abstractdownloaditem.cpp
    ${CMAKE_SOURCE_DIR}/src/io/ifilehandler.cpp
    ${CMAKE_SOURCE_DIR}/src/io/texthandler.cpp
    ${CMAKE_SOURCE_DIR}/test/io/utils/dummydownloaditem.cpp
    ${CMAKE_SOURCE_DIR}/test/io/utils/dummydownloadmanager.cpp
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_texthandler.cpp
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
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})
