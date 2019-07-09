include(${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/CMakeLists.txt)


add_library(gumbo_static_lib STATIC
    ${GUMBO_SOURCES}            
    )
