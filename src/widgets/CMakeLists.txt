set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/autoclosedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/combobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/downloadqueueview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/autoclosedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/combobox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/downloadqueueview.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.ui
    )
