##############################################################################
# Launcher
##############################################################################


# Rename application
if(RELEASE_MODE)
    set(Launcher_NAME "launcher")
else(RELEASE_MODE)
    set(Launcher_NAME "launcherd")
endif(RELEASE_MODE)


##############################################################################
### Sources
##############################################################################
set(Launcher_SOURCES ${Launcher_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/launcher.cpp
    )

SET(Launcher_RESOURCES ${Launcher_RESOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources_win.rc
    )


##############################################################################
# Linking the executable
##############################################################################
add_executable(${Launcher_NAME}
    WIN32           # build a Win32 application, not a console application
    ${Launcher_SOURCES}
    ${Launcher_RESOURCES}
    )

# Qt5
target_link_libraries(${Launcher_NAME} Qt5::Core)


##############################################################################
# Deploying
##############################################################################
install(TARGETS ${Launcher_NAME}
    RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

set(Launcher_DOCS
    ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/launcher-manifest-chrome.json
    ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/launcher-manifest-firefox.json
    ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/ReadMe.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/install.bat
    ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/uninstall.bat
    )

install (
    FILES ${Launcher_DOCS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT release_docs
    )

