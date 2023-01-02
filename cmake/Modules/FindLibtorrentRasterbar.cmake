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

function(parse_version filename)
    file(READ ${filename} versionFile)
    if (NOT versionFile)
        message(FATAL_ERROR "Unable to determine LibTorrent version. Version file is missing.")
    endif()

    string(REGEX MATCH "#define LIBTORRENT_VERSION_MAJOR ([0-9]*)" _ ${versionFile})
    set(LIBTORRENT_VERSION_MAJOR ${CMAKE_MATCH_1})

    string(REGEX MATCH "#define LIBTORRENT_VERSION_MINOR ([0-9]*)" _ ${versionFile})
    set(LIBTORRENT_VERSION_MINOR ${CMAKE_MATCH_1})

    string(REGEX MATCH "#define LIBTORRENT_VERSION_TINY ([0-9]*)" _ ${versionFile})
    set(LIBTORRENT_VERSION_TINY ${CMAKE_MATCH_1})

    set(text "${LIBTORRENT_VERSION_MAJOR}.${LIBTORRENT_VERSION_MINOR}.${LIBTORRENT_VERSION_TINY}")

    set(LibtorrentRasterbar_VERSION ${text} PARENT_SCOPE)
endfunction()

parse_version("${CMAKE_SOURCE_DIR}/version")

if(EXISTS ${LibtorrentRasterbar_ROOT})
    set(LibtorrentRasterbar_FOUND True)
    set(LibtorrentRasterbar_INCLUDE_DIRS "${LibtorrentRasterbar_ROOT}/include/")
    set(LibtorrentRasterbar_LIBRARIES "${LibtorrentRasterbar_ROOT}/lib/libtorrent-rasterbar.a")
    set(LibtorrentRasterbar_OPENSSL_ENABLED True)
    parse_version("${LibtorrentRasterbar_ROOT}/include/libtorrent/version.hpp")

    if(NOT EXISTS ${LibtorrentRasterbar_INCLUDE_DIRS})
        message(FATAL_ERROR "File doesn't exist: ${LibtorrentRasterbar_INCLUDE_DIRS}")
    endif()
    if(NOT EXISTS ${LibtorrentRasterbar_LIBRARIES})
        message(FATAL_ERROR "File doesn't exist: ${LibtorrentRasterbar_LIBRARIES}")
    endif()

    # Hack: to build DZA, the same precompiler definitions must be used as for building Libtorrent
    add_compile_definitions(NDEBUG) # Libtorrent is a release build
    # add_compile_definitions(TORRENT_DISABLE_LOGGING)
    # add_compile_definitions(TORRENT_DISABLE_ALERT_MSG)
    # add_compile_definitions(TORRENT_DISABLE_SUPERSEEDING)
    # add_compile_definitions(TORRENT_DISABLE_SHARE_MODE)
    # add_compile_definitions(TORRENT_DISABLE_MUTABLE_TORRENTS)
    # add_compile_definitions(TORRENT_DISABLE_STREAMING)
    # add_compile_definitions(TORRENT_DISABLE_PREDICTIVE_PIECES)
    # add_compile_definitions(TORRENT_LINKING_SHARED)
    # add_compile_definitions(TORRENT_BUILDING_SHARED)
    # add_compile_definitions(TORRENT_DISABLE_DHT)
    # add_compile_definitions(TORRENT_DISABLE_ENCRYPTION)
    # add_compile_definitions(TORRENT_DISABLE_EXTENSIONS)
    # add_compile_definitions(TORRENT_USE_INVARIANT_CHECKS)
    # add_compile_definitions(TORRENT_EXPENSIVE_INVARIANT_CHECKS)
    add_compile_definitions(TORRENT_NO_DEPRECATE)
    # add_compile_definitions(TORRENT_PRODUCTION_ASSERTS)
    # add_compile_definitions(TORRENT_USE_ASSERTS=0)
    # add_compile_definitions(TORRENT_USE_SYSTEM_ASSERTS)
    # add_compile_definitions(TORRENT_HAVE_MMAP)
    add_compile_definitions(TORRENT_USE_OPENSSL)
    # add_compile_definitions(TORRENT_USE_GNUTLS)
    add_compile_definitions(TORRENT_USE_LIBCRYPTO)
    # add_compile_definitions(TORRENT_USE_LIBGCRYPT)
    add_compile_definitions(TORRENT_SSL_PEERS)
endif()

#message("</FindLibtorrentRasterbar.cmake>")
