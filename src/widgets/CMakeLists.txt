set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/advancedsettingswidget.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/systemtray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/textedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/texteditorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/torrentwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/urlformwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/urllineedit.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/advancedsettingswidget.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/systemtray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/texteditorwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/torrentwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/urlformwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/urllineedit.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/advancedsettingswidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filtertip.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/masktip.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pathwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/streamwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/texteditorwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/torrentwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/urlformwidget.ui
    )
