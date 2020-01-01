include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/autoupdater/github-releases-autoupdater/include/)

if(MSVC OR MSYS OR MINGW) # for detecting Windows compilers
    set(SPECIFIC "updateinstaller_win.cpp" )
else() # MacOS or Unix Compilers
    if(UNIX AND NOT APPLE) # Linux of FreeBSD
        if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD") # FreeBSD
            set(SPECIFIC "updateinstaller_freebsd.cpp" )
        else() # Linux 
            set(SPECIFIC "updateinstaller_linux.cpp" )         
        endif()
    else() # MacOS 
        set(SPECIFIC "updateinstaller_mac.cpp" )  
    endif()
endif()


set(AUTOUPDATER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/autoupdater/github-releases-autoupdater/src/${SPECIFIC}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/autoupdater/github-releases-autoupdater/src/cautoupdatergithub.cpp
    )

set(AUTOUPDATER_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/autoupdater/github-releases-autoupdater/src/cautoupdatergithub.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/autoupdater/github-releases-autoupdater/src/updateinstaller.hpp
    )
