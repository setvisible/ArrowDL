#-------------------------------------------------
# Boost
#-------------------------------------------------
# Note:
# Boost is required to build Libtorrent
#
# Libtorrent version 1.2.5 requires Boost version 1.58 or later.
# Here Boost version 1.72 is used on every platform.
#
# Libtorrent uses:
# - Boost.Asio
# - Boost.Optional
# - Boost.System
# - Boost.Multiprecision
# - Boost.Intrusive
# - Boost.Pool
# - Boost.Python (for bindings)
# - Boost.CRC 
# - ...and various other boost libraries
# 

# find_package(Boost 1.72.0)
find_package(Boost 1.58.0)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
