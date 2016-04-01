#!/bin/bash
# Sets up a new host to run the server.
# Should be run as root after spinning the server up.
# Assumes ubuntu Ubuntu 14.04.4 x64. I currently run this on digitalocean.

set -e

# update server
apt-get -y update
apt-get -y upgrade

# install dependencies
apt-get -y install git
# apt-get -y install upstart
apt-get -y install libsdl2-dev libsdl2-net-dev
apt-get -y install supervisor

service supervisor start &

# install caddy
wget https://github.com/mholt/caddy/releases/download/v0.8.2/caddy_linux_amd64.tar.gz
tar -xvzf caddy_linux_amd64.tar.gz caddy
cp caddy /usr/local/bin/caddy
setcap cap_net_bind_service=+ep /usr/local/bin/caddy
    
cat > /etc/supervisor/conf.d/caddy.conf <<- EOF
[program:caddy]
command=caddy -conf="/home/game/web/Caddyfile" -agree -email steve.olsen@gmail.com
directory=/home/game/web
autostart=false
autorestart=true
startretries=3
stderr_logfile=/var/log/caddy/caddy.err.log
stdout_logfile=/var/log/caddy/caddy.out.log
user=game

EOF
mkdir -p /var/log/caddy

cat > /etc/supervisor/conf.d/gameserver.conf <<- EOF
[program:gameserver]
command=/home/game/server/game_server
directory=/home/game/server
autostart=false
autorestart=true
startretries=3
stderr_logfile=/var/log/gameserver/gameserver.err.log
stdout_logfile=/var/log/gameserver/gameserver.out.log
user=game

EOF
mkdir -p /var/log/gameserver

# let non root users user supervisorctl
useradd -m game

supervisorctl reread
supervisorctl update

# chmod a+rw /var/run/supervisor.sock

# # echo "game ALL=NOPASSWD:/usr/local/bin/restart_game.sh" | (EDITOR="tee -a" visudo)
# # echo "game ALL=NOPASSWD:/usr/local/bin/restart_caddy.sh" | (EDITOR="tee -a" visudo)

# # echo "game ALL=NOPASSWD:/usr/sbin/service caddy *" | (EDITOR="tee -a" visudo)
# # echo "game ALL=NOPASSWD:/usr/sbin/service gameserver *" | (EDITOR="tee -a" visudo)

# mkdir -p /home/game/.ssh
# cat > /home/game/.ssh/authorized_keys <<- EOF
# ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDC4ZixdO678yOLXVPxrQd4VAp6wZGxdXU4drDDKgEOEO6F51QIh6z3WuVD0m3+3EvBiEqMC3c4iVpEdUB9wOzQYnfNIsSlNvDtf2BbAUAKe6yg/bV95ifGtsN8mP46Mai1h++pURFa5tu4SsnBcL0hz7oB8TjPON4P8fdoA67eCz8y6GXU9cBqrEf8bC7pBcJhcrfeb4A55KDwQ/5hri5KMmSVzCYptkX2ZZ9ZGwxfxhx6DEogNnqEtENF9eOKm2PQoMEVfGNNzaTlucMjnSLKV+BIka1LulEpsd4Pi1LNcHmOdhaKBmtEInMyyxDNFN4NoL3k6NgrYa8xNeWP9+RZ steveo@Stephens-MacBook-Pro.local
# EOF
# chown game /home/game/.ssh
# chown game /home/game/.ssh/authorized_keys

shutdown -r now
