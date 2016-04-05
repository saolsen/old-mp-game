SERVER_HOST = DEV_HOST

CFLAGS = -g -O2 -Wall -std=c11 -DSERVER_HOST=${SERVER_HOST}

all: client

# Check for os. TODO(stephen): When we add windows this needs to be updated.
OS := $(shell uname)
ifeq ($(OS),Darwin)
# OSX
PLATFORM := "osx"

client: nanovg
# Make bundle directory.
	mkdir -p ./build/client/osx/Game.app/Contents/{MacOS,Resources,Frameworks}
# Build exe
	clang ${CFLAGS} \
	-framework OpenGL \
	-F lib -framework SDL2 -framework SDL2_net \
	-rpath @executable_path/../Frameworks \
	-I lib/nanovg/src \
	-L build/lib -l nanovg \
	src/game_client.c -o ./build/client/osx/Game.app/Contents/MacOS/Game
# Copy in files.
	cp ./etc/Info.plist ./build/client/osx/Game.app/Contents/
	cp -r ./lib/SDL2.framework ./build/client/osx/Game.app/Contents/Frameworks/
	cp -r ./lib/SDL2_net.framework ./build/client/osx/Game.app/Contents/Frameworks/
	cp -r ./data/* ./build/client/osx/Game.app/Contents/Resources/

server:
# Make directory.
	mkdir -p ./build/server/osx
# Build exe
	clang ${CFLAGS} \
	-F lib -framework SDL2 -framework SDL2_net \
	src/game_server.c -o ./build/server/osx/game_server

else
# Linux
PLATFORM := "linux"

client: nanovg
# Make directory.
	mkdir -p ./build/client/linux
# Build exe
	clang ${CFLAGS} \
	`sdl2-config --cflags` \
	-I lib/nanovg/src \
	-L build/lib -l nanovg \
	src/game_client.c -o ./build/client/linux/Game \
	`sdl2-config --libs` -lSDL2_net -lGL -lGLEW -lm
	cp -r ./data/* ./build/client/linux/

server:
# Make directory.
	mkdir -p ./build/server/linux
# Build exe
	clang ${CFLAGS} \
	`sdl2-config --cflags` \
	src/game_server.c -o ./build/server/linux/game_server \
	`sdl2-config --libs` -lSDL2_net

endif

nanovg:
	mkdir -p ./build/lib/
	cc -Wall -c lib/nanovg/src/nanovg.c -o build/lib/nanovg.o
	ar -cvq build/lib/libnanovg.a build/lib/nanovg.o

# The site is hosted with Caddy. Right now it's hella simple.
site:
	cp ./version.txt ./web/version.txt

# This you call against a brand new server that you want to use.
setup_server:
	ssh -A root@game.steveindusteves.com 'bash -s' < scripts/setup_server_host.sh

# This you can't just call, first you gotta do some manuel work which is to make a directory
# of your version. eg, 0.0.1 and then copy down all the build artifacts from github.
deploy:
	scp version.txt root@game.steveindusteves.com:/root/version.txt
	scp -r `cat version.txt` root@game.steveindusteves.com:/root/`cat version.txt`
	ssh -A root@game.steveindusteves.com 'bash -s' < scripts/update_server_host.sh
