// Gotta do some extern c shit to get this to reload.
// @TODO: Still wanna make the chat a circular buffer to be safe.

#include "game.h"

PlatformAPI *platform = NULL;

#include "game_chat.cpp"

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    if (!platform) {
        platform = memory->platform_api;
    }

    GameState *gamestate = NULL;
    if (memory->persistent_arena->size > 0) {
        gamestate = (GameState*)memory->persistent_arena->base;
        
    } else {                
        INFO("Initializing");
        gamestate = arenaPushType(memory->persistent_arena, GameState);
        chatStateAllocate(memory->persistent_arena, &gamestate->chat_state);
    }

    // Chat
    ChatState *chatstate = &gamestate->chat_state;
    chatDisplay(chatstate, memory->display_height);

    /* platform->platformLogMessage("Hello, logging"); */
}
