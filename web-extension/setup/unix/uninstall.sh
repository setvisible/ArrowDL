#!/bin/bash

uninstall_manifest () {
    local TITLE=$1
    local SOURCE=$2
    local DESTINATION=$3
    local TARGET_NAME=$4

    echo "Removing Manifest.JSON to $TITLE directory..."

    TARGET=$DESTINATION/$TARGET_NAME.json

    rm -f $TARGET

    echo "Removed: $TARGET"
    echo "Done."
    echo ""
}

TITLE='Mozilla' 
SOURCE=./launcher-manifest-firefox.json
DESTINATION=~/.mozilla/native-messaging-hosts
TARGET_NAME="ArrowDL"
uninstall_manifest $TITLE $SOURCE $DESTINATION $TARGET_NAME 


TITLE='Chromium' 
SOURCE=./launcher-manifest-chrome.json
DESTINATION=~/.config/chromium/NativeMessagingHosts/
TARGET_NAME="com.setvisible.arrowdl"
uninstall_manifest $TITLE $SOURCE $DESTINATION $TARGET_NAME 
