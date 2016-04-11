// Gotta do some extern c shit to get this to reload.
// @TODO: Compiling is taking FOREVER with the new imgui stuff, maybe I should compile it seperately
#include "game.h"

PlatformAPI *platform = NULL;

extern "C" GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    if (!platform) {
        platform = memory->platform_api;
    }
    /* platform->platformLogMessage("Hello, logging"); */
    INFO("WORKIN");
}
