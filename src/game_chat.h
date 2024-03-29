#ifndef _game_chat_h
#define _game_chat_h

#include "game.h"

const i32 input_buf_size = 256;
const i32 text_buf_size = 1024;
const i32 chat_msgs_count = 256;

struct ChatMsg {
    i32 msg_index;
    i32 length;
    i32 user_id;
    // Add like the id of the person that sent the message too.
};

struct ChatState {
    char* chat_input_buf;
    i32 chat_input_buf_capacity;
    /* i32 chat_input_buf_max_count; */
    
    char* chat_text_buf;
    i32 chat_text_buf_capacity;
    i32 next_text_char;
    i32 oldest_text_char;
    /* i32 chat_text_buf_count; */
    /* i32 chat_text_buf_max_count; */

    ChatMsg* chat_msgs;
    i32 chat_msgs_capacity;
    i32 next_msg;
    i32 oldest_msg;
    /* i32 chat_msgs_count; */
    /* i32 chat_msgs_max_count; */
    /* i32 chat_msgs_next_msg; */

    bool scroll_to_bottom;
    i32 my_id;
};

#endif
