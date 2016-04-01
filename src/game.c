#include "game.h"

PlatformAPI *platform = NULL;

GAME_UPDATE_AND_RENDER(gameUpdateAndRender)
{
    if (!platform) {
        platform = memory->platform_api;
    }

    /* platform->platformLogMessage("Hello, logging"); */
    INFO("Hello Logging");
}
