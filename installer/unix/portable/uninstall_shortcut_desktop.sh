#!/bin/bash

remove_shortcut () {
    TARGET=$1

    if [ -f "$TARGET" ]; then
        rm -f $TARGET
        echo "Removed: $TARGET"
        echo
    else
        echo "Canceled: Already removed: $TARGET"
        echo
    fi
}

# #######################################################
echo "Removing launcher shortcut..."

if [[ "$EUID" = 0 ]]; then # if root
    LAUNCHER_DESTINATION=/usr/share/applications
else
    LAUNCHER_DESTINATION=~/.local/share/applications
fi

mkdir -p $LAUNCHER_DESTINATION

TARGET=$LAUNCHER_DESTINATION/ArrowDL.desktop

remove_shortcut $TARGET


# #######################################################
echo "Removing desktop shortcut..."

DESTINATION=$(xdg-user-dir DESKTOP)
TARGET=$DESTINATION/ArrowDL.desktop

remove_shortcut $TARGET
