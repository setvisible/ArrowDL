string( TOUPPER "${CMAKE_BUILD_TYPE}" BuildTypeUpperCase )
if(BuildTypeUpperCase STREQUAL "DEBUG")
    set(TARGET_NAME "DownZemAlld")
else()
    set(TARGET_NAME "DownZemAll")
endif()

find_package(GoogleGumboParser REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    LinguistTools
    Network
    Widgets
)

qt_standard_project_setup()

include(${CMAKE_SOURCE_DIR}/3rd/qtsingleapplication/CMakeLists.txt)

include(${CMAKE_SOURCE_DIR}/src/core/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/src/dialogs/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/src/io/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/src/ipc/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/src/widgets/CMakeLists.txt)

set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
)

set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
)

set(MY_FORMS ${MY_FORMS}
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui
)

qt_add_resources(MY_SOURCES
    ${CMAKE_SOURCE_DIR}/src/resources.qrc
)

set(MY_RC_FILE
    ${CMAKE_SOURCE_DIR}/src/resources_win.rc
)

add_executable(${TARGET_NAME} WIN32 # build a Win32 application, not a console application
    ${QTSINGLEAPPLICATION_SOURCES}
    ${MY_SOURCES}
    ${MY_FORMS}
    #${MY_HEADERS}
    #${QTSINGLEAPPLICATION_HEADERS}
    ${MY_RC_FILE}
)

target_include_directories(${TARGET_NAME}
    PRIVATE
        ${Boost_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIRS}
        ${LibtorrentRasterbar_INCLUDE_DIRS}
        ${Project_INCLUDE_DIRS}
)

target_compile_definitions(${TARGET_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:QT_DEBUG>
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
        $<$<CONFIG:Release>:QT_NO_DEBUG>
        $<$<CONFIG:Debug>:QT_DEPRECATED_WARNINGS>
        UNICODE
)

qt_add_translations(${TARGET_NAME}
    TS_FILES
        ${CMAKE_SOURCE_DIR}/src/locale/dza_ar_EG.ts  # Arabic
        ${CMAKE_SOURCE_DIR}/src/locale/dza_de_DE.ts  # German
        ${CMAKE_SOURCE_DIR}/src/locale/dza_en_US.ts  # English (default)
        ${CMAKE_SOURCE_DIR}/src/locale/dza_es_ES.ts  # Spanish
        ${CMAKE_SOURCE_DIR}/src/locale/dza_fr_FR.ts  # French
        ${CMAKE_SOURCE_DIR}/src/locale/dza_hu_HU.ts  # Hungarian
        ${CMAKE_SOURCE_DIR}/src/locale/dza_it_IT.ts  # Italian
        ${CMAKE_SOURCE_DIR}/src/locale/dza_ja_JP.ts  # Japanese
        ${CMAKE_SOURCE_DIR}/src/locale/dza_ko_KR.ts  # Korean
        ${CMAKE_SOURCE_DIR}/src/locale/dza_nl_NL.ts  # Dutch
        ${CMAKE_SOURCE_DIR}/src/locale/dza_pl_PL.ts  # Polish
        ${CMAKE_SOURCE_DIR}/src/locale/dza_pt_BR.ts  # Portuguese (Brazil)
        ${CMAKE_SOURCE_DIR}/src/locale/dza_pt_PT.ts  # Portuguese (Portugal)
        ${CMAKE_SOURCE_DIR}/src/locale/dza_ru_RU.ts  # Russian
        ${CMAKE_SOURCE_DIR}/src/locale/dza_vi_VN.ts  # Vietnamese
        ${CMAKE_SOURCE_DIR}/src/locale/dza_zh_CN.ts  # Chinese (Simplified & Traditional)
    QM_FILES_OUTPUT_VARIABLE
        qm_files
)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            ${GoogleGumboParser_LIBRARIES}
            ${LibtorrentRasterbar_LIBRARIES}
            wsock32
            ws2_32
            Iphlpapi
            # debug
            # dbghelp

            version.dll # might be "C:/Windows/System32/version.dll" or "C:/Windows/SysWOW64/version.dll"

            crypt32  # required by openssl
            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}

            Qt::Core
            Qt::Gui
            Qt::Network
            Qt::Widgets
    )

else() # MacOS or Unix Compilers

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            ${GoogleGumboParser_LIBRARIES}
            ${LibtorrentRasterbar_LIBRARIES}

            Threads::Threads

            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}

            Qt::Core
            Qt::Gui
            Qt::Network
            Qt::Widgets
    )

endif()

install(
    TARGETS
        ${TARGET_NAME}
    RUNTIME
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}
)

install(
    FILES
        ${CMAKE_SOURCE_DIR}/LICENSE.md
        ${CMAKE_SOURCE_DIR}/TROUBLESHOOTING.md
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}
)

install(
    FILES
        ${qm_files}
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/locale
)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    # ***********************************************
    # windeployqt is not used here in order         *
    # to control the most minimalist deployment     *
    # and reduce the size of the archive            *
    # ***********************************************

    ## Qt Libraries
    get_target_property(QtCore_LOCATION Qt6::Core LOCATION)
    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)

    install(
        FILES
            ${QT_DLL_DIR}/Qt6Core.dll
            ${QT_DLL_DIR}/Qt6Gui.dll
            ${QT_DLL_DIR}/Qt6Widgets.dll
            ${QT_DLL_DIR}/Qt6Network.dll
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )

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

        install(
            FILES
                ${libgcc}
                ${QT_DLL_DIR}/libstdc++-6.dll
                ${QT_DLL_DIR}/libwinpthread-1.dll
            DESTINATION
                ${CMAKE_INSTALL_PREFIX}
        )

    else() # MSVC

    endif()

    ## Qt Platform Plugin
    install(
        FILES
            # ${QT_DLL_DIR}/../plugins/platforms/qdirect2d.dll
            ${QT_DLL_DIR}/../plugins/platforms/qminimal.dll
            # ${QT_DLL_DIR}/../plugins/platforms/qoffscreen.dll
            ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/platforms
    )

    ## Qt Image Formats Plugin
    install(
        FILES
            ${QT_DLL_DIR}/../plugins/imageformats/qgif.dll
            # ${QT_DLL_DIR}/../plugins/imageformats/qicns.dll
            ${QT_DLL_DIR}/../plugins/imageformats/qico.dll
            ${QT_DLL_DIR}/../plugins/imageformats/qjpeg.dll
            # ${QT_DLL_DIR}/../plugins/imageformats/qsvg.dll
            # ${QT_DLL_DIR}/../plugins/imageformats/qtga.dll
            # ${QT_DLL_DIR}/../plugins/imageformats/qtiff.dll
            # ${QT_DLL_DIR}/../plugins/imageformats/qwbmp.dll
            # ${QT_DLL_DIR}/../plugins/imageformats/qwebp.dll
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/imageformats
    )

    ## Qt Icon Engine Plugin
    install(
        FILES
            ${QT_DLL_DIR}/../plugins/iconengines/qsvgicon.dll
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/iconengines
    )

    ## Qt Styles Plugin
    install(
        FILES
            ${QT_DLL_DIR}/../plugins/styles/qwindowsvistastyle.dll
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/styles
    )

    ## OpenSSL
    install(
        FILES
            ${OPENSSL_CRYPTO_BIN}
            ${OPENSSL_SSL_BIN}
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )
    install(
        FILES
            # ${QT_DLL_DIR}/../plugins/tls/qcertonlybackend.dll
            ${QT_DLL_DIR}/../plugins/tls/qopensslbackend.dll
            # ${QT_DLL_DIR}/../plugins/tls/qschannelbackend.dll
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/tls
    )

else() # MacOS or Unix Compilers

    ## Qt Libraries
    get_target_property(QtCore_LOCATION Qt6::Core LOCATION)
    get_filename_component(QT_SHARED_LIB_DIR ${QtCore_LOCATION} PATH)

    install(
        FILES
            # Rem: ${QT_SHARED_LIB_DIR}/libQt6Core.so is not a file but a namelink (symbolic link)
            ${QT_SHARED_LIB_DIR}/libQt6Core.so # required by libQt6Gui
            ${QT_SHARED_LIB_DIR}/libQt6Core.so.6
            ${QT_SHARED_LIB_DIR}/libQt6Core.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6Gui.so # required by libQt6Widgets
            ${QT_SHARED_LIB_DIR}/libQt6Gui.so.6
            ${QT_SHARED_LIB_DIR}/libQt6Gui.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6Widgets.so
            ${QT_SHARED_LIB_DIR}/libQt6Widgets.so.6
            ${QT_SHARED_LIB_DIR}/libQt6Widgets.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6Network.so
            ${QT_SHARED_LIB_DIR}/libQt6Network.so.6
            ${QT_SHARED_LIB_DIR}/libQt6Network.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6DBus.so # required by libQt6Gui
            ${QT_SHARED_LIB_DIR}/libQt6DBus.so.6
            ${QT_SHARED_LIB_DIR}/libQt6DBus.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libicudata.so # required by libQt6Core
            ${QT_SHARED_LIB_DIR}/libicudata.so.56
            ${QT_SHARED_LIB_DIR}/libicudata.so.56.1

            ${QT_SHARED_LIB_DIR}/libicui18n.so # required by libQt6Core
            ${QT_SHARED_LIB_DIR}/libicui18n.so.56
            ${QT_SHARED_LIB_DIR}/libicui18n.so.56.1

            ${QT_SHARED_LIB_DIR}/libicuio.so
            ${QT_SHARED_LIB_DIR}/libicuio.so.56
            ${QT_SHARED_LIB_DIR}/libicuio.so.56.1

            ${QT_SHARED_LIB_DIR}/libicule.so
            ${QT_SHARED_LIB_DIR}/libicule.so.56
            ${QT_SHARED_LIB_DIR}/libicule.so.56.1

            ${QT_SHARED_LIB_DIR}/libiculx.so
            ${QT_SHARED_LIB_DIR}/libiculx.so.56
            ${QT_SHARED_LIB_DIR}/libiculx.so.56.1

            ${QT_SHARED_LIB_DIR}/libicutest.so
            ${QT_SHARED_LIB_DIR}/libicutest.so.56
            ${QT_SHARED_LIB_DIR}/libicutest.so.56.1

            ${QT_SHARED_LIB_DIR}/libicutu.so
            ${QT_SHARED_LIB_DIR}/libicutu.so.56
            ${QT_SHARED_LIB_DIR}/libicutu.so.56.1

            ${QT_SHARED_LIB_DIR}/libicuuc.so # required by libQt6Core
            ${QT_SHARED_LIB_DIR}/libicuuc.so.56
            ${QT_SHARED_LIB_DIR}/libicuuc.so.56.1

            ${QT_SHARED_LIB_DIR}/libQt6XcbQpa.so # required by libqxcb
            ${QT_SHARED_LIB_DIR}/libQt6XcbQpa.so.6 
            ${QT_SHARED_LIB_DIR}/libQt6XcbQpa.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6OpenGL.so # required by libqxcb
            ${QT_SHARED_LIB_DIR}/libQt6OpenGL.so.6 
            ${QT_SHARED_LIB_DIR}/libQt6OpenGL.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6OpenGLWidgets.so # in case?
            ${QT_SHARED_LIB_DIR}/libQt6OpenGLWidgets.so.6 
            ${QT_SHARED_LIB_DIR}/libQt6OpenGLWidgets.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6WaylandClient.so # required by libqwayland-generic and libqwayland-egl
            ${QT_SHARED_LIB_DIR}/libQt6WaylandClient.so.6
            ${QT_SHARED_LIB_DIR}/libQt6WaylandClient.so.6.5.3

            ${QT_SHARED_LIB_DIR}/libQt6WaylandEglClientHwIntegration.so # required by libqwayland-egl
            ${QT_SHARED_LIB_DIR}/libQt6WaylandEglClientHwIntegration.so.6
            ${QT_SHARED_LIB_DIR}/libQt6WaylandEglClientHwIntegration.so.6.5.3
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )

    ## Qt Platform Plugin
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqeglfs.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqlinuxfb.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqminimal.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqminimalegl.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqoffscreen.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqvkkhrdisplay.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqvnc.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqwayland-egl.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqwayland-generic.so
            ${QT_SHARED_LIB_DIR}/../plugins/platforms/libqxcb.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/platforms
    )

    ## Qt Image Formats Plugin
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/imageformats/libqgif.so
            ${QT_SHARED_LIB_DIR}/../plugins/imageformats/libqico.so
            ${QT_SHARED_LIB_DIR}/../plugins/imageformats/libqjpeg.so
            ${QT_SHARED_LIB_DIR}/../plugins/imageformats/libqsvg.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/imageformats
    )

    # Qt Icon Engine Plugin
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/iconengines/libqsvgicon.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/iconengines
    )

    ## Qt Styles Plugin
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/platformthemes/libqgtk3.so
            ${QT_SHARED_LIB_DIR}/../plugins/platformthemes/libqxdgdesktopportal.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/platformthemes
    )

    ## Qt Network
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/networkinformation/libqglib.so
            ${QT_SHARED_LIB_DIR}/../plugins/networkinformation/libqnetworkmanager.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/networkinformation
    )

    ## OpenSSL
    install(
        FILES
            ${OPENSSL_CRYPTO_BIN}
            ${OPENSSL_SSL_BIN}
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/tls/libqcertonlybackend.so
            ${QT_SHARED_LIB_DIR}/../plugins/tls/libqopensslbackend.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/tls
    )

    ## Qt XCB GL Integrations Plugin
    install(
        FILES
            ${QT_SHARED_LIB_DIR}/../plugins/xcbglintegrations/libqxcb-egl-integration.so
            ${QT_SHARED_LIB_DIR}/../plugins/xcbglintegrations/libqxcb-glx-integration.so
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/xcbglintegrations
    )

endif()

## YT-DLP
if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    install(
        FILES
            ${CMAKE_SOURCE_DIR}/3rd/yt-dlp/bin/yt-dlp.exe
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )

else() # MacOS or Unix Compilers

    install(
        FILES
            ${CMAKE_SOURCE_DIR}/3rd/yt-dlp/bin/yt-dlp
        PERMISSIONS
            OWNER_EXECUTE
            OWNER_WRITE
            OWNER_READ
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}
    )

endif()

## Desktop Shortcut
if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    # Nothing

else() # MacOS or Unix Compilers

    if(UNIX AND NOT APPLE) # Linux or FreeBSD

        install (
            FILES
                ${CMAKE_SOURCE_DIR}/installer/unix/portable/DownZemAll.desktop
                ${CMAKE_SOURCE_DIR}/installer/unix/portable/DownZemAll_64x64.png
                ${CMAKE_SOURCE_DIR}/installer/unix/portable/install_shortcut_desktop.sh
                ${CMAKE_SOURCE_DIR}/installer/unix/portable/uninstall_shortcut_desktop.sh
                ${CMAKE_SOURCE_DIR}/installer/unix/portable/README
            DESTINATION
                ${CMAKE_INSTALL_PREFIX}/desktop_shortcut
        )

    else() # MacOS

        # Nothing

    endif()
endif()
