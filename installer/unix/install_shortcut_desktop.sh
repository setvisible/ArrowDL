#!/bin/bash

echo "Creating DownZemAll.desktop..."

SOURCE=./DownZemAll.desktop
DESTINATION=~/Desktop

mkdir -p $DESTINATION

TARGET=$DESTINATION/DownZemAll.desktop

cp $SOURCE $TARGET

REPLACE=/ABSOLUTE/PATH/TO/APP/DIRECTORY
BY=$PWD

REPLACE_escaped=$(echo $REPLACE | sed 's_/_\\/_g')
BY_escaped=$(echo $BY | sed 's_/_\\/_g')

sed -i 's/'"$REPLACE_escaped"'/'"$BY_escaped"'/g' $TARGET

echo "Created: $TARGET"
echo "Done."
