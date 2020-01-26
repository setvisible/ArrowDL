set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/autoclosedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/combobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/customstyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/customstyleoptionprogressbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/downloadqueueview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filtertip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/masktip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/streamwidget.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/autoclosedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/combobox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/downloadqueueview.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filtertip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/masktip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/streamwidget.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filtertip.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/masktip.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/streamwidget.ui
    )
