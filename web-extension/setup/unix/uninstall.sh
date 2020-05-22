#!/bin/bash

echo "Removing Manifest.JSON to Mozilla directory..."

DESTINATION=~/.mozilla/native-messaging-hosts
TARGET=$DESTINATION/DownRightNow.json

rm -f $TARGET

echo "Removed: $TARGET"
echo "Done."
