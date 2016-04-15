#ifndef _game_h
#define _game_h
/*
Goal 1
- chat.
Goal 2
- move players around.

==========
Networking
==========
Set up networking accross game barrior.
  (*) PlatformSendPacket
  (*) a queue of incoming packets as part of the game memory
  (*) Have a flag for reliability, map it almost directly to the flags for enet.
  (*) pack the buffer, just push in all the data to a big arena, let there be a packet
      header that defines the length.


*/

#include "game_platform.h"
#include "imgui.h"

#include "game_message.h"
#include "game_chat.h"

struct GameState {
    i32 my_id;
    bool initialized;
    // Chat state
    ChatState chat_state;
};

#endif
