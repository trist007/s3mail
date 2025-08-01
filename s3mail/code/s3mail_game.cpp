#include "s3mail_platform.h"

// UI update functions
void UpdateButton(UIButton* btn, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    btn->is_hovered = platform->Win32PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void RenderButton(UIButton* btn, PlatformAPI* platform) {
    if (btn->is_pressed) {
        platform->Win32SetColor(0.2f, 0.2f, 0.6f);
    } else if (btn->is_hovered) {
        platform->Win32SetColor(0.4f, 0.4f, 0.8f);
    } else {
        platform->Win32SetColor(0.3f, 0.3f, 0.7f);
    }
    
    platform->Win32DrawRect(btn->x, btn->y, btn->width, btn->height);
    
    platform->Win32SetColor(0.0f, 0.0f, 0.0f);
    platform->Win32DrawRectOutline(btn->x, btn->y, btn->width, btn->height);
    
    platform->Win32SetColor(1.0f, 1.0f, 1.0f);
    platform->Win32DrawText(btn->text, btn->x + 5, btn->y + 8);
}

void UpdateList(UIList* list, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    
    if (mouse_down && platform->Win32PointInRect(mouse_x, gl_y, list->x, list->y, list->width, list->height)) {
        int item_height = 25;
        int clicked_item = (gl_y - list->y) / item_height;
        if (clicked_item >= 0 && clicked_item < list->item_count) {
            list->selected_item = clicked_item;
        }
    }
}

void RenderList(UIList* list, PlatformAPI* platform) {
    // Background
    platform->Win32SetColor(0.9f, 0.9f, 0.9f);
    platform->Win32DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    platform->Win32SetColor(0.0f, 0.0f, 0.0f);
    platform->Win32DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    for (int i = 0; i < list->item_count; i++) {
        float item_y = list->y + i * item_height;
        
        if (i == list->selected_item) {
            platform->Win32SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            platform->Win32DrawRect(list->x, item_y, list->width, item_height);
        }
        
        platform->Win32SetColor(0.0f, 0.0f, 0.0f);
        platform->Win32DrawText(list->items[i], list->x + 5, item_y + 5);
    }
}

// Main game functions that get hot reloaded
__declspec(dllexport)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    // The platform layer will handle glClear for us
    
    // Draw purple header stripe - try changing this color and recompiling!
    platform->Win32SetColor(0.3f, 0.3f, 0.7f);  // Easy to experiment with colors now
    platform->Win32DrawRect(0, 675, 1200, 450);
    
    platform->Win32SetColor(0.0f, 0.0f, 0.0f);
    platform->Win32DrawRectOutline(0, 675, 1200, 450);
    
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
        platform->Win32DrawText(preview_text, 640, 320);
    }
    
    // Status bar
    platform->Win32SetColor(0.8f, 0.8f, 0.8f);
    platform->Win32DrawRect(0, 0, state->window_width, 25);
    platform->Win32SetColor(0.0f, 0.0f, 0.0f);
    platform->Win32DrawText("S3Mail - Hot Reloadable Ready!", 10, 8);
    
    // Handle button clicks
    if (state->compose_button.is_pressed) {
        platform->Win32ShowMessage("Compose clicked from hot-reloaded DLL!");
    }
    if (state->delete_button.is_pressed) {
        platform->Win32ShowMessage("Delete clicked from hot-reloaded DLL!");
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