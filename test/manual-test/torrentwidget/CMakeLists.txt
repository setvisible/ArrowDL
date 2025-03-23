set(MY_TEST_TARGET TorrentWidget)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Widgets
)

qt_standard_project_setup()

set(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrent.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentmessage.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentpiecemap.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentprogressbar.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentwidget.cpp
)

set(MY_TEST_HEADERS
    ${CMAKE_SOURCE_DIR}/src/core/format.h
    ${CMAKE_SOURCE_DIR}/src/core/theme.h
    ${CMAKE_SOURCE_DIR}/src/torrent/itorrentcontext.h
    ${CMAKE_SOURCE_DIR}/src/torrent/torrent.h
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentmessage.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyle.h
    ${CMAKE_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.h
    ${CMAKE_SOURCE_DIR}/src/widgets/globals.h
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentpiecemap.h
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentprogressbar.h
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentwidget.h
)

set(MY_TEST_FORMS
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentpiecemap.ui
    ${CMAKE_SOURCE_DIR}/src/widgets/torrentwidget.ui
)

qt_add_resources(MY_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

set(MY_TEST_RC_FILE
    ${CMAKE_SOURCE_DIR}/src/resources_win.rc
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_SOURCE_DIR}/test/utils/dummytorrentanimator.h
    ${CMAKE_SOURCE_DIR}/test/utils/dummytorrentanimator.cpp
    ${CMAKE_SOURCE_DIR}/test/utils/dummytorrentfactory.h
    ${CMAKE_SOURCE_DIR}/test/utils/dummytorrentfactory.cpp
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
