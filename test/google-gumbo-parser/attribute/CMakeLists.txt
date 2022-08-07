set(MY_TEST_TARGET tst_attribute)

find_package(GoogleGumboParser REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Test
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/test/google-gumbo-parser/test_utils.h
    ${CMAKE_SOURCE_DIR}/test/google-gumbo-parser/test_utils.cpp
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_attribute.cpp
    ${MY_TEST_SOURCES}
)

target_link_libraries(${MY_TEST_TARGET} PRIVATE
    ${GoogleGumboParser_LIBRARIES}
    Qt::Core
    Qt::Test
)

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})
