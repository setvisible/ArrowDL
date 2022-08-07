set(MY_TEST_TARGET tst_textedit)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Test
    Widgets
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/widgets/textedit.cpp
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_textedit.cpp
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
        Qt::Widgets
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})
