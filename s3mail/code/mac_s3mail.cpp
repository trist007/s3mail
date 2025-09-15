#include "s3mail_platform.h"
#include "s3mail.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <ApplicationServices/ApplicationServices.h>

// Platform state
global_variable bool32 GlobalRunning;
global_variable CGLContextObj g_opengl_context = NULL;
global_variable WindowRef g_window = NULL;
global_variable EventHandlerUPP g_window_event_handler_upp = NULL;

// Platform API implementation
void MacShowMessage(void *Window, const char *message)
{
    CFStringRef messageStr = CFStringCreateWithCString(NULL, message, kCFStringEncodingUTF8);
    CFStringRef titleStr = CFSTR("S3Mail");
    
    CFUserNotificationDisplayAlert(0, kCFUserNotificationNoteAlertLevel,
                                   NULL, NULL, NULL,
                                   titleStr, messageStr,
                                   CFSTR("OK"), NULL, NULL, NULL);
    
    CFRelease(messageStr);
}

void MacInvalidateWindow(void *Window)
{
    if (g_window) {
        InvalWindowRect(g_window, GetWindowPortBounds(g_window, NULL));
    }
}

internal void
MacExecuteAWSCLI(game_state *GameState, char *command)
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        execl("/bin/sh", "sh", "-c", command, NULL);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(pipefd[1]); // Close write end
        GameState->awscli.stdout_read = pipefd[0];
        GameState->awscli.process_pid = pid;
        GameState->awscli.process_running = true;
    } else {
        // Fork failed
        close(pipefd[0]);
        close(pipefd[1]);
        GameState->awscli.process_running = false;
    }
}

internal char*
MacReadProcessOutput(int stdout_read)
{
    local_persist char buffer[4096];
    ssize_t bytes_read;
    
    // Set non-blocking mode
    int flags = fcntl(stdout_read, F_GETFL, 0);
    fcntl(stdout_read, F_SETFL, flags | O_NONBLOCK);
    
    bytes_read = read(stdout_read, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return buffer;
    }
    
    return NULL; // No data available yet
}

// Dynamic library management
time_t MacGetLastWriteTime(char *filename)
{
    struct stat file_stat;
    time_t result = 0;
    
    if (stat(filename, &file_stat) == 0) {
        result = file_stat.st_mtime;
    }
    
    return result;
}

internal MacGameCode
MacLoadGameCode(char *dylib_path, char *temp_dylib_path)
{
    MacGameCode result = {};
    
    // Copy the dylib to temp location
    FILE *source = fopen(dylib_path, "rb");
    if (!source) return result;
    
    FILE *dest = fopen(temp_dylib_path, "wb");
    if (!dest) {
        fclose(source);
        return result;
    }
    
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }
    
    fclose(source);
    fclose(dest);
    
    result.last_write_time = MacGetLastWriteTime(dylib_path);
    result.dylib = dlopen(temp_dylib_path, RTLD_NOW);
    
    if (result.dylib) {
        result.UpdateAndRender = (game_update_and_render *)
            dlsym(result.dylib, "GameUpdateAndRender");
            
        result.HandleKeyPress = (game_handle_key_press *)
            dlsym(result.dylib, "GameHandleKeyPress");
            
        result.InitializeUI = (game_initialize_ui *)
            dlsym(result.dylib, "GameInitializeUI");
            
        result.is_valid = (result.UpdateAndRender &&
                          result.HandleKeyPress &&
                          result.InitializeUI);
    }
    
    if (!result.is_valid) {
        result.UpdateAndRender = 0;
        result.HandleKeyPress = 0;
        result.InitializeUI = 0;
    }
    
    return result;
}

void MacUnloadGameCode(MacGameCode *game_code)
{
    if (game_code->dylib) {
        dlclose(game_code->dylib);
        game_code->dylib = 0;
    }
    game_code->is_valid = false;
    game_code->UpdateAndRender = 0;
    game_code->HandleKeyPress = 0;
    game_code->InitializeUI = 0;
}

internal void
MacGetEXEFileName(mac_state *MacState)
{
    uint32_t size = sizeof(MacState->EXEFileName);
    if (_NSGetExecutablePath(MacState->EXEFileName, &size) == 0) {
        MacState->OnePastLastEXEFileNameSlash = MacState->EXEFileName;
        for (char *Scan = MacState->EXEFileName; *Scan; ++Scan) {
            if (*Scan == '/') {
                MacState->OnePastLastEXEFileNameSlash = Scan + 1;
            }
        }
    }
}

internal void
MacBuildEXEPathFileName(mac_state *MacState, char *FileName,
                        int DestCount, char *Dest)
{
    CatStrings(MacState->OnePastLastEXEFileNameSlash - MacState->EXEFileName, MacState->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

internal int
MacGetCurrentWorkingDirectory(char *dir)
{
    if (getcwd(dir, PATH_MAX) != NULL) {
        return strlen(dir);
    } else {
        strcpy(dir, "Error getting directory");
        return 0;
    }
}

internal int
MacListFilesInDirectory(char *directory, EmailMetadata **email_array)
{
    *email_array = (EmailMetadata *)malloc(Megabytes(100));
    int email_count = 0;
    
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        return 0;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && email_count < MAX_EMAILS) {
        // Skip directories and hidden files
        if (entry->d_type == DT_REG && entry->d_name[0] != '.') {
            strncpy((*email_array)[email_count].filename, entry->d_name, 
                   sizeof((*email_array)[email_count].filename) - 1);
            (*email_array)[email_count].filename[sizeof((*email_array)[email_count].filename) - 1] = '\0';
            email_count++;
        }
    }
    
    closedir(dir);
    return email_count;
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if (Memory) {
        free(Memory);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    FILE *file = fopen(Filename, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        if (file_size > 0) {
            Result.Contents = malloc(file_size);
            if (Result.Contents) {
                size_t bytes_read = fread(Result.Contents, 1, file_size, file);
                if (bytes_read == file_size) {
                    Result.ContentsSize = file_size;
                } else {
                    free(Result.Contents);
                    Result.Contents = 0;
                }
            }
        }
        
        fclose(file);
    }
    
    return Result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
    
    FILE *file = fopen(Filename, "wb");
    if (file) {
        size_t bytes_written = fwrite(Memory, 1, MemorySize, file);
        Result = (bytes_written == MemorySize);
        fclose(file);
    }
    
    return Result;
}

// Key mapping from Carbon to your VK codes
uint32 CarbonKeyToVKCode(UInt32 keyCode)
{
    switch (keyCode) {
        case kVK_Escape: return VK_ESCAPE;
        case kVK_Return: return VK_RETURN;
        case kVK_LeftArrow: return VK_LEFT;
        case kVK_RightArrow: return VK_RIGHT;
        case kVK_DownArrow: return VK_DOWN;
        case kVK_UpArrow: return VK_UP;
        case kVK_Delete: return VK_BACK;
        case kVK_ForwardDelete: return VK_DELETE;
        // Add more key mappings as needed
        default: return keyCode; // Return raw keycode for unhandled keys
    }
}

// OpenGL initialization
int MacInitOpenGL(void)
{
    // Create OpenGL pixel format
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFADoubleBuffer,
        kCGLPFAColorSize, 32,
        kCGLPFADepthSize, 24,
        kCGLPFAStencilSize, 8,
        0
    };
    
    CGLPixelFormatObj pixelFormat;
    GLint numPixelFormats;
    
    if (CGLChoosePixelFormat(attributes, &pixelFormat, &numPixelFormats) != kCGLNoError) {
        return 0;
    }
    
    if (CGLCreateContext(pixelFormat, NULL, &g_opengl_context) != kCGLNoError) {
        CGLDestroyPixelFormat(pixelFormat);
        return 0;
    }
    
    CGLDestroyPixelFormat(pixelFormat);
    
    if (CGLSetCurrentContext(g_opengl_context) != kCGLNoError) {
        return 0;
    }
    
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    return 1;
}

// Event handler for Carbon window
OSStatus WindowEventHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    OSStatus result = eventNotHandledErr;
    UInt32 eventClass = GetEventClass(event);
    UInt32 eventKind = GetEventKind(event);
    
    game_state *GameState = (game_state *)userData;
    
    switch (eventClass) {
        case kEventClassWindow: {
            switch (eventKind) {
                case kEventWindowClose: {
                    GlobalRunning = false;
                    result = noErr;
                } break;
                
                case kEventWindowDrawContent: {
                    // Trigger redraw
                    result = noErr;
                } break;
            }
        } break;
        
        case kEventClassKeyboard: {
            if (eventKind == kEventRawKeyDown) {
                UInt32 keyCode;
                GetEventParameter(event, kEventParamKeyCode, typeUInt32, NULL, sizeof(keyCode), NULL, &keyCode);
                
                uint32 vkCode = CarbonKeyToVKCode(keyCode);
                
                if (vkCode == VK_ESCAPE) {
                    GlobalRunning = false;
                }
                
                // Call game key handler here if needed
                result = noErr;
            }
        } break;
        
        case kEventClassMouse: {
            Point mouseLocation;
            GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(mouseLocation), NULL, &mouseLocation);
            
            // Convert global to window coordinates
            SetPortWindowPort(g_window);
            GlobalToLocal(&mouseLocation);
            
            GameState->mouse_x = mouseLocation.h;
            GameState->mouse_y = mouseLocation.v;
            
            if (eventKind == kEventMouseDown) {
                GameState->mouse_down = 1;
            } else if (eventKind == kEventMouseUp) {
                GameState->mouse_down = 0;
                
                // Handle button clicks
                if (GameState->compose_button.is_pressed) {
                    MacShowMessage(NULL, "Compose clicked!");
                }
                if (GameState->delete_button.is_pressed) {
                    MacShowMessage(NULL, "Delete clicked!");
                }
            }
            
            InvalWindowRect(g_window, GetWindowPortBounds(g_window, NULL));
            result = noErr;
        } break;
    }
    
    return result;
}

int main(int argc, const char * argv[])
{
    mac_state MacState = {};
    
    // Initialize game memory
    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(64);
    GameMemory.TransientStorageSize = Gigabytes(1);
    
    size_t TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    void *GameMemoryBlock = malloc(TotalSize);
    GameMemory.PermanentStorage = GameMemoryBlock;
    GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);
    
    MacGetEXEFileName(&MacState);
    
    char SourceGameCodeDylibFullPath[MAC_STATE_FILE_NAME_COUNT];
    MacBuildEXEPathFileName(&MacState, "libs3mail_game.dylib",
                            sizeof(SourceGameCodeDylibFullPath), SourceGameCodeDylibFullPath);
    
    char TempGameCodeDylibFullPath[MAC_STATE_FILE_NAME_COUNT];
    MacBuildEXEPathFileName(&MacState, "libs3mail_game_temp.dylib",
                            sizeof(TempGameCodeDylibFullPath), TempGameCodeDylibFullPath);
    
    // Load initial game code
    MacGameCode gamecode = MacLoadGameCode(SourceGameCodeDylibFullPath, TempGameCodeDylibFullPath);
    
    // Initialize game state
    game_state GameState = {};
    
    // Setup Platform API
    PlatformAPI mac_platform = {};
    mac_platform.ShowMessage = MacShowMessage;
    mac_platform.InvalidateWindow = MacInvalidateWindow;
    mac_platform.ExecuteAWSCLI = MacExecuteAWSCLI;
    mac_platform.ReadProcessOutput = MacReadProcessOutput;
    mac_platform.ListFilesInDirectory = MacListFilesInDirectory;
    mac_platform.GetCurrentWorkingDirectory = MacGetCurrentWorkingDirectory;
    mac_platform.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
    mac_platform.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
    mac_platform.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
    mac_platform.GameState = &GameState;
    
    // Create window using Carbon
    Rect windowBounds = {100, 100, 100 + WINDOW_HEIGHT_HD, 100 + WINDOW_WIDTH_HD};
    
    OSStatus err = CreateNewWindow(kDocumentWindowClass,
                                   kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute,
                                   &windowBounds,
                                   &g_window);
    
    if (err != noErr || !g_window) {
        return 1;
    }
    
    mac_platform.Window = g_window;
    
    CFStringRef windowTitle = CFSTR("S3Mail");
    SetWindowTitleWithCFString(g_window, windowTitle);
    
    // Install event handler
    EventTypeSpec eventTypes[] = {
        {kEventClassWindow, kEventWindowClose},
        {kEventClassWindow, kEventWindowDrawContent},
        {kEventClassKeyboard, kEventRawKeyDown},
        {kEventClassMouse, kEventMouseDown},
        {kEventClassMouse, kEventMouseUp},
        {kEventClassMouse, kEventMouseMoved}
    };
    
    g_window_event_handler_upp = NewEventHandlerUPP(WindowEventHandler);
    InstallWindowEventHandler(g_window, g_window_event_handler_upp,
                              GetEventTypeCount(eventTypes), eventTypes,
                              &GameState, NULL);
    
    // Initialize OpenGL
    if (!MacInitOpenGL()) {
        return 1;
    }
    
    // Load emails
    char emailDir[PATH_MAX];
    snprintf(emailDir, sizeof(emailDir), "%s/.email", getenv("HOME"));
    GameState.email_count = MacListFilesInDirectory(emailDir, &GameState.email_array);
    
    // Get current date
    char date[32];
    GetDate(date, sizeof(date));
    
    // Extract email headers
    thread_context Thread = {};
    ExtractHeader(&Thread, date, GameState.email_array, GameState.email_count, 
                  mac_platform.DEBUGPlatformReadEntireFile, emailDir, HEADER_FROM);
    ExtractHeader(&Thread, date, GameState.email_array, GameState.email_count, 
                  mac_platform.DEBUGPlatformReadEntireFile, emailDir, HEADER_SUBJECT);
    ExtractHeader(&Thread, date, GameState.email_array, GameState.email_count, 
                  mac_platform.DEBUGPlatformReadEntireFile, emailDir, HEADER_DATE);
    
    // Sort emails by date
    qsort(GameState.email_array, GameState.email_count, sizeof(EmailMetadata), CompareByTimestamp);
    
    // Process today's emails
    for (int i = 0; i < GameState.email_count; i++) {
        if (CheckIfEmailReceivedToday(date, GameState.email_array[i].date)) {
            ChangeDateHeaderIfToday(GameState.email_array[i].date);
        }
    }
    
    // Initialize UI
    if (gamecode.is_valid) {
        gamecode.InitializeUI(&Thread, &GameMemory, &GameState, &mac_platform);
    }
    
    // Initialize font
    char PathToFont[256];
    snprintf(PathToFont, sizeof(PathToFont), "%s/dev/s3mail/s3mail/code/fonts/liberation-mono.ttf", getenv("HOME"));
    
    if (!InitFont(&GameState, PathToFont)) {
        MacShowMessage(NULL, "Failed to load font");
    }
    
    ShowWindow(g_window);
    
    GlobalRunning = true;
    
    // Main game loop
    while (GlobalRunning) {
        // Process events
        EventRef event;
        while (ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &event) == noErr) {
            SendEventToEventTarget(event, GetEventDispatcherTarget());
            ReleaseEvent(event);
        }
        
        // Check for DLL reload
        time_t new_write_time = MacGetLastWriteTime(SourceGameCodeDylibFullPath);
        if (new_write_time != gamecode.last_write_time) {
            MacUnloadGameCode(&gamecode);
            gamecode = MacLoadGameCode(SourceGameCodeDylibFullPath, TempGameCodeDylibFullPath);
            
            if (gamecode.is_valid && gamecode.InitializeUI) {
                gamecode.InitializeUI(&Thread, &GameMemory, &GameState, &mac_platform);
            }
        }
        
        // Render
        if (gamecode.is_valid) {
            glClear(GL_COLOR_BUFFER_BIT);
            gamecode.UpdateAndRender(&Thread, &GameMemory, &GameState, &mac_platform);
            CGLFlushDrawable(g_opengl_context);
        }
        
        // Limit frame rate
        usleep(16667); // ~60 FPS
    }
    
    // Cleanup
    if (g_opengl_context) {
        CGLDestroyContext(g_opengl_context);
    }
    
    if (g_window_event_handler_upp) {
        DisposeEventHandlerUPP(g_window_event_handler_upp);
    }
    
    free(GameMemoryBlock);
    
    return 0;
}
