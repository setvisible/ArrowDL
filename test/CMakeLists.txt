##############################################################################
# Tests
##############################################################################

if(ENABLE_TESTS)

    enable_testing()

    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/attribute/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/charref/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/parser/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/stringbuffer/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/stringpiece/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/tokenizer/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/utf8/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/google-gumbo-parser/vector/CMakeLists.txt)

    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/abstractsettings/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/downloadengine/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/downloadmanager/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/format/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/mask/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/regex/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/core/resourceitem/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/io/jsonhandler/CMakeLists.txt)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/io/texthandler/CMakeLists.txt)

    # Copy the dlls to run the tests
    # Or
    # Add the directory to the DLLs to the PATH environment temporarly

    ## Qt Libraries
    get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)
    set(MY_QT_ENVIRONMENT_PATH ${QT_DLL_DIR}/..)

    # set any extra environment variables to use during the execution of the script here:
    set_tests_properties(  # use TESTS from cmake 3.12.0

        # tests of Gumbo
        tst_attribute
        tst_charref
        tst_parser
        tst_stringbuffer
        tst_stringpiece
        tst_tokenizer
        tst_utf8
        tst_vector

        # tests of DownZemAll
        tst_abstractsettings
        tst_downloadengine
        tst_downloadmanager
        tst_format
        tst_mask
        tst_regex
        tst_resourceitem
        tst_jsonhandler
        tst_texthandler

        PROPERTIES ENVIRONMENT
        # The following line must escape semi-colons ";" with "\;"
        "PATH=%PATH%\;${MY_QT_ENVIRONMENT_PATH}/bin\;${MY_QT_ENVIRONMENT_PATH}/plugins"
        )

endif(ENABLE_TESTS)
