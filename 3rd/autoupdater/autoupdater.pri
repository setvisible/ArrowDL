# github-releases-autoupdater
# https://github.com/VioletGiraffe/github-releases-autoupdater
#

INCLUDEPATH += $$PWD/github-releases-autoupdater/include/


HEADERS += \
	$$PWD/github-releases-autoupdater/src/cautoupdatergithub.h \
	$$PWD/github-releases-autoupdater/src/updateinstaller.hpp

SOURCES += \
	$$PWD/github-releases-autoupdater/src/cautoupdatergithub.cpp

win*:SOURCES += $$PWD/github-releases-autoupdater/src/updateinstaller_win.cpp
mac*:SOURCES += $$PWD/github-releases-autoupdater/src/updateinstaller_mac.cpp
linux*:SOURCES += $$PWD/github-releases-autoupdater/src/updateinstaller_linux.cpp
freebsd:SOURCES += $$PWD/github-releases-autoupdater/src/updateinstaller_freebsd.cpp
