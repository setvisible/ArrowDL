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

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    set(Launcher_DOCS
        ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/windows/launcher-manifest-chrome.json
        ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/windows/launcher-manifest-firefox.json
        ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/windows/ReadMe.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/windows/install.bat
        ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/windows/uninstall.bat
        )

else() # MacOS or Unix Compilers
    if(UNIX AND NOT APPLE) # Linux or FreeBSD
        set(Launcher_DOCS
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/unix/launcher-manifest-chrome.json
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/unix/launcher-manifest-firefox.json
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/unix/README
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/unix/install.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/unix/uninstall.sh
            )

    else() # MacOS
        set(Launcher_DOCS
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/macx/launcher-manifest-chrome.json
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/launcher/macx/launcher-manifest-firefox.json
            ${CMAKE_CURRENT_SOURCE_DIR}/web-extension/setup/macx/TODO
            )
    endif()
endif()

install (
    FILES ${Launcher_DOCS}
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT release_docs
    )
