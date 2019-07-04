set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/browserwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/jobview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/browserwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/jobview.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/browserwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/filterwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/linkwidget.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/maskwidget.ui
    )
