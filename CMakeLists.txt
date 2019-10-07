# CMakeLists.txt
#
# Copyright (C) 2019 Sebastien Vavassori
#
# This code is released under the LGPL license v3.0.
# For conditions of distribution and use, see the disclaimer
# and license in LICENSE

cmake_minimum_required(VERSION 2.8)
#cmake_policy(VERSION 2.8)

project(DownZemAll)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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

set(DownZemAll_NAME "DownZemAll")

# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# to distinguish between debug and release
set(CMAKE_DEBUG_POSTFIX "d")



##############################################################################
# build options
##############################################################################
#TODO: enable_testing()
#option(ENABLE_TESTS   "Set to ON to build test applications (default)" OFF)
option(ENABLE_TESTS   "Set to ON to build test applications (default)" ON)


##############################################################################
# definitions
##############################################################################
# Some build settings that depend on whether we want a release or a
# debug version
if(RELEASE_MODE)
    # Make sure that debugging is disabled in Qt
    add_definitions(-DQT_NO_DEBUG)
else()
    # Make sure that debugging is enabled in Qt
    add_definitions(-DQT_DEBUG)
endif()

# Ask for Qt deprecated uses to be reported
add_definitions(-DQT_DEPRECATED_WARNINGS)

# Make sure that Unicode is defined
# Note: at least needed for QtSingleApplication on Windows...
add_definitions(-DUNICODE)


##############################################################################
## Qt5
##############################################################################
# https://cmake.org/cmake/help/v3.0/manual/cmake-qt.7.html
# find_package(Qt5Core    REQUIRED)
# find_package(Qt5Gui     REQUIRED)
# find_package(Qt5Network REQUIRED)
# find_package(Qt5Widgets REQUIRED)

find_package(Qt5 5.1 COMPONENTS Core)
find_package(Qt5 5.1 COMPONENTS Gui)
find_package(Qt5 5.1 COMPONENTS Widgets)
find_package(Qt5 5.1 COMPONENTS Network)
if(ENABLE_TESTS)
    find_package(Qt5 5.1 COMPONENTS Test)
endif(ENABLE_TESTS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


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
include(${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/CMakeLists.txt)



##############################################################################
# Qt -- Run the MOC, resource, etc.
##############################################################################
qt5_wrap_cpp(downzemall_HEADERS_MOC    
    ${MY_HEADERS}
    ${QTSINGLEAPPLICATION_HEADERS}
    )
qt5_wrap_ui(downzemall_FORMS_HEADERS
    ${MY_FORMS}
    )

add_library(configwin
    ${downzemall_HEADERS_MOC}
    ${downzemall_FORMS_HEADERS}
    )

qt5_use_modules(configwin Widgets Network)

##############################################################################
# Linking the executable
##############################################################################
add_executable(${DownZemAll_NAME}
    WIN32
    ${MY_SOURCES}
    ${QTSINGLEAPPLICATION_SOURCES}
    ${configwin}
    ${MY_RESOURCES}
    )

# Gumbo
target_link_libraries(${DownZemAll_NAME}
    gumbo_static_lib
    )

# Qt5
qt5_use_modules(${DownZemAll_NAME} Core Gui Widgets Network)

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
# TODO include(${CMAKE_CURRENT_SOURCE_DIR}/test/widgets/pathwidget/CMakeLists.txt)

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
# TODO  tst_pathwidget

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
if (MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    ## Qt Libraries
    get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)

    install(FILES
        ${QT_DLL_DIR}/libgcc_s_dw2-1.dll
        ${QT_DLL_DIR}/libstdc++-6.dll
        ${QT_DLL_DIR}/libwinpthread-1.dll
        ${QT_DLL_DIR}/Qt5Core.dll
        ${QT_DLL_DIR}/Qt5Gui.dll
        ${QT_DLL_DIR}/Qt5Widgets.dll
        ${QT_DLL_DIR}/Qt5Network.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

    ## Qt Platform Plugin
    install(FILES
        ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}/platforms
    )

    ## OpenSSL
    install(FILES
        ./3rd/openssl/v1.0.2/windows_x86_32bits/libeay32.dll
        ./3rd/openssl/v1.0.2/windows_x86_32bits/ssleay32.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

endif(MSVC OR MSYS OR MINGW)


##############################################################################
# Deploying executable
##############################################################################
install(TARGETS ${DownZemAll_NAME}
        RUNTIME
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
