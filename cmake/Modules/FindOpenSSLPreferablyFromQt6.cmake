#-------------------------------------------------
# OpenSSL Eventually From Qt6
#-------------------------------------------------
# Tip: Try to always define OpenSSL_ROOT_DIR explicitly if possible...
#
#-------------------------------------------------

if(NOT OpenSSL_ROOT_DIR)

    if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

        # Warning: Qt6_DIR is not always the same:
        # On remote 'Github Actions' runner : Qt6_DIR: D:\a\project\my-qt\Qt\6.3.1\mingw_64
        # On local it points to             : Qt6_DIR: D:\a\project\my-qt\6.3.1\mingw_64\lib\cmake\Qt6

        if(Qt6Core_DIR)

            # Qt6Core_DIR: D:\a\project\my-qt\6.3.1\mingw_64\lib\cmake\Qt6Core
            # TODO use cmake_path?
            get_filename_component(Qt6_OPENSSL_DIR ${Qt6Core_DIR} DIRECTORY)
            get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY) # like "cd ..". Move to parent directory.
            get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
            get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
            get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
            set(OpenSSL_ROOT_DIR "${Qt6_OPENSSL_DIR}/Tools/OpenSSLv3/Win_x64/")

        endif()

    else() # MacOS or Unix Compilers

    endif()

endif()

if(EXISTS "${OpenSSL_ROOT_DIR}/include/openssl/ssl.h")

    if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers

        set(OPENSSL_CRYPTO_LIBRARY "${OpenSSL_ROOT_DIR}/lib/libcrypto.lib" CACHE PATH "Location of the OpenSSL Crypto Lib")
        set(OPENSSL_SSL_LIBRARY "${OpenSSL_ROOT_DIR}/lib/libssl.lib" CACHE PATH "Location of the OpenSSL SSL Lib")
        set(OPENSSL_INCLUDE_DIRS "${OpenSSL_ROOT_DIR}/include" CACHE PATH "Location of the OpenSSL include files")
        set(OPENSSL_CRYPTO_BIN "${OpenSSL_ROOT_DIR}/bin/libcrypto-3-x64.dll" CACHE PATH "Location of the OpenSSL Crypto DLL")
        set(OPENSSL_SSL_BIN "${OpenSSL_ROOT_DIR}/bin/libssl-3-x64.dll" CACHE PATH "Location of the OpenSSL SSL DLL")

    else() # MacOS or Unix Compilers

        # To use preinstalled libssl-dev in Ubuntu:
        # OpenSSL_ROOT_DIR = '/usr'
        set(OPENSSL_CRYPTO_LIBRARY "${OpenSSL_ROOT_DIR}/lib/x86_64-linux-gnu/libcrypto.so" CACHE PATH "Location of the OpenSSL Crypto Lib")
        set(OPENSSL_SSL_LIBRARY "${OpenSSL_ROOT_DIR}/lib/x86_64-linux-gnu/libssl.so" CACHE PATH "Location of the OpenSSL SSL Lib")
        set(OPENSSL_INCLUDE_DIRS "${OpenSSL_ROOT_DIR}/include" CACHE PATH "Location of the OpenSSL include files")
        set(OPENSSL_CRYPTO_BIN "${OpenSSL_ROOT_DIR}/lib/x86_64-linux-gnu/libcrypto.so.3" CACHE PATH "Location of the OpenSSL Crypto DLL")
        set(OPENSSL_SSL_BIN "${OpenSSL_ROOT_DIR}/lib/x86_64-linux-gnu/libssl.so.3" CACHE PATH "Location of the OpenSSL SSL DLL")

        # ...or to build it from Qt:
        # OpenSSL_ROOT_DIR = '~/work/DownZemAll/qt/Qt/Tools/OpenSSLv3/src'
        # set(OPENSSL_CRYPTO_LIBRARY "${OpenSSL_ROOT_DIR}/lib/libcrypto.so" CACHE PATH "Location of the OpenSSL Crypto Lib")
        # set(OPENSSL_SSL_LIBRARY "${OpenSSL_ROOT_DIR}/lib/libssl.so" CACHE PATH "Location of the OpenSSL SSL Lib")
        # set(OPENSSL_INCLUDE_DIRS "${OpenSSL_ROOT_DIR}/include" CACHE PATH "Location of the OpenSSL include files")
        # set(OPENSSL_CRYPTO_BIN "${OpenSSL_ROOT_DIR}/lib/libcrypto.so.3" CACHE PATH "Location of the OpenSSL Crypto DLL")
        # set(OPENSSL_SSL_BIN "${OpenSSL_ROOT_DIR}/lib/libssl.so.3" CACHE PATH "Location of the OpenSSL SSL DLL")

    endif()

    set(OPENSSL_FOUND True)
    set(LIB_EAY ${OPENSSL_CRYPTO_LIBRARY})
    set(SSL_EAY ${OPENSSL_SSL_LIBRARY})

else()

    message("Can't find OpenSSL with Qt6.")
    message("The path doesn't exist: '${OpenSSL_ROOT_DIR}'.")
    message("Tip: OpenSSL's libs and source code must be installed explicitly, see Qt's Tools options.")

endif()

#if(NOT OPENSSL_FOUND)
# Rem: This package points to the system's OpenSSL, but doesn't give the INCLUDE DIR.
#    find_package(OpenSSL REQUIRED)
#endif()
