#ifndef _game_h
#define _game_h

// There's some trickyness to including the right headers on different os's.
#include <SDL2/SDL.h>

#ifdef __APPLE__
#include <SDL2_net/SDL_net.h>
#endif

#ifdef __linux__
#include <SDL2/SDL_net.h>
#endif

#endif // _game_h
