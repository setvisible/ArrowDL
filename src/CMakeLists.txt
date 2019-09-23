set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui
    )

SET(MY_RESOURCES ${MY_RESOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources_win.rc
    )

