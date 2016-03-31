/*
  Platform level includes, basically anything external to the game is included here and only used from the
  game_client.c and game_server.c files.
 */
#ifndef _game_platform_h
#define _game_platform_h

// There's some trickyness to including the right headers on different os's.
#include <SDL2/SDL.h>

#ifdef __APPLE__
#include <SDL2_net/SDL_net.h>
#endif

#ifdef __linux__
#include <SDL2/SDL_net.h>
#endif

#endif // _game_platform_h
