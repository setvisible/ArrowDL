#-------------------------------------------------
# Boost
#-------------------------------------------------
# Note:
# Boost is required to build Libtorrent
#
find_package(Boost)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    if(Boost_VERSION_STRING)
        # if CMake version >= 3.16.2 # Appveyor
        # if CMake version >= 3.14.7
        set(BOOST_VERSION_STR ${Boost_VERSION_STRING})
    else()
        if(Boost_VERSION_MAJOR)
            # if CMake version >= 3.16.2 # Appveyor
            # if CMake version >= 3.14.7
            set(BOOST_VERSION_STR "${Boost_VERSION_MAJOR}.${Boost_VERSION_MINOR}.${Boost_VERSION_PATCH}")
        else()
            if(Boost_MAJOR_VERSION)
                # if CMake version >= 3.9.2 # Travis
                # if CMake version >= 3.5
                # if CMake version >= 3.0.2
                set(BOOST_VERSION_STR "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
            else()
                set(BOOST_VERSION_STR "---")
            endif()
        endif()
    endif()
    if(NOT BOOST_VERSION_STR)
        set(BOOST_VERSION_STR "---")
    endif()
else()
    message(FATAL_ERROR "Unable to find Boost. Try set path to Boost with Boost_INCLUDE_DIR.")
endif()
