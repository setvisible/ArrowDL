set(MY_TEST_TARGET StreamWidget)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Widgets
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/checkabletablemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/core/fileutils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/stream.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/checkableitemdelegate.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/checkabletableview.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/streamformatpicker.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/streamlistwidget.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/streamtoolbox.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/streamwidget.cpp
)

set(MY_TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/core/checkabletablemodel.h
    ${CMAKE_SOURCE_DIR}/src/core/fileutils.h
    ${CMAKE_SOURCE_DIR}/src/core/format.h
    ${CMAKE_SOURCE_DIR}/src/core/stream.h
    ${CMAKE_SOURCE_DIR}/src/core/theme.h
    ${CMAKE_SOURCE_DIR}/src/widgets/checkableitemdelegate.h
    ${CMAKE_SOURCE_DIR}/src/widgets/checkabletableview.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.h
    ${CMAKE_SOURCE_DIR}/src/widgets/globals.h
    ${CMAKE_SOURCE_DIR}/src/widgets/streamformatpicker.h
    ${CMAKE_SOURCE_DIR}/src/widgets/streamlistwidget.h
    ${CMAKE_SOURCE_DIR}/src/widgets/streamtoolbox.h
    ${CMAKE_SOURCE_DIR}/src/widgets/streamwidget.h
)

set(MY_TEST_FORMS
    ${CMAKE_SOURCE_DIR}/src/widgets/streamformatpicker.ui
    ${CMAKE_SOURCE_DIR}/src/widgets/streamlistwidget.ui
    ${CMAKE_SOURCE_DIR}/src/widgets/streamtoolbox.ui
    ${CMAKE_SOURCE_DIR}/src/widgets/streamwidget.ui
)

qt_add_resources(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

set(MY_TEST_RC_FILE
    ${CMAKE_SOURCE_DIR}/src/resources_win.rc
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_SOURCE_DIR}/test/utils/dummystreamfactory.h
    ${CMAKE_SOURCE_DIR}/test/utils/dummystreamfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${MY_TEST_SOURCES}
    ${MY_TEST_HEADERS}
    ${MY_TEST_FORMS}
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
        Qt::Network
        Qt::Widgets
    )
