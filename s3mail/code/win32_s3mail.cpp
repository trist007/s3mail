#include "s3mail_platform.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Platform state
static HWND g_hwnd = 0;
static HDC g_hdc = 0;
static HGLRC g_hglrc = 0;
static GameState g_game_state = {0};

// DLL hot reloading
typedef struct {
    HMODULE dll;
    FILETIME last_write_time;
    game_update_and_render *UpdateAndRender;
    game_handle_key_press *HandleKeyPress;
    bool32 is_valid;
} Win32_GameCode;

static Win32_GameCode g_game_code = {0};

// Platform API implementation
void Win32_SetColor(float r, float g, float b)
{
    glColor3f(r, g, b);
}

void Win32_DrawRect(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Win32_DrawRectOutline(float x, float y, float width, float height)
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void Win32_DrawText(const char *text, float x, float y)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, g_game_state.font_texture_id);
    
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
            stbtt_GetBakedQuad(g_game_state.cdata,
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

int Win32_PointInRect(int px, int py, float x, float y, float width, float height)
{
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

void Win32_ShowMessage(const char *message)
{
    MessageBox(g_hwnd, message, "S3Mail", MB_OK);
}

void Win32_InvalidateWindow(void)
{
    InvalidateRect(g_hwnd, 0, FALSE);
}

// DLL management
FILETIME Win32_GetLastWriteTime(const char *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    FILETIME Result = {0};
    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        Result = data.ftLastWriteTime;
    }
    return Result;
}

Win32_GameCode Win32_LoadGameCode(const char *dll_path, const char *temp_dll_path, char *lock_filename)
{
    Win32_GameCode Result = {0};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(lock_filename, GetFileExInfoStandard, &Ignored))
    {
        Result.last_write_time = Win32_GetLastWriteTime(dll_path);
        
        CopyFile(dll_path, temp_dll_path, FALSE);
        
        Result.dll = LoadLibrary(temp_dll_path);
        if(Result.dll)
        {
            Result.UpdateAndRender = (game_update_and_render*)GetProcAddress(Result.dll, "GameUpdateAndRender");
            Result.HandleKeyPress = (game_handle_key_press*)GetProcAddress(Result.dll, "GameHandleKeyPress");
            Result.is_valid = (Result.UpdateAndRender && Result.HandleKeyPress);
        }
    }
    
    if (!Result.is_valid)
    {
        Result.UpdateAndRender = 0;
        Result.HandleKeyPress = 0;
    }
    
    return Result;
}

void Win32_UnloadGameCode(Win32_GameCode *game_code)
{
    if (game_code->dll)
    {
        FreeLibrary(game_code->dll);
        game_code->dll = 0;
    }
    game_code->is_valid = false;
    game_code->UpdateAndRender = 0;
    game_code->HandleKeyPress = 0;
}

// Initialize game state
void Win32_InitializeGameState(void)
{
    // NOTE(trist007): try using CW_USEDEFAULT
    g_game_state.window_width = 1200;
    g_game_state.window_height = 800;
    
    // Initialize UI elements
    g_game_state.compose_button = {10, 635, 100, 30, "Compose", 0, 0};
    g_game_state.delete_button = {120, 635, 100, 30, "Delete", 0, 0};
    
    g_game_state.folder_list = {10, 495, 200, 125, {"Trash", "Junk", "Drafts", "Sent", "Inbox"}, 5, 0};
    g_game_state.email_list = {220, 40, 900, 580, {"Email 3", "Email 2", "Email 1"}, 3, -1};
    g_game_state.contact_list = {10, 150, 200, 150, {"Papi", "Mom", "Glen", "Vito"}, 4, -1};
}

// OpenGL and font initialization (same as before)
int Win32_InitOpenGL(HWND hwnd)
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

int Win32_InitFont(const char *font_path)
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
    stbtt_BakeFontBitmap(font_buffer, 0, 24.0f, font_bitmap, 512, 512, 32, 96, g_game_state.cdata);
    
    glGenTextures(1, &g_game_state.font_texture_id);
    glBindTexture(GL_TEXTURE_2D, g_game_state.font_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    free(font_bitmap);
    return 1;
}

void Win32_HandleResize(int width, int height)
{
    if (height == 0) height = 1;
    
    g_game_state.window_width = width;
    g_game_state.window_height = height;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        if (!Win32_InitOpenGL(hwnd)) return -1;
        if (!Win32_InitFont("C:\\dev\\s3mail\\s3mail\\code\\fonts\\liberation-mono.ttf"))
        {
            MessageBox(hwnd, "Failed to load font", "Warning", MB_OK);
        }
        Win32_InitializeGameState();
        return 0;
        
        case WM_DESTROY:
        Win32_UnloadGameCode(&g_game_code);
        PostQuitMessage(0);
        return 0;
        
        case WM_SIZE:
        Win32_HandleResize(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hwnd, 0, FALSE);
        return 0;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            
            // Check for DLL reload
            FILETIME new_write_time = Win32_GetLastWriteTime("s3mail_game.dll");
            if (CompareFileTime(&new_write_time, &g_game_code.last_write_time) != 0)
            {
                Win32_UnloadGameCode(&g_game_code);
                g_game_code = Win32_LoadGameCode("s3mail_game.dll", "s3mail_game_temp.dll", "lock.tmp");
            }
            
            // Setup Platform API
            PlatformAPI win32 = {0};
            win32.SetColor = Win32_SetColor;
            win32.DrawRect = Win32_DrawRect;
            win32.DrawRectOutline = Win32_DrawRectOutline;
            win32.DrawText = Win32_DrawText;
            win32.PointInRect = Win32_PointInRect;
            win32.ShowMessage = Win32_ShowMessage;
            win32.InvalidateWindow = Win32_InvalidateWindow;
            
            // Call game code
            if (g_game_code.is_valid)
            {
                glClear(GL_COLOR_BUFFER_BIT);
                g_game_code.UpdateAndRender(&g_game_state, &win32);
            }
            
            SwapBuffers(g_hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_MOUSEMOVE:
        g_game_state.mouse_x = LOWORD(lParam);
        g_game_state.mouse_y = HIWORD(lParam);
        InvalidateRect(hwnd, 0, FALSE);
        return 0;
        
        case WM_LBUTTONDOWN:
        g_game_state.mouse_down = 1;
        InvalidateRect(hwnd, 0, FALSE);
        return 0;
        
        case WM_LBUTTONUP:
        g_game_state.mouse_down = 0;
        InvalidateRect(hwnd, 0, FALSE);
        return 0;
        
        case WM_KEYDOWN:
        if (g_game_code.is_valid)
        {
            g_game_code.HandleKeyPress(&g_game_state, (int)wParam);
        }
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        return 0;
        
        default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Load initial game code
    g_game_code = Win32_LoadGameCode("s3mail_game.dll", "s3mail_game_temp.dll", "lock.tmp");
    
    // Register and create window
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "S3MailWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    
    if (!RegisterClass(&wc)) return 1;
    
    g_hwnd = CreateWindowEx(WS_EX_TOPMOST,
                            "S3MailWindow",
                            "S3Mail",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            3000,
                            200,
                            1200,
                            800,
                            0,
                            0,
                            hInstance,
                            0);
    
    if (!g_hwnd) return 1;
    
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
    MSG msg;
    while (GetMessage(&msg, 0, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
