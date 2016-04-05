#define GAME_CLIENT
#include "game_platform.h"

// @TODO: Have a hot reloading version for debugging.
#include "game.cpp"

char* PROD_HOST = "game.steveindusteves.com";
char* DEV_HOST = "localhost";

// Set this in build system for prod builds to be the real server.
#ifndef SERVER_HOST
#define SERVER_HOST DEV_HOST
#endif

// Set up platform API. Probably could be shared between client and server.
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
    char *data = "Hello I am the client";
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

    // This is some real programming, will be a good first thing to get back in the right mindset.
    // want a rolling chat buffer, you can append messages and I'll be displaying messages too.
    // once it fills up, when new messages come it in should free up the oldest ones.
    
    // First pass at a chat buffer.
    // 4kb of text, @TODO: probably way small.
    char *chat_buf = (char*)calloc(1, 1024*4);
    char* chat_messages[256]; // Pointers to individual messages.

    // add a message,
    // write the string to the next space in chat_buf
    // add pointer to chat_messages
    // if there is not enough space in chat buf remove the oldest messages
    // one at a time until there is enough free space.
    // handle wrapping around the end of the buffer nicely, dont want a chat message to span around
    // so we can just ignore the end of the buffer.

    // there are about 10 thousand systems like this that I need to implement and get good at just doing
    // a good first pass and iterating. This is the most important skill so I need to get good at it.

    char chat_input_buf[256];
    int chat_input_buf_length = 0;

    for (int i=0; i<256; i++) {
        chat_input_buf[i] = 0;
    }
 
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {

                // @TODO: Probably want to be able to move a carrot and actually edit this, not just backspace.
            case SDL_TEXTINPUT: {
                /* fprintf(stderr, "Text Input: %s\n", event.text.text); */

                int input_len = strlen(event.text.text);
                if (chat_input_buf_length + input_len < 255) {
                    for (int i = 0; i < input_len; i++) {
                        chat_input_buf[chat_input_buf_length + i] = event.text.text[i];
                    }
                    chat_input_buf_length += input_len;
                }
                
            } break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (chat_input_buf_length > 0) {
                        chat_input_buf[chat_input_buf_length-- - 1] = 0;
                    }
                }

                break;
                
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

        GameMemory memory;
        memory.platform_api = &platform_api;
        gameUpdateAndRender(&memory);

        SDL_GL_SwapWindow(window);
    }
}
