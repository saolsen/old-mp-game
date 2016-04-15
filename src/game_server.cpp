// @TODO: Ping everybody every second so I can test that pings come in to my game code.

// I'm currently using sdl as my standard library but I may want to actually use linux stuff
// at least for the server.

#define GAME_SERVER
#include "game_system.h"

// Server Headers
#include "game_player.h"
#include "game_message.h"

// Server Code
#include "game_message.cpp"

int main()
{
    fprintf(stderr, "Hello, this is the server!\n");

    if (SDL_Init(0) == -1) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Try using enet instead.

    if (enet_initialize() != 0) {
        fprintf(stderr, "Error initializing ENet\n");
        exit(1);
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost *server;

    address.host = ENET_HOST_ANY;
    address.port = 1234;

    server = enet_host_create(&address, 64, 4, 0, 0);

    if (server == NULL) {
        fprintf(stderr, "Error creating ENet server host\n");
        exit(1);
    }

    ServerPlayerStore *player_store = (ServerPlayerStore*)malloc(sizeof(*player_store));
    player_store->last_used_id = 0;
    player_store->records_base = (ServerPlayerRecord*)calloc(1, sizeof(ServerPlayerRecord) * 128);
    player_store->records_count = 0;
    player_store->records_capacity = 128;
    player_store->first_free_record = NULL;

    for (int i=0; i<LEN(player_store->id_hash); i++) {
        player_store->id_hash[i] = NULL;
    }

    bool running = true;
    // ENetPacket *packet;
    while (running) {
        ENetEvent net_event;
        while (enet_host_service(server, &net_event, 0) > 0) {
            switch (net_event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                // Add player.
                ServerPlayerRecord *player_record = NULL;
                if (player_store->first_free_record) {
                    player_record = player_store->first_free_record;
                    player_store->first_free_record = player_record->next_in_hash;
                } else {
                    if (player_store->records_count < player_store->records_capacity) {
                        player_record = player_store->records_base + player_store->records_count++;
                    }
                }

                if (player_record) {
                    i32 id = ++player_store->last_used_id;

                    // Add to hash.
                    u32 hash_bucket = id & (LEN(player_store->id_hash) - 1);
                    player_record->next_in_hash = player_store->id_hash[hash_bucket];
                    player_store->id_hash[hash_bucket] = player_record;

                    player_record->player.id = id;
                    player_record->enet_peer = net_event.peer;

                    // @TODO: Is this bad / non portable?
                    net_event.peer->data = (void*)(u64)id;
                    
                } else {
                    // Error, server is full.
                }
                
                fprintf(stderr, "A client has connected. id: %i\n", player_record->player.id);
                PacketYourID your_id_packet;
                packetYourIDInit(&your_id_packet);
                your_id_packet.your_id = player_record->player.id;

                ENetPacket *p = enet_packet_create(&your_id_packet, your_id_packet.header.size, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(net_event.peer, 1, p);
                
            } break;
            case ENET_EVENT_TYPE_RECEIVE: {
                if (net_event.channelID > 0) {
                    i32 player_id = (i32)(i64)net_event.peer->data;
                    
                    PacketHeader *packet = (PacketHeader*)net_event.packet->data;
                    fprintf(stderr, "\tPACKET TYPE: %i\n", packet->type);

                    switch(packet->type) {
                    case PacketType_PING_PACKET: {
                        PingPacket *ping = (PingPacket*)packet;
                        fprintf(stderr,"Got Pinged: %s\n", ping->msg);
                    } break;

                    case PacketType_CHAT_MSG: {
                        PacketChatMsg *chat_msg = (PacketChatMsg*)packet;
                        fprintf(stderr,
                                "Got chat message from %i: %s\n",
                                player_id, chat_msg->msg);
                        chat_msg->player_id = player_id;
                        // reply
                        PingPacket pong;
                        pingPacketInit(&pong);
                        snprintf(pong.msg, LEN(pong.msg), "I got your chat message.");

                        ENetPacket *p = enet_packet_create(&pong, pong.header.size, ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(net_event.peer, 1, p);
                        // platform->platformSendPacket((PacketHeader*)&pong);
                        
                        // Relay message to other connected clients.
                        for (i32 player_index = 0;
                             player_index < player_store->records_count;
                             player_index++) {
                            ServerPlayerRecord *player_record = player_store->records_base + player_index;
                            if (player_record->player.id != 0 &&
                                player_record->player.id != player_id) {
                                fprintf(stderr, "Relaying message to %i\n", player_record->player.id);
                                fprintf(stderr, "sending packet with type %i and size %i\n",
                                        chat_msg->header.type,
                                        chat_msg->header.size);
                                // Send to all connected players that aren't this one.
                                ENetPacket *chat_packet = enet_packet_create(chat_msg,
                                                                             chat_msg->header.size,
                                                                             ENET_PACKET_FLAG_RELIABLE);
                                enet_peer_send((ENetPeer*)player_record->enet_peer, 1, chat_packet);
                            }
                        }
                        enet_host_flush(server);
                        
                    } break;
                        
                    default:
                        break;
                    }

                    enet_packet_destroy(net_event.packet);
                }
            } break;
            case ENET_EVENT_TYPE_DISCONNECT: {
                i32 id = (i32)(i64)net_event.peer->data;

                u32 hash_bucket = id & (LEN(player_store->id_hash) - 1);
                for (ServerPlayerRecord **record = &player_store->id_hash[hash_bucket];
                     *record;
                     record = &(*record)->next_in_hash) {
                    if ((*record)->player.id == id) {
                        // thing before me gotta point to the thing after me.
                        ServerPlayerRecord *removed_record = *record;
                        *record = (*record)->next_in_hash;

                        removed_record->next_in_hash = player_store->first_free_record;
                        removed_record->player.id = 0;
                        
                        player_store->first_free_record = removed_record;
                        break;
                    }
                }
                
                // remove the player.
                fprintf(stderr, "A client has disconnected. id %i\n", id);
            } break;
            default:
                break;
            }
        }

        // ping connected peers.
        for (int player_index = 0; player_index < player_store->records_count; player_index++) {
            ServerPlayerRecord *player_record = player_store->records_base + player_index;
            if (player_record->player.id != 0) {
                PingPacket ping_packet;
                pingPacketInit(&ping_packet);

                snprintf(ping_packet.msg, 256, "Hello World");

                ENetPacket *packet = enet_packet_create(&ping_packet,
                                                        sizeof(ping_packet),
                                                        ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send((ENetPeer*)player_record->enet_peer, 1, packet);
            }
        }
        
        // chill 4 a sec.
        enet_host_flush(server);
        SDL_Delay(1000);
    }

    enet_host_destroy(server);
    exit(0);
}
