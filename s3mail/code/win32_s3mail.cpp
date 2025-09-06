#include "s3mail_platform.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <GL/gl.h>


// Platform state
global_variable bool32 GlobalRunning;
global_variable HDC g_hdc = 0;
global_variable HGLRC g_hglrc = 0;

// Platform API implementation
void Win32SetColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

void Win32DrawRectRatio(float x, float y, float width, float height)
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

void Win32DrawRect(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Win32DrawRectOutlineRatio(float x, float y, float width, float height)
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

void Win32DrawRectOutline(float x, float y, float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Win32DrawText(game_state *GameState, const char *text, float x, float y)
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

int Win32PointInRect(int px, int py, float x, float y, float width, float height)
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

void Win32ShowMessage(HWND Window, const char *message)
{
    MessageBox(Window, message, "S3Mail", MB_OK);
}

void Win32InvalidateWindow(HWND Window)
{
    InvalidateRect(Window, 0, FALSE);
}

internal void
Win32ExecuteAWSCLI(game_state *GameState, char *command)
{
    //GameState->awscli = {};
    
    char full_command[1024];
    sprintf(full_command, "cmd /c %s", command);
    
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    HANDLE stdout_read, stdout_write;
    CreatePipe(&stdout_read, &stdout_write, &sa, 0);
    
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    si.hStdOutput = stdout_write;
    si.hStdError = stdout_write;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    //bool32 result = CreateProcess(NULL, full_command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    bool32 result = CreateProcess(0, full_command, 0, 0, TRUE, 0, 0, 0, &si, &pi);
    
    if(result)
    {
        // Wait for process to complete
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        GameState->awscli.process_info = pi;
        GameState->awscli.stdout_read = stdout_read;
        GameState->awscli.process_running = true;
    }
    else
    {
        CloseHandle(stdout_read);
        /*
        GameState->awscli.process_info = (PROCESS_INFORMATION){0};
        GameState->awscli.stdout_read = 0;
        GameState->awscli.process_running = false;
*/
    }
    
    CloseHandle(stdout_write);
}

internal char*
Win32ReadProcessOutput(HANDLE stdout_read)
{
    local_persist char buffer[4096];
    DWORD bytes_available;
    DWORD bytes_read;
    
    // Check if data is available without blocking
    if(PeekNamedPipe(stdout_read, 0, 0, 0, &bytes_available, 0))
    {
        if(bytes_available > 0)
        {
            // Data is available, read it
            if(ReadFile(stdout_read, buffer, sizeof(buffer) - 1, &bytes_read, 0))
            {
                buffer[bytes_read] = '\0';
                return buffer;
            }
        }
    }
    
    return NULL; // No data available yet
}

// DLL management
FILETIME Win32GetLastWriteTime(char *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    FILETIME Result = {0};
    
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        Result = data.ftLastWriteTime;
    }
    else
    {
        DWORD error = GetLastError();
    }
    
    return Result;
}

internal Win32GameCode
Win32LoadGameCode(char *dll_path, char *temp_dll_path, char *lock_filename)
{
    Win32GameCode Result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(lock_filename, GetFileExInfoStandard, &Ignored))
    {
        Result.last_write_time = Win32GetLastWriteTime(dll_path);
        
        BOOL copy_result = CopyFile(dll_path, temp_dll_path, FALSE);
        
        Result.dll = LoadLibrary(temp_dll_path);
        
        if(Result.dll)
        {
            Result.UpdateAndRender = (game_update_and_render *)
                GetProcAddress(Result.dll, "GameUpdateAndRender");
            
            Result.HandleKeyPress = (game_handle_key_press *)
                GetProcAddress(Result.dll, "GameHandleKeyPress");
            
            Result.InitializeUI = (game_initialize_ui *)
                GetProcAddress(Result.dll, "GameInitializeUI");
            
            Result.is_valid = (Result.UpdateAndRender &&
                               Result.HandleKeyPress &&
                               Result.InitializeUI);
        }
        else
        {
            DWORD error = GetLastError();
        }
    }
    else
    {
    }
    
    if (!Result.is_valid)
    {
        Result.UpdateAndRender = 0;
        Result.HandleKeyPress = 0;
        Result.InitializeUI = 0;
    }
    
    return Result;
}

void Win32UnloadGameCode(Win32GameCode *game_code)
{
    if (game_code->dll)
    {
        FreeLibrary(game_code->dll);
        game_code->dll = 0;
    }
    game_code->is_valid = false;
    game_code->UpdateAndRender = 0;
    game_code->HandleKeyPress = 0;
    game_code->InitializeUI = 0;
}

// OpenGL and font initialization (same as before)
int Win32InitOpenGL(HWND hwnd)
{
    g_hdc = GetDC(hwnd);
    if (!g_hdc) return 0;
    
    PIXELFORMATDESCRIPTOR pfd = {
        // NOTE(trist007): try paste here of pixelformatdescriptor so it appends(pastes on the same line)
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(g_hdc, &pfd);
    if (!pixelFormat) return 0;
    
    if (!SetPixelFormat(g_hdc, pixelFormat, &pfd)) return 0;
    
    g_hglrc = wglCreateContext(g_hdc);
    if (!g_hglrc) return 0;
    
    if(!wglMakeCurrent(g_hdc, g_hglrc)) return 0;
    
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    return 1;
}

int Win32InitFont(game_state *GameState, const char *font_path)
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
    return 1;
}

void Win32HandleResizey(int width, int height)
{
    if (height == 0) height = 1;
    
    //state->window_width = width;
    //state->window_height = height;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

internal void
Win32GetEXEFileName(win32_state *GameState)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, GameState->EXEFileName, sizeof(GameState->EXEFileName));
    GameState->OnePastLastEXEFileNameSlash = GameState->EXEFileName;
    for(char *Scan = GameState->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            GameState->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

internal void
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

internal void
Win32BuildEXEPathFileName(win32_state *GameState, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(GameState->OnePastLastEXEFileNameSlash - GameState->EXEFileName, GameState->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

internal DWORD
Win32GetCurrentWorkingDirectory(char *dir)
{
    DWORD result = GetCurrentDirectory(MAX_PATH, dir);
    
    if(result > 0 && result < MAX_PATH)
    {
        
    }
    else
    {
        StringCchCopy(dir, sizeof(dir), "Error getting directory");
    }
    
    return(result);
}

internal int
Win32ListFilesInDirectory(char *directory, EmailMetadata **email_array)
{
    *email_array = (EmailMetadata *)VirtualAlloc(0, Megabytes(100), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    int email_count = 0;
    
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char searchPath[MAX_PATH];
    
    snprintf(searchPath, MAX_PATH, "%s\\*.*", directory);
    
    hFind = FindFirstFile(searchPath, &findData);
    if(hFind == INVALID_HANDLE_VALUE)
    {
        // dir doesn't exist or no files found
        return(0);
    }
    
    do
    {
        // skip dirs and special entries
        if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if(email_count < MAX_EMAILS)
            {
                StringCchCopy((*email_array)[email_count++].filename, sizeof((*email_array)[email_count++].filename), findData.cFileName);
            }
        }
    } while(FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    
    return(email_count);
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal void
Win32ExtractHeader(thread_context *Thread, char *date, EmailMetadata *email_array, int32 email_count,
                   debug_platform_read_entire_file *ReadEntireFile, char *path, HeaderType header_type)
{
    
    char *header_name;
    int32 header_len;
    
    // Set up header string and length
    switch(header_type)
    {
        case HEADER_FROM:
        {
            header_name = "From:";
            header_len = 5;
            break;
        }
        case HEADER_SUBJECT:
        {
            header_name = "Subject:";
            header_len = 8;
            break;
        }
        case HEADER_DATE:
        {
            header_name = "Date:";
            header_len = 5;
            break;
        }
        default: // add this to handle unexpected values
        {
            header_name = "Unknown";
            header_len = 8;
            
            // don't know what to search for
            return; 
        }
    }
    
    for(int count = 0;
        count < email_count;
        count++)
    {
        char *Match = "Unknown";
        char full_path_to_file[128];
        
        StringCchPrintf(full_path_to_file, sizeof(full_path_to_file), "%s/%s", path, email_array[count].filename);
        
        debug_read_file_result ReadResult = ReadEntireFile(Thread, full_path_to_file);    
        if(ReadResult.ContentsSize != 0)
        {
            char *line = strtok((char *)ReadResult.Contents, "\n");
            while(line != NULL)
            {
                if(strncmp(line, header_name, header_len) == 0)
                {
                    Match = line + header_len;
                    
                    // Skip any leading whitespace
                    while(*Match == ' ' || *Match == '\t')
                    {
                        Match++;
                    }
                    
                    // break out of the while loop
                    break;
                }
                
                // move to the next line if current line wasn't a match
                line = strtok(NULL, "\n");
            }
        }
        
        switch(header_type)
        {
            case HEADER_FROM:
            {
                // Check if this is in the "Name <email>" format
                char *AngleBracketStart = strchr(Match, '<');
                if(AngleBracketStart != NULL)
                {
                    // Copy everything before the '<'
                    int name_len = AngleBracketStart - Match;
                    
                    // Remove trailing whitespace from the name
                    while((name_len > 0) && ((Match[name_len - 1] == ' ') || (Match[name_len - 1] == '\t') || (Match[name_len - 1] == '\r')))
                    {
                        name_len--;
                    }
                    
                    snprintf(email_array[count].from, sizeof(email_array[count].from), "%.*s", name_len, Match);
                }
                else
                {
                    // No angle brackets found, use the whole string
                    snprintf(email_array[count].from, sizeof(email_array[count].from), "%s", Match);
                }
                
                break;
            }
            case HEADER_SUBJECT:
            {
                char *CarriageReturn = strchr(Match, '\r');
                int len = 0;
                size_t name_len = strlen(Match);
                if(CarriageReturn != NULL)
                {
                    name_len = CarriageReturn - Match;
                    
                    // Remove trailing whitespace from the name
                    while((name_len > 0) && ((Match[name_len - 1] == ' ') || (Match[name_len - 1] == '\t') || (Match[name_len - 1] == '\r')))
                    {
                        name_len--;
                    }
                    
                    if(name_len <= INT_MAX)
                    {
                        len = (int)name_len;
                    }
                    
                    snprintf(email_array[count].subject, sizeof(email_array[count].subject), "%.*s", len, Match);
                }
                else
                {
                    snprintf(email_array[count].subject, sizeof(email_array[count].subject), "%s", Match);
                }
                
                DecodeSubjectIfNeeded(email_array[count].subject);
                break;
            }
            case HEADER_DATE:
            {
                snprintf(email_array[count].date, sizeof(email_array[count].date), "%s", Match);
                email_array[count].parsed_time = ParseEmailDate(Match);
                break;
            }
        }
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {                    
                    // TODO(casey): Logging
                    DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE(casey): File read successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // TODO(casey): Logging
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(Result);
}

internal void
Win32ProcessPendingMessages(Win32GameCode *gamecode, game_state *GameState, PlatformAPI *platform)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                
                // NOTE(casey): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                
                if(WasDown != IsDown)
                {
                    if(gamecode->is_valid && IsDown && gamecode->HandleKeyPress)
                    {
                        gamecode->HandleKeyPress(GameState, VKCode, platform);
                    }
                    
                    if(VKCode == VK_ESCAPE)
                    {
                        PostQuitMessage(0);
                    }
                    
                    if(IsDown)
                    {
                        bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                        if((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        if((VKCode == VK_RETURN) && AltKeyWasDown)
                        {
                            if(Message.hwnd)
                            {
                            }
                        }
                    }
                }
                
            } break;
            
            case WM_MOUSEMOVE:
            {
                GameState->mouse_x = LOWORD(Message.lParam);
                GameState->mouse_y = HIWORD(Message.lParam);
                
                InvalidateRect(Message.hwnd, 0, FALSE);
            } break;
            
            case WM_LBUTTONDOWN:
            {
                GameState->mouse_down = 1;
                SetCapture(Message.hwnd);
                
                InvalidateRect(Message.hwnd, 0, FALSE);
            } break;
            
            case WM_LBUTTONUP:
            {
                GameState->mouse_down = 0;
                ReleaseCapture();
                
                // Handle button clicks
                if (GameState->compose_button_ratio.is_pressed) {
                    MessageBox(Message.hwnd, "Compose clicked!", "S3Mail", MB_OK);
                }
                if (GameState->delete_button_ratio.is_pressed) {
                    MessageBox(Message.hwnd, "Delete clicked!", "S3Mail", MB_OK);
                }
                InvalidateRect(Message.hwnd, 0, FALSE);
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

// Window procedure
internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(Window, &ps);
            
            //SwapBuffers(g_hdc);
            EndPaint(Window, &ps);
        } break;
        
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, uMsg, wParam, lParam);
        } break;
    }
    
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    win32_state Win32State = {};
    
    Win32GetEXEFileName(&Win32State);
    
    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "s3mail_game.dll",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
    
    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "s3mail_game_temp.dll",
                              sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);
    
    char GameCodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.tmp",
                              sizeof(GameCodeLockFullPath), GameCodeLockFullPath);
    
    
    // Load initial game code
    Win32GameCode gamecode = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                               TempGameCodeDLLFullPath,
                                               GameCodeLockFullPath);
    
    // Initialize game_state
    game_state GameState = {};
    
    // Register and create window
    WNDCLASS wc = {};
    wc.lpfnWndProc = Win32MainWindowCallback;
    wc.hInstance = hInstance;
    wc.lpszClassName = "S3MailWindow";
    //wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // white background
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // black background
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    
    if (!RegisterClass(&wc)) return 1;
    
    //HWND Window = CreateWindowEx(WS_EX_TOPMOST,
    HWND Window = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
                                 "S3MailWindow",
                                 "S3Mail",
                                 WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                 1600,
                                 200,
                                 WINDOW_WIDTH_HD,
                                 WINDOW_HEIGHT_HD,
                                 0,
                                 0,
                                 hInstance,
                                 0);
    
    
    // Setup Platform API
    PlatformAPI win32 = {};
    win32.SetColor = Win32SetColor;
    win32.DrawRect = Win32DrawRect;
    win32.DrawRectRatio = Win32DrawRectRatio;
    win32.DrawRectOutline = Win32DrawRectOutline;
    win32.DrawRectOutlineRatio = Win32DrawRectOutlineRatio;
    win32.DrawText = Win32DrawText;
    win32.HandleResizey = Win32HandleResizey;
    win32.PointInRect = Win32PointInRect;
    win32.ShowMessage = Win32ShowMessage;
    win32.InvalidateWindow = Win32InvalidateWindow;
    win32.ExecuteAWSCLI = Win32ExecuteAWSCLI;
    win32.ReadProcessOutput = Win32ReadProcessOutput;
    win32.ListFilesInDirectory = Win32ListFilesInDirectory;
    win32.GetCurrentWorkingDirectory = Win32GetCurrentWorkingDirectory;
    win32.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
    win32.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
    win32.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
    win32.GameState = &GameState;
    win32.Window = Window;
    
    thread_context Thread = {};
    
    GameState.email_array = 0;
    GameState.email_count = Win32ListFilesInDirectory("C:/Users/Tristan/.email", &GameState.email_array);
    
    // get time now
    char date[32];
    GetDate(date, sizeof(date));
    
    // Extract email Headers
    Win32ExtractHeader(&Thread, date, GameState.email_array, GameState.email_count, win32.DEBUGPlatformReadEntireFile,
                       "C:/Users/Tristan/.email", HEADER_FROM);
    Win32ExtractHeader(&Thread, date, GameState.email_array, GameState.email_count, win32.DEBUGPlatformReadEntireFile,
                       "C:/Users/Tristan/.email", HEADER_SUBJECT);
    Win32ExtractHeader(&Thread, date, GameState.email_array, GameState.email_count, win32.DEBUGPlatformReadEntireFile,
                       "C:/Users/Tristan/.email", HEADER_DATE);
    
    // sort emails by Date Header
    qsort(GameState.email_array, GameState.email_count, sizeof(EmailMetadata), CompareByTimestamp);
    
    // if email was received today just show time instead of whole date
    for(int i = 0;
        i < GameState.email_count;
        i++)
    {
        if(CheckIfEmailReceivedToday(date, GameState.email_array[i].date))
        {
            // Change to just show time implying it was received today
            ChangeDateHeaderIfToday(GameState.email_array[i].date);
        }
    }
    
    if (gamecode.is_valid)
    {
        gamecode.InitializeUI(&GameState, &win32);
    }
    
    
    char PathToFont[256];
    
    if (Window)
    {
        if (!Win32InitOpenGL(Window)) return -1;
        //Win32HandleResizey(state, state.window_width, state.window_height); 
        Win32HandleResizey(WINDOW_WIDTH_HD, WINDOW_HEIGHT_HD);
        
        char *buffer = 0;
        DWORD size = GetEnvironmentVariable("HOMEPATH", NULL, 0);
        if(size)
        {
            buffer = (char *)malloc(size);
            if(buffer && GetEnvironmentVariable("HOMEPATH", buffer, size) > 0)
            {
                for(char *p = buffer;
                    *p;
                    p++)
                {
                    // NOTE(trist007): needs an escape char '\'
                    if(*p == '\\')
                    {
                        *p = '/';
                    }
                }
                //snprintf(PathToFont, sizeof(PathToFont), "C:%s/dev/s3mail/s3mail/code/fonts/liberation-mono.ttf", buffer);
                strcpy(PathToFont, "C:/dev/s3mail/s3mail/code/fonts/liberation-mono.ttf");
            }
            
            free(buffer);
        }
        
        if (!Win32InitFont(&GameState, PathToFont))
        {
            MessageBox(Window, "Failed to load font", "Warning", MB_OK);
        }
        
        ShowWindow(Window, nCmdShow);
        UpdateWindow(Window);
        
        GlobalRunning = true;
        
        uint32 LoadCounter = 0;
        
        // main game loop
        while (GlobalRunning)
        {
            // Check for DLL reload
            FILETIME new_write_time = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
            if (CompareFileTime(&new_write_time, &gamecode.last_write_time) != 0)
            {
                Win32UnloadGameCode(&gamecode);
                
                gamecode = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                             TempGameCodeDLLFullPath,
                                             GameCodeLockFullPath);
                // Add some debugging to see what's happening
                if (gamecode.is_valid) {
                    if (gamecode.InitializeUI) {
                        gamecode.InitializeUI(&GameState, &win32);
                        // Maybe add a debug message here to confirm it ran
                        OutputDebugString("Successfully reinitialized UI after DLL reload\n");
                    } else {
                        OutputDebugString("ERROR: InitializeUI is null after DLL reload\n");
                    }
                } else {
                    OutputDebugString("ERROR: DLL reload failed - gamecode not valid\n");
                }
            }
            
            
            // Call game code
            if (gamecode.is_valid)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                gamecode.UpdateAndRender(&GameState, &win32);
                SwapBuffers(g_hdc);
            }
            
            Win32ProcessPendingMessages(&gamecode, &GameState, &win32);
        }
    }
    else
    {
        // NOTE(trist007): Logging here would be nice!!
    }
    
    return(0);
}
