#ifndef _game_packet_h
#define _game_packet_h

#include "game_platform.h"

enum PacketType {
    PacketType_PING_PACKET = 0,
    PacketType_CHAT_MSG = 1,
    PacketType_YOUR_ID = 2,
};

struct PingPacket {
    PacketHeader header;
    char msg[256];       // @TODO: Compression.
};

struct PacketChatMsg {
    PacketHeader header;
    char msg[256];
    i32 player_id;
};

struct PacketYourID {
    PacketHeader header;
    i32 your_id;
};

#endif
