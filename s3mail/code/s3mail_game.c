#include "s3mail_platform.h"
#include <GL/gl.h>

// Email Layout format string
#define EMAIL_FORMAT "%-30.30s | %-75.75s | %-25.25s"

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
        
        // Calculate y from the top instead of the bottom 
        //float item_y = list->y + i * item_height;
        float item_y = (list->y + list->height) - ((i + 1) * item_height);
        
        if (i == list->selected_item) {
            platform->SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            platform->DrawRect(list->x, item_y, list->width, item_height);
        }
        
        platform->SetColor(0.0f, 0.0f, 0.0f);
        platform->DrawText(platform->GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void RenderListWithHeader(UIList* list, PlatformAPI* platform) {
    // Background
    platform->SetColor(0.9f, 0.9f, 0.9f);
    platform->DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    
    char header_text[256];
    StringCchPrintf(header_text, sizeof(header_text),
                    EMAIL_FORMAT, "From", "Subject", "Received");
    
    float header_y = (list->y + list->height) - item_height;  // Top row
    platform->SetColor(0.7f, 0.7f, 0.7f);  // Different color for header
    platform->DrawRect(list->x, header_y, list->width, item_height);
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawText(platform->GameState, header_text, list->x + 5, header_y + 5);
    
    for (int i = 0; i < list->item_count; i++) {
        
        // Calculate y from the top instead of the bottom 
        //float item_y = list->y + i * item_height;
        float item_y = (list->y + list->height) - ((i + 2) * item_height);
        
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
        GameState->window_width = WINDOW_WIDTH_HD;
        GameState->window_height = WINDOW_HEIGHT_HD;
        
        // Initialize UI elements
        GameState->compose_button.x = 10;
        GameState->compose_button.y = 1235;
        GameState->compose_button.width = 100;
        GameState->compose_button.height = 30;
        StringCchCopy(GameState->compose_button.text, ArrayCount(GameState->compose_button.text), "Compose");
        GameState->compose_button.is_hovered = 0;
        GameState->compose_button.is_pressed = 0;
        
        GameState->delete_button.x = 120;
        GameState->delete_button.y = 1235;
        GameState->delete_button.width = 100;
        GameState->delete_button.height = 30;
        StringCchCopy(GameState->delete_button.text, ArrayCount(GameState->delete_button.text), "Delete");
        GameState->delete_button.is_hovered = 0;
        GameState->delete_button.is_pressed = 0;
        
        GameState->folder_list.x = 10;
        GameState->folder_list.y = 1095;
        GameState->folder_list.width = 200;
        GameState->folder_list.height = 125;
        StringCchCopy(GameState->folder_list.items[0], sizeof(GameState->folder_list.items[0]), "Inbox");
        StringCchCopy(GameState->folder_list.items[1], sizeof(GameState->folder_list.items[1]), "Sent");
        StringCchCopy(GameState->folder_list.items[2], sizeof(GameState->folder_list.items[2]), "Draft");
        StringCchCopy(GameState->folder_list.items[3], sizeof(GameState->folder_list.items[3]), "Junk");
        StringCchCopy(GameState->folder_list.items[4], sizeof(GameState->folder_list.items[4]), "Trash");
        GameState->folder_list.item_count = 5;
        GameState->folder_list.selected_item = 0;
        
        GameState->contact_list.x = 10;
        GameState->contact_list.y = 980;
        GameState->contact_list.width = 200;
        GameState->contact_list.height = 100;
        StringCchCopy(GameState->contact_list.items[0], sizeof(GameState->contact_list.items[0]), "Papi");
        StringCchCopy(GameState->contact_list.items[1], sizeof(GameState->contact_list.items[1]), "Mom");
        StringCchCopy(GameState->contact_list.items[2], sizeof(GameState->contact_list.items[2]), "Glen");
        StringCchCopy(GameState->contact_list.items[3], sizeof(GameState->contact_list.items[3]), "Vito");
        GameState->contact_list.item_count = 4;
        GameState->contact_list.selected_item = -1;
        
        GameState->email_list.x = 220;
        GameState->email_list.y = 40;
        GameState->email_list.width = 2000;
        GameState->email_list.height = 1180;
        
        /*
        StringCchCopy(GameState->email_list.items[0], sizeof(GameState->email_array[0].from), GameState->email_array[0].from);
        StringCchCopy(GameState->email_list.items[0], sizeof(GameState->email_array[0].subject), GameState->email_array[0].subject);
        StringCchCopy(GameState->email_list.items[0], sizeof(GameState->email_array[0].date), GameState->email_array[0].date);
*/
        
        // NOTE(trist007): testing Email headers display
        for(int i = 0;
            i < GameState->email_count;
            i++)
        {
            StringCchPrintf(GameState->email_list.items[i], sizeof(GameState->email_list.items[i]),
                            EMAIL_FORMAT,
                            GameState->email_array[i].from,
                            GameState->email_array[i].subject,
                            GameState->email_array[i].date);
        }
        
        /*
        StringCchPrintf(GameState->email_list.items[1], sizeof(GameState->email_list.items[1]),
                        GameState->email_array[0].subject);
        
        StringCchPrintf(GameState->email_list.items[1], sizeof(GameState->email_list.items[1]),
                        GameState->email_array[0].date);
*/
        
        GameState->email_list.item_count = GameState->email_count;
        GameState->email_list.selected_item = -1;
        
        
        // Initialize starting mode
        GameState->current_mode = MODE_FOLDER;
    }
}

// Main game functions that get hot reloaded
__declspec(dllexport)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    
    // Draw purple header stripe - try changing this color and recompiling!
    platform->SetColor(0.3f, 0.3f, 0.7f);  // Easy to experiment with colors now
    platform->DrawRect(0,1275, 2250, 200);
    
    platform->SetColor(0.0f, 0.0f, 0.0f);
    platform->DrawRectOutline(0, 1275, 2250, 200);
    
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
    RenderListWithHeader(&GameState->email_list, platform);
    RenderList(&GameState->contact_list, platform);
    
    // Print Legend From | Subject | Received
    /*
    char header_text[256];
    StringCchPrintf(header_text, sizeof(header_text),
                    EMAIL_FORMAT, "From", "Subject", "Received");
    platform->DrawText(platform->GameState, header_text, 225, 1225);
*/
    
    // Email preview
    if(GameState->show_aws_output)
    {
        // show aws output instead of preview
        platform->DrawText(platform->GameState, GameState->aws_output_buffer, 640, 320);
    }
    else if (GameState->email_list.selected_item >= 0)
    {
        char preview_text[256];
        sprintf_s(preview_text, sizeof(preview_text), "Preview of %s - Hot Reloaded!", GameState->email_list.items[GameState->email_list.selected_item]);
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
                case 'K':
                // Move up in folder list
                if (GameState->folder_list.selected_item == 0)
                {
                    (GameState->folder_list.selected_item = GameState->folder_list.item_count - 1);
                } 
                else
                {
                    GameState->folder_list.selected_item--;
                } break;
                
                case 'J':
                // Move down in folder list
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
                        GameState->email_list.selected_item = 0;
                    }
                    break;
                }
            }
        } break;
        
        case MODE_EMAIL:
        {
            switch (key_code)
            {
                case 'K':
                // Move up in email list
                if (GameState->email_list.selected_item == 0)
                {
                    (GameState->email_list.selected_item = GameState->email_list.item_count - 1);
                } 
                else
                {
                    GameState->email_list.selected_item--;
                } break;
                
                case 'J':
                // Move down in email list
                if (GameState->email_list.selected_item == (GameState->email_list.item_count - 1))
                {
                    GameState->email_list.selected_item = 0;
                }
                else
                {
                    GameState->email_list.selected_item++;
                } break;
                
                case 'D':
                {
                    // delete email
                } break;
                
                case 'L':
                {
                    // list files in directory
                    platform->ListFilesInDirectory("C:/Users/Tristan/.email", &GameState->email_array);
                } break;
                
                case 'S':
                {
                    platform->ExecuteAWSCLI(GameState, "aws s3 sync s3://www.darkterminal.net/incoming/ C:/Users/Tristan/.email");
                    
                    char* output_text = platform->ReadProcessOutput(GameState->awscli.stdout_read);
                    if(output_text)
                    {
                        //platform->DrawText(platform->GameState, output_text, 640, 320);
                        strncpy(GameState->aws_output_buffer, output_text, sizeof(GameState->aws_output_buffer) - 1);
                        GameState->aws_output_buffer[sizeof(GameState->aws_output_buffer) - 1] = '\0';
                        GameState->show_aws_output = true;
                    }
                    
                } break;
                
                // open email message
                case VK_RETURN:
                {
                    GameState->current_mode = MODE_READING_EMAIL;
                    break;
                }
                
                // back to preview mode
                case '/':
                {
                    GameState->show_aws_output = false;
                    break;
                }
                
                case 'I':
                // Go back to folder mode
                {
                    GameState->current_mode = MODE_FOLDER;
                    GameState->email_list.selected_item = -1;
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
            switch (key_code)
            {
                // scroll down by a page
                case VK_SPACE:
                {
                    // code for scrolling down by a page
                    break;
                }
                
                // delete email
                case 'D':
                {
                    // code for deleting email
                    break;
                }
                
                // forward email
                case 'F':
                {
                    // code for forwarding email
                    break;
                }
                
                // reply email
                case 'R':
                {
                    // code for reply email
                    break;
                }
                
                // show headers
                case 'H':
                {
                    // code for show email headers
                    break;
                }
                
                // go back to email_list
                case 'I':
                {
                    GameState->current_mode = MODE_EMAIL;
                    break;
                }
            }
            // Reading email mode handling
        } break;
    }
}