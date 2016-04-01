#!/bin/bash
# This is the script called on travis.

# SCP new files over
# Call update_server_host on server to update and restart.

mkdir $(cat version.txt)
# copy local files in.
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    cp build/client/game-linux-$(cat version.txt).tar.gz $(cat version.txt)/
    cp build/server/game-server-$(cat version.txt).tar.gz $(cat version.txt)/
    cp game-site-$(cat version.txt).tar.gz $(cat version.txt)/
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    cp ./build/client/game-osx-$(cat version.txt).zip $(cat version.txt)/
fi

scp -i game_rsa -r $(cat version.txt) root@game.steveindusteves.com:/root/$(cat version.txt) -o stricthostkeychecking=no
