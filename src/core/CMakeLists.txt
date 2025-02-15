set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/core/abstractdownloaditem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/abstractsettings.cpp
    ${CMAKE_SOURCE_DIR}/src/core/checkabletablemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/core/downloadfileitem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/downloadmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/downloadstreamitem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/downloadtorrentitem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/file.cpp
    ${CMAKE_SOURCE_DIR}/src/core/fileaccessmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/fileutils.cpp
    ${CMAKE_SOURCE_DIR}/src/core/format.cpp
    ${CMAKE_SOURCE_DIR}/src/core/htmlparser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/locale.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mask.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mimedatabase.cpp
    ${CMAKE_SOURCE_DIR}/src/core/model.cpp
    ${CMAKE_SOURCE_DIR}/src/core/networkmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/regex.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourceitem.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourcemodel.cpp
    ${CMAKE_SOURCE_DIR}/src/core/session.cpp
    ${CMAKE_SOURCE_DIR}/src/core/settings.cpp
    ${CMAKE_SOURCE_DIR}/src/core/stream.cpp
    ${CMAKE_SOURCE_DIR}/src/core/streammanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/theme.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrent.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentbasecontext.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentcontext.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentcontext_p.cpp
    ${CMAKE_SOURCE_DIR}/src/core/torrentmessage.cpp
    ${CMAKE_SOURCE_DIR}/src/core/updatechecker.cpp
    ${CMAKE_SOURCE_DIR}/src/core/updateinstaller.cpp
)

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
set(MY_HEADERS ${MY_HEADERS}
    ${CMAKE_SOURCE_DIR}/src/core/downloadfileitem.h
    ${CMAKE_SOURCE_DIR}/src/core/downloadmanager.h
    ${CMAKE_SOURCE_DIR}/src/core/downloadstreamitem.h
    ${CMAKE_SOURCE_DIR}/src/core/downloadtorrentitem.h
    ${CMAKE_SOURCE_DIR}/src/core/model.h
    ${CMAKE_SOURCE_DIR}/src/core/resourcemodel.h
    ${CMAKE_SOURCE_DIR}/src/core/settings.h
    ${CMAKE_SOURCE_DIR}/src/core/updatechecker.h
    ${CMAKE_SOURCE_DIR}/src/core/updatechecker_p.h
    ${CMAKE_SOURCE_DIR}/src/core/updateinstaller.h
)
