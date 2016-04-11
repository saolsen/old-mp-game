#define GAME_CLIENT
#include "game_platform.h"

#include "imgui_demo.cpp"

const char* PROD_HOST = "game.steveindusteves.com";
const char* DEV_HOST = "localhost";

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
    fprintf(stderr, "lib path: %s\n", path_buf);

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

struct ChatMsg {
    char* msg;
    int   length;
    // Add like the id of the person that sent the message too.
};

// Chat
static char chat_text_buf[1024];
static ChatMsg chat_msgs[512];
int next_msg = 0;
int next_txt_pos = 0;
int num_msgs = 0;

// @TODO: Since this is a circular thing I need to take the advice from gaffer on games on how
// to wrap the pointer around. At first I think I'll just make this a linear buffer and then later
// do that

void add_chat_msg(const char* msg, int length)
{
    // @TODO: Make this a circular buffer.
    assert(next_msg < sizeof(chat_msgs) / sizeof(chat_msgs[0]));
    assert(next_txt_pos+length+1 < sizeof(chat_text_buf) / sizeof(chat_text_buf[0]));
    ChatMsg *chat_msg = chat_msgs + next_msg++;
    char* str_loc = chat_text_buf + next_txt_pos;

    chat_msg->msg = str_loc;
    chat_msg->length = length;

    memcpy(str_loc, msg, length);
    next_txt_pos += length;
    chat_text_buf[next_txt_pos++] = 0;

    num_msgs++;
}

int main()
{
    CurrentGame game = {.handle = NULL,
                        .id = 0,
                        .gameUpdateAndRender = NULL };
    gameReload(&game);
    
    const char * msg1 = "Hello wudup with you";
    add_chat_msg(msg1, strlen(msg1));
    const char * msg2 = "This is the message bumba 2";
    add_chat_msg(msg2, strlen(msg2));
    
    PlatformAPI platform_api;
    platform_api.platformLogMessage = &platformLogMessage;
    
    fprintf(stderr, "Hello World, This is the client!\n");

    if (SDL_Init(0) == -1) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    if (SDLNet_Init() == -1) {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        exit(2);
    }

    UDPsocket udpsock = SDLNet_UDP_Open(0);
    if (!udpsock) {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        exit(2);
    }

    // Ping the server.
    // I don't really like that you have to call this to allocate packets. I'd rather do the memory management myself. I had that system with an arena that was working very nicely. That way I don't have to resize.
    const char *data = "Hello I am the client";
    int len = strlen(data) + 1;
    UDPpacket *packet = SDLNet_AllocPacket(len);
    if (!packet) {
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
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

    // setup imgui
    ImGui_ImplSdlGL3_Init(window);

    char chat_input_buf[256];

    for (int i=0; i<256; i++) {
        chat_input_buf[i] = 0;
    }

    int update_time = 0;
    int frame_time = 0;

    uint64_t last_counter = SDL_GetPerformanceCounter();
    int last_start_time = SDL_GetTicks();
 
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
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        // @TODO: Handle high dpi and window resizing.

        ImGui_ImplSdlGL3_NewFrame(window);

        ImGui::ShowTestWindow();
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

        // Chat

        // @TODO: This still sorta sux.
        // Fix positioning to be relative to screen size.
        // Fix scrolling. Maybe also have scroll bar on the left.
        bool is_open = true;
        ImGui::SetNextWindowPos(ImVec2(0,500), true);
        ImGui::SetNextWindowSize(ImVec2(362,270), true);
        ImGui::Begin("Chat", &is_open, ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize);
        // Make tall enough to see chat at the bottom.
        // Always scroll buffer to the bottomr.

        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
        // @TODO: There are better ways to do this.
        for (int msg_index = 0; msg_index < num_msgs; msg_index++) {
            ChatMsg *msg = chat_msgs + msg_index;
            ImGui::Text(msg->msg);
        }
        ImGui::SetScrollHere(); // Probably not always...
        ImGui::EndChild();
        ImGui::Separator();
        bool add_msg = false;
        if(ImGui::InputText("", chat_input_buf, sizeof(chat_input_buf),
                            ImGuiInputTextFlags_EnterReturnsTrue)) {
            add_msg = true;
        }

        // Keep focused here on enter.
        if (ImGui::IsItemHovered() ||
            (ImGui::IsRootWindowOrAnyChildFocused() &&
             !ImGui::IsAnyItemActive() &&
             !ImGui::IsMouseClicked(0))) {
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Send") || add_msg) {
            // Send chat message.
            // INFO("Sending: %s", chat_input_buf);
            add_chat_msg(chat_input_buf, strlen(chat_input_buf));

            // Clear input buffer.
            for (int i=0; i<256; i++) {
                chat_input_buf[i] = 0;
            }
        }
        ImGui::End();

        GameMemory memory;
        memory.platform_api = &platform_api;
        game.gameUpdateAndRender(&memory);

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
