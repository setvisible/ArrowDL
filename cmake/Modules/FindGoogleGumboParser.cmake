#.rst:
# FindGoogleGumboParser
# ---------------------
#
# Try to find GoogleGumboParser.
#
# This will define the following variables:
#
# ``GoogleGumboParser_FOUND``
#       True if GoogleGumboParser is available.
#       If false, do not try to link to GoogleGumboParser
#
# ``GoogleGumboParser_VERSION``
#       The version of GoogleGumboParser
#
# ``GoogleGumboParser_INCLUDE_DIRS``
#       Where to find gumbo.h
#
# ``GoogleGumboParser_LIBRARIES``
#     The names of the libraries to link against
#
#message("<FindGoogleGumboParser.cmake>")

set(GoogleGumboParser_LIBRARIES "gumbo_static_lib")
set(GoogleGumboParser_FOUND True)
set(GoogleGumboParser_VERSION "0.10.1")

set(GoogleGumboParser_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/
)
if(MSVC)
    set(GoogleGumboParser_INCLUDE_DIRS
        ${GoogleGumboParser_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/
    )
endif()

include_directories(${GoogleGumboParser_INCLUDE_DIRS})

if(NOT TARGET ${GoogleGumboParser_LIBRARIES})

    project(${GoogleGumboParser_LIBRARIES}
        VERSION 0.10.1
        DESCRIPTION "Google Gumbo Parser"
    )

    set(GUMBO_SOURCES
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/attribute.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/char_ref.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/error.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/parser.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/string_buffer.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/string_piece.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tag.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tokenizer.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/utf8.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/util.c
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/vector.c
    )

    set(GUMBO_HEADERS
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/attribute.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/char_ref.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/error.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/gumbo.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/insertion_mode.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/parser.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/string_buffer.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/string_piece.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tag_enum.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tag_gperf.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tag_sizes.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tag_strings.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tokenizer.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/tokenizer_states.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/token_type.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/utf8.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/util.h
        ${CMAKE_SOURCE_DIR}/3rd/google-gumbo-parser/0.10.1/src/vector.h
    )

    add_library(${GoogleGumboParser_LIBRARIES} STATIC
        ${GUMBO_SOURCES}
        ${GUMBO_HEADERS} # only to see headers in IDE-generated project.
    )
endif()

#message("</FindGoogleGumboParser.cmake>")
