#-------------------------------------------------------------------------------
# libtorrent
#
# Author: 	Arvid Norberg, arvid@libtorrent.org
# Version: 	1.2.5
#
# http://libtorrent.org/
# https://github.com/arvidn/libtorrent
#
#-------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/)

set(VER_MAJOR 1)  # from version.hpp
set(VER_MINOR 2)
set(VER_TINY  5)

set(SOVERSION "10")


add_definitions(-DNDEBUG)                   # eliminates debug info
add_definitions(-DTORRENT_DISABLE_LOGGING)  # eliminates logging alerts

# build libtorrent as a static (not shared) library
add_definitions(-DTORRENT_BUILDING_SHARED)       # tagged with __declspec(dllexport)
add_definitions(-DTORRENT_LINKING_SHARED)   # tagged with __declspec(dllimport)

add_definitions(-DBOOST_ASIO_ENABLE_CANCELIO) # enable cancel() in asio on windows

# libtorrent private definitions:
add_definitions(-DTORRENT_BUILDING_LIBRARY)
add_definitions(-D_FILE_OFFSET_BITS=64) # boost/config.hpp
add_definitions(-DBOOST_EXCEPTION_DISABLE)
add_definitions(-DBOOST_ASIO_HAS_STD_CHRONO)

add_definitions(-DTORRENT_NO_DEPRECATE) # eliminates a little bit of code

# target_compile_definitions(torrent-rasterbar
    # PRIVATE TORRENT_BUILDING_SHARED
    # INTERFACE TORRENT_LINKING_SHARED
# )

#target_compile_definitions(torrent-rasterbar
#	PUBLIC
#		# $<$<CONFIG:Debug>:TORRENT_USE_ASSERTS>
#		BOOST_ASIO_ENABLE_CANCELIO
#	PRIVATE
#		TORRENT_BUILDING_LIBRARY
#		_FILE_OFFSET_BITS=64
#		BOOST_EXCEPTION_DISABLE
#		BOOST_ASIO_HAS_STD_CHRONO
#)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    add_definitions(-D_WIN32_WINNT=0x0600) # target Windows Vista or later
    add_definitions(-DWIN32_LEAN_AND_MEAN) # prevent winsock1 to be included

    if(MSVC)
        add_definitions(-DBOOST_ALL_NO_LIB) # Do not link any Boost lib silently
        add_definitions(-D_SCL_SECURE_NO_DEPRECATE)  # avoids warnings about unsafe functions like sprintf
        add_definitions(-D_CRT_SECURE_NO_DEPRECATE)  # disable bogus deprecation warnings on msvc8

    endif()

else() # MacOS or Unix Compilers

    # set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

endif()

# ==============================================================================
# Kademlia
# Distributed hash table for decentralized peer-to-peer computer networks
# designed by Petar Maymounkov and David Mazières in 2002
# ==============================================================================
set(LIBTORRENT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/announce_flags.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/dht_observer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/dht_settings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/dht_state.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/dht_storage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/dht_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/direct_request.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/dos_blocker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/ed25519.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/find_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/get_item.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/get_peers.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/item.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/msg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/node_entry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/node_id.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/observer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/put_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/refresh.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/routing_table.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/rpc_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/sample_infohashes.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/traversal_algorithm.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/kademlia/types.hpp
    )

set(LIBTORRENT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/dht_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/dht_storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/dht_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/find_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/put_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/msg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/node_entry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/node_id.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/refresh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/routing_table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/rpc_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/traversal_algorithm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/dos_blocker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/get_peers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/get_item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/ed25519.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/sample_infohashes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/kademlia/dht_settings.cpp
    )

set(LIBTORRENT_SOURCES ${LIBTORRENT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/add_scalar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/fe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/ge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/key_exchange.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/keypair.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/sc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/sign.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/../ed25519/src/verify.cpp
    )

set(LIBTORRENT_SOURCES ${LIBTORRENT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/hasher512.cpp
    )

set(LIBTORRENT_HEADERS ${LIBTORRENT_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/add_torrent_params.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/address.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/alert.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/alert_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/alert_types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/announce_entry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/assert.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bandwidth_limit.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bandwidth_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bandwidth_queue_entry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bandwidth_socket.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bdecode.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bencode.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bitfield.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/block_cache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bloom_filter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/broadcast_socket.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/bt_peer_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/chained_buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/choker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/close_reason.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/ConvertUTF.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/copy_ptr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/crc32c.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/create_torrent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/deadline_timer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/debug.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_buffer_holder.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_buffer_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_io_job.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_io_thread.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_io_thread_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_job_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/disk_observer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/download_priority.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/ed25519.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/entry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/enum_net.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/error_code.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/extensions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/file.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/file_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/file_storage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/fingerprint.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/flags.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/gzip.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/hasher.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/hasher512.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/heterogeneous_queue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/hex.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/http_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/http_parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/http_seed_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/http_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/http_tracker_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/i2p_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/identify_client.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/index_range.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/invariant_check.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/io_service.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/io_service_fwd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/ip_filter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/ip_voter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/lazy_entry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/link.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/linked_list.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/lsd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/magnet_uri.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/natpmp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/netlink.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/operations.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/optional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/packet_buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/packet_pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/parse_url.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/part_file.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/pe_crypto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_class_set.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_class_type_filter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_connection_handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_connection_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_id.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_list.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/peer_request.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/performance_counters.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/pex_flags.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/piece_block.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/piece_block_progress.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/piece_picker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/platform_util.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/portmap.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/proxy_base.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/puff.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/random.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/read_resume_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/receive_buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/request_blocks.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/resolve_links.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/resolver.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/resolver_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/session.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/session_handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/session_settings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/session_stats.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/session_status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/session_types.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/settings_pack.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/sha1.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/sha1_hash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/sha512.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/sliding_average.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/socket.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/socket_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/socks5_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/span.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/ssl_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/stack_allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/stat.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/stat_cache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/storage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/storage_defs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/string_util.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/string_view.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/tailqueue.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/time.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/timestamp_history.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent_flags.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent_handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent_peer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent_peer_allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/torrent_status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/tracker_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/udp_socket.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/udp_tracker_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/union_endpoint.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/units.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/upnp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/utf8.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/utp_socket_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/utp_stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/vector_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/version.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/web_connection_base.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/web_peer_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/write_resume_data.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/xml_parse.hpp
    )

set(LIBTORRENT_HEADERS ${LIBTORRENT_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/extensions/smart_ban.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/extensions/ut_metadata.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/extensions/ut_pex.hpp
    )

set(LIBTORRENT_HEADERS ${LIBTORRENT_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/aligned_storage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/aligned_union.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/alloca.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/allocating_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/array.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/bind_to_device.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/keepalive.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/block_cache_reference.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/byteswap.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/cppint_import_export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/cpuid.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/deferred_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/deprecated.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/deque.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/dev_random.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/disable_warnings_pop.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/disable_warnings_push.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/disk_job_fence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/escape_string.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/export.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/ffs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/file_progress.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/has_block.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/instantiate_connection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/ip_notifier.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/listen_socket_handle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/lsd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/merkle.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/noexcept_movable.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/numeric_cast.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/openssl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/path.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/portmap.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/proxy_settings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/range.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/route.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/scope_end.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/session_call.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/session_impl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/session_interface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/session_settings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/session_udp_sockets.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/set_socket_buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/socket_type.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/storage_piece_set.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/storage_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/string_ptr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/suggest_piece.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/throw.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/time.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/torrent_impl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/unique_ptr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/vector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/win_crypto_provider.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/include/libtorrent/aux_/win_util.hpp
    )

set(LIBTORRENT_SOURCES ${LIBTORRENT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/web_connection_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/alert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/alert_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/announce_entry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/assert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bandwidth_limit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bandwidth_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bandwidth_queue_entry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bdecode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bitfield.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bloom_filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/broadcast_socket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/block_cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/bt_peer_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/chained_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/choker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/close_reason.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ConvertUTF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/cpuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/crc32c.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/create_torrent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_buffer_holder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_buffer_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_io_job.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_io_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_io_thread_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_job_fence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/disk_job_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/entry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/enum_net.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/error_code.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/escape_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/file_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/file_storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/fingerprint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/generate_peer_id.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/gzip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/hasher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/hex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/http_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/http_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/http_seed_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/http_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/http_tracker_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/i2p_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/identify_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/instantiate_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ip_filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ip_notifier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ip_voter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/lazy_bdecode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/listen_socket_handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/lsd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/magnet_uri.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/merkle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/natpmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/parse_url.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/part_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/pe_crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/performance_counters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/peer_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/peer_connection_handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/peer_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/peer_class_set.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/piece_picker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/platform_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/packet_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/proxy_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/peer_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/puff.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/random.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/receive_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/read_resume_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/write_resume_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/request_blocks.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/resolve_links.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/resolver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/session_call.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/session_handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/session_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/session_settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/proxy_settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/settings_pack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/sha1_hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/smart_ban.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/socket_io.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/socket_type.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/socks5_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/stat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/stat_cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/storage_piece_set.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/storage_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/session_stats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/string_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/torrent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/torrent_handle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/torrent_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/torrent_peer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/torrent_peer_allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/torrent_status.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/time.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/timestamp_history.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/tracker_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/udp_socket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/udp_tracker_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/upnp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ut_metadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ut_pex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/utf8.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/utp_socket_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/utp_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/web_peer_connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/xml_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/file_progress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/ffs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/add_torrent_params.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/peer_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/stack_allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/sha1.cpp
    )

set(LIBTORRENT_SOURCES ${LIBTORRENT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libtorrent/1.2.5/src/sha512.cpp
    )

#OTHERS+= Makefile.am

add_library(libtorrent_static_lib STATIC
    ${LIBTORRENT_SOURCES}            
    )
