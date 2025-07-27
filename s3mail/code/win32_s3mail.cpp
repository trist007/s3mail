#include <windows.h>

#include <GL/gl.h>

#include <stdio.h>
#include <string.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO(casey): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

//
// NOTE(casey): Types
//
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

typedef size_t memory_index;

typedef float real32;
typedef double real64;

#define internal static
#define local_persist static
#define global_variable static

// Global variables
global_variable HWND g_hwnd = 0;
global_variable HDC g_hdc = 0;
global_variable HGLRC g_hglrc = 0;
global_variable int g_window_width = 1200;
global_variable int g_window_height = 800;
global_variable int g_mouse_x = 0, g_mouse_y = 0;
global_variable int g_mouse_down = 0;

// Global font data for stb_truetype
global_variable unsigned char font_buffer[1024*1024];
global_variable stbtt_fontinfo font;
global_variable unsigned char* font_bitmap;
global_variable stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
global_variable GLuint font_texture_id;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int InitOpenGL(HWND hwnd);
int InitFont(const char* font_path);
void CleanupOpenGL(void);
void RenderFrame(void);
void HandleResize(int width, int height);

// Simple UI elements for email client
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

// UI state
static UIButton compose_button = {10, 635, 100, 30, "Compose", 0, 0};
static UIButton delete_button = {120, 635, 100, 30, "Delete", 0, 0};
static UIList folder_list = {10, 495, 200, 125, {"Trash", "Junk", "Drafts", "Sent", "Inbox"}, 5, 0};
static UIList email_list = {220, 40, 900, 580, {"Email 3", "Email 2", "Email 1"}, 3, -1};
static UIList nomas = {10, 150, 200, 150, { "Papi", "Mon", "Glenn", "Vito"}, 4, -1};

// Simple rendering functions
void SetColor(float r, float g, float b) {
    glColor3f(r, g, b);
}

void DrawRect(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void DrawRectOutline(float x, float y, float width, float height) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void DrawText(const char* text, float x, float y) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    
    float current_x = x;
    float current_y = y;
    
    glBegin(GL_QUADS);
    for (int i = 0; text[i]; i++) {
        if (text[i] >= 32 && text[i] < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, text[i] - 32, &current_x, &current_y, &q, 1);
            
            // Flip Y coordinates OpenGL uses top-left origin, STB Truetype uses bottom-left origin
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

int PointInRect(int px, int py, float x, float y, float width, float height) {
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

void UpdateButton(UIButton* btn, int mouse_x, int mouse_y, int mouse_down) {
    // Convert screen coordinates to OpenGL coordinates
    int gl_y = g_window_height - mouse_y;
    
    btn->is_hovered = PointInRect(mouse_x, gl_y, btn->x, btn->y, btn->width, btn->height);
    btn->is_pressed = btn->is_hovered && mouse_down;
}

void RenderButton(UIButton* btn) {
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
    
    // Draw text placeholder
    SetColor(1.0f, 1.0f, 1.0f);
    DrawText(btn->text, btn->x + 5, btn->y + 8);
}

void UpdateList(UIList* list, int mouse_x, int mouse_y, int mouse_down) {
    int gl_y = g_window_height - mouse_y;
    
    if (mouse_down && PointInRect(mouse_x, gl_y, list->x, list->y, list->width, list->height)) {
        int item_height = 25;
        int clicked_item = (gl_y - list->y) / item_height;
        if (clicked_item >= 0 && clicked_item < list->item_count) {
            list->selected_item = clicked_item;
        }
    }
}

void RenderList(UIList* list) {
    // Background
    SetColor(0.9f, 0.9f, 0.9f);
    DrawRect(list->x, list->y, list->width, list->height);
    
    // Border
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(list->x, list->y, list->width, list->height);
    
    // Items
    int item_height = 25;
    for (int i = 0; i < list->item_count; i++) {
        float item_y = list->y + i * item_height;
        
        if (i == list->selected_item) {
            SetColor(0.5f, 0.7f, 1.0f);
            DrawRect(list->x, item_y, list->width, item_height);
        }
        
        SetColor(0.0f, 0.0f, 0.0f);
        DrawText(list->items[i], list->x + 5, item_y + 5);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
        if (!InitOpenGL(hwnd))
        {
            return -1;
        }
        
        // Initialize font after OpenGL context is created
        if (!InitFont("C:\\dev\\s3mail\\s3mail\\code\\fonts\\liberation-mono.ttf"))
        {
            MessageBox(hwnd, "Failed to load font", "Warning", MB_OK);
            // Continue anyways - will use placeholder rectangles
        }
        return 0;
        
        case WM_DESTROY:
        CleanupOpenGL();
        PostQuitMessage(0);
        return 0;
        
        case WM_SIZE:
        g_window_width = LOWORD(lParam);
        g_window_height = HIWORD(lParam);
        HandleResize(g_window_width, g_window_height);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RenderFrame();
            SwapBuffers(g_hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_MOUSEMOVE:
        g_mouse_x = LOWORD(lParam);
        g_mouse_y = HIWORD(lParam);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
        case WM_LBUTTONDOWN:
        g_mouse_down = 1;
        SetCapture(hwnd);
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
        case WM_LBUTTONUP:
        g_mouse_down = 0;
        ReleaseCapture();
        
        // Handle button clicks
        if (compose_button.is_pressed) {
            MessageBox(hwnd, "Compose clicked!", "S3Mail", MB_OK);
        }
        if (delete_button.is_pressed) {
            MessageBox(hwnd, "Delete clicked!", "S3Mail", MB_OK);
        }
        
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
        case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        else if (wParam == 'J')
        {
            // Move down
            if (email_list.selected_item < email_list.item_count - 1)
            {
                email_list.selected_item++;
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        else if (wParam == 'K')
        {
            // Move up
            if (email_list.selected_item > 0)
            {
                email_list.selected_item--;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int InitOpenGL(HWND hwnd) {
    g_hdc = GetDC(hwnd);
    if (!g_hdc) return 0;
    
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                                      // version
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,                                     // color depth
        0, 0, 0, 0, 0, 0,                      // color bits ignored
        0,                                      // no alpha buffer
        0,                                      // shift bit ignored
        0,                                      // no accumulation buffer
        0, 0, 0, 0,                            // accum bits ignored
        24,                                     // 24-bit z-buffer
        8,                                      // 8-bit stencil buffer
        0,                                      // no auxiliary buffer
        PFD_MAIN_PLANE,                        // main layer
        0,                                      // reserved
        0, 0, 0                                // layer masks ignored
    };
    
    int pixelFormat = ChoosePixelFormat(g_hdc, &pfd);
    if (!pixelFormat) return 0;
    
    if (!SetPixelFormat(g_hdc, pixelFormat, &pfd)) return 0;
    
    g_hglrc = wglCreateContext(g_hdc);
    if (!g_hglrc) return 0;
    
    if (!wglMakeCurrent(g_hdc, g_hglrc)) return 0;
    
    // Set up OpenGL state
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    HandleResize(g_window_width, g_window_height);
    
    return 1;
}

void CleanupOpenGL(void) {
    if (font_bitmap) {
        free(font_bitmap);
        font_bitmap = NULL;
    }
    if (font_texture_id) {
        glDeleteTextures(1, &font_texture_id);
        font_texture_id = 0;
    }
    if (g_hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hglrc);
        g_hglrc = NULL;
    }
    if (g_hdc) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
}

int InitFont(const char* font_path) {
    FILE* file = fopen(font_path, "rb");
    if (!file) return 0;
    
    size_t size = fread(font_buffer, 1, 1024*1024, file);
    fclose(file);
    
    if (size == 0) return 0;
    
    stbtt_InitFont(&font, font_buffer, 0);
    
    // Create bitmap for characters
    font_bitmap = (unsigned char *)malloc(512 * 512);
    stbtt_BakeFontBitmap(font_buffer, 0, 24.0f, font_bitmap, 512, 512, 32, 96, cdata);
    
    // Create OpenGL texture
    glGenTextures(1, &font_texture_id);
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return 1;
}

void HandleResize(int width, int height) {
    if (height == 0) height = 1;
    
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Set up 2D orthographic projection (0,0 at bottom-left)
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RenderFrame(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw cool purple style stripe
    SetColor(0.3f, 0.3f, 0.7f);
    DrawRect(0, 675, 1200, 450);
    
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(0, 675, 1200, 450);
    
    // Update UI elements
    UpdateButton(&compose_button, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateButton(&delete_button, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateList(&folder_list, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateList(&email_list, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateList(&nomas, g_mouse_x, g_mouse_y, g_mouse_down);
    
    // Render UI elements
    RenderButton(&compose_button);
    RenderButton(&delete_button);
    RenderList(&folder_list);
    RenderList(&email_list);
    RenderList(&nomas);
    
    // Draw email preview area
    /*
SetColor(1.0f, 1.0f, 1.0f);
    DrawRect(15, 50, 550, 300);
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(15, 50, 550, 300);
    */
    
    if (email_list.selected_item >= 0) {
        char preview_text[256];
        sprintf(preview_text, "Preview of %s", email_list.items[email_list.selected_item]);
        DrawText(preview_text, 640, 320);
    }
    
    // Status bar
    SetColor(0.8f, 0.8f, 0.8f);
    DrawRect(0, 0, g_window_width, 25);
    SetColor(0.0f, 0.0f, 0.0f);
    DrawText("S3Mail - Ready", 10, 8);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "S3MailWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
        return 1;
    }
    
    // Create window
    g_hwnd = CreateWindow(
                          "S3MailWindow",
                          "S3Mail - OpenGL Email Client",
                          WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          //CW_USEDEFAULT, CW_USEDEFAULT,
                          3000, 200, // spawn on the right hand side of the screen
                          g_window_width,
                          g_window_height,
                          0,
                          0,
                          hInstance,
                          0);
    
    if (!g_hwnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK);
        return 1;
    }
    
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}