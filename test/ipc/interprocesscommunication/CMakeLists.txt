set(MY_TEST_TARGET tst_interprocesscommunication)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Test
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/checkabletablemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/core/fileutils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_SOURCE_DIR}/src/core/model.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourcemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/ipc/interprocesscommunication.cpp
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_interprocesscommunication.cpp
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
