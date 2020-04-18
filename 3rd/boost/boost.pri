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
include($$PWD/macroFindBoostFromPath.pri)
include($$PWD/macroGetBoostVersion.pri)

# If Boost path is not in the PATH, add the path here. Otherwise leave it empty.
BOOST_ROOT_DIR =


isEmpty(BOOST_ROOT_DIR) {
    # Try to find the Boost directory from the environment variable called PATH on Windows
    win32{
        BOOST_ROOT_DIR = $$findBoostFromPath()
    }

    # Try to find Boost in /usr/include/boost on Unix
    unix{
        ITEM = /usr/include
        exists( $$system_path( $$ITEM/boost/version.hpp ) ) {
            BOOST_ROOT_DIR = $$ITEM
        }
    }
}

isEmpty( BOOST_ROOT_DIR ) {
    error("Variable 'BOOST_ROOT_DIR' should not be empty.")
}
!exists( $$BOOST_ROOT_DIR/boost/version.hpp ) {
    error("Cannot find directory '$${BOOST_ROOT_DIR}'. Variable 'BOOST_ROOT_DIR' should contain a valid path to Boost.")
}

BOOST_VERSION = $$getBoostVersion( $$BOOST_ROOT_DIR/boost/version.hpp )
!equals(BOOST_VERSION, "107200") {
    error("'BOOST_VERSION' should be 1.72.0, but is $${BOOST_VERSION}.")
}

INCLUDEPATH += $${BOOST_ROOT_DIR}
