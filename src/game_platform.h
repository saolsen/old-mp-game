// Non platform stuff for everybody to use. Also defines the interface between the platform layer
// and the game layer.


// I really wanna call this game_platform.h, can call that game_dependencies.h
#ifndef _game_h

#ifdef __cplusplus
extern "C" {
#endif

// Only rely on c libraries. tbd how to render shit, prolly just gonna be platform apis.
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <string.h>

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
#define PLATFORM_LOG_MESSAGE(name) void name(const char* format, ...)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

typedef struct {
    PlatformLogMessage *platformLogMessage;
} PlatformAPI;

typedef struct {
    u8 *base;
    i64 size;
    i64 max_size;
} MemoryArena;

void
arenaAllocate(MemoryArena* arena, u8* memory, i64 size)
{
    arena->base = memory;
    arena->size = 0;
    arena->max_size = size;
}

#include <stdio.h>
    
u8*
arenaPushSize(MemoryArena *arena, i64 size)
{
    printf("ArenaPushing: %lli, ", size);
    assert(arena->size + size < arena->max_size);

    u8 *result = arena->base + arena->size; // @TODO: Align
    arena->size += size;

    printf("ArenaSpaceRemaining: %lli\n", arena->max_size - arena->size);
    
    return result;
}

#define arenaPushType(arena, type) (type*)arenaPushSize(arena, sizeof(type))
#define arenaPushArray(arena, type, n) (type*)arenaPushSize(arena, sizeof(type)*n);

typedef struct {
    PlatformAPI *platform_api;

    MemoryArena *persistent_arena;
    MemoryArena *transient_arena;

    // window size
    i32 window_width;
    i32 window_height;

    i32 display_width;
    i32 display_height;
    
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

// Shared helper macros.
#define LEN(e) (sizeof(e) / sizeof(e[0]))
    
#ifdef __cplusplus
}
#endif

#define _game_h
#endif
