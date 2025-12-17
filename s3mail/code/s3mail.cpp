#include <windows.h>

#include "s3mail_shared.h"
#include "s3mail.h"
#include "s3mail_platform.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <time.h>
#include <GL/gl.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Email Layout format string
#define EMAIL_FORMAT "%-30.30s | %-69.69s | %-25.25s"

void
DrawTextGame(game_state *GameState, const char *text, float x, float y)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, GameState->font_texture_id);
    
    float current_x = x;
    float current_y = y;
    
    glBegin(GL_QUADS);
    for (int i = 0;
         text[i];
         i++)
    {
        if (text[i] >= 32 && text[i] < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(GameState->cdata,
                               512,
                               512,
                               text[i] - 32,
                               &current_x,
                               &current_y,
                               &q,
                               1);
            
            float y_flipped_0 = y - (q.y0 - y);
            float y_flipped_1 = y - (q.y1 - y);
            
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, y_flipped_0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, y_flipped_0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, y_flipped_1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, y_flipped_1);
        }
    }
    glEnd();
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void
DrawTextGameEmail(game_state *GameState, const char *text, float x, float y)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, GameState->font_texture_id);
    
    float current_x = x;
    float current_y = y;
    float line_height = 30.0f;
    
    glBegin(GL_QUADS);
    for (int i = 0;
         text[i];
         i++)
    {
        if(text[i] == '\n' || text[i] == '\r')
        {
            // Handle newline
            current_x = x;  // reset to start of line
            current_y += line_height;   // move down one line
            
            // skip \r\n combinations
            if(text[i] == '\r' && text[i+1] == '\n')
            {
                i++;  // skip to next newline
            }
            
        }
        else if(text[i] >= 32 && text[i] < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(GameState->cdata,
                               512,
                               512,
                               text[i] - 32,
                               &current_x,
                               &current_y,
                               &q,
                               1);
            
            float y_flipped_0 = y - (q.y0 - y);
            float y_flipped_1 = y - (q.y1 - y);
            
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, y_flipped_0);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, y_flipped_0);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, y_flipped_1);
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, y_flipped_1);
        }
    }
    glEnd();
    
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void
SetColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

void
DrawRect(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void
DrawRectRatio(float x, float y, float width, float height)
{
    x *= WINDOW_WIDTH_HD;
    y *= WINDOW_HEIGHT_HD;
    width *= WINDOW_WIDTH_HD;
    height *= WINDOW_HEIGHT_HD;
    
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void
DrawRectOutline(float x, float y, float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void
DrawRectOutlineRatio(float x, float y, float width, float height)
{
    x *= WINDOW_WIDTH_HD;
    y *= WINDOW_HEIGHT_HD;
    width *= WINDOW_WIDTH_HD;
    height *= WINDOW_HEIGHT_HD;
    
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

int
PointInRect(int px, int py, float x, float y, float width, float height)
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

// UI update functions
void
UpdateButtonRatio(UIButtonRatio* btn, int mouse_x, int mouse_y, int mouse_down, int window_height)
{
    
    btn->x = WINDOW_WIDTH_HD * btn->x_ratio;
    btn->y = WINDOW_HEIGHT_HD * btn->y_ratio;
    btn->width = WINDOW_WIDTH_HD * btn->width_ratio;
    btn->height = WINDOW_HEIGHT_HD * btn->height_ratio;
    
    int gl_y = window_height - mouse_y;
    btn->is_hovered =  PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void
UpdateButton(UIButton* btn, int mouse_x, int mouse_y, int mouse_down, int window_height)
{
    
    int gl_y = window_height - mouse_y;
    btn->is_hovered =  PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void
RenderButtonRatio(UIButtonRatio* btn, game_state *GameState)
{
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
    DrawTextGame(GameState, btn->text, btn->x + 5, btn->y + 8);
}

void
RenderButton(UIButton* btn, game_state* GameState)
{
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
    DrawTextGame(GameState, btn->text, btn->x + 5, btn->y + 8);
}

void
UpdateListRatio(UIListRatio* list, int mouse_x, int mouse_y, int mouse_down, int window_height)
{
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

void
UpdateList(UIList* list, int mouse_x, int mouse_y, int mouse_down, int window_height)
{
    int gl_y = window_height - mouse_y;
    
    if (mouse_down &&  PointInRect(mouse_x, gl_y, list->x, list->y, list->width, list->height))
    {
        int item_height = 25;
        int clicked_item = (gl_y - list->y) / item_height;
        if (clicked_item >= 0 && clicked_item < list->item_count) {
            list->selected_item = clicked_item;
        }
    }
}

void
UpdateEmailContent(EmailContent* email, int mouse_x, int mouse_y, int mouse_down, int window_height)
{
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

void
UpdateReplyEmail(EmailContent* email, int mouse_x, int mouse_y, int mouse_down, int window_height)
{
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

void
RenderListRatio(UIListRatio* list, game_state* GameState)
{
    
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
        DrawTextGame(GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void
RenderList(UIList* list, game_state* GameState)
{
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
        DrawTextGame(GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void
RenderListWithHeader(UIListRatio* list, game_state* GameState)
{
    
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
    DrawTextGame(GameState, header_text, list->x + 5, header_y + 5);
    
    for (int i = 0; i < list->item_count; i++)
    {
        
        // Calculate y from the top instead of the bottom 
        //float item_y = list->y + i * item_height;
        float item_y = (list->y + list->height) - ((i + 2) * item_height);
        
        if (i == list->selected_item) {
            SetColor(0.5f, 0.7f, 1.0f);  // You can easily tweak these colors now!
            DrawRect(list->x, item_y, list->width, item_height);
        }
        
        SetColor(0.0f, 0.0f, 0.0f);
        DrawTextGame(GameState, list->items[i], list->x + 5, item_y + 5);
    }
}

void
RenderEmailContent(EmailContent* email, game_state* GameState)
{
    
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
    
    if(GameState->email_content[0] == '\0')
    {
        SetColor(1.0f, 0.0f, 0.0f); // Red text for debug
        DrawTextGameEmail(GameState, "EMAIL CONTENT IS EMPTY!", 
                          (0.1146f*WINDOW_WIDTH_HD), (0.1f*WINDOW_HEIGHT_HD));
    }
    else
    {
        // Draw email content within the content area
        SetColor(0.0f, 0.0f, 0.0f);
        
        int render_start_line;
        int render_end_line;
        
        /*
        int lines_to_show = 30;
        int start_line = email->scroll_offset;
        int end_line = min(start_line + lines_to_show, email->item_count);
*/
        
        if(GameState->email_array->showHeaders)
        {
            render_start_line = 0;
            render_end_line = GameState->line_count;
        }
        else
        {
            render_start_line = GameState->email_array->textplain_start;
            render_end_line = GameState->email_array->textplain_end;
        }
        
        // render lines from render_start to render_end
        int lines_to_show = 30;
        int start_line = render_start_line + email->scroll_offset;
        int end_line = min(start_line + lines_to_show, render_end_line);
        
        for(int i = start_line;
            i < end_line;
            i++)
        {
            float line_y = email->y + (email->height - ((i - start_line + 1) * 25));
            DrawTextGameEmail(GameState,
                              GameState->parsed_email[i],
                              email->x + 5,
                              line_y);
        }
        
        
    }
}

void
RenderReplyContent(EmailContent* email, game_state* GameState)
{
    
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
    
    
    
    if(GameState->email_content[0] == '\0')
    {
        SetColor(1.0f, 0.0f, 0.0f); // Red text for debug
        DrawTextGameEmail(GameState, "EMAIL CONTENT IS EMPTY!", 
                          (0.1146f*WINDOW_WIDTH_HD), (0.1f*WINDOW_HEIGHT_HD));
    }
    else
    {
        // Recipients
        GameState->recipient_list.x_ratio = 0.1346f;
        GameState->recipient_list.y_ratio = 0.7731f;
        GameState->recipient_list.width_ratio = 0.85f;
        GameState->recipient_list.height_ratio = 0.0278f;
        strncpy(GameState->recipient_list.text, GameState->email_array->from,
                sizeof(GameState->email_array->from));
        //GameState->recipient_list.is_hovered = 0;
        //GameState->recipient_list.is_pressed = 0;
        
        // Draw email content within the content area
        SetColor(0.0f, 0.0f, 0.0f);
        
        int render_start_line;
        int render_end_line;
        
        /*
        int lines_to_show = 30;
        int start_line = email->scroll_offset;
        int end_line = min(start_line + lines_to_show, email->item_count);
*/
        
        if(GameState->email_array->showHeaders)
        {
            render_start_line = 0;
            render_end_line = GameState->line_count;
        }
        else
        {
            render_start_line = GameState->email_array->textplain_start;
            render_end_line = GameState->email_array->textplain_end;
        }
        
        // render lines from render_start to render_end
        int lines_to_show = 30;
        int start_line = render_start_line + email->scroll_offset;
        int end_line = min(start_line + lines_to_show, render_end_line);
        
        for(int i = start_line;
            i < end_line;
            i++)
        {
            float line_y = email->y + (email->height - ((i - start_line + 1) * 25));
            DrawTextGameEmail(GameState,
                              GameState->parsed_email[i],
                              email->x + 5,
                              line_y);
        }
        
        
    }
}

// Helper function to calculate character position after rendering n chars
void
CalculateTextPosition(game_state* GameState, char *text, int char_count, float start_x,
                      float start_y, float *out_x, float *out_y)
{
    float current_x = start_x;
    float current_y = start_y;
    float line_height = 30.0f;
    
    for(int i = 0;
        i < char_count && text[i];
        i++)
    {
        char c = text[i];
        
        if(c == '\n')
        {
            current_x = start_x;
            current_y -= line_height;
        }
        else if(c >= 32 && c < 128)
        {
            stbtt_bakedchar *b = &GameState->cdata[c - 32];
            current_x += b->xadvance;
        }
    }
    
    *out_x = current_x;
    *out_y = current_y;
}

void
RenderTextInput(game_state* GameState, float x, float y, float width, float height)
{
    // Draw background
    SetColor(1.0f, 1.0f, 1.0f);
    DrawRect(x, y, width, height);
    
    // Draw border
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(x, y, width, height);
    
    // Draw text
    SetColor(0.0f, 0.0f, 0.0f);
    //DrawTextGameEmail(GameState, GameState->reply_body.buffer, x + 5, y + height - 25);
    
    float text_start_x = x + 5;
    float text_start_y = y + height - 25;
    DrawTextGameEmail(GameState, GameState->reply_body.buffer, text_start_x, text_start_y);
    
    // Draw cursor if active
    if (GameState->reply_body.is_active && GameState->reply_body.cursor_visible)
    {
        float cursor_x, cursor_y;
        CalculateTextPosition(GameState, GameState->reply_body.buffer,
                              GameState->reply_body.cursor_position,
                              text_start_x, text_start_y,
                              &cursor_x, &cursor_y);
        SetColor(0.0f, 0.0f, 0.0f);
        DrawRect(cursor_x, cursor_y, 2, 20); // Vertical cursor line
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
        
        // body showing list of emails
        GameState->email_list.x_ratio = 0.1146f;
        GameState->email_list.y_ratio= 0.039f;
        GameState->email_list.width_ratio = 0.87f;
        GameState->email_list.height_ratio = 0.721f;
        
        GameState->email.x_ratio = 0.1146f;
        GameState->email.y_ratio= 0.039f;
        GameState->email.width_ratio = 0.87f;
        GameState->email.height_ratio = 0.721f;
        GameState->email.scroll_offset = 0;
        
        // To
        GameState->to_button.x_ratio = 0.1146f;
        GameState->to_button.y_ratio = 0.7731f;
        GameState->to_button.width_ratio = 0.87f;
        GameState->to_button.height_ratio = 0.0278f;
        strncpy(GameState->to_button.text,
                "To: ", 4);
        
        GameState->to_button.is_hovered = 0;
        GameState->to_button.is_pressed = 0;
        
        // From
        GameState->from_button.x_ratio = 0.1146f;
        GameState->from_button.y_ratio = 0.7731f;
        GameState->from_button.width_ratio = 0.87f;
        //GameState->from_button.height_ratio = 0.0278f;
        GameState->from_button.height_ratio = 0.0278f;
        strncpy(GameState->from_button.text,
                "From: ", 6);
        
        GameState->from_button.is_hovered = 0;
        GameState->from_button.is_pressed = 0;
        
        // Reply text input
        GameState->reply_body.buffer[0] = '\0';
        GameState->reply_body.cursor_position = 0;
        GameState->reply_body.buffer_length = 0;
        GameState->reply_body.is_active = 0;
        GameState->reply_body.blink_timer = 0.0f;
        GameState->reply_body.cursor_visible = 1;
        
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
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    
    // Draw purple header stripe - try changing this color and recompiling!
    SetColor(0.3f, 0.3f, 0.7f);  // Easy to experiment with colors now
    //DrawRect(0, 0.8102f, 1.0f, 0.1852f);
    DrawRectRatio(0.0f, 0.8102f, 1.0f, 0.1852f);
    
    SetColor(0.0f, 0.0f, 0.0f);
    //DrawRectOutline(0, 0.8102f, 1.0f, 0.1852f);
    DrawRectOutlineRatio(0.0f, 0.8102f, 1.0f, 0.1852f);
    
    // Update UI elements
    UpdateButtonRatio(&GameState->compose_button, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
    UpdateButtonRatio(&GameState->delete_button, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
    UpdateListRatio(&GameState->folder_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
    
    switch(GameState->current_mode)
    {
        case MODE_FOLDER:
        case MODE_CONTACT:
        case MODE_EMAIL:
        {
            UpdateListRatio(&GameState->email_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
        } break;
        
        case MODE_READING_EMAIL:
        {
            UpdateEmailContent(&GameState->email, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
        } break;
        
        case MODE_REPLYING_EMAIL:
        {
            //UpdateReplyEmail(&GameState->email, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
        }
    }
    
    UpdateListRatio(&GameState->contact_list, GameState->mouse_x, GameState->mouse_y, GameState->mouse_down, GameState->window_height);
    
    // Render UI elements
    RenderButtonRatio(&GameState->compose_button, GameState);
    RenderButtonRatio(&GameState->delete_button, GameState);
    RenderListRatio(&GameState->folder_list, GameState);
    
    switch(GameState->current_mode)
    {
        case MODE_FOLDER:
        case MODE_CONTACT:
        case MODE_EMAIL:
        {
            RenderListWithHeader(&GameState->email_list, GameState);
        } break;
        
        case MODE_READING_EMAIL:
        {
            RenderButtonRatio(&GameState->from_button, GameState);
            RenderEmailContent(&GameState->email, GameState);
        } break;
        
        case MODE_REPLYING_EMAIL:
        {
            // Update cursor blink, assuming 60 fps
            GameState->reply_body.blink_timer += 0.016f;
            if(GameState->reply_body.blink_timer > 0.5f)
            {
                GameState->reply_body.cursor_visible = !GameState->reply_body.cursor_visible;
                
                GameState->reply_body.blink_timer = 0.0f;
            }
            
            RenderButtonRatio(&GameState->to_button, GameState);
            
            // Render the reply composition
            RenderTextInput(GameState,
                            0.1146f * WINDOW_WIDTH_HD,
                            0.4f * WINDOW_HEIGHT_HD,
                            0.87f * WINDOW_WIDTH_HD,
                            0.35f * WINDOW_HEIGHT_HD);
            
            //RenderReplyContent(&GameState->email, GameState);
        } break;
    }
    
    RenderListRatio(&GameState->contact_list, GameState);
    
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
            DrawTextGame(GameState, GameState->aws_output_buffer, 640, 320);
        }
        else if (GameState->email_list.selected_item >= 0)
        {
            char preview_text[256];
            sprintf_s(preview_text, sizeof(preview_text), "Preview of %s - Hot Reloaded!", GameState->email_list.items[GameState->email_list.selected_item]);
            DrawTextGame(GameState, preview_text, 640, 120);
        }
    }
    
    // Status bar
    SetColor(0.8f, 0.8f, 0.8f);
    DrawRect(0, 0, GameState->window_width, 25);
    SetColor(0.0f, 0.0f, 0.0f);
    DrawTextGame(GameState, "S3Mail - Hot Reloadable Ready!", 10, 8);
    
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
                    
                    char* output_text = platform->ReadProcessOutput(&GameState->awscli->stdout_read);
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
                    
                    debug_read_file_result Result = Memory->DEBUGPlatformReadEntireFile(&Thread, PathToFile, Memory);
                    
                    /*
                    size_t copy_size = (Result.ContentsSize) <
                    (sizeof(GameState->email_content) - 1) ?
                        Result.ContentsSize : sizeof(GameState->email_content) - 1;
                    */
                    
                    memmove(GameState->email_content, Result.Contents, Result.ContentsSize - 1);
                    GameState->email_content[Result.ContentsSize] = '\0';
                    Memory->DEBUGPlatformFreeFileMemory(&Thread, Result.Contents);
                    GameState->email_array->header_lines = FindHeaderLines(GameState->email_content);
                    
                    // NOTE(trist007): adding +6 to show From: 
                    memmove(GameState->from_button.text+6, GameState->email_array->from, sizeof(GameState->email_array->from));
                    ParseEmail(GameState->email_content, GameState->parsed_email, &GameState->line_count);
                    
                    int textplain_start_line = FindTextPlainContent(GameState->parsed_email, GameState->line_count);
                    
                    if(textplain_start_line >= 0)
                    {
                        int textplain_end_line = FindTextPlainEnd(GameState->parsed_email,
                                                                  GameState->line_count,
                                                                  textplain_start_line);
                        
                        GameState->email_array->textplain_start = textplain_start_line;
                        GameState->email_array->textplain_end = textplain_end_line;
                    }
                    
                    
                    GameState->email.selected_item = 0;
                    GameState->email.item_count = GameState->line_count;
                    
                    // reset scroll position for new email
                    GameState->email.scroll_offset = 0;
                    
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
                // show headers
                case 'H':
                {
                    GameState->email_array->showHeaders =
                        !GameState->email_array->showHeaders;
                    
                    // reset scroll offset
                    GameState->email.scroll_offset = 0;
                    //GameState->email.scroll_offset = 0;
                } break;
                // scroll down one line
                /*
                case 'J':
                {
                    if(GameState->email.scroll_offset < GameState->email.item_count - 1)
                    {
                        GameState->email.scroll_offset++;
                    }
                } break;
                */
                case 'J':
                {
                    int render_start_line = GameState->email_array->showHeaders ? 
                        0 : GameState->email_array->textplain_start;
                    int render_end_line = GameState->email_array->showHeaders ? 
                        GameState->line_count : GameState->email_array->textplain_end;
                    int lines_per_page = 30;
                    int total_lines = render_end_line - render_start_line;
                    
                    // Only scroll if content extends beyond visible area
                    if(total_lines > lines_per_page && 
                       GameState->email.scroll_offset < total_lines - lines_per_page)
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
                    int render_start_line = GameState->email_array->showHeaders ? 
                        0 : GameState->email_array->textplain_start;
                    int render_end_line = GameState->email_array->showHeaders ? 
                        GameState->line_count : GameState->email_array->textplain_end;
                    int lines_per_page = 30;
                    int total_lines = render_end_line - render_start_line;
                    
                    // Only scroll if content extends beyond visible area
                    if(total_lines > lines_per_page)
                    {
                        GameState->email.scroll_offset += lines_per_page;
                        int max_scroll = total_lines - lines_per_page;
                        if(GameState->email.scroll_offset > max_scroll)
                        {
                            GameState->email.scroll_offset = max_scroll;
                        }
                    }
                } break;
                /*
                case VK_SPACE:
                {
                    int lines_per_page = 30;
                    GameState->email.scroll_offset += lines_per_page;
                    if(GameState->email.scroll_offset >= GameState->email.item_count)
                    {
                        GameState->email.scroll_offset = GameState->email.item_count - 1;
                    }
                } break;
                */
                // scroll up a page
                case VK_OEM_MINUS:
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
                    GameState->current_mode = MODE_REPLYING_EMAIL;
                    
                    // pre-populate the reply buffer with the email thread
                    char separator[] = "\n--------------------------------------------------\n";
                    size_t separator_len = strlen(separator);
                    
                    // start with empty reply aread
                    GameState->reply_body.buffer[0] = '\0';
                    GameState->reply_body.buffer_length = 0;
                    GameState->reply_body.cursor_position = 0;
                    
                    // add some blank lines for the user to type their response
                    strcat(GameState->reply_body.buffer, "\n\n");
                    GameState->reply_body.buffer_length = 2;
                    GameState->reply_body.cursor_position = 0;
                    
                    // add separator
                    strcat(GameState->reply_body.buffer, separator);
                    GameState->reply_body.buffer_length += separator_len;
                    
                    // add original email content text/plain section only
                    int start_line = GameState->email_array->textplain_start;
                    int end_line = GameState->email_array->textplain_end;
                    
                    // copy the plain text content
                    for(int i = start_line;
                        i < end_line;
                        i++)
                    {
                        size_t line_len = strlen(GameState->parsed_email[i]);
                        
                        // check if we have room
                        if(GameState->reply_body.buffer_length + line_len + 1 < sizeof(GameState->reply_body.buffer) - 1)
                        {
                            strcat(GameState->reply_body.buffer, GameState->parsed_email[i]);
                            strcat(GameState->reply_body.buffer, "\n");
                            GameState->reply_body.buffer_length += line_len + 1;
                        }
                    }
                    
                    // set cursor position to the beginning
                    GameState->reply_body.cursor_position = 0;
                    GameState->reply_body.is_active = 1;
                    
                    // fill in To: header
                    memmove(GameState->to_button.text+4, GameState->email_array->from, sizeof(GameState->email_array->from));
                } break;
                
                // go back to email_list
                case 'I':
                {
                    GameState->current_mode = MODE_EMAIL;
                    GameState->email_array->showHeaders = 0;
                } break;
            }
            // Reading email mode handling
        } break;
        case MODE_REPLYING_EMAIL:
        {
            // Activate text input when user starts typing
            // all printable chars in ASCII except i (105) and y (121)
            if(!GameState->reply_body.is_active &&
               (key_code >= 32 && key_code < 105) &&
               (key_code >= 106 && key_code < 121) &&
               (key_code >= 122 &&key_code < 128))
            {
                GameState->reply_body.is_active = 1;
            }
            
            if(GameState->reply_body.is_active)
            {
                switch (key_code)
                {
                    // Backspace
                    case VK_BACK:
                    {
                        // Remove character before cursor
                        memmove(&GameState->reply_body.buffer[GameState->reply_body.cursor_position - 1],
                                &GameState->reply_body.buffer[GameState->reply_body.cursor_position],
                                GameState->reply_body.buffer_length -
                                GameState->reply_body.cursor_position + 1);
                        
                        //GameState->reply_body.buffer[GameState->reply_body.cursor_position]
                        //= '\n';
                        GameState->reply_body.cursor_position--;
                        GameState->reply_body.buffer_length--;
                    } break;
                    
                    case VK_LEFT:
                    {
                        if(GameState->reply_body.cursor_position > 0)
                        {
                            GameState->reply_body.cursor_position--;
                        }
                    } break;
                    
                    case VK_RIGHT:
                    {
                        if(GameState->reply_body.cursor_position < GameState->reply_body.buffer_length)
                        {
                            GameState->reply_body.cursor_position++;
                        }
                    } break;
                    
                    
                    case VK_RETURN:
                    {
                        if(GameState->reply_body.buffer_length < sizeof(GameState->reply_body.buffer) - 1)
                        {
                            memmove(&GameState->reply_body.buffer[GameState->reply_body.cursor_position + 1],
                                    &GameState->reply_body.buffer[GameState->reply_body.cursor_position],
                                    GameState->reply_body.buffer_length - GameState->reply_body.cursor_position + 1);
                            
                            GameState->reply_body.buffer[GameState->reply_body.cursor_position] = '\n';
                            GameState->reply_body.cursor_position++;
                            GameState->reply_body.buffer_length++;
                        }
                    } break;
                    
                    // exit text input mode
                    case VK_ESCAPE:
                    {
                        GameState->reply_body.is_active = 0;
                        //GameState->current_mode = MODE_READING_EMAIL;
                    } break;
                    
                    default:
                    {
                        // handle regular char input
                        if(GameState->reply_body.buffer_length < sizeof(GameState->reply_body.buffer) - 1)
                        {
                            key_translation key = platform->KeyCodeToChar(key_code);
                            
                            if(key.valid)
                            {
                                
                                /*
    char c = 0;
                                bool32 shift_held = platform->IsKeyPressed(VK_SHIFT);
                                
                                
                                // Handle letters
                                if(key_code >= 'A' && key_code <= 'Z')
                                {
                                    c = shift_held ? key_code : (key_code + 32);
                                }
                                
                                // Handle all other printable chars (including space)
                                else if(key_code >= '0' && key_code <= '9')
                                {
                                    // Handle shift+number for symbols like !, @, #, etc.
                                    if(shift_held)
                                    {
                                        const char shift_numbers[] = ")!@#$%^&*(";
                                        c = shift_numbers[key_code - '0'];
                                    }
                                    else
                                    {
                                        c = (char)key_code;
                                    }
                                }
                                
                                // Handle space and other directly mapped keys
                                else if(key_code == VK_SPACE)
                                {
                                    c = ' ';
                                }
                                
                                // Handle OEM keys
                                else if(key_code == VK_OEM_MINUS)  // minus/underscore key
                                {
                                    c = shift_held ? '_' : '-';
                                }
                                
                                else if(key_code == VK_OEM_PLUS)   // equals/plus key
                                {
                                    c = shift_held ? '+' : '=';
                                }
                                
                                else if(key_code == VK_OEM_COMMA)
                                {
                                    c = shift_held ? '<' : ',';
                                }
                                
                                else if(key_code == VK_OEM_PERIOD)
                                {
                                    c = shift_held ? '>' : '.';
                                }
                                
                                else if(key_code == VK_OEM_2)  // forward slash/question mark
                                {
                                    c = shift_held ? '?' : '/';
                                }
                                
                                else if(key_code >= 32 && key_code < 128)
                                {
                                    c = (char)key_code;
                                }
                                
                                if(c != 0)
                                {
                                    */
                                // Insert character at cursor position
                                memmove(&GameState->reply_body.buffer[GameState->reply_body.cursor_position + 1],
                                        &GameState->reply_body.buffer[GameState->reply_body.cursor_position],
                                        GameState->reply_body.buffer_length - GameState->reply_body.cursor_position + 1);
                                
                                GameState->reply_body.buffer[GameState->reply_body.cursor_position] = key.character;
                                GameState->reply_body.cursor_position++;
                                GameState->reply_body.buffer_length++;
                            }
                        }
                    } break;
                }
            }
            else
            {
                switch(key_code)
                {
                    // De-activate edit mode into command mode
                    case 'I':
                    {
                        GameState->current_mode = MODE_READING_EMAIL;
                        
                        // zero out reply_body.buffer
                        memset(GameState->reply_body.buffer, 0, sizeof(GameState->reply_body.buffer));
                        GameState->reply_body.buffer_length = 0;
                        GameState->reply_body.cursor_position = 0;
                        GameState->reply_body.buffer_length = 0;
                    } break;
                    
                    // send email
                    case 'Y':
                    {
                        // TODO(trist007): send email command
                    } break;
                }
            }
        } break;
    }
}