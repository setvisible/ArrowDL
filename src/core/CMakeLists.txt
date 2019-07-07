set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/htmlparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/jobclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/jobmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/regex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourcemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/engine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/htmlparser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/jobclient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/jobclient_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/jobmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/mask.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/regex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourceitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resourcemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/settings.h
    )
