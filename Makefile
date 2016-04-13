SERVER_HOST = DEV_HOST

CXXFLAGS = -g -O2 -Wall -std=c++11 -DSERVER_HOST=${SERVER_HOST}

all: client

# Check for os. TODO(stephen): When we add windows this needs to be updated.
OS := $(shell uname)
ifeq ($(OS),Darwin)
# OSX
PLATFORM := "osx"

client: imgui
# Make bundle directory.
	mkdir -p ./build/client/osx/Game.app/Contents/{MacOS,Resources,Frameworks}
# Build dylib
	clang++ ${CXXFLAGS} \
	-I lib/imgui \
	-dynamiclib -undefined dynamic_lookup \
	-o ./build/client/osx/Game.app/Contents/Resources/libgame.dylib \
	src/game.cpp
# Build exe
	clang++ ${CXXFLAGS} \
	-framework OpenGL \
	-F lib -framework SDL2 -framework SDL2_net \
	-rpath @executable_path/../Frameworks \
	-I lib/imgui \
	-I lib \
	-L build/lib -limgui \
	src/game_client.cpp -o ./build/client/osx/Game.app/Contents/MacOS/Game
# Copy in files.
	cp ./etc/Info.plist ./build/client/osx/Game.app/Contents/
	cp -r ./lib/SDL2.framework ./build/client/osx/Game.app/Contents/Frameworks/
	cp -r ./lib/SDL2_net.framework ./build/client/osx/Game.app/Contents/Frameworks/
	cp -r ./data/* ./build/client/osx/Game.app/Contents/Resources/

server:
# Make directory.
	mkdir -p ./build/server/osx
# Build exe
	clang++ ${CXXFLAGS} \
	-F lib -framework SDL2 -framework SDL2_net \
	src/game_server.cpp -o ./build/server/osx/game_server

else
# Linux
PLATFORM := "linux"

client: imgui
# Make directory.
	mkdir -p ./build/client/linux
# Build dylib
	clang++ ${CXXFLAGS} \
	-I lib/imgui \
	-fPIC -shared \
	-o ./build/client/linux/libgame.so \
	src/game.cpp
# Build exe
	clang++ ${CXXFLAGS} \
	`sdl2-config --cflags` \
	-I lib/imgui \
	-I lib \
	-L build/lib \
	src/game_client.cpp -o ./build/client/linux/Game \
	`sdl2-config --libs` -lSDL2_net -lGL -lGLEW -limgui
	cp -r ./data/* ./build/client/linux/

server:
# Make directory.
	mkdir -p ./build/server/linux
# Build exe
	clang++ ${CXXFLAGS} \
	`sdl2-config --cflags` \
	src/game_server.cpp -o ./build/server/linux/game_server \
	`sdl2-config --libs` -lSDL2_net

endif

imgui:
	mkdir -p ./build/lib
	cd build/lib ; clang++ -c ../../lib/imgui/imgui_draw.cpp ../../lib/imgui/imgui.cpp
	ar rcs build/lib/libimgui.a build/lib/imgui_draw.o build/lib/imgui.o

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

tail_logs:
	ssh -A root@game.steveindusteves.com 'tail -f /var/log/gameserver/gameserver.err.log'
