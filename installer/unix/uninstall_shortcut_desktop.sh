#!/bin/bash

echo "Removing Manifest.JSON to Mozilla directory..."

DESTINATION=~/Desktop
TARGET=$DESTINATION/DownZemAll.desktop

rm -f $TARGET

echo "Removed: $TARGET"
echo "Done."
