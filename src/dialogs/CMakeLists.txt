set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/adddownloaddialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/compilerdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/informationdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/preferencedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/wizarddialog.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/adddownloaddialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/compilerdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/informationdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/preferencedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/wizarddialog.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/adddownloaddialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/compilerdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/informationdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/preferencedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/wizarddialog.ui
    )
