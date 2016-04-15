#include "game_message.h"

// @TODO: Figure out how I want to do this, I think for a lot of this tuff I want to use
// an arena to make it super easy to push memory on the end of the struct.
// for now I'll just allocate in code but probably it's better to do that here and take an arena.

void
pingPacketInit(PingPacket *packet)
{
    packet->header.flag = PacketFlag_RELIABLE;
    packet->header.size = sizeof(*packet);
    packet->header.type = PacketType_PING_PACKET;
}

void
packetChatMsgInit(PacketChatMsg *packet)
{
    packet->header.type = PacketType_CHAT_MSG;
    packet->header.size = sizeof(*packet);
    packet->header.flag = PacketFlag_RELIABLE;
}

void
packetYourIDInit(PacketYourID *packet)
{
    packet->header.type = PacketType_YOUR_ID;
    packet->header.size = sizeof(*packet);
    packet->header.flag = PacketFlag_RELIABLE;
}
