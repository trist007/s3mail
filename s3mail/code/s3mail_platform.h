#ifndef S3MAIL_PLATFORM_H
#define S3MAIL_PLATFORM_H

#if COMPILER_MSVC
#include <intrin.h>
#endif

#define MAX_EMAILS 100
// TODO(trist007): use Tsoding's dynamic C array here
typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Numbers;

#define da_append(xs, x)\
do {\
if(xs.count >= xs.capacity) {\
if(xs.capacity == 0) xs.capacity = 32;\
else xs.capacity *= 2;\
xs.items = realloc(xs.items, xs.capacity*sizeof(*xs.items));\
}\
xs.items[xs.count++] = x;\
} while (0)

/*
// Usage of da_append
Numbers xs = {};
for(int x = 0; x < 10; x++) da_append(xs, x);
for(size_t i = 0, i < xs.count; ++i) printf("%d\n", xs.items[i]);
return 0;
*/

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

#include "s3mail.h"

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
    // TODO(trist007): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

// KeyCode Translation
typedef struct key_translation
{
    char character;
    bool32 valid;
} key_translation;

//typedef KEY_CODE_TO_CHAR(KeyCodeToChar);

// Process struct for awscli
/*
struct Win32ProcessHandle
{
    PROCESS_INFORMATION process_info;
    HANDLE stdout_read;
    bool32 process_running;
};
*/
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

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *Filename, void *Memory)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread, char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);


typedef struct game_memory
{
    bool32 IsInitialized;
    
    uint64 PermanentStorageSize;
    void *PermanentStorage;
    
    uint64 TransientStorageSize;
    void *TransientStorage;
    
    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
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

struct game_state;
struct EmailMetadata;

// Platform services provided by EXE to DLL
typedef struct {
    // Platform functions
    void (*ShowMessage)(HWND Window, const char *message);
    void (*InvalidateWindow)(HWND Window);
    
    // awscli
    void (*ExecuteAWSCLI)(game_state *GameState, char* command);
    char* (*ReadProcessOutput)(HANDLE stdout_read);
    
    // file IO
    int (*ListFilesInDirectory)(char *directory, EmailMetadata **email_array);
    DWORD (*GetCurrentWorkingDirectory)(char *dir);
    
    // Input helpers
    bool32 (*IsKeyPressed)(int vk_code);
    key_translation (*KeyCodeToChar)(int key_code);
    
    // Game state
    game_state *GameState;
    HWND Window;
} PlatformAPI;

void SerializeRecipients(game_state *GameState);
void SendEmail(game_state *GameState, PlatformAPI *platform);

// DLL export signature
#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread, game_memory *Memory, game_state *GameState, PlatformAPI* platform)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_HANDLE_KEY_PRESS(name) void name(game_state *GameState, int key_code, PlatformAPI* platform, game_memory *Memory)
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