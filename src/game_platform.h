/*
  Platform level includes, basically anything external to the game is included here and only used from the
  game_client.c and game_server.c files.
 */
#ifndef _game_platform_h
#define _game_platform_h

// Everybody stuff
#include <SDL2/SDL.h>

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

#ifdef _WIN32
#endif

#ifdef __APPLE__

#include <OpenGL/gl3.h>

#endif // apple client

#ifdef __linux__

#define GL3_PROTOTYPES 1
#include <GL/glew.h>

#endif // linux client
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
