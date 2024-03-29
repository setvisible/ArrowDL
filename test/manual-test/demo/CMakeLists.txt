set(MY_TEST_TARGET Demo)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/abstractdownloaditem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/downloadengine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mimedatabase.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/downloadqueueview.cpp
)

set(MY_TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/core/abstractdownloaditem.h
    ${CMAKE_SOURCE_DIR}/src/core/downloadengine.h
    ${CMAKE_SOURCE_DIR}/src/core/format.h
    ${CMAKE_SOURCE_DIR}/src/core/idownloaditem.h
    ${CMAKE_SOURCE_DIR}/src/core/mimedatabase.h
    ${CMAKE_SOURCE_DIR}/src/core/theme.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.h
    ${CMAKE_SOURCE_DIR}/src/widgets/downloadqueueview.h
    ${CMAKE_SOURCE_DIR}/src/widgets/downloadqueueview_p.h
    ${CMAKE_SOURCE_DIR}/src/widgets/globals.h
)

qt_add_resources(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

set(MY_TEST_RC_FILE
    ${CMAKE_SOURCE_DIR}/src/resources_win.rc
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_SOURCE_DIR}/test/utils/fakedownloaditem.h
    ${CMAKE_SOURCE_DIR}/test/utils/fakedownloadmanager.h
    ${CMAKE_SOURCE_DIR}/test/utils/fakedownloaditem.cpp
    ${CMAKE_SOURCE_DIR}/test/utils/fakedownloadmanager.cpp
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
