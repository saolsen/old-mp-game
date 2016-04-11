/*
  Platform level includes, basically anything external to the game is included here and only used from the
  game_client.c and game_server.c files.
 */
#ifndef _game_platform_h
#define _game_platform_h

#include "game.h"

// Everybody stuff
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

// @TODO: Use sdl version of stat, not sure if there even is one tho, maybe this is just a dev
// feature.
#include <sys/stat.h>

#ifdef _WIN32
#endif

#ifdef __APPLE__
#include <SDL2_net/SDL_net.h>

#endif // apple both

#ifdef __linux__
#include <SDL2/SDL_net.h>
#endif // linux both

// Client stuff
#ifdef GAME_CLIENT
#include <SDL2/SDL_loadso.h>

#ifdef _WIN32
#endif

#ifdef __APPLE__
    #include <OpenGL/gl3.h>
const char* library = "libgame.dylib";
#endif // apple client

#ifdef __linux__
    #define GL_PROTOTYPES 1
    #define GLEW_STATIC
    #include <GL/glew.h>
const char* library = "libgame.so";
#endif // linux client

#include "imgui_impl_sdl_gl3.cpp"

#endif // client

// Server stuff
#ifdef GAME_SERVER

#ifdef _WIN32
#endif

#ifdef __APPLE__

#endif // apple server

#ifdef __linux__

#endif // linux server
#endif // server

#endif // _game_platform_h
