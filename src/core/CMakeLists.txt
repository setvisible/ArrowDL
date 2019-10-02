set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/abstractsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloaditem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/htmlparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mimedatabase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/regex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourcemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloaditem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/downloadmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourcemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings.h
    )
