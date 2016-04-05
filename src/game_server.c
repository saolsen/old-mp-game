// I'm currently using sdl as my standard library but I may want to actually use linux stuff
// at least for the server.

#define GAME_SERVER
#include "game_platform.h"

int main()
{
    // @TODO: My own logging macros, I don't like these sdl ones.
    SDL_Log("Hello World, This is the server!\n");

    if (SDL_Init(0) == -1) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    if (SDLNet_Init() == -1) {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    UDPsocket udpsock = SDLNet_UDP_Open(1234);
    if (!udpsock) {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(2);
    }

    // I don't think I want to use SDLNet udp channels. That seems useful for like a broadcast
    // thing but I think I want more control over how that's going to happen.
    // I want to do a lot of the stuff enet does, including acks and other ways of doing
    // reliability

    int running = 1;
    while (running) {
        UDPpacket *packet = SDLNet_AllocPacket(1024);

        /* eeSDL_Log("Checking the network for packets\n"); */
        // Handle incoming packets.
        while (SDLNet_UDP_Recv(udpsock, packet) > 0) {
            SDL_Log("Received Packet: %s\n", packet->data);
            printf("channel: %i, data: %s, len: %i, maxlen: %i, status: %i: host: %u, port: %u",
                   packet->channel,
                   packet->data,
                   packet->len,
                   packet->maxlen,
                   packet->status,
                   packet->address.host,
                   packet->address.port);
        }

        // chill 4 a sec.
        SDL_Delay(1000);
    }

    // Cleanup
    SDLNet_UDP_Close(udpsock);
    udpsock = NULL;
    SDLNet_Quit();
}
