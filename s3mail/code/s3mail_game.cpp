#include "s3mail_platform.h"
#include "s3mail.h"
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>

// Email Layout format string
#define EMAIL_FORMAT "%-30.30s | %-69.69s | %-25.25s"

internal bool32 reinit_ui = true;

// UI update functions
void UpdateButtonRatio(UIButtonRatio* btn, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    
    btn->x = WINDOW_WIDTH_HD * btn->x_ratio;
    btn->y = WINDOW_HEIGHT_HD * btn->y_ratio;
    btn->width = WINDOW_WIDTH_HD * btn->width_ratio;
    btn->height = WINDOW_HEIGHT_HD * btn->height_ratio;
    
    int gl_y = window_height - mouse_y;
    btn->is_hovered =  PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void UpdateButton(UIButton* btn, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    
    int gl_y = window_height - mouse_y;
    btn->is_hovered =  PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void RenderButtonRatio(UIButtonRatio* btn, PlatformAPI* platform) {
    if (btn->is_pressed) {
        SetColor(0.2f, 0.2f, 0.6f);
    } else if (btn->is_hovered) {
        SetColor(0.4f, 0.4f, 0.8f);
    } else {
        SetColor(0.3f, 0.3f, 0.7f);
    }
    
    btn->x = WINDOW_WIDTH_HD * btn->x_ratio;
    btn->y = WINDOW_HEIGHT_HD * btn->y_ratio;
    btn->width = WINDOW_WIDTH_HD * btn->width_ratio;
    btn->height = WINDOW_HEIGHT_HD * btn->height_ratio;
    
    DrawRect(btn->x, btn->y, btn->width, btn->height);
    
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(btn->x, btn->y, btn->width, btn->height);
    
    SetColor(1.0f, 1.0f, 1.0f);
    DrawTextGame(platform->GameState, btn->text, btn->x + 5, btn->y + 8);
}

void RenderButton(UIButton* btn, PlatformAPI* platform) {
    if (btn->is_pressed) {
        SetColor(0.2f, 0.2f, 0.6f);
    } else if (btn->is_hovered) {
        SetColor(0.4f, 0.4f, 0.8f);
    } else {
        SetColor(0.3f, 0.3f, 0.7f);
    }
    
    DrawRect(btn->x, btn->y, btn->width, btn->height);
    
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(btn->x, btn->y, btn->width, btn->height);
    
    SetColor(1.0f, 1.0f, 1.0f);
    DrawTextGame(platform->GameState, btn->text, btn->x + 5, btn->y + 8);
}

void UpdateListRatio(UIListRatio* list, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    
    list->x = WINDOW_WIDTH_HD * list->x_ratio;
    list->y = WINDOW_HEIGHT_HD * list->y_ratio;
    list->width = WINDOW_WIDTH_HD * list->width_ratio;
    list->height = WINDOW_HEIGHT_HD * list->height_ratio;
    
    if (mouse_down &&  PointInRect(mouse_x, gl_y, list->x, list->y, list->width, list->height)) {
        int item_height = 25;
        int clicked_item = (gl_y - list->y) / item_height;
        if (clicked_item >= 0 && clicked_item < list->item_count) {
            list->selected_item = clicked_item;
        }
    }
}

void UpdateList(UIList* list, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    
    if (mouse_down &&  PointInRect(mouse_x, gl_y, list->x, list->y, list->width, list->height)) {
        int item_height = 25;
        int clicked_item = (gl_y - list->y) / item_height;
        if (clicked_item >= 0 && clicked_item < list->item_count) {
            list->selected_item = clicked_item;
        }
    }
}

void UpdateEmailContent(EmailContent* email, int mouse_x, int mouse_y, int mouse_down, int window_height, PlatformAPI* platform) {
    int gl_y = window_height - mouse_y;
    
    email->x = WINDOW_WIDTH_HD * email->x_ratio;
    email->y = WINDOW_HEIGHT_HD * email->y_ratio;
    email->width = WINDOW_WIDTH_HD * email->width_ratio;
    email->height = WINDOW_HEIGHT_HD * email->height_ratio;
    
    if (mouse_down &&  PointInRect(mouse_x, gl_y, email->x, email->y, email->width, email->height)) {
        int item_height = 25;
        int clicked_item = (gl_y - email->y) / item_height;
        if (clicked_item >= 0 && clicked_item < email->item_count) {
            email->selected_item = clicked_item + email->scroll_offset;
        }
    }
    
}

void RenderListRatio(UIListRatio* list, PlatformAPI* platform) {
    
    list->x = WINDOW_WIDTH_HD * list->x_ratio;
    list->y = WINDOW_HEIGHT_HD * list->y_ratio;
    list->width = WINDOW_WIDTH_HD * list->width_ratio;
    list->height = WINDOW_HEIGHT_HD * list->height_ratio;
    
    // Background
    SetColor(0.9f, 0.9f, 0.9f);
    DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    for (int i = 0; i < list->item_count; i++) {
        
        // Calculate y from the top instead of the bottom 
        //float item_y = list->y + i * item_height;
        float item_y = (list->y + list->height) - ((i + 1) * item_height);
        
        if (i == list->selected_item) {
            SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            DrawRect(list->x, item_y, list->width, item_height);
        }
        
        SetColor(0.0f, 0.0f, 0.0f);
        DrawTextGame(platform->GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void RenderList(UIList* list, PlatformAPI* platform) {
    // Background
    SetColor(0.9f, 0.9f, 0.9f);
    DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    for (int i = 0; i < list->item_count; i++) {
        
        // Calculate y from the top instead of the bottom 
        //float item_y = list->y + i * item_height;
        float item_y = (list->y + list->height) - ((i + 1) * item_height);
        
        if (i == list->selected_item) {
            SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            DrawRect(list->x, item_y, list->width, item_height);
        }
        
        SetColor(0.0f, 0.0f, 0.0f);
        DrawTextGame(platform->GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void RenderListWithHeader(UIListRatio* list, PlatformAPI* platform) {
    
    list->x = WINDOW_WIDTH_HD * list->x_ratio;
    list->y = WINDOW_HEIGHT_HD * list->y_ratio;
    list->width = WINDOW_WIDTH_HD * list->width_ratio;
    list->height = WINDOW_HEIGHT_HD * list->height_ratio;
    
    // Background
    SetColor(0.9f, 0.9f, 0.9f);
    DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    
    char header_text[256];
    snprintf(header_text, sizeof(header_text),
             EMAIL_FORMAT, "From", "Subject", "Received");
    
    float header_y = (list->y + list->height) - item_height;  // Top row
    SetColor(0.7f, 0.7f, 0.7f);  // Different color for header
    DrawRect(list->x, header_y, list->width, item_height);
    SetColor(0.0f, 0.0f, 0.0f);
    DrawTextGame(platform->GameState, header_text, list->x + 5, header_y + 5);
    
    for (int i = 0; i < list->item_count; i++) {
        
        // Calculate y from the top instead of the bottom 
        //float item_y = list->y + i * item_height;
        float item_y = (list->y + list->height) - ((i + 2) * item_height);
        
        if (i == list->selected_item) {
            SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            DrawRect(list->x, item_y, list->width, item_height);
        }
        
        SetColor(0.0f, 0.0f, 0.0f);
        DrawTextGame(platform->GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void RenderEmailContent(EmailContent* email, PlatformAPI* platform) {
    
    email->x = WINDOW_WIDTH_HD * email->x_ratio;
    email->y = WINDOW_HEIGHT_HD * email->y_ratio;
    email->width = WINDOW_WIDTH_HD * email->width_ratio;
    email->height = WINDOW_HEIGHT_HD * email->height_ratio;
    
    // Background
    SetColor(0.9f, 0.9f, 0.9f);
    DrawRect(email->x, email->y, email->width, email->height);
    
    // Border
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(email->x, email->y, email->width, email->height);
    
    if(platform->GameState->email_content[0] == '\0')
    {
        SetColor(1.0f, 0.0f, 0.0f); // Red text for debug
        DrawTextGameEmail(platform->GameState, "EMAIL CONTENT IS EMPTY!", 
                          (0.1146f*WINDOW_WIDTH_HD), (0.1f*WINDOW_HEIGHT_HD));
    }
    else
    {
        // Draw email content within the content area
        SetColor(0.0f, 0.0f, 0.0f);
        int lines_to_show = 30;
        int start_line = email->scroll_offset;
        int end_line = min(start_line + lines_to_show, email->item_count);
        
        for(int i = start_line; i < end_line; i++) {
            float line_y = email->y + (email->height - ((i - start_line + 1) * 25));
            DrawTextGameEmail(platform->GameState, 
                              platform->GameState->parsed_email[i], 
                              email->x + 5, 
                              line_y);
        }
        /*
        DrawTextGameEmail(platform->GameState,
                          platform->GameState->parsed_email[platform->GameState->email.selected_item],
                          (0.1146f*WINDOW_WIDTH_HD),
                          (0.0092f*WINDOW_HEIGHT_HD));
    }
*/
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
        GameState->compose_button.x_ratio = 0.0052f;
        GameState->compose_button.y_ratio = 0.7731f;
        GameState->compose_button.width_ratio = 0.0521f;
        GameState->compose_button.height_ratio = 0.02778;
        strncpy(GameState->compose_button.text,
                "Compose", ArrayCount(GameState->compose_button.text));
        
        
        GameState->compose_button.is_hovered = 0;
        GameState->compose_button.is_pressed = 0;
        
        GameState->delete_button.x_ratio = 0.0625f;
        GameState->delete_button.y_ratio = 0.7731f;
        GameState->delete_button.width_ratio = 0.0469f;
        GameState->delete_button.height_ratio = 0.0278f;
        strncpy(GameState->delete_button.text,
                "Delete", ArrayCount(GameState->delete_button.text));
        GameState->delete_button.is_hovered = 0;
        GameState->delete_button.is_pressed = 0;
        
        GameState->folder_list.x_ratio = 0.0052f;
        GameState->folder_list.y_ratio = 0.6435f;
        GameState->folder_list.width_ratio = 0.1042f;
        GameState->folder_list.height_ratio = 0.1157f;
        strncpy(GameState->folder_list.items[0], "Inbox", sizeof(GameState->folder_list.items[0]));
        strncpy(GameState->folder_list.items[1], "Sent", sizeof(GameState->folder_list.items[1]));
        strncpy(GameState->folder_list.items[2], "Draft", sizeof(GameState->folder_list.items[2]));
        strncpy(GameState->folder_list.items[3], "Junk", sizeof(GameState->folder_list.items[3]));
        strncpy(GameState->folder_list.items[4], "Trash", sizeof(GameState->folder_list.items[4]));
        GameState->folder_list.item_count = 5;
        GameState->folder_list.selected_item = 0;
        
        GameState->contact_list.x_ratio = 0.0052f;
        GameState->contact_list.y_ratio = 0.5370f;
        GameState->contact_list.width_ratio = 0.1042f;
        GameState->contact_list.height_ratio = 0.0926f;
        strncpy(GameState->contact_list.items[0], "Papi", sizeof(GameState->contact_list.items[0]));
        strncpy(GameState->contact_list.items[1], "Mom", sizeof(GameState->contact_list.items[1]));
        strncpy(GameState->contact_list.items[2], "Glen", sizeof(GameState->contact_list.items[2]));
        strncpy(GameState->contact_list.items[3], "Vito", sizeof(GameState->contact_list.items[3]));
        GameState->contact_list.item_count = 4;
        GameState->contact_list.selected_item = -1;
        
        GameState->email_list.x_ratio = 0.1146f;
        GameState->email_list.y_ratio= 0.039f;
        GameState->email_list.width_ratio = 0.87f;
        GameState->email_list.height_ratio = 0.721f;
        
        GameState->email.x_ratio = 0.1146f;
        GameState->email.y_ratio= 0.039f;
        GameState->email.width_ratio = 0.87f;
        GameState->email.height_ratio = 0.721f;
        GameState->email.scroll_offset = 0;
        
        // NOTE(trist007): testing Email headers display
        for(int i = 0;
            i < GameState->email_count;
            i++)
        {
            snprintf(GameState->email_list.items[i], sizeof(GameState->email_list.items[i]),
                     EMAIL_FORMAT,
                     GameState->email_array[i].from,
                     GameState->email_array[i].subject,
                     GameState->email_array[i].date);
        }
        
        GameState->email_list.item_count = GameState->email_count;
        GameState->email_list.selected_item = -1;
    }
}

// Main game functions that get hot reloaded
__declspec(dllexport)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    
    // Draw purple header stripe - try changing this color and recompiling!
    SetColor(0.3f, 0.3f, 0.7f);  // Easy to experiment with colors now
    //DrawRect(0, 0.8102f, 1.0f, 0.1852f);
    DrawRectRatio(0.0f, 0.8102f, 1.0f, 0.1852f);
    
    SetColor(0.0f, 0.0f, 0.0f);
    //DrawRectOutline(0, 0.8102f, 1.0f, 0.1852f);
    DrawRectOutlineRatio(0.0f, 0.8102f, 1.0f, 0.1852f);
    
    // Update UI elements
    UpdateButtonRatio(&GameState->compose_button, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    UpdateButtonRatio(&GameState->delete_button, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    UpdateListRatio(&GameState->folder_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    
    if(GameState->current_mode == MODE_FOLDER ||
       GameState->current_mode == MODE_EMAIL ||
       GameState->current_mode == MODE_CONTACT ||
       GameState->current_mode == MODE_EMAIL)
    {
        UpdateListRatio(&GameState->email_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    }
    else
    {
        UpdateEmailContent(&GameState->email, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    }
    
    UpdateListRatio(&GameState->contact_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height, platform);
    
    // Render UI elements
    RenderButtonRatio(&GameState->compose_button, platform);
    RenderButtonRatio(&GameState->delete_button, platform);
    RenderListRatio(&GameState->folder_list, platform);
    
    if(GameState->current_mode == MODE_FOLDER ||
       GameState->current_mode == MODE_EMAIL ||
       GameState->current_mode == MODE_CONTACT ||
       GameState->current_mode == MODE_EMAIL)
    {
        RenderListWithHeader(&GameState->email_list, platform);
    }
    else
    {
        RenderEmailContent(&GameState->email, platform);
    }
    
    RenderListRatio(&GameState->contact_list, platform);
    
    // Print Legend From | Subject | Received
    /*
    char header_text[256];
    StringCchPrintf(header_text, sizeof(header_text),
                    EMAIL_FORMAT, "From", "Subject", "Received");
    DrawTextGame(platform->GameState, header_text, 225, 1225);
*/
    
    // Email preview
    if(GameState->current_mode != MODE_READING_EMAIL)
    {
        if(GameState->show_aws_output)
        {
            // show aws output instead of preview
            DrawTextGame(platform->GameState, GameState->aws_output_buffer, 640, 320);
        }
        else if (GameState->email_list.selected_item >= 0)
        {
            char preview_text[256];
            sprintf_s(preview_text, sizeof(preview_text), "Preview of %s - Hot Reloaded!", GameState->email_list.items[GameState->email_list.selected_item]);
            DrawTextGame(platform->GameState, preview_text, 640, 120);
        }
    }
    
    // Status bar
    SetColor(0.8f, 0.8f, 0.8f);
    DrawRect(0, 0, GameState->window_width, 25);
    SetColor(0.0f, 0.0f, 0.0f);
    DrawTextGame(platform->GameState, "S3Mail - Hot Reloadable Ready!", 10, 8);
    
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
    thread_context Thread;
    
    char PathToFile[256] = {};
    
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
                    } break;
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
                        strncpy(GameState->aws_output_buffer, output_text, sizeof(GameState->aws_output_buffer) - 1);
                        GameState->aws_output_buffer[sizeof(GameState->aws_output_buffer) - 1] = '\0';
                        GameState->show_aws_output = true;
                    }
                    
                } break;
                
                // open email message
                case VK_SPACE:
                {
                    GameState->current_mode = MODE_READING_EMAIL;
                    
                    char *filename = GameState->email_array[GameState->email_list.selected_item].filename;
                    
                    snprintf(PathToFile, MAX_PATH, "C:/Users/Tristan/.email/%s", filename);
                    
                    debug_read_file_result Result = platform->DEBUGPlatformReadEntireFile(&Thread, PathToFile);
                    
                    size_t copy_size = (Result.ContentsSize) <
                    (sizeof(GameState->email_content) - 1) ?
                        Result.ContentsSize : sizeof(GameState->email_content) - 1;
                    
                    memmove(GameState->email_content, Result.Contents, copy_size);
                    GameState->email_content[copy_size] = '\0';
                    platform->DEBUGPlatformFreeFileMemory(&Thread, Result.Contents);
                    ParseEmail(GameState->email_content, GameState->parsed_email, &GameState->line_count);
                    GameState->email.selected_item = 0;
                    GameState->email.item_count = GameState->line_count;
                    
                } break;
                
                // back to preview mode
                case '/':
                {
                    GameState->show_aws_output = false;
                } break;
                
                case 'I':
                // Go back to folder mode
                {
                    GameState->current_mode = MODE_FOLDER;
                    GameState->email_list.selected_item = -1;
                } break;
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
                // scroll down one line
                case 'J':
                {
                    if(GameState->email.scroll_offset < GameState->email.item_count - 1)
                    {
                        GameState->email.scroll_offset++;
                    }
                } break;
                
                // scroll up one line
                case 'K':
                {
                    if(GameState->email.scroll_offset > 0)
                    {
                        GameState->email.scroll_offset--;
                    }
                } break;
                
                // scroll down a page
                case VK_SPACE:
                {
                    int lines_per_page = 30;
                    GameState->email.scroll_offset += lines_per_page;
                    if(GameState->email.scroll_offset >= GameState->email.item_count)
                    {
                        GameState->email.scroll_offset = GameState->email.item_count - 1;
                    }
                } break;
                
                // scroll up a page
                case '-':
                {
                    int lines_per_page = 30;
                    GameState->email.scroll_offset -= lines_per_page;
                    if(GameState->email.scroll_offset < 0)
                    {
                        GameState->email.scroll_offset = 0;
                    }
                } break;
                
                // delete email
                case 'D':
                {
                    // code for deleting email
                } break;
                
                // forward email
                case 'F':
                {
                    // code for forwarding email
                } break;
                
                // reply email
                case 'R':
                {
                    // code for reply email
                } break;
                
                // show headers
                case 'H':
                {
                    // code for show email headers
                } break;
                
                // go back to email_list
                case 'I':
                {
                    GameState->current_mode = MODE_EMAIL;
                } break;
            }
            // Reading email mode handling
        } break;
    }
}