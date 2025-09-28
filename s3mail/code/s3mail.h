#ifndef S3MAIL_H
#define S3MAIL_H

#include "s3mail_platform.h"

// enums
typedef enum {
    MODE_FOLDER = 0,
    MODE_EMAIL,
    MODE_CONTACT,
    MODE_READING_EMAIL,
    MODE_COUNT
} app_mode;

typedef enum {
    HEADER_FROM,
    HEADER_SUBJECT,
    HEADER_DATE
} HeaderType;


// UI structures
struct UIButton
{
    float x, y, width, height;
    char text[256];
    int is_hovered;
    int is_pressed;
};

struct UIButtonRatio
{
    float x_ratio, y_ratio, width_ratio, height_ratio;
    float x, y, width, height;
    char text[256];
    int is_hovered;
    int is_pressed;
};

struct UIListRatio
{
    float x_ratio, y_ratio, width_ratio, height_ratio;
    float x, y, width, height;
    char items[MAX_EMAILS][256];
    int item_count;
    int selected_item;
};

struct UIList
{
    float x, y, width, height;
    char items[MAX_EMAILS][256];
    int item_count;
    int selected_item;
};

struct EmailContent
{
    float x_ratio, y_ratio, width_ratio, height_ratio;
    float x, y, width, height;
    char items[MAX_EMAILS][256];
    int item_count;
    int selected_item;
    int scroll_offset;
};

// Email
struct EmailMetadata
{
    char filename[MAX_PATH];
    char email[256];
    char subject[256];
    char from[256];
    char date[256];
    int header_lines;
    bool32 showHeaders;
    FILETIME timestamp;
    time_t parsed_time;
    DWORD file_size;
};

struct Win32ProcessHandle;
struct thread_context;

// Game state that persists across DLL reloads
struct game_state
{
    int window_width;
    int window_height;
    int mouse_x, mouse_y;
    int mouse_down;
    app_mode current_mode;
    
    // UI elements
    UIButtonRatio compose_button;
    UIButtonRatio delete_button;
    UIListRatio folder_list;
    UIListRatio email_list;
    UIListRatio contact_list;
    EmailContent email;
    
    // font
    GLuint font_texture_id;
    stbtt_bakedchar cdata[96];
    
    // awscli
    Win32ProcessHandle *awscli;
    char aws_output_buffer[4096];
    bool32 show_aws_output;
    
    // email
    EmailMetadata *email_array;
    int32 email_count;
    char email_content[32768];
    char parsed_email[1000][256];
    int32 line_count;
    
    // Memory
    uint64 TotalSize;
    void *GameMemoryBlock;
    
    void *permanent_storage;
    uint64 permanent_storage_size;
};

// OpenGL functions
void SetColor(float r, float g, float b);
void DrawRect(float x, float y, float width, float height);
void DrawRectRatio(float x, float y, float width, float height);
void DrawRectOutline(float x, float y, float width, float height);
void DrawRectOutlineRatio(float x, float y, float width, float height);
int PointInRect(int px, int py, float x, float y, float width, float height);
void DrawTextGame(game_state *GameState, const char *text, float x, float y);
void DrawTextGameEmail(game_state *GameState, const char *text, float x, float y);

#endif //S3MAIL_H