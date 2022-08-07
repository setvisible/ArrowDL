if(NOT MSVC)
    message("Test/Demo/WebEngine: MinGW is currently not supported by Chromium which is the base of QtWebEngine")

else()
    set(MY_TEST_TARGET WebEngine)

    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        WebEngine
    )

    qt_standard_project_setup()

    set(MY_TEST_SOURCES
    )

    set(MY_TEST_HEADERS
    )

    qt_add_resources(MY_TEST_SOURCES
        ${CMAKE_SOURCE_DIR}/src/resources.qrc
    )

    set(MY_TEST_RC_FILE
        ${CMAKE_SOURCE_DIR}/src/resources_win.rc
    )

    add_executable(${MY_TEST_TARGET} WIN32
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
            Qt::Widgets
            Qt::WebEngine
        )

endif()
