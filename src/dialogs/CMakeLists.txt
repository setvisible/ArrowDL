set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addbatchdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addcontentdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addstreamdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addtorrentdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addurlsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/batchrenamedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/compilerdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/editiondialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/homedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/informationdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/preferencedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/streamdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/tutorialdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/updatedialog.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addbatchdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addcontentdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addstreamdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addtorrentdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addurlsdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/batchrenamedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/compilerdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/editiondialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/homedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/informationdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/preferencedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/streamdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/tutorialdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/updatedialog.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addbatchdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addcontentdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addstreamdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addtorrentdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/addurlsdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/batchrenamedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/compilerdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/editiondialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/homedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/informationdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/preferencedialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/streamdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/tutorialdialog.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/updatedialog.ui
    )
