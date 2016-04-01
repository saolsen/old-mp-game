#!/bin/bash
echo "DEPLOYING"
cd $(cat version.txt)
tar -xvzf game-server-$(cat ../version.txt).tar.gz
mv linux server

tar -xvzf game-site-$(cat ../version.txt).tar.gz

sed -i "1s/.*/game.steveindusteves.com/" ./web/Caddyfile

# TODO: maybe add some builds in here first?
# TODO: rsync
rsync -vaI web /home/game/
rsync -vaI server /home/game/

chown game /home/game/server
chown game /home/game/web

rsync -vaI game-* /home/game/web/builds

supervisorctl restart caddy &
supervisorctl restart gameserver &

