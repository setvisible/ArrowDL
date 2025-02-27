set(MY_SOURCES ${MY_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/torrent/torrent.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentcontext.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/torrentmessage.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/torrent/workerthread.cpp
)

# Rem: set here the headers related to the Qt MOC (i.e., with associated *.ui)
# set(MY_HEADERS ${MY_HEADERS}
#     ${CMAKE_SOURCE_DIR}/src/torrent/torrent.h
# )
