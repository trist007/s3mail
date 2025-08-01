/* date = July 27th 2025 2:48 pm */

#ifndef S3MAIL_PLATFORM_H
#define S3MAIL_PLATFORM_H

#include <windows.h>
#include <GL/gl.h>
#include <stdint.h>
#include <stdio.h>
#include "stb_truetype.h"

// Types
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

// UI structures
typedef struct {
    float x, y, width, height;
    char text[256];
    int is_hovered;
    int is_pressed;
} UIButton;

typedef struct {
    float x, y, width, height;
    char items[10][256];
    int item_count;
    int selected_item;
} UIList;

// Platform services provided by EXE to DLL
typedef struct {
    // Rendering functions
    void (*Win32SetColor)(float r, float g, float b);
    void (*Win32DrawRect)(float x, float y, float width, float height);
    void (*Win32DrawRectOutline)(float x, float y, float width, float height);
    void (*Win32DrawText)(const char* text, float x, float y);
    
    // Utility functions
    int (*Win32PointInRect)(int px, int py, float x, float y, float width, float height);
    
    // Platform functions
    void (*Win32ShowMessage)(const char *message);
    void (*Win32InvalidateWindow)(void);
} PlatformAPI;

// Game state that persists across DLL reloads
typedef struct {
    int window_width;
    int window_height;
    int mouse_x, mouse_y;
    int mouse_down;
    
    // UI elements
    UIButton compose_button;
    UIButton delete_button;
    UIList folder_list;
    UIList email_list;
    UIList contact_list;
    
    // OpenGL font data
    GLuint font_texture_id;
    stbtt_bakedchar cdata[96];
    
    // Memory
    void *permanent_storage;
    uint64 permanent_storage_size;
} GameState;

// DLL interface - functions the DLL must export
typedef struct {
    void (*UpdateAndRender)(GameState *state, PlatformAPI *platform);
    void (*HandleKeyPress)(GameState *state, int key_code);
} GameAPI;

// DLL export signature
#define GAME_UPDATE_AND_RENDER(name) void name(GameState *state, PlatformAPI* platform)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_HANDLE_KEY_PRESS(name) void name(GameState *state, int key_code)
typedef GAME_HANDLE_KEY_PRESS(game_handle_key_press);

// DLL hot reloading
typedef struct {
    HMODULE dll;
    FILETIME last_write_time;
    game_update_and_render *UpdateAndRender;
    game_handle_key_press *HandleKeyPress;
    bool32 is_valid;
} Win32GameCode;

#endif //S3MAIL_PLATFORM_H
