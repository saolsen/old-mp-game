#define GAME_CLIENT
#include "game_system.h"

#include "imgui_demo.cpp"

const char* PROD_HOST = "game.steveindusteves.com";
const char* DEV_HOST = "localhost";

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)

// Set this in build system for prod builds to be the real server.
#ifndef SERVER_HOST
#define SERVER_HOST DEV_HOST
#endif

// Dll reloading
struct CurrentGame {
    void *handle;
    ino_t id;
    GameUpdateAndRender *gameUpdateAndRender;
};

bool
gameReload(CurrentGame* current_game)
{
    bool game_reloaded = false;

    char path_buf[256];
    char *base_path = SDL_GetBasePath();
    snprintf(path_buf, 256, "%s%s", base_path, library);

    struct stat attr;
    if ((stat(path_buf, &attr) == 0) && (current_game->id != attr.st_ino)) {
        fprintf(stderr, "New library to load.\n");

        if (current_game->handle) {
            SDL_UnloadObject(current_game->handle);
        }

        void *handle = SDL_LoadObject(path_buf);

        if (handle) {
            current_game->handle = handle;
            current_game->id = attr.st_ino;

            GameUpdateAndRender *game_update_fn = (GameUpdateAndRender*)SDL_LoadFunction(handle, "gameUpdateAndRender");
            if (game_update_fn != NULL) {
                current_game->gameUpdateAndRender = *game_update_fn;
                fprintf(stderr, "Reloaded Game Library\n");
                game_reloaded = true;
            } else {
                fprintf(stderr, "Error loading gameUpdateAndRender function.\n");
                SDL_UnloadObject(handle);
                current_game->handle = NULL;
                current_game->id = 0;
            }
        } else {
            fprintf(stderr, "Error loading game dll\n");
            current_game->handle = NULL;
            current_game->id = 0;
        }
    }

    return game_reloaded;
}

// Set up platform API. Probably could be shared between client and server.
PLATFORM_LOG_MESSAGE(platformLogMessage)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

ENetPeer *enet_peer;

PLATFORM_SEND_PACKET(platformSendPacket)
{
    //PacketHeader *packet;
    u32 enet_flag;
    i32 channel; // @TODO: decide this.
    switch(packet->flag) {
    case PacketFlag_RELIABLE:
        enet_flag = ENET_PACKET_FLAG_RELIABLE;
        channel = 1;
        break;
    case PacketFlag_ORDERED:
        enet_flag = 0;
        channel = 1;
        break;
    case PacketFlag_UNORDERED:
        enet_flag = ENET_PACKET_FLAG_UNSEQUENCED;
        channel = 1;
        break;
    }
    
    ENetPacket *enet_packet = enet_packet_create(packet,
                                                 packet->size,
                                                 enet_flag);
    enet_peer_send(enet_peer, channel, enet_packet);
}


int main()
{
    CurrentGame game = {.handle = NULL,
                        .id = 0,
                        .gameUpdateAndRender = NULL };
    gameReload(&game);
    
    // const char * msg1 = "Hello wudup with you";
    // add_chat_msg(msg1, strlen(msg1));
    // const char * msg2 = "This is the message bumba 2";
    // add_chat_msg(msg2, strlen(msg2));
    
    PlatformAPI platform_api;
    platform_api.platformLogMessage = platformLogMessage;
    platform_api.platformSendPacket = platformSendPacket;

    if (SDL_Init(0) == -1) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // if (SDLNet_Init() == -1) {
    //     fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
    //     exit(2);
    // }

    // UDPsocket udpsock = SDLNet_UDP_Open(0);
    // if (!udpsock) {
    //     fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    //     exit(2);
    // }

    // // Ping the server.
    // // I don't really like that you have to call this to allocate packets. I'd rather do the memory management myself. I had that system with an arena that was working very nicely. That way I don't have to resize.
    // const char *data = "Hello I am the client";
    // int len = strlen(data) + 1;
    // UDPpacket *packet = SDLNet_AllocPacket(len);
    // if (!packet) {
    //     fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
    // }

    // memcpy(packet->data, data, len);
    // packet->len = len;

    // IPaddress address;
    // SDLNet_ResolveHost(&address, SERVER_HOST, 1234);
    // packet->address.host = address.host;
    // packet->address.port = address.port;

    // SDLNet_UDP_Send(udpsock, -1, packet);
    // SDLNet_FreePacket(packet);

    // Set Up Enet
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        exit(1);
    }

    ENetHost *client;
    client = enet_host_create(NULL,
                              1,
                              4,
                              57600 / 8, // from tutorial, this is like a 56k modem bandwidth.
                              14400 / 8);

    if (client == NULL) {
        fprintf(stderr, "Error occured while trying to create an ENet client host.\n");
    }

    ENetAddress address;

    enet_address_set_host(&address, SERVER_HOST);
    address.port = 1234;

    enet_peer = enet_host_connect(client, &address, 4, 0);

    if (enet_peer == NULL) {
        fprintf(stderr, "Error, no available peers for initiating an ENet connection.\n");
        exit(1);
    }

    // Set up normal sdl stuff.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Error initializing SDL: %s", SDL_GetError());
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window *window = SDL_CreateWindow("Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1024,
                                          768,
                                          SDL_WINDOW_OPENGL |
                                          SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        fprintf(stderr, "Error creating opengl context: %s\n", SDL_GetError());
    }

    // @TODO: I think windows is going to need this too.
#ifdef __linux__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        fprintf(stderr, "Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    SDL_DisplayMode display_mode;
    SDL_GetDisplayMode(0, 0, &display_mode);
    fprintf(stderr, "Display Mode update hz: %i\n", display_mode.refresh_rate);

    float game_update_hz = (float)display_mode.refresh_rate;
    float target_seconds_per_frame = 1.0f / game_update_hz;

    // Setup imgui
    ImGui_ImplSdlGL3_Init(window);

    // Setup game memory.
    GameMemory *memory = (GameMemory*)calloc(1, sizeof(GameMemory));
    // Having the arenas allocated by the platform because I think I want them to be
    // resizable. Maybe that's dumb and I should just pass the memory and give a platform
    // api function to allocate more.
    MemoryArena *persistent_arena = (MemoryArena*)calloc(1, sizeof(MemoryArena));
    MemoryArena *transient_arena = (MemoryArena*)calloc(1, sizeof(MemoryArena));

    memory->platform_api = &platform_api;
    
    memory->persistent_arena = persistent_arena;
    memory->transient_arena = transient_arena;
    const long persistent_arena_size = MEGABYTES(128);
    arenaAllocate(memory->persistent_arena,
                  (u8*)calloc(1, persistent_arena_size), // @TODO: No idea yet.
                  persistent_arena_size);
    arenaAllocate(memory->transient_arena,
                  (u8*)calloc(1, persistent_arena_size), // @TODO: No idea yet.
                  persistent_arena_size);

    int update_time = 0;
    int frame_time = 0;

    uint64_t last_counter = SDL_GetPerformanceCounter();
    int last_start_time = SDL_GetTicks();

    ENetPacket *packets[256];
    int num_packets;
 
    int running = 1;
    while (running) {
        int start_time = SDL_GetTicks();
        frame_time = start_time - last_start_time;
        last_start_time = start_time;

        gameReload(&game);
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSdlGL3_ProcessEvent(&event);
            switch(event.type) {
                
            case SDL_QUIT:
                running = 0;
                break;
            default:
                break;
            }
        }

        memory->num_packets = 0;
        num_packets = 0;
        ENetEvent net_event;
        while (num_packets < 256 && enet_host_service(client, &net_event, 0) > 0) {
            switch (net_event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                fprintf(stderr, "Connected!\n");
            } break;

            case ENET_EVENT_TYPE_RECEIVE: {
                // @NOTE: Packets are only valid for the frame they come in on. If you want them
                // longer you better copy them.
                if (net_event.channelID > 0) {
                    ENetPacket **enet_packet = packets + num_packets++;
                    *enet_packet = net_event.packet;
                    
                    PacketHeader **packet = memory->packets + memory->num_packets++;
                    *packet = (PacketHeader*)net_event.packet->data;
                }
            } break;
            default:
                break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // @TODO: Handle high dpi and window resizing.

        ImGui_ImplSdlGL3_NewFrame(window);

        // ImGui::ShowTestWindow();
        // Look at the console example for doing a chat widget.
        // Put chat in the lower left, no resize, no border no menu etc...

        // Test
        ImGui::Begin("Welcome To The Game");
        ImGui::Text("Performance %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
        ImGui::End();

        // There's finna be 2 ui needs.
        // First is ingame stuff, this I can still use imgui for if i'm careful.
        // I need chat and connection stuff and like menu stuff (options) whatever.
        // I should style this and position it absolutely and make it look good.

        // The rest is dev tools/debug stuff. This is gonna be strait up imgui goodness like
        // logs browsing and profiler output or and like maybe a dev console and tweaking things
        // or debugging things. Possibly even like asset generation stuff and world generators.
        // This should just work of a debug menu and be classig IMGUI widgets.
        memory->ticks = start_time; // @TODO; Not exactly sure what I want to pass here.
        // start_time is the time the frame started at.
        memory->dt = (float)(frame_time * 0.001f); // @TODO: this is the last frame's length,
        // not this one.

        int w, h;
        int display_w, display_h;
        SDL_GetWindowSize(window, &w, &h);
        SDL_GL_GetDrawableSize(window, &display_w, &display_h);

        memory->window_width = w;
        memory->window_height = h;
        memory->display_width = display_w;
        memory->display_height = display_h;

        game.gameUpdateAndRender(memory);

        // Free packets from last frame.
        for (int packet_index = 0; packet_index < num_packets; packet_index++) {
            enet_packet_destroy(packets[packet_index]);
        }

        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        ImGui::Render();

        // @TODO: Graph this and see how long we are actually sleeping and if it's accurate.
        update_time = SDL_GetTicks() - start_time;
        int time_till_vsync = target_seconds_per_frame*1000.0 - (SDL_GetTicks() - start_time);
        if (time_till_vsync > 4) {
            SDL_Delay(time_till_vsync - 3);
        }

        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        last_counter = end_counter;
    }
}
