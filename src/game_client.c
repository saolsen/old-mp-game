#define GAME_CLIENT
#include "game_platform.h"

char* PROD_HOST = "game.steveindusteves.com";
char* DEV_HOST = "localhost";    

// Set this in build system for prod builds to be the real server.
#ifndef SERVER_HOST
#define SERVER_HOST DEV_HOST
#endif

int main()
{
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
}
