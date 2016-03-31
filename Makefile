CFLAGS = -g -O2 -Wall -std=c99

all: client

# Check for os. TODO(stephen): When we add windows this needs to be updated.
OS := $(shell uname)
ifeq ($(OS),Darwin)
# OSX
PLATFORM := "osx"

client:
# Make bundle directory.
	mkdir -p ./build/client/osx/Game.app/Contents/{MacOS,Resources,Frameworks}
# Build exe
	clang ${CFLAGS} \
	-framework OpenGL \
	-F lib -framework SDL2 -framework SDL2_net \
	-rpath @executable_path/../Frameworks \
	src/game_client.c -o ./build/client/osx/Game.app/Contents/MacOS/Game
# Copy in files.
	cp ./etc/Info.plist ./build/osx/Game.app/Contents/
	cp -r ./lib/SDL2.framework ./build/client/osx/Game.app/Contents/Frameworks/
	cp -r ./lib/SDL2_net.framework ./build/client/osx/Game.app/Contents/Frameworks/

server:
# Make directory.
	mkdir -p ./build/server
# Build exe
	clang ${CFLAGS} \
	-F lib -framework SDL2 -framework SDL2_net \
	src/game_server.c -o ./build/server/GameServer

else
# Linux
PLATFORM := "linux"

client:
# Make directory.
	mkdir -p ./build/linux
# Build exe
	clang ${CFLAGS} \
	`sdl2-config --cflags` \
	src/game_client.c -o ./build/linux/Game \
	`sdl2-config --libs`

server:
# Make directory.
	mkdir -p ./build/server
# Build exe
	clang ${CFLAGS} \
	`sdl2-config --cflags` \
	src/game_server.c -o ./build/server/GameServer \
	`sdl2-config --libs`

endif

# Get this working well
# Then hook up travis
# Can make a website in here too, makes it easy to deploy it to the same box.
# Don't know if or how to lock it down yet, don't care.

