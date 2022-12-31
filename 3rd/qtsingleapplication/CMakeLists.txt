include_directories(${CMAKE_SOURCE_DIR}/3rd/qtsingleapplication/include/)

set(QTSINGLEAPPLICATION_SOURCES
    ${CMAKE_SOURCE_DIR}/3rd/qtsingleapplication/src/qtlocalpeer.cpp
    ${CMAKE_SOURCE_DIR}/3rd/qtsingleapplication/src/qtsingleapplication.cpp
)

set(QTSINGLEAPPLICATION_HEADERS
    ${CMAKE_SOURCE_DIR}/3rd/qtsingleapplication/src/qtlocalpeer.h
    ${CMAKE_SOURCE_DIR}/3rd/qtsingleapplication/src/qtsingleapplication.h
)
