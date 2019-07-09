
set(MY_TEST_TARGET tst_tokenizer)

set(MY_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/test_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/test_utils.cpp
    )

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/tokenizer/tst_tokenizer.cpp
    ${MY_TEST_SOURCES}
    )

target_link_libraries(${MY_TEST_TARGET}
    gumbo_static_lib
    )

add_test(NAME ${MY_TEST_TARGET} COMMAND ${MY_TEST_TARGET})

# Qt
qt5_use_modules(${MY_TEST_TARGET} Core Test )
