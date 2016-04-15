// Gotta do some extern c shit to get this to reload.
// @TODO: Still wanna make the chat a circular buffer to be safe.
#include <stdio.h>

#include "game.h"

PlatformAPI *platform = NULL;

#include "game_message.cpp"
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
    }

    // Wait until we have and ID to allocate the chat.
    if (!gamestate->initialized && gamestate->my_id != 0) {
        chatStateAllocate(memory->persistent_arena, &gamestate->chat_state, gamestate->my_id);
        gamestate->initialized = true;
    }

    ChatState *chatstate = &gamestate->chat_state;

    // Ok, so this seems like it works really well, just loop over the packets for the frame.
    if (memory->num_packets > 0) {
        INFO("%i packets to respond to", memory->num_packets);
    }
    for (i32 packet_index = 0; packet_index < memory->num_packets; packet_index++) {
        PacketHeader *packet = memory->packets[packet_index];
        
        switch(packet->type) {
        case PacketType_PING_PACKET: {
            PingPacket *ping = (PingPacket*)packet;
            INFO("Got Pinged: %s", ping->msg);
            // send back a pong.
            PingPacket pong;
            pingPacketInit(&pong);
            snprintf(pong.msg, LEN(pong.msg), "I the client have heard you!");

            platform->platformSendPacket((PacketHeader*)&pong);
        } break;

        case PacketType_CHAT_MSG: {
            if (gamestate->initialized) {
                PacketChatMsg *chat_msg = (PacketChatMsg*)packet;
                INFO("Got a chat message: %s", chat_msg->msg);
                chatAddMessage(chatstate, chat_msg->msg, strlen(chat_msg->msg), chat_msg->player_id);
            }
            
        } break;

        case PacketType_YOUR_ID: {
            PacketYourID *your_id = (PacketYourID*)packet;
            gamestate->my_id = your_id->your_id;
            
        } break;
            
        default:
            break;
        }
    }

    // Chat
    if (gamestate->initialized) {
        chatDisplay(chatstate, memory->display_height);
    }

    /* platform->platformLogMessage("Hello, logging"); */
}
