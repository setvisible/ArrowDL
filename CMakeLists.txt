# CMakeLists.txt
#
# Copyright (C) 2019-present Sebastien Vavassori
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
# add a dependency on the version file
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

if(MSVC) # MSVC Compiler
    # CMAKE_BUILD_TYPE and CMAKE_CONFIGURATION_TYPES only work with Unix makefiles, not with Visual projects.
    message(STATUS "To build a release with MSVC, there's no command in CMAKE. So use cmake --build . --config Release")
    set(RELEASE_MODE 1)

else() # Other Windows compilers (MSYS, MINGW) and MacOS or Unix Compilers

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

endif()


##############################################################################
# build options
##############################################################################
option(ENABLE_APPLICATION "Set to ON to build the Application (default)" ON)
option(ENABLE_LAUNCHER "Set to ON to build the Launcher (default)" ON)

option(ENABLE_TESTS "Set to ON to build test applications (default)" ON)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    option(USE_QT_WINEXTRAS "Set to ON to use Qt WinExtras (default)" ON)
endif()

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

if(USE_QT_WINEXTRAS)
    add_definitions(-DUSE_QT_WINEXTRAS)
endif(USE_QT_WINEXTRAS)

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
if(USE_QT_WINEXTRAS)
    find_package(Qt5 COMPONENTS WinExtras REQUIRED)
endif(USE_QT_WINEXTRAS)
if(USE_QT_WEBENGINE)
    find_package(Qt5 COMPONENTS WebEngineWidgets REQUIRED)
endif(USE_QT_WEBENGINE)

if(ENABLE_TESTS)
    find_package(Qt5 COMPONENTS Test REQUIRED)
endif(ENABLE_TESTS)


##############################################################################
## 3RD-PARTY DEPENDENCIES
##############################################################################
if(ENABLE_APPLICATION OR ENABLE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/3rd/CMakeLists.txt)
    configure_file( ./src/config_3rd.h.cmake ./src/config_3rd.h )
endif(ENABLE_APPLICATION OR ENABLE_TESTS)


##############################################################################
### Include
##############################################################################
if(ENABLE_APPLICATION OR ENABLE_TESTS)

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    include_directories(./include/)

endif(ENABLE_APPLICATION OR ENABLE_TESTS)


##############################################################################
# Building
##############################################################################
if(ENABLE_APPLICATION)
    include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)
endif(ENABLE_APPLICATION)

if(ENABLE_LAUNCHER)
    include(${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/CMakeLists.txt)
endif(ENABLE_LAUNCHER)


##############################################################################
# Unit Tests
##############################################################################
if(ENABLE_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt)
endif(ENABLE_TESTS)


##############################################################################
## Deploying documentation and files
##############################################################################
set(MY_RELEASE_DOCS
    ./LICENSE.md
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
    # and reduce the size of the archive            *
    # ***********************************************

    ## Qt Libraries
    get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)

    install(FILES
        ${QT_DLL_DIR}/Qt5Core.dll
        ${QT_DLL_DIR}/Qt5Gui.dll
        ${QT_DLL_DIR}/Qt5Widgets.dll
        ${QT_DLL_DIR}/Qt5Network.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        )

    if(USE_QT_WINEXTRAS)
        install(FILES
            ${QT_DLL_DIR}/Qt5WinExtras.dll
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            )
    endif(USE_QT_WINEXTRAS)

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
            DESTINATION ${CMAKE_INSTALL_PREFIX}
            )

    else() # MSVC

        if(USE_QT_WEBENGINE)
            # https://doc.qt.io/qt-5/qtwebengine-deploying.html
            install(FILES
                # Deploying Paths Config File
                ./3rd/qtwebengine/qt.conf
                DESTINATION ${CMAKE_INSTALL_PREFIX}
                )
            install(FILES
                # Deploying Libraries
                ${QT_DLL_DIR}/Qt5WebEngineCore.dll
                ${QT_DLL_DIR}/Qt5WebEngineWidgets.dll
                # Also necessary:
                ${QT_DLL_DIR}/Qt5Positioning.dll
                ${QT_DLL_DIR}/Qt5PrintSupport.dll
                ${QT_DLL_DIR}/Qt5Qml.dll
                ${QT_DLL_DIR}/Qt5Quick.dll
                ${QT_DLL_DIR}/Qt5QuickWidgets.dll
                ${QT_DLL_DIR}/Qt5WebChannel.dll
                DESTINATION ${CMAKE_INSTALL_PREFIX}
                )
            install(FILES
                # Deploying QML Imports
                # Deploying Qt WebEngine Processes
                ${QT_DLL_DIR}/QtWebEngineProcess.exe
                DESTINATION ${CMAKE_INSTALL_PREFIX}
                )
            install(FILES
                # Deploying Resources
                ${QT_DLL_DIR}/../resources/icudtl.dat
                ${QT_DLL_DIR}/../resources/qtwebengine_devtools_resources.pak
                ${QT_DLL_DIR}/../resources/qtwebengine_resources.pak
                ${QT_DLL_DIR}/../resources/qtwebengine_resources_100p.pak
                ${QT_DLL_DIR}/../resources/qtwebengine_resources_200p.pak
                DESTINATION ${CMAKE_INSTALL_PREFIX}/resources
                )
            install(FILES
                # Deploying Translations
                ${QT_DLL_DIR}/../translations/qtwebengine_locales/en-US.pak
                DESTINATION ${CMAKE_INSTALL_PREFIX}/translations/qtwebengine_locales
                )
        endif(USE_QT_WEBENGINE)

    endif()

    ## Qt Platform Plugin
    install(FILES
        ${QT_DLL_DIR}/../plugins/platforms/qminimal.dll
        ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}/platforms
        )

    ## Qt Image Formats Plugin
    install(FILES
        ${QT_DLL_DIR}/../plugins/imageformats/qgif.dll
        ${QT_DLL_DIR}/../plugins/imageformats/qico.dll
        ${QT_DLL_DIR}/../plugins/imageformats/qjpeg.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}/imageformats
        )

    ## Qt Icon Engine Plugin
    install(FILES
        ${QT_DLL_DIR}/../plugins/iconengines/qsvgicon.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}/iconengines
        )

    ## Qt Styles Plugin
    install(FILES
        ${QT_DLL_DIR}/../plugins/styles/qwindowsvistastyle.dll
        DESTINATION ${CMAKE_INSTALL_PREFIX}/styles
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


## yt-dlp
if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    install(FILES
        ./3rd/yt-dlp/bin/yt-dlp.exe
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        )

else() # MacOS or Unix Compilers
    install(FILES
        ./3rd/yt-dlp/bin/yt-dlp
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        )
endif()
