include(${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/CMakeLists.txt)

# Add Gumbo sources and headers to the application sources and headers
set(MY_SOURCES ${MY_SOURCES}
    ${GUMBO_SOURCES}
    )

set(MY_HEADERS ${MY_HEADERS}
    ${GUMBO_HEADERS}
    )
