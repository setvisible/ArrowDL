#.rst:
# FindLibtorrentRasterbar
# ---------------------
#
# Try to find LibtorrentRasterbar.
# Input: 
# This will define the following variables:
#
# ``LibtorrentRasterbar_FOUND``
#       True if LibtorrentRasterbar is available.
#       If false, do not try to link to LibtorrentRasterbar
#
# ``LibtorrentRasterbar_VERSION``
#       The version of LibtorrentRasterbar
#
# ``LibtorrentRasterbar_INCLUDE_DIRS``
#       Where to find torrent.hpp
#
# ``LibtorrentRasterbar_LIBRARIES``
#       The names of the libraries to link against
#
# ``LibtorrentRasterbar_OPENSSL_ENABLED``
#       If libtorrent-rasterbar uses and links against OpenSSL
#
#message("<FindLibtorrentRasterbar.cmake>")

if(EXISTS ${LibtorrentRasterbar_ROOT})
    set(LibtorrentRasterbar_FOUND True)
    set(LibtorrentRasterbar_INCLUDE_DIRS "${LibtorrentRasterbar_ROOT}/include/")
    set(LibtorrentRasterbar_LIBRARIES "${LibtorrentRasterbar_ROOT}/lib/libtorrent-rasterbar.a")
    set(LibtorrentRasterbar_OPENSSL_ENABLED True)
    set(LibtorrentRasterbar_VERSION "2.0.6")
endif()

#message("</FindLibtorrentRasterbar.cmake>")
