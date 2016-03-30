#!/bin/bash
mkdir -p lib

# Figure out if we are linux or osx (or some day, windows)
if [ "$(uname)" == "Darwin" ]; then
    # For mac we need the SDL and SDL2 frameworks. I just included them in the repo
    # because I don't want to depend on stuff.
    echo "No osx dependencies!"
    
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    # For linux we need to install SDL and SDL_Net.
    sudo apt-get -qq update
    sudo apt-get install build-essential -y
    sudo apt-get install libsdl2-dev -y
    sudo apt-get install libsdl2-net-dev -y
    
fi
# For windows I think there's an sdl dll I include or something.
