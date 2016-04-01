#define GAME_CLIENT
#include "game_platform.h"

#include "game.h"

// @TODO: Have a hot reloading version for debugging.
#include "game.c"

char* PROD_HOST = "game.steveindusteves.com";
char* DEV_HOST = "localhost";    

// Set this in build system for prod builds to be the real server.
#ifndef SERVER_HOST
#define SERVER_HOST DEV_HOST
#endif

// Set up platform API. Probably could be shared between client and server.
//#define PLATFORM_LOG_MESSAGE(name) void name(char* format, ...)
PLATFORM_LOG_MESSAGE(platformLogMessage)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

int main()
{
    PlatformAPI platform_api;
    platform_api.platformLogMessage = &platformLogMessage;
    
    SDL_Log("Hello World, This is the client!\n");

    if (SDL_Init(0) == -1) {
        SDL_LogError("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    if (SDLNet_Init() == -1) {
        SDL_LogError("SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    UDPsocket udpsock = SDLNet_UDP_Open(0);
    if (!udpsock) {
        SDL_LogError("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(2);
    }

    // Ping the server.
    // I don't really like that you have to call this to allocate packets. I'd rather do the memory management myself. I had that system with an arena that was working very nicely. That way I don't have to resize.
    char *data = "Hello I am the client";
    int len = strlen(data) + 1;
    UDPpacket *packet = SDLNet_AllocPacket(len);
    if (!packet) {
        SDL_LogError("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
    }

    memcpy(packet->data, data, len);
    packet->len = len;

    IPaddress address;
    SDLNet_ResolveHost(&address, SERVER_HOST, 1234);
    packet->address.host = address.host;
    packet->address.port = address.port;

    SDLNet_UDP_Send(udpsock, -1, packet);
    SDLNet_FreePacket(packet);

    // Set up normal sdl stuff.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError("Error initializing SDL: %s", SDL_GetError());
    }

    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window = SDL_CreateWindow("Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1024,
                                          768,
                                          SDL_WINDOW_OPENGL |
                                          SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        SDL_LogError("Error creating window: %s\n", SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        SDL_LogError("Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            default:
                break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        GameMemory memory;
        memory.platform_api = &platform_api;
        gameUpdateAndRender(&memory);

        SDL_GL_SwapWindow(window);
    }
    
}
