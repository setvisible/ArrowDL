# =========================================================
# From
#   libtorrent-rasterbar/examples/simple_client.cpp
# =========================================================
set(MY_TEST_TARGET simple_client)

set(MY_TEST_SOURCES
    # ${CMAKE_SOURCE_DIR}/src/.../.cpp
)

add_executable(${MY_TEST_TARGET} WIN32
    ${CMAKE_CURRENT_SOURCE_DIR}/simple_client.cpp
    ${MY_TEST_SOURCES}
)

target_include_directories(${MY_TEST_TARGET}
    PRIVATE
        ${Boost_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIRS}
        ${LibtorrentRasterbar_INCLUDE_DIRS}
    )

target_compile_definitions(${MY_TEST_TARGET}
    PRIVATE
        #BOOST_ALL_NO_LIB
        #BOOST_ASIO_ENABLE_CANCELIO # enable cancel() in asio on windows
        #BOOST_ASIO_HAS_STD_CHRONO
        #BOOST_MULTI_INDEX_DISABLE_SERIALIZATION
        #BOOST_NO_DEPRECATED
        #BOOST_SYSTEM_NO_DEPRECATED
        # # BOOST_ASIO_NO_DEPRECATED
        # # _FILE_OFFSET_BITS=64 # boost/config.hpp
        WIN32_LEAN_AND_MEAN # prevent winsock1 to be included
    )

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

    target_link_libraries(${MY_TEST_TARGET}
        PRIVATE
            ${LibtorrentRasterbar_LIBRARIES}
            wsock32
            ws2_32
            Iphlpapi
            #debug
            #dbghelp

            crypt32  # required by openssl
            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}
        )

else() # MacOS or Unix Compilers

    target_link_libraries(${MY_TEST_TARGET}
        PRIVATE
            ${LibtorrentRasterbar_LIBRARIES}

            Threads::Threads

            ${OPENSSL_CRYPTO_LIBRARY}
            ${OPENSSL_SSL_LIBRARY}
    )

endif()
