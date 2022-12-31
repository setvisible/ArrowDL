string( TOUPPER "${CMAKE_BUILD_TYPE}" BuildTypeUpperCase )
if(BuildTypeUpperCase STREQUAL "DEBUG")
    set(TARGET_NAME "launcherd")
else()
    set(TARGET_NAME "launcher")
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
)

qt_standard_project_setup()

set(MY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher.cpp
)

set(MY_RC_FILE
    ${CMAKE_SOURCE_DIR}/src/resources_win.rc
)

add_executable(${TARGET_NAME} WIN32 # build a Win32 application, not a console application
    ${MY_SOURCES}
    ${MY_RC_FILE}
)

target_compile_definitions(${TARGET_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:QT_DEBUG>
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
        $<$<CONFIG:Release>:QT_NO_DEBUG>
        $<$<CONFIG:Debug>:QT_DEPRECATED_WARNINGS>
        UNICODE
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
        Qt::Core
)

install(
    TARGETS
        ${TARGET_NAME}
    RUNTIME
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}
)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    install(
        FILES
            ${CMAKE_SOURCE_DIR}/web-extension/launcher/windows/launcher-manifest-chrome.json
            ${CMAKE_SOURCE_DIR}/web-extension/launcher/windows/launcher-manifest-firefox.json
            ${CMAKE_SOURCE_DIR}/web-extension/setup/windows/ReadMe.txt
            ${CMAKE_SOURCE_DIR}/web-extension/setup/windows/install.bat
            ${CMAKE_SOURCE_DIR}/web-extension/setup/windows/uninstall.bat
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )

else() # MacOS or Unix Compilers
    if(UNIX AND NOT APPLE) # Linux or FreeBSD

        install(
            FILES
                ${CMAKE_SOURCE_DIR}/web-extension/launcher/unix/launcher-manifest-chrome.json
                ${CMAKE_SOURCE_DIR}/web-extension/launcher/unix/launcher-manifest-firefox.json
                ${CMAKE_SOURCE_DIR}/web-extension/setup/unix/README
                ${CMAKE_SOURCE_DIR}/web-extension/setup/unix/install.sh
                ${CMAKE_SOURCE_DIR}/web-extension/setup/unix/uninstall.sh
            DESTINATION
                ${CMAKE_INSTALL_PREFIX}
        )

    else() # MacOS

        install(
            FILES
                ${CMAKE_SOURCE_DIR}/web-extension/launcher/macx/launcher-manifest-chrome.json
                ${CMAKE_SOURCE_DIR}/web-extension/launcher/macx/launcher-manifest-firefox.json
                ${CMAKE_SOURCE_DIR}/web-extension/setup/macx/TODO
            DESTINATION
                ${CMAKE_INSTALL_PREFIX}
        )

    endif()
endif()
