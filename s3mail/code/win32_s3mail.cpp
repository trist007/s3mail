#include "s3mail_platform.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <GL/gl.h>

global_variable bool32 GlobalRunning;

// Platform state
static HDC g_hdc = 0;
static HGLRC g_hglrc = 0;

// Platform API implementation
void Win32SetColor(float r, float g, float b)
{
    glColor3f(r, g, b);
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

void Win32DrawRectOutline(float x, float y, float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Win32DrawText(GameState *state, const char *text, float x, float y)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, state->font_texture_id);
    
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
            stbtt_GetBakedQuad(state->cdata,
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

// DLL management
FILETIME Win32GetLastWriteTime(const char *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    FILETIME Result = {0};
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        Result = data.ftLastWriteTime;
    }
    return Result;
}

Win32GameCode Win32LoadGameCode(const char *dll_path, const char *temp_dll_path, const char *lock_filename)
{
    Win32GameCode Result = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(lock_filename, GetFileExInfoStandard, &Ignored))
    {
        Result.last_write_time = Win32GetLastWriteTime(dll_path);
        
        CopyFile(dll_path, temp_dll_path, FALSE);
        
        Result.dll = LoadLibrary(temp_dll_path);
        if(Result.dll)
        {
            Result.UpdateAndRender = (game_update_and_render*)GetProcAddress(Result.dll, "GameUpdateAndRender");
            Result.HandleKeyPress = (game_handle_key_press*)GetProcAddress(Result.dll, "GameHandleKeyPress");
            Result.InitializeUI = (game_initialize_ui*)GetProcAddress(Result.dll, "GameInitializeUI");
            Result.is_valid = (Result.UpdateAndRender && Result.HandleKeyPress && Result.InitializeUI);
        }
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

// Initialize game state
GameState Win32InitializeGameState()
{
    GameState state = {0};
    
    // NOTE(trist007): try using CW_USEDEFAULT
    state.window_width = 1200;
    state.window_height = 800;
    
    // Initialize UI elements
    state.compose_button = {10, 635, 100, 30, "Compose", 0, 0};
    state.delete_button = {120, 635, 100, 30, "Delete", 0, 0};
    
    state.folder_list = {10, 495, 200, 125, {"Trash", "Junk", "Drafts", "Sent", "Inbox"}, 5, 0};
    state.email_list = {220, 40, 900, 580, {"Email 3", "Email 2", "Email 1"}, 3, -1};
    state.contact_list = {10, 150, 200, 150, {"Papi", "Mom", "Glen", "Vito"}, 4, -1};
    
    return state;
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

int Win32InitFont(GameState *state, const char *font_path)
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
    stbtt_BakeFontBitmap(font_buffer, 0, 24.0f, font_bitmap, 512, 512, 32, 96, state->cdata);
    
    glGenTextures(1, &state->font_texture_id);
    glBindTexture(GL_TEXTURE_2D, state->font_texture_id);
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
Win32ProcessPendingMessages(Win32GameCode *gamecode, GameState *state, PlatformAPI *platform)
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
                    if(gamecode->is_valid && gamecode->HandleKeyPress)
                    {
                        gamecode->HandleKeyPress(state, VKCode);
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
                state->mouse_x = LOWORD(Message.lParam);
                state->mouse_y = HIWORD(Message.lParam);
                
                InvalidateRect(Message.hwnd, 0, FALSE);
            } break;
            
            case WM_LBUTTONDOWN:
            {
                state->mouse_down = 1;
                SetCapture(Message.hwnd);
                
                InvalidateRect(Message.hwnd, 0, FALSE);
            } break;
            
            case WM_LBUTTONUP:
            {
                state->mouse_down = 0;
                ReleaseCapture();
                
                // Handle button clicks
                if (state->compose_button.is_pressed) {
                    MessageBox(Message.hwnd, "Compose clicked!", "S3Mail", MB_OK);
                }
                if (state->delete_button.is_pressed) {
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
    // Load initial game code
    Win32GameCode gamecode = Win32LoadGameCode("s3mail_game.dll", "s3mail_game_temp.dll", "lock.tmp");
    
    // Initialize GameState
    GameState state = Win32InitializeGameState();
    
    // Register and create window
    WNDCLASS wc = {0};
    wc.lpfnWndProc = Win32MainWindowCallback;
    wc.hInstance = hInstance;
    wc.lpszClassName = "S3MailWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    
    if (!RegisterClass(&wc)) return 1;
    
    HWND Window = CreateWindowEx(WS_EX_TOPMOST,
                                 "S3MailWindow",
                                 "S3Mail",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 1000,
                                 200,
                                 1200,
                                 800,
                                 0,
                                 0,
                                 hInstance,
                                 0);
    
    if (Window)
    {
        
        if (!Win32InitOpenGL(Window)) return -1;
        //Win32HandleResizey(state, state.window_width, state.window_height); 
        Win32HandleResizey(1200, 800);
        if (!Win32InitFont(&state, "C:\\dev\\s3mail\\s3mail\\code\\fonts\\liberation-mono.ttf"))
        {
            MessageBox(Window, "Failed to load font", "Warning", MB_OK);
        }
        
        ShowWindow(Window, nCmdShow);
        UpdateWindow(Window);
        
        // Setup Platform API
        PlatformAPI win32 = {0};
        win32.SetColor = Win32SetColor;
        win32.DrawRect = Win32DrawRect;
        win32.DrawRectOutline = Win32DrawRectOutline;
        win32.DrawText = Win32DrawText;
        win32.HandleResizey = Win32HandleResizey;
        win32.PointInRect = Win32PointInRect;
        win32.ShowMessage = Win32ShowMessage;
        win32.InvalidateWindow = Win32InvalidateWindow;
        win32.State = &state;
        win32.Window = Window;
        
        GlobalRunning = true;
        
        if (gamecode.is_valid)
        {
            gamecode.InitializeUI(&state, &win32);
        }
        
        // main game loop
        while (GlobalRunning)
        {
            // Check for DLL reload
            FILETIME new_write_time = Win32GetLastWriteTime("s3mail_game.dll");
            if (CompareFileTime(&new_write_time, &gamecode.last_write_time) != 0)
            {
                Win32UnloadGameCode(&gamecode);
                gamecode = Win32LoadGameCode("s3mail_game.dll", "s3mail_game_temp.dll", "lock.tmp");
            }
            
            Win32ProcessPendingMessages(&gamecode, &state, &win32);
            
            // Call game code
            if (gamecode.is_valid)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                gamecode.UpdateAndRender(&state, &win32);
                SwapBuffers(g_hdc);
            }
        }
    }
    else
    {
        // NOTE(trist007): Logging here would be nice!!
    }
    
    return(0);
}
