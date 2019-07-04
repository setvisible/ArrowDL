include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/)

set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/attribute.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/char_ref.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/string_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/string_piece.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tag.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tokenizer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/utf8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/vector.c
    )

set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/attribute.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/char_ref.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/gumbo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/insertion_mode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/string_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/string_piece.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tag_enum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tag_gperf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tag_sizes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tag_strings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tokenizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/tokenizer_states.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/token_type.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/utf8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/google-gumbo-parser/gumbo-0.10.1/src/vector.h
    )
