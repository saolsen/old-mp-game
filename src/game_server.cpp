// I'm currently using sdl as my standard library but I may want to actually use linux stuff
// at least for the server.

#define GAME_SERVER
#include "game_system.h"

int main()
{
    fprintf(stderr, "Hello, this is the server!\n");

    if (SDL_Init(0) == -1) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Try using enet instead.

    if (enet_initialize() != 0) {
        fprintf(stderr, "Error initializing ENet\n");
        exit(1);
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost *server;

    address.host = ENET_HOST_ANY;
    address.port = 1234;

    server = enet_host_create(&address, 64, 4, 0, 0);

    if (server == NULL) {
        fprintf(stderr, "Error creating ENet server host\n");
        exit(1);
    }

    bool running = true;
    // ENetPacket *packet;
    while (running) {
        ENetEvent net_event;
        while (enet_host_service(server, &net_event, 0) > 0) {
            switch (net_event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                fprintf(stderr, "A client has connected!\n");
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                fprintf(stderr, "A client has dis connected!\n");
                break;
            default:
                break;
            }
        }
        
        // chill 4 a sec.
        enet_host_flush(server);
        SDL_Delay(1000);
    }

    enet_host_destroy(server);
    exit(0);
}



    // if (SDLNet_Init() == -1) {
    //     fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    //     exit(2);
    // }

    // UDPsocket udpsock = SDLNet_UDP_Open(1234);
    // if (!udpsock) {
    //     fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    //     exit(2);
    // }

    // // I don't think I want to use SDLNet udp channels. That seems useful for like a broadcast
    // // thing but I think I want more control over how that's going to happen.
    // // I want to do a lot of the stuff enet does, including acks and other ways of doing
    // // reliability

    // int running = 1;
    // while (running) {
    //     UDPpacket *packet = SDLNet_AllocPacket(1024);

    //     /* eeSDL_Log("Checking the network for packets\n"); */
    //     // Handle incoming packets.
    //     while (SDLNet_UDP_Recv(udpsock, packet) > 0) {
    //         SDL_Log("Received Packet: %s\n", packet->data);
    //         printf("channel: %i, data: %s, len: %i, maxlen: %i, status: %i: host: %u, port: %u",
    //                packet->channel,
    //                packet->data,
    //                packet->len,
    //                packet->maxlen,
    //                packet->status,
    //                packet->address.host,
    //                packet->address.port);
    //     }

    //     // chill 4 a sec.
    //     SDL_Delay(1000);
    // }

    // // Cleanup
    // SDLNet_UDP_Close(udpsock);
    // udpsock = NULL;
    // SDLNet_Quit();
