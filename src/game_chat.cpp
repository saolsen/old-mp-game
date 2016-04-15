#include "game_chat.h"
#include "imgui.h"

void
chatStateAllocate(MemoryArena *arena, ChatState *chatstate, i32 my_id)
{
    chatstate->chat_input_buf = arenaPushArray(arena, char, input_buf_size);
    chatstate->chat_input_buf_capacity = input_buf_size;
        
    chatstate->chat_text_buf = arenaPushArray(arena, char, text_buf_size);
    chatstate->chat_text_buf_capacity = text_buf_size;
    chatstate->next_text_char = 0;
    chatstate->oldest_text_char = 0;
    // chatstate->chat_text_buf_count = 0;
    // chatstate->chat_text_buf_max_count = text_buf_size;

    chatstate->chat_msgs = arenaPushArray(arena, ChatMsg, chat_msgs_count);
    chatstate->chat_msgs_capacity = chat_msgs_count;
    chatstate->next_msg = 0;
    chatstate->oldest_msg = 0;
    
    // chatstate->chat_msgs_count = 0;
    // chatstate->chat_msgs_max_count = chat_msgs_count;
    // chatstate->chat_msgs_next_msg = 0;

    chatstate->scroll_to_bottom = false;

    chatstate->my_id = my_id;
}

inline void
chatDropOldestMsg(ChatState *chatstate)
{
    ChatMsg *oldest = chatstate->chat_msgs + chatstate->oldest_msg++;
    chatstate->oldest_msg %= chatstate->chat_msgs_capacity;

    chatstate->oldest_text_char += oldest->length;
    chatstate->oldest_text_char %= chatstate->chat_text_buf_capacity;
}


inline bool
chatBufferIsFull(ChatState *chatstate)
{
    return ((chatstate->next_msg + 1) % chatstate->chat_msgs_capacity) == chatstate->oldest_msg;
}


inline i32
chatTextBufFreeSpace(ChatState *chatstate)
{
    i32 freespace;

    if (chatstate->next_text_char >= chatstate->oldest_text_char) {
        freespace = chatstate->next_text_char + chatstate->chat_text_buf_capacity;
        freespace += chatstate->oldest_text_char;
    } else {
        freespace = chatstate->oldest_text_char - chatstate->next_text_char;
    }

    return freespace;
}


void
chatAddMessage(ChatState *chatstate, const char* msg, int length, i32 user_id)
{
    if (length > chatstate->chat_text_buf_capacity) {
        // This message is longer than our whole text buffer. So nah...
        return;
    }

    if (chatBufferIsFull(chatstate)) {
        chatDropOldestMsg(chatstate);
    }

    ChatMsg *item = chatstate->chat_msgs + chatstate->next_msg++;
    chatstate->next_msg %= chatstate->chat_msgs_capacity;

    while (chatTextBufFreeSpace(chatstate) < length) {
        chatDropOldestMsg(chatstate);
    }

    item->length = length;
    item->msg_index = chatstate->next_text_char;
    item->user_id = user_id;

    // write message to buffer
    for (i32 i=0; i < length; i++) {
        i32 buf_index = (chatstate->next_text_char + i) %
            chatstate->chat_text_buf_capacity;
        chatstate->chat_text_buf[buf_index] = msg[i];
    }

    chatstate->next_text_char += length;
    chatstate->next_text_char %= chatstate->chat_text_buf_capacity;

    chatstate->scroll_to_bottom = true;
}

void
chatDisplay(ChatState *chatstate, int display_height)
{
    bool is_open = true;
    ImGui::SetNextWindowPos(ImVec2(0,display_height - 300), true);
    ImGui::SetNextWindowSize(ImVec2(361,300), true);
    ImGui::Begin("Chat", &is_open, ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize);
    // Make tall enough to see chat at the bottom.
    // Always scroll buffer to the bottomr.

    ImGui::BeginChild("ScrollingRegion",
                      ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()),
                      false, ImGuiWindowFlags_HorizontalScrollbar);

    for (i32 message_index = chatstate->oldest_msg;
         message_index != chatstate->next_msg;
         message_index = (message_index + 1) % chatstate->chat_msgs_capacity) {

        ChatMsg *msg = chatstate->chat_msgs + message_index;

        if (msg->user_id == chatstate->my_id) {
            ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Me");
        } else {
            ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "uid: %i", msg->user_id);
        }

        // if (message_index % 2 == 0) {
        //     ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Steve");

        // } else {
        //     ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Somebody Else");
        // }

        // Kinda crappy but might be fine.
        char text_buf[input_buf_size+1];
        int i;
        for (i=0; i<msg->length; i++) {
            int c = (msg->msg_index + i) % chatstate->chat_text_buf_capacity;
            text_buf[i] = chatstate->chat_text_buf[c];
        }
        text_buf[i] = 0;
        ImGui::TextWrapped("%s", text_buf);
    }

    // // @TODO: There are better ways to do this.
    // for (int msg_index = 0; msg_index < chatstate->chat_msgs_count; msg_index++) {

        
    //     ChatMsg *msg = chatstate->chat_msgs + msg_index;
    //     ImGui::TextWrapped("%s", msg->msg);
    // }
    if (chatstate->scroll_to_bottom) {
        ImGui::SetScrollHere(); // @TODO: Probably not always...
    }
    chatstate->scroll_to_bottom = false;
    ImGui::EndChild();

    bool add_msg = false;    
    if(ImGui::InputText("", chatstate->chat_input_buf, chatstate->chat_input_buf_capacity,
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
        add_msg = true;
    }

    // Keep focused here on enter.
    if (ImGui::IsItemHovered() ||
        (ImGui::IsRootWindowOrAnyChildFocused() &&
         !ImGui::IsAnyItemActive() &&
         !ImGui::IsMouseClicked(0))) {
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
    }

    ImGui::SameLine();
    if (ImGui::Button("Send") || add_msg) {
        chatAddMessage(chatstate, chatstate->chat_input_buf, strlen(chatstate->chat_input_buf), chatstate->my_id);

        // Send packet.
        PacketChatMsg chat_msg;
        packetChatMsgInit(&chat_msg);
        snprintf(chat_msg.msg, LEN(chat_msg.msg), "%s", chatstate->chat_input_buf);
        platform->platformSendPacket((PacketHeader*)&chat_msg);

        // Clear input buffer.
        for (int i=0; i<chatstate->chat_input_buf_capacity; i++) {
            chatstate->chat_input_buf[i] = 0;
        }
    }
    ImGui::End();
}
