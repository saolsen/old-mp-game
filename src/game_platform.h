// Non platform stuff for everybody to use. Also defines the interface between the platform layer
// and the game layer.

#ifndef _game_platform_h
#define _game_platform_h

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

typedef enum {
    PacketFlag_RELIABLE,  // will be returned in the order they are sent.
    PacketFlag_ORDERED,   // will always get back the most recent one but may miss packets.
    PacketFlag_UNORDERED, // raw udp, maybe you get it maybe you don't.
} PacketFlag;

typedef struct {
    PacketFlag flag;
    i32 size;       // Size includes this header.
    i32 type;       // User defined type, so that you can cast this header to your packet type.
} PacketHeader;

// Platform specific api.
#define PLATFORM_LOG_MESSAGE(name) void name(const char* format, ...)
typedef PLATFORM_LOG_MESSAGE(PlatformLogMessage);

// Packet must start with a packet header and then have the rest of the data packed after it.
// Probably that means a struct with PacketHeader as the first element.
// size bytes will be sent over the wire as the packet data with the packet flags.
#define PLATFORM_SEND_PACKET(name) void name(PacketHeader* packet)
typedef PLATFORM_SEND_PACKET(PlatformSendPacket);

typedef struct {
    PlatformLogMessage *platformLogMessage;
    PlatformSendPacket *platformSendPacket;
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
    
u8*
arenaPushSize(MemoryArena *arena, i64 size)
{
    assert(arena->size + size < arena->max_size);

    u8 *result = arena->base + arena->size; // @TODO: Align
    arena->size += size;
    
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

    // @TODO: Controller Input

    PacketHeader* packets[256];
    i32 num_packets;
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

#endif
