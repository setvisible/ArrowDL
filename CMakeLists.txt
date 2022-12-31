# see https://doc.qt.io/qt-6/cmake-get-started.html

cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

option(BUILD_LAUNCHER "Set to ON to build the Launcher (default)" ON)
option(BUILD_MAIN_APPLICATION "Set to ON to build the Application (default)" ON)
option(BUILD_TESTS "Set to ON to build test applications (default)" ON)

set(Boost_ROOT "" CACHE PATH "Where to find Boost root (i.e. directory where the 'INSTALL' file is)")
set(LibtorrentRasterbar_ROOT "" CACHE PATH "Where to find LibtorrentRasterbar root")
set(OpenSSL_ROOT_DIR "" CACHE PATH "Where to find OpenSSL root")


function(verify_install_dir)
    if(CMAKE_INSTALL_PREFIX STREQUAL ""
            OR CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files"
            OR CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files (x86)"
            OR CMAKE_INSTALL_PREFIX MATCHES "^/usr/local") # sanity check
        message(WARNING " - Please define a custom install path, with 'CMAKE_INSTALL_PREFIX'.")
    endif()
endfunction()

verify_install_dir()

message("")
message("------------------------------------------------------------------------")
message("FYI, the project will be installed here:")
message(" - '${CMAKE_INSTALL_PREFIX}'.")
message("with:")
message(" - BUILD_LAUNCHER = ${BUILD_LAUNCHER}")
message(" - BUILD_MAIN_APPLICATION = ${BUILD_MAIN_APPLICATION}")
message(" - BUILD_TESTS = ${BUILD_TESTS}")
message("------------------------------------------------------------------------")
message("")

function(parse_version filename)
    # read 'version' file into a variable (stripping any newlines or spaces)
    file(READ ${filename} versionFile)
    if (NOT versionFile)
        message(FATAL_ERROR "Unable to determine version. Version file is missing.")
    endif()
    string(STRIP "${versionFile}" DOWNZEMALL_VERSION)
    # add a dependency on the version file
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS version)

    # split version string into components, note CMAKE_MATCH_0 is the entire regexp match
    string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" output_variable ${DOWNZEMALL_VERSION} )

    set(APP_VERSION ${output_variable} PARENT_SCOPE)
    set(APP_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
    set(APP_VERSION_MINOR ${CMAKE_MATCH_2} PARENT_SCOPE)
    set(APP_VERSION_PATCH ${CMAKE_MATCH_3} PARENT_SCOPE)
endfunction()

parse_version("${CMAKE_SOURCE_DIR}/version")

message("------------------------------------------------------------------------")
message("FYI, current version file:")
message(" - APP_VERSION = ${APP_VERSION}")
message(" - APP_VERSION_MAJOR = ${APP_VERSION_MAJOR}")
message(" - APP_VERSION_MINOR = ${APP_VERSION_MINOR}")
message(" - APP_VERSION_PATCH = ${APP_VERSION_PATCH}")
message("------------------------------------------------------------------------")
message("")

project("DownZemAll"
    DESCRIPTION
        "DownZemAll - Download Manager"
    LANGUAGES CXX
    VERSION
        ${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}
)

find_package(BoostForLibtorrent)
find_package(OpenSSLPreferablyFromQt6)
find_package(LibtorrentRasterbar)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

else() # MacOS or Unix Compilers

    find_package(Threads REQUIRED)

endif()

message("------------------------------------------------------------------------")
message("FYI, the project dependencies are found:")
message(" - LibtorrentRasterbar_FOUND = ${LibtorrentRasterbar_FOUND}")
message(" - LibtorrentRasterbar_ROOT = ${LibtorrentRasterbar_ROOT}")
message(" - LibtorrentRasterbar_LIBRARIES = ${LibtorrentRasterbar_LIBRARIES}")
message(" - LibtorrentRasterbar_INCLUDE_DIRS = ${LibtorrentRasterbar_INCLUDE_DIRS}")
message(" - LibtorrentRasterbar_VERSION = ${LibtorrentRasterbar_VERSION}")
message(" - LibtorrentRasterbar_OPENSSL_ENABLED = ${LibtorrentRasterbar_OPENSSL_ENABLED}")
message("")
message(" - Boost_FOUND = ${Boost_FOUND}")
message(" - Boost_ROOT = ${Boost_ROOT}")
message(" - Boost_INCLUDE_DIR = ${Boost_INCLUDE_DIR}")
message(" - BOOST_VERSION_STR = ${BOOST_VERSION_STR}")
message("")
message(" - OpenSSL_ROOT_DIR = ${OpenSSL_ROOT_DIR}")
message(" - OPENSSL_FOUND = ${OPENSSL_FOUND}")
message(" - OPENSSL_INCLUDE_DIRS = ${OPENSSL_INCLUDE_DIRS}")
message(" - OPENSSL_CRYPTO_LIBRARY = ${OPENSSL_CRYPTO_LIBRARY}")
message(" - OPENSSL_SSL_LIBRARY = ${OPENSSL_SSL_LIBRARY}")
message("------------------------------------------------------------------------")
message("")

set(CMAKE_CXX_STANDARD 20)  # Qt6 requires a compiler supporting C++ version 17 or newer.
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # print an error if the compiler is too old.

add_compile_definitions(
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00
)

if(MSVC)
    message(FATAL_ERROR "There is little to none support for building with MSVC and most users won't bother with it.")
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/config.h.in ${CMAKE_SOURCE_DIR}/src/config.h)
configure_file(${CMAKE_SOURCE_DIR}/src/config_3rd.h.in ${CMAKE_SOURCE_DIR}/src/config_3rd.h)

set(Project_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/include/
)

if(BUILD_LAUNCHER)
    add_subdirectory(web-extension/launcher)
endif()

if(BUILD_MAIN_APPLICATION)
    add_subdirectory(src)
endif()

if(BUILD_TESTS)
    enable_testing() # must be *before* add_subdirectory
    add_subdirectory(test)
endif()
