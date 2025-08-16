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
    platform->DrawText(platform->GameState, btn->text, btn->x + 5, btn->y + 8);
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
        platform->DrawText(platform->GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

__declspec(dllexport)
GAME_INITIALIZE_UI(GameInitializeUI)
{
    if (GameState)
    {
        // NOTE(trist007): try using CW_USEDEFAULT
        GameState->window_width = 1200;
        GameState->window_height = 800;
        
        // Initialize UI elements
        GameState->compose_button = {10, 635, 100, 30, "Compose", 0, 0};
        GameState->delete_button = {120, 635, 100, 30, "Delete", 0, 0};
        
        GameState->folder_list = {10, 495, 200, 125, {"Trash", "Junk", "Drafts", "Sent", "Inbox"}, 5, 4};
        GameState->email_list = {220, 40, 900, 580, {"Email 3", "Email 2", "Email 1"}, 3, -1};
        GameState->contact_list = {10, 150, 200, 150, {"Papi", "Mom", "Glen", "Vito"}, 4, -1};
        
        // Initialize starting mode
        GameState->current_mode = MODE_FOLDER;
    }
}

// Main game functions that get hot reloaded
__declspec(dllexport)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    
    // Draw purple header stripe - try changing this color and recompiling!
    platform->SetColor(0.3f, 0.3f, 0.7f);  // Easy to experiment with colors now
    platform->DrawRect(0, 675, 1200, 450);
    
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawRectOutline(0, 675, 1200, 450);
    
    // Update UI elements
    UpdateButton(&GameState->compose_button, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    UpdateButton(&GameState->delete_button, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    UpdateList(&GameState->folder_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    UpdateList(&GameState->email_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    UpdateList(&GameState->contact_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    
    // Render UI elements
    RenderButton(&GameState->compose_button, platform);
    RenderButton(&GameState->delete_button, platform);
    RenderList(&GameState->folder_list, platform);
    RenderList(&GameState->email_list, platform);
    RenderList(&GameState->contact_list, platform);
    
    // Email preview
    if (GameState->email_list.selected_item >= 0) {
        char preview_text[256];
        sprintf(preview_text, "Preview of %s - Hot Reloaded!", GameState->email_list.items[GameState->email_list.selected_item]);
        platform->DrawText(platform->GameState, preview_text, 640, 320);
    }
    
    // Status bar
    platform->SetColor(0.8f, 0.8f, 0.8f);
    platform->DrawRect(0, 0, GameState->window_width, 25);
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawText(platform->GameState, "S3Mail - Hot Reloadable Ready!", 10, 8);
    
    // Handle button clicks
    if (GameState->compose_button.is_pressed) {
        platform->ShowMessage(platform->Window, "Compose clicked from hot-reloaded DLL!");
    }
    if (GameState->delete_button.is_pressed) {
        platform->ShowMessage(platform->Window, "Delete clicked from hot-reloaded DLL!");
    }
}

__declspec(dllexport)
GAME_HANDLE_KEY_PRESS(GameHandleKeyPress) {
    switch(GameState->current_mode)
    {
        case MODE_FOLDER:
        {
            // Inbox is at the bottom so it's reversed
            switch (key_code)
            {
                case 'J':
                // Move down in folder list
                if (GameState->folder_list.selected_item == 0)
                {
                    (GameState->folder_list.selected_item = GameState->folder_list.item_count - 1);
                } 
                else
                {
                    GameState->folder_list.selected_item--;
                } break;
                
                case 'K':
                // Move up in folder list
                if (GameState->folder_list.selected_item == (GameState->folder_list.item_count - 1))
                {
                    GameState->folder_list.selected_item = 0;
                }
                else
                {
                    GameState->folder_list.selected_item++;
                } break;
                
                case VK_SPACE:
                // Enter email mode
                {
                    GameState->current_mode = MODE_EMAIL;
                    if(GameState->email_list.item_count > 0)
                    {
                        GameState->email_list.selected_item = (GameState->email_list.item_count - 1);
                    }
                    break;
                }
            }
        } break;
        
        case MODE_EMAIL:
        {
            switch (key_code)
            {
                case 'J':
                // Move down in email list
                if (GameState->email_list.selected_item == 0)
                {
                    (GameState->email_list.selected_item = GameState->email_list.item_count - 1);
                } 
                else
                {
                    GameState->email_list.selected_item--;
                } break;
                
                case 'K':
                // Move up in email list
                if (GameState->email_list.selected_item == (GameState->email_list.item_count - 1))
                {
                    GameState->email_list.selected_item = 0;
                }
                else
                {
                    GameState->email_list.selected_item++;
                } break;
                
                case VK_RETURN:
                {
                    GameState->current_mode = MODE_READING_EMAIL;
                    break;
                }
                
                case 'I':
                // Go back to folder mode
                {
                    GameState->current_mode = MODE_FOLDER;
                    break;
                }
            }
        } break;
        
        case MODE_CONTACT:
        {
            // Contact mode handling
        } break;
        
        case MODE_READING_EMAIL:
        {
            // Reading email mode handling
        } break;
    }
}