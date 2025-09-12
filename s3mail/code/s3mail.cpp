#include <windows.h>
#include "stb_truetype.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "s3mail.h"
#include "s3mail_platform.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strsafe.h>
#include <time.h>
#include <GL/gl.h>

// OpenGL functions
void
HandleResizey(int width, int height)
{
    if (height == 0) height = 1;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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
            current_y -= line_height;   // move down one line
            
            // ski; \r\n combinations
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

int
InitFont(game_state *GameState, const char *font_path)
{
    static unsigned char font_buffer[1024*1024];
    static unsigned char *font_bitmap;
    stbtt_fontinfo font;
    
    FILE *file = fopen(font_path, "rb");
    if (!file) return 0;
    
    size_t size = fread(font_buffer, 1, 1024*1024, file);
    fclose(file);
    if (size == 0) return 0;
    
    stbtt_InitFont(&font, font_buffer, 0);
    
    font_bitmap = (unsigned char*)malloc(512*512);
    stbtt_BakeFontBitmap(font_buffer, 0, 24.0f, font_bitmap, 512, 512, 32, 96, GameState->cdata);
    
    glGenTextures(1, &GameState->font_texture_id);
    glBindTexture(GL_TEXTURE_2D, GameState->font_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    free(font_bitmap);
    
    return(1);
}

int
PointInRect(int px, int py, float x, float y, float width, float height)
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

// utility functions
int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(casey): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}

void
DecodeQPString(char *input, char* output, size_t output_size)
{
    char *src = input;
    char *dst = output;
    
    while(*src && (size_t)(dst - output) < (output_size - 1))
    {
        if(*src == '=' && src[1] && src[2])
        {
            // Convert hex to char
            int hex_val = 0;
            sscanf(src + 1, "%2x", &hex_val);
            *dst++ = (char)hex_val;
            src += 3;
        }
        else if(*src == '_')
        {
            *dst++ = ' ';  // Underscore represents space
            src++;
        }
        else if(*src != '\0')
        {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
}

void
DecodeSubjectIfNeeded(char *subject)
{
    char *start = strstr(subject, "=?utf-8?Q?");
    if(start)
    {
        char *encoded_start = start + 10;  // Skip "=?utf-8?Q?"
        char *encoded_end = strstr(encoded_start, "?=");
        
        if(encoded_end)
        {
            // Extract just the encoded portion
            size_t encoded_len = encoded_end - encoded_start;
            char encoded_part[256];
            snprintf(encoded_part, sizeof(encoded_part), "%.*s", (int)encoded_len, encoded_start);
            
            // Decode it
            char decoded[256];
            DecodeQPString(encoded_part, decoded, sizeof(decoded));
            
            // Replace the entire encoded section with decoded text
            size_t prefix_len = start - subject;
            char result[256];
            snprintf(result, sizeof(result), "%.*s%s%s", 
                     (int)prefix_len, subject,  // Everything before encoding
                     decoded,                   // Decoded text
                     encoded_end + 2);          // Everything after "?="
            
            strncpy(subject, result, 256);
        }
    }
}

void
GetDate(char *date, size_t buffer_size)
{
    
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    
    strftime(date, buffer_size, "%a, %d, %Y", local_time);
}

int
CheckIfEmailReceivedToday(char *date, char *date_header)
{
    // Date header: Wed, 27 Aug 2025 08:14:03
    // date = Fri, 29, 2025
    int Result = 0;
    
    int day1 = 0;
    int day2 = 0;
    int year1 = 0;
    int year2 = 0;
    
    // days match?
    if(strncmp(date, date_header, 3) == 0)
    {
        day1 = atoi(date+5);
        day2 = atoi(date_header+5);
        if(day1 == day2)
        {
            year1 = atoi(strrchr(date, ' ') + 1);
            year2 = atoi(date_header + 12);
            if(year1 == year2)
            {
                Result = 1;
                return(Result);
            }
        }
    }
    
    return(Result);
}

void
ChangeDateHeaderIfToday(char *date_header)
{
    // Date header: Wed, 27 Aug 2025 08:14:03
    char *SemiColon = strchr(date_header, ':');
    if(SemiColon)
    {
        char *time_start = SemiColon - 2;
        
        // copy the time to email_array[count].date
        memmove(date_header, time_start, strlen(time_start) + 1);
    }
}

int
MonthNameToNumber(char *month)
{
    if(strcmp(month, "Jan") == 0) return 1;
    if(strcmp(month, "Feb") == 0) return 2;
    if(strcmp(month, "Mar") == 0) return 3;
    if(strcmp(month, "Apr") == 0) return 4;
    if(strcmp(month, "May") == 0) return 5;
    if(strcmp(month, "Jun") == 0) return 6;
    if(strcmp(month, "Jul") == 0) return 7;
    if(strcmp(month, "Aug") == 0) return 8;
    if(strcmp(month, "Sep") == 0) return 9;
    if(strcmp(month, "Oct") == 0) return 10;
    if(strcmp(month, "Nov") == 0) return 11;
    if(strcmp(month, "Dev") == 0) return 12;
    
    return(1);
}

int
CompareByTimestamp(const void *a, const void *b)
{
    EmailMetadata *email1 = (EmailMetadata*)a;
    EmailMetadata *email2 = (EmailMetadata*)b;
    
    if(email1->parsed_time > email2->parsed_time) return -1;
    if(email1->parsed_time < email2->parsed_time) return 1;
    
    return(0);
}
