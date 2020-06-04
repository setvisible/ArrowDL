#!/bin/bash

add_shortcut () {
    SOURCE=$1
    TARGET=$2

    if [ ! -f "$SOURCE" ]; then
        echo "Error: Source can't be found: $TARGET"
        return
    fi

    TARGET_DIR=$(dirname "${TARGET}")
    if [ ! -d "$TARGET_DIR" ]; then
        echo "Error: Destination directory doesn't exist: $TARGET_DIR"
        return
    fi

    if [ -f "$TARGET" ]; then
        echo "Canceled: Already exists: $TARGET"
        echo
        return
    fi

    cp $SOURCE $TARGET
    chmod +x $TARGET

    REPLACE=/ABSOLUTE/PATH/TO/APP/DIRECTORY
    BY=$PWD

    REPLACE_escaped=$(echo $REPLACE | sed 's_/_\\/_g')
    BY_escaped=$(echo $BY | sed 's_/_\\/_g')

    sed -i 's/'"$REPLACE_escaped"'/'"$BY_escaped"'/g' $TARGET

    if [ -f "$TARGET" ]; then
        echo "Created: $TARGET"
        echo
    else
        echo "Error: Can't create $TARGET"
        echo
    fi
}

# #######################################################
echo "Creating launcher shortcut..."

if [[ "$EUID" = 0 ]]; then # if root
    LAUNCHER_DESTINATION=/usr/share/applications
else
    LAUNCHER_DESTINATION=~/.local/share/applications
fi

mkdir -p $LAUNCHER_DESTINATION

SOURCE=./DownZemAll.desktop
TARGET=$LAUNCHER_DESTINATION/DownZemAll.desktop

add_shortcut $SOURCE $TARGET


# #######################################################
echo "Creating desktop shortcut..."

SOURCE=./DownZemAll.desktop
DESTINATION=$(xdg-user-dir DESKTOP)
TARGET=$DESTINATION/DownZemAll.desktop

add_shortcut $SOURCE $TARGET
