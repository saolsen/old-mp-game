#ifndef _game_player_h
#define _game_player_h

struct Player {
    i32 id;
};

struct ServerPlayerRecord {
    Player player;
    void* enet_peer;
    ServerPlayerRecord *next_in_hash;
};

struct ServerPlayerStore {
    i32 last_used_id;
    
    ServerPlayerRecord *records_base;
    i32 records_count;
    i32 records_capacity;
    ServerPlayerRecord *first_free_record;
    
    // @NOTE: Must be a power of 2 for hash function;
    ServerPlayerRecord *id_hash[128];
};

#endif
