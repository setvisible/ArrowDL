# CMakeLists.txt
#
# Copyright (C) 2019 Sebastien Vavassori
#
# This code is released under the LGPL license v3.0.
# For conditions of distribution and use, see the disclaimer
# and license in LICENSE

cmake_minimum_required(VERSION 3.1.0)

project(DownZemAll)

if(MSVC)
    # No flags
else(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)


##############################################################################
## Versioning
##############################################################################

# read 'version' file into a variable (stripping any newlines or spaces)
file(READ version versionFile)
if (NOT versionFile)
    message(FATAL_ERROR "Unable to determine DownZemAll version. Version file is missing.")
endif()
string(STRIP "${versionFile}" DOWNZEMALL_VERSION)
# add a dependency on the versino file
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS version)

# split version string into components, note CMAKE_MATCH_0 is the entire regexp match
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" APP_VERSION ${DOWNZEMALL_VERSION} )
set(APP_VERSION_MAJOR ${CMAKE_MATCH_1})
set(APP_VERSION_MINOR ${CMAKE_MATCH_2})
set(APP_VERSION_PATCH ${CMAKE_MATCH_3})


configure_file( ./src/config.h.cmake ./src/config.h )


##############################################################################
# Rename the executable for DEBUG and RELEASE
##############################################################################

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    if(MSYS OR MINGW)
        # Change the default build type to Release
        if(NOT CMAKE_BUILD_TYPE)
            set(CMAKE_BUILD_TYPE Release CACHE STRING
                "Choose the type of build, options are: Debug Release." FORCE)
            set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
        endif(NOT CMAKE_BUILD_TYPE)

        # to distinguish between debug and release
        set(CMAKE_DEBUG_POSTFIX "d")

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            set(RELEASE_MODE 1)
        else()
            set(RELEASE_MODE 0)
        endif()

    else() # MSVC Compiler
        #
        # CMAKE_BUILD_TYPE and CMAKE_CONFIGURATION_TYPES obviously only
        # apply for Unix makefiles, not for Visual projects.
        #
        message(STATUS "To build a release with MSVC, there's no command in CMAKE. So use cmake --build . --config Release")
        set(RELEASE_MODE 1)

    endif()

else() # MacOS or Unix Compilers

    # to do

endif()


# Rename application
if(RELEASE_MODE)
    set(DownZemAll_NAME "DownZemAll")
else(RELEASE_MODE)
    set(DownZemAll_NAME "DownZemAlld")
endif(RELEASE_MODE)


##############################################################################
# build options
##############################################################################
option(ENABLE_TESTS "Set to ON to build test applications (default)" ON)

if(MSVC)
    option(USE_QT_WEBENGINE "Set to ON to use Qt WebEngine (default)" ON)
endif()

##############################################################################
# definitions
##############################################################################
# Some build settings that depend on whether we want a release or a
# debug version
if(RELEASE_MODE)
    # Make sure that debugging is disabled in Qt
    add_definitions(-DQT_NO_DEBUG)
else(RELEASE_MODE)
    # Make sure that debugging is enabled in Qt
    add_definitions(-DQT_DEBUG)
endif(RELEASE_MODE)

# Ask for Qt deprecated uses to be reported
add_definitions(-DQT_DEPRECATED_WARNINGS)

# Make sure that Unicode is defined
# Note: at least needed for QtSingleApplication on Windows...
add_definitions(-DUNICODE)


if(USE_QT_WEBENGINE)
    add_definitions(-DUSE_QT_WEBENGINE)
endif(USE_QT_WEBENGINE)

##############################################################################
## Qt5
##############################################################################

set(CMAKE_AUTOMOC ON)   # To automatically run MOC when building (Meta Object Compiler)
set(CMAKE_AUTORCC ON)   # To automatically run RCC when building (Resource Compiler)
set(CMAKE_AUTOUIC ON)   # To automatically run UIC when building (User Interface Compiler)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
if(USE_QT_WEBENGINE)
    find_package(Qt5 COMPONENTS WebEngineWidgets REQUIRED)
endif(USE_QT_WEBENGINE)

if(ENABLE_TESTS)
    find_package(Qt5 COMPONENTS Test REQUIRED)
endif(ENABLE_TESTS)


##############################################################################
## 3RD-PARTY DEPENDENCIES
##############################################################################
include(${CMAKE_CURRENT_SOURCE_DIR}/3rd/CMakeLists.txt)


##############################################################################
### Include
##############################################################################
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(./include/)


##############################################################################
### Sources
##############################################################################
include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/core/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/io/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/CMakeLists.txt)


##############################################################################
# Linking the executable
##############################################################################
add_executable(${DownZemAll_NAME}
    WIN32           # build a Win32 application, not a console application
    ${MY_FORMS}
    ${MY_SOURCES}
    ${QTSINGLEAPPLICATION_SOURCES}
    ${MY_RESOURCES}
    )

# Gumbo
target_link_libraries(${DownZemAll_NAME}
    gumbo_static_lib
    )

# Windows only: Version.dll
if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    # might be "C:/Windows/System32/version.dll" or "C:/Windows/SysWOW64/version.dll"
    target_link_libraries(${DownZemAll_NAME}
        version.dll
        )
endif()

# Qt5
target_link_libraries(${DownZemAll_NAME} Qt5::Core)
target_link_libraries(${DownZemAll_NAME} Qt5::Gui)
target_link_libraries(${DownZemAll_NAME} Qt5::Widgets)
target_link_libraries(${DownZemAll_NAME} Qt5::Network)
if(USE_QT_WEBENGINE)
    target_link_libraries(${DownZemAll_NAME} Qt5::WebEngineWidgets)
endif(USE_QT_WEBENGINE)


##############################################################################
# Unit Tests
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


##############################################################################
## Deploying documentation and files
##############################################################################
set(MY_RELEASE_DOCS
    ./LICENSE
    ./TROUBLESHOOTING.md
    )
install (
    FILES ${MY_RELEASE_DOCS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT release_docs
    )

## 3rd-party libraries
if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    # ***********************************************
    # windeployqt is not used here in order         *
    # to control the most minimalist deployment     *
    # ***********************************************

    ## Qt Libraries
    get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)

    if(MSYS OR MINGW)
        # Hack to find libGCC
        #
        # Mingw libGCC comes with 3 flavors of exception handling
        # - DW2     Dwarf-2 Exception Handling      (32-bit only)
        # - SJLJ    SetJump LongJump                (both 32-bit & 64-bit)
        # - SEH     Structured Exception Handling   (64-bit)
        #
        if (EXISTS ${QT_DLL_DIR}/libgcc_s_dw2-1.dll)
            set(libgcc "${QT_DLL_DIR}/libgcc_s_dw2-1.dll")
        else()
            if (EXISTS ${QT_DLL_DIR}/libgcc_s_seh-1.dll)
                set(libgcc "${QT_DLL_DIR}/libgcc_s_seh-1.dll")
            else()
                message(FATAL_ERROR "Can't find LibGCC in path ${QT_DLL_DIR}")
            endif()
        endif()

        install(FILES
            ${libgcc}
            ${QT_DLL_DIR}/libstdc++-6.dll
            ${QT_DLL_DIR}/libwinpthread-1.dll
            ${QT_DLL_DIR}/Qt5Core.dll
            ${QT_DLL_DIR}/Qt5Gui.dll
            ${QT_DLL_DIR}/Qt5Widgets.dll
            ${QT_DLL_DIR}/Qt5Network.dll
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            )

    else() # MSVC
        install(FILES
            ${QT_DLL_DIR}/Qt5Core.dll
            ${QT_DLL_DIR}/Qt5Gui.dll
            ${QT_DLL_DIR}/Qt5Widgets.dll
            ${QT_DLL_DIR}/Qt5Network.dll
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            )

        if(USE_QT_WEBENGINE)
            install(FILES
                ${QT_DLL_DIR}/Qt5WebEngineCore.dll
                ${QT_DLL_DIR}/Qt5WebEngineWidgets.dll
                ${QT_DLL_DIR}/Qt5Positioning.dll
                ${QT_DLL_DIR}/Qt5PrintSupport.dll
                ${QT_DLL_DIR}/Qt5Qml.dll
                ${QT_DLL_DIR}/Qt5Quick.dll
                ${QT_DLL_DIR}/Qt5QuickWidgets.dll
                ${QT_DLL_DIR}/Qt5WebChannel.dll
                DESTINATION ${CMAKE_INSTALL_PREFIX}
                )
        endif(USE_QT_WEBENGINE)

    endif()

    ## Qt Platform Plugin
    install(FILES
        ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}/platforms
        )

    ## OpenSSL
    if (CMAKE_SIZEOF_VOID_P EQUAL 4) # 32-bit
        install(FILES
            ./3rd/openssl/v1.1.1/windows_x86_32bits/libcrypto-1_1.dll
            ./3rd/openssl/v1.1.1/windows_x86_32bits/libssl-1_1.dll
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            )

    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit
        install(FILES
            ./3rd/openssl/v1.1.1/windows_x86_64bits/libcrypto-1_1-x64.dll
            ./3rd/openssl/v1.1.1/windows_x86_64bits/libssl-1_1-x64.dll
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            )
    endif()

endif(MSVC OR MSYS OR MINGW)


##############################################################################
# Deploying executable
##############################################################################
install(TARGETS ${DownZemAll_NAME}
    RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
