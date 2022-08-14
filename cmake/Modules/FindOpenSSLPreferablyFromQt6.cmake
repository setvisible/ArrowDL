#-------------------------------------------------
# OpenSSL Eventually From Qt6
#-------------------------------------------------
if(Qt6_DIR)
    # TODO use cmake_path
    get_filename_component(Qt6_OPENSSL_DIR ${Qt6_DIR} DIRECTORY)
    get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
    get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
    get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
    get_filename_component(Qt6_OPENSSL_DIR ${Qt6_OPENSSL_DIR} DIRECTORY)
    set(Qt6_OPENSSL_DIR "${Qt6_OPENSSL_DIR}/Tools/OpenSSL/Win_x64/")

    if (EXISTS ${Qt6_OPENSSL_DIR})
        set(OPENSSL_CRYPTO_LIBRARY "${Qt6_OPENSSL_DIR}/lib/libcrypto.lib" CACHE PATH "Location of the OpenSSL Crypto Lib")
        set(OPENSSL_SSL_LIBRARY "${Qt6_OPENSSL_DIR}/lib/libssl.lib" CACHE PATH "Location of the OpenSSL SSL Lib")
        set(OPENSSL_INCLUDE_DIRS "${Qt6_OPENSSL_DIR}/include" CACHE PATH "Location of the OpenSSL include files")
        set(OPENSSL_CRYPTO_BIN "${Qt6_OPENSSL_DIR}/bin/libcrypto-1_1-x64.dll" CACHE PATH "Location of the OpenSSL Crypto DLL")
        set(OPENSSL_SSL_BIN "${Qt6_OPENSSL_DIR}/bin/libssl-1_1-x64.dll" CACHE PATH "Location of the OpenSSL SSL DLL")
        set(OPENSSL_FOUND True)
        set(LIB_EAY ${OPENSSL_CRYPTO_LIBRARY})
        set(SSL_EAY ${OPENSSL_SSL_LIBRARY})
        #message("OpenSSL found on Qt6")
    else()
        message("Can't find OpenSSL with Qt6. The path doesn't exist: '${Qt6_OPENSSL_DIR}'")
    endif()
endif()

if (NOT OPENSSL_FOUND)
    find_package(OpenSSL REQUIRED)
endif()
