include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/qtsingleapplication/include/)

set(QTSINGLEAPPLICATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/qtsingleapplication/src/qtlocalpeer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/qtsingleapplication/src/qtsingleapplication.cpp
    )

set(QTSINGLEAPPLICATION_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/qtsingleapplication/src/qtlocalpeer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/qtsingleapplication/src/qtsingleapplication.h
    )
