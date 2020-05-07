##############################################################################
# Application
##############################################################################


# Rename application
if(RELEASE_MODE)
    set(DownZemAll_NAME "DownZemAll")
else(RELEASE_MODE)
    set(DownZemAll_NAME "DownZemAlld")
endif(RELEASE_MODE)


##############################################################################
### Sources
##############################################################################
include(${CMAKE_CURRENT_SOURCE_DIR}/src/core/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/dialogs/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/io/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/ipc/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/CMakeLists.txt)

set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
    )

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.h
    )

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.ui
    )

SET(MY_RESOURCES ${MY_RESOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources_win.rc
    )


##############################################################################
# Linking the executable
##############################################################################
add_executable(${DownZemAll_NAME}
    WIN32           # build a Win32 application, not a console application
    ${MY_FORMS}
    ${MY_SOURCES}
    ${AUTOUPDATER_SOURCES}
    ${QTSINGLEAPPLICATION_SOURCES}
    ${MY_RESOURCES}
    )

# Gumbo
target_link_libraries(${DownZemAll_NAME}
    gumbo_static_lib
    )

# LibTorrent
target_link_libraries(${DownZemAll_NAME}
    libtorrent_static_lib
    )

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    target_link_libraries(${DownZemAll_NAME}
        wsock32
        ws2_32
        Iphlpapi
        )

else() # MacOS or Unix Compilers
    target_link_libraries(${DownZemAll_NAME}
        Threads::Threads
        )
endif()


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
if(USE_QT_WINEXTRAS)
    target_link_libraries(${DownZemAll_NAME} Qt5::WinExtras)
endif(USE_QT_WINEXTRAS)
if(USE_QT_WEBENGINE)
    target_link_libraries(${DownZemAll_NAME} Qt5::WebEngineWidgets)
endif(USE_QT_WEBENGINE)


##############################################################################
# Deploying
##############################################################################
install(TARGETS ${DownZemAll_NAME}
    RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    )


