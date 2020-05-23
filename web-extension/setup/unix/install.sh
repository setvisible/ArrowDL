#!/bin/bash

echo "Copying Manifest.JSON to Mozilla directory..."

SOURCE=./launcher-manifest-firefox.json
DESTINATION=~/.mozilla/native-messaging-hosts

mkdir -p $DESTINATION

TARGET=$DESTINATION/DownRightNow.json

cp $SOURCE $TARGET

REPLACE=/ABSOLUTE/PATH/TO/APP/DIRECTORY/launcher
BY=$PWD/launcher

REPLACE_escaped=$(echo $REPLACE | sed 's_/_\\/_g')
BY_escaped=$(echo $BY | sed 's_/_\\/_g')

sed -i 's/'"$REPLACE_escaped"'/'"$BY_escaped"'/g' $TARGET

echo "Created: $TARGET"
echo "Done."
