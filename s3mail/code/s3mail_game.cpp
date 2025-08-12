#include "s3mail_platform.h"
#include <GL/gl.h>

internal bool32 reinit_ui = true;

// UI update functions
void UpdateButton(UIButton* btn, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    btn->is_hovered = platform->PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void RenderButton(UIButton* btn, PlatformAPI* platform) {
    if (btn->is_pressed) {
        platform->SetColor(0.2f, 0.2f, 0.6f);
    } else if (btn->is_hovered) {
        platform->SetColor(0.4f, 0.4f, 0.8f);
    } else {
        platform->SetColor(0.3f, 0.3f, 0.7f);
    }
    
    platform->DrawRect(btn->x, btn->y, btn->width, btn->height);
    
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawRectOutline(btn->x, btn->y, btn->width, btn->height);
    
    platform->SetColor(1.0f, 1.0f, 1.0f);
    platform->DrawText(platform->State, btn->text, btn->x + 5, btn->y + 8);
}

void UpdateList(UIList* list, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    
    if (mouse_down && platform->PointInRect(mouse_x, gl_y, list->x, list->y, list->width, list->height)) {
        int item_height = 25;
        int clicked_item = (gl_y - list->y) / item_height;
        if (clicked_item >= 0 && clicked_item < list->item_count) {
            list->selected_item = clicked_item;
        }
    }
}

void RenderList(UIList* list, PlatformAPI* platform) {
    // Background
    platform->SetColor(0.9f, 0.9f, 0.9f);
    platform->DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    for (int i = 0; i < list->item_count; i++) {
        float item_y = list->y + i * item_height;
        
        if (i == list->selected_item) {
            platform->SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            platform->DrawRect(list->x, item_y, list->width, item_height);
        }
        
        platform->SetColor(0.0f, 0.0f, 0.0f);
        platform->DrawText(platform->State, list->items[i], list->x + 5, item_y + 5);
    }
}

__declspec(dllexport)
GAME_INITIALIZE_UI(GameInitializeUI)
{
    //bool32 Result = true;
    if (state)
    {
        // NOTE(trist007): try using CW_USEDEFAULT
        state->window_width = 1200;
        state->window_height = 800;
        
        // Initialize UI elements
        state->compose_button = {10, 635, 100, 30, "Compose", 0, 0};
        state->delete_button = {120, 635, 100, 30, "Delete", 0, 0};
        
        state->folder_list = {10, 495, 200, 125, {"Trash", "Junk", "Drafts", "Sent", "Inbox"}, 5, 0};
        state->email_list = {220, 40, 900, 580, {"Email 3", "Email 2", "Email 1"}, 3, -1};
        state->contact_list = {10, 150, 200, 150, {"Pappi", "Mom", "Glenn", "Vito"}, 4, -1};
        
        // Debug output
        char debug_msg[256];
        sprintf(debug_msg, "GameInitializeUI set contact[0] to: '%s'\n", state->contact_list.items[0]);
        OutputDebugString(debug_msg);
    }
    
    OutputDebugString("=== GameInitializeUI COMPLETE ===\n");
}

// Main game functions that get hot reloaded
__declspec(dllexport)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    // The platform layer will handle glClear for us
    
    //GameInitializeUI(state, platform);
    
    // Draw purple header stripe - try changing this color and recompiling!
    platform->SetColor(0.3f, 0.3f, 0.7f);  // Easy to experiment with colors now
    platform->DrawRect(0, 675, 1200, 450);
    
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawRectOutline(0, 675, 1200, 450);
    
    // Update UI elements
    UpdateButton(&state->compose_button, state->mouse_x, state->mouse_y, state->mouse_down, state->window_height, platform);
    UpdateButton(&state->delete_button, state->mouse_x, state->mouse_y, state->mouse_down, state->window_height, platform);
    UpdateList(&state->folder_list, state->mouse_x, state->mouse_y, state->mouse_down, state->window_height, platform);
    UpdateList(&state->email_list, state->mouse_x, state->mouse_y, state->mouse_down, state->window_height, platform);
    UpdateList(&state->contact_list, state->mouse_x, state->mouse_y, state->mouse_down, state->window_height, platform);
    
    // Render UI elements
    RenderButton(&state->compose_button, platform);
    RenderButton(&state->delete_button, platform);
    RenderList(&state->folder_list, platform);
    RenderList(&state->email_list, platform);
    RenderList(&state->contact_list, platform);
    
    // Email preview
    if (state->email_list.selected_item >= 0) {
        char preview_text[256];
        sprintf(preview_text, "Preview of %s - Hot Reloaded!", state->email_list.items[state->email_list.selected_item]);
        platform->DrawText(platform->State, preview_text, 640, 320);
    }
    
    // Status bar
    platform->SetColor(0.8f, 0.8f, 0.8f);
    platform->DrawRect(0, 0, state->window_width, 25);
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawText(platform->State, "S3Mail - Hot Reloadable Ready!", 10, 8);
    
    // Handle button clicks
    if (state->compose_button.is_pressed) {
        platform->ShowMessage(platform->Window, "Compose clicked from hot-reloaded DLL!");
    }
    if (state->delete_button.is_pressed) {
        platform->ShowMessage(platform->Window, "Delete clicked from hot-reloaded DLL!");
    }
}

__declspec(dllexport)
GAME_HANDLE_KEY_PRESS(GameHandleKeyPress) {
    switch (key_code) {
        case 'J':
        // Move down in email list
        if (state->email_list.selected_item < state->email_list.item_count - 1) {
            state->email_list.selected_item++;
        }
        break;
        
        case 'K':
        // Move up in email list
        if (state->email_list.selected_item > 0) {
            state->email_list.selected_item--;
        }
        break;
        
        case 'H':
        // Move left in folder list
        if (state->folder_list.selected_item > 0) {
            state->folder_list.selected_item--;
        }
        break;
        
        case 'L':
        // Move right in folder list
        if (state->folder_list.selected_item < state->folder_list.item_count - 1) {
            state->folder_list.selected_item++;
        }
        break;
    }
}
