#include "game_chat.h"
#include "imgui.h"

void
chatStateAllocate(MemoryArena *arena, ChatState *chatstate)
{
    chatstate->chat_input_buf = arenaPushArray(arena, char, input_buf_size);
    chatstate->chat_input_buf_max_count = input_buf_size;
        
    chatstate->chat_text_buf = arenaPushArray(arena, char, text_buf_size);
    chatstate->chat_text_buf_count = 0;
    chatstate->chat_text_buf_max_count = text_buf_size;

    chatstate->chat_msgs = arenaPushArray(arena, ChatMsg, chat_msgs_count);
    chatstate->chat_msgs_count = 0;
    chatstate->chat_msgs_max_count = chat_msgs_count;
    chatstate->chat_msgs_next_msg = 0;

    chatstate->scroll_to_bottom = false;
}


void
chatAddMessage(ChatState *chatstate, const char* msg, int length)
{
    // @TODO: Make this a circular buffer.
    assert(chatstate->chat_msgs_count < chatstate->chat_msgs_max_count);
    assert(chatstate->chat_text_buf_count+length+1 < chatstate->chat_text_buf_max_count);

    ChatMsg *chat_msg = chatstate->chat_msgs + chatstate->chat_msgs_count++;
    char* str_loc = chatstate->chat_text_buf + chatstate->chat_text_buf_count;

    chat_msg->msg = str_loc;
    chat_msg->length = length;

    memcpy(str_loc, msg, length);
    chatstate->chat_text_buf_count += length;

    char* null_term = chatstate->chat_text_buf + chatstate->chat_text_buf_count++;
    null_term = 0;

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

    // @TODO: There are better ways to do this.
    for (int msg_index = 0; msg_index < chatstate->chat_msgs_count; msg_index++) {
        if (msg_index % 2 == 0) {
            ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Steve");

        } else {
            ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "Somebody Else");
        }
        
        ChatMsg *msg = chatstate->chat_msgs + msg_index;
        ImGui::TextWrapped("%s", msg->msg);
    }
    if (chatstate->scroll_to_bottom) {
        ImGui::SetScrollHere(); // @TODO: Probably not always...
    }
    chatstate->scroll_to_bottom = false;
    ImGui::EndChild();
    // ImGui::Separator();
    bool add_msg = false;
    
    if(ImGui::InputText("", chatstate->chat_input_buf, chatstate->chat_input_buf_max_count,
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
        chatAddMessage(chatstate, chatstate->chat_input_buf, strlen(chatstate->chat_input_buf));

        // Clear input buffer.
        for (int i=0; i<chatstate->chat_input_buf_max_count; i++) {
            chatstate->chat_input_buf[i] = 0;
        }
    }
    ImGui::End();
}
