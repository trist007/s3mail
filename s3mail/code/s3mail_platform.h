#ifndef S3MAIL_PLATFORM_H
#define S3MAIL_PLATFORM_H

#if COMPILER_MSVC
#include <intrin.h>
#endif

#define MAX_EMAILS 100

// Types
#include <stdint.h>
#include <stddef.h>

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

#define internal static
#define local_persist static
#define global_variable static

#include <windows.h>
#include <GL/gl.h>
#include "stb_truetype.h"

time_t ParseEmailDate(char *date_header);

// resolution 16:9
//#define WINDOW_WIDTH_HD 2160
//#define WINDOW_HEIGHT_HD 1440
#define WINDOW_WIDTH_HD 1920
#define WINDOW_HEIGHT_HD 1080


#if S3MAIL_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

// this only works for static arrays
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))


inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

// UI structures
typedef struct {
    float x, y, width, height;
    char text[256];
    int is_hovered;
    int is_pressed;
} UIButton;

typedef struct {
    float x_ratio, y_ratio, width_ratio, height_ratio;
    float x, y, width, height;
    char text[256];
    int is_hovered;
    int is_pressed;
} UIButtonRatio;

typedef struct {
    float x_ratio, y_ratio, width_ratio, height_ratio;
    float x, y, width, height;
    char items[MAX_EMAILS][256];
    int item_count;
    int selected_item;
} UIListRatio;

typedef struct {
    float x_ratio, y_ratio, width_ratio, height_ratio;
    float x, y, width, height;
    char items[MAX_EMAILS][256];
    int item_count;
    int selected_item;
    int scroll_offset;
} EmailContent;

typedef struct {
    float x, y, width, height;
    char items[MAX_EMAILS][256];
    int item_count;
    int selected_item;
} UIList;

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

// Process struct for awscli
typedef struct {
    PROCESS_INFORMATION process_info;
    HANDLE stdout_read;
    bool32 process_running;
} ProcessHandle;

// Email
typedef struct {
    char filename[MAX_PATH];
    char email[256];
    char subject[256];
    char from[256];
    char date[256];
    FILETIME timestamp;
    time_t parsed_time;
    DWORD file_size;
} EmailMetadata;

// Game state that persists across DLL reloads
typedef struct {
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
    ProcessHandle awscli;
    char aws_output_buffer[4096];
    bool32 show_aws_output;
    
    // email
    EmailMetadata *email_array;
    int32 email_count;
    char email_content[4096];
    char parsed_email[1000][256];
    int32 line_count;
    
    // Memory
    uint64 TotalSize;
    void *GameMemoryBlock;
    
    void *permanent_storage;
    uint64 permanent_storage_size;
} game_state;

typedef struct game_memory
{
    bool32 IsInitialized;
    
    uint64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup
    
    uint64 TransientStorageSize;
    void *TransientStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup
    
    //debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
    //debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    //debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
} game_memory;

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
typedef struct win32_state
{
    uint64 TotalSize;
    void *GameMemoryBlock;
    //win32_replay_buffer ReplayBuffers[4];
    
    HANDLE RecordingHandle;
    int InputRecordingIndex;
    
    HANDLE PlaybackHandle;
    int InputPlayingIndex;
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
} win32_state;

// File IO
typedef struct thread_context
{
    int Placeholder;
} thread_context;

typedef struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
} debug_read_file_result;

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread, char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

// Platform services provided by EXE to DLL
typedef struct {
    // Platform functions
    void (*ShowMessage)(HWND Window, const char *message);
    void (*InvalidateWindow)(HWND Window);
    
    // awscli
    void (*ExecuteAWSCLI)(game_state *GameState, char* command);
    char* (*ReadProcessOutput)(HANDLE stdout_read);
    
    // file IO
    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    int (*ListFilesInDirectory)(char *directory, EmailMetadata **email_array);
    DWORD (*GetCurrentWorkingDirectory)(char *dir);
    
    // Game state
    game_state *GameState;
    HWND Window;
} PlatformAPI;

// DLL interface - functions the DLL must export
/*
typedef struct {
    void (*UpdateAndRender)(game_state *GameState, PlatformAPI *platform);
    void (*HandleKeyPress)(game_state *GameState, int key_code);
    void (*InitializeUI)(game_state *GameState);
} GameAPI;
*/

// DLL export signature
#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread, game_memory *Memory, game_state *GameState, PlatformAPI* platform)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_HANDLE_KEY_PRESS(name) void name(game_state *GameState, int key_code, PlatformAPI* platform)
typedef GAME_HANDLE_KEY_PRESS(game_handle_key_press);

#define GAME_INITIALIZE_UI(name) void name(thread_context *Thread, game_memory *Memory, game_state *GameState, PlatformAPI* platform)
typedef GAME_INITIALIZE_UI(game_initialize_ui);

// DLL hot reloading
typedef struct {
    HMODULE dll;
    FILETIME last_write_time;
    game_update_and_render *UpdateAndRender;
    game_handle_key_press *HandleKeyPress;
    game_initialize_ui *InitializeUI;
    bool32 is_valid;
} Win32GameCode;

typedef struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
} game_button_state;

#endif //S3MAIL_PLATFORM_H