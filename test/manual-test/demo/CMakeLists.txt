set(MY_TEST_TARGET Demo)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/abstractjob.cpp
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mimedatabase.cpp
    ${CMAKE_SOURCE_DIR}/src/core/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/queueview.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/queueviewitemdelegate.cpp
)

set(MY_TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/core/abstractjob.h
    ${CMAKE_SOURCE_DIR}/src/core/format.h
    ${CMAKE_SOURCE_DIR}/src/core/mimedatabase.h
    ${CMAKE_SOURCE_DIR}/src/core/scheduler.h
    ${CMAKE_SOURCE_DIR}/src/core/theme.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.h
    ${CMAKE_SOURCE_DIR}/src/widgets/queueview.h
    ${CMAKE_SOURCE_DIR}/src/widgets/queueviewitemdelegate.h
    ${CMAKE_SOURCE_DIR}/src/widgets/globals.h
)

qt_add_resources(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

set(MY_TEST_RC_FILE
    ${CMAKE_SOURCE_DIR}/src/resources_win.rc
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_SOURCE_DIR}/test/utils/fakejob.h
    ${CMAKE_SOURCE_DIR}/test/utils/fakescheduler.h
    ${CMAKE_SOURCE_DIR}/test/utils/fakejob.cpp
    ${CMAKE_SOURCE_DIR}/test/utils/fakescheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${MY_TEST_SOURCES}
    ${MY_TEST_HEADERS}
    ${MY_TEST_RC_FILE}
)

target_include_directories(${MY_TEST_TARGET}
    PRIVATE
        ${Project_INCLUDE_DIRS}
)

target_link_libraries(${MY_TEST_TARGET}
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
    )
