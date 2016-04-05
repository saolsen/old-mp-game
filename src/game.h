// Non platform stuff for everybody to use. Also defines the interface between the platform layer
// and the game layer.
#ifndef _game_h
#define _game_h

// Only rely on c libraries. tbd how to render shit, prolly just gonna be platform apis.
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <string.h>

// Shared libraries
#include "nanovg.h"

typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    r32;
typedef double   r64;

// Platform specific api.
// I think I can do the define thing here to make it easy to then later make this function in the
// platform code.
#define PLATFORM_LOG_MESSAGE(name) void name(char* format, ...)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

typedef struct {
    PlatformLogMessage *platformLogMessage;
} PlatformAPI;

typedef struct {
    int todo;
} MemoryArena;

typedef struct {
    PlatformAPI *platform_api;

    MemoryArena *persistent_arena;
    MemoryArena *transient_arena;
    
    r32 dt;
    u64 ticks;
    
} GameMemory;

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory *memory)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

extern PlatformAPI *platform;
extern GameUpdateAndRender gameUpdateAndRender;

// Helpers that use platform api, can only be called once platform is set.
#define INFO(format, ...) platform->platformLogMessage("[INFO] (%s:%d) " format "\n", \
                                                       __FILE__, __LINE__, ##__VA_ARGS__)

#endif
