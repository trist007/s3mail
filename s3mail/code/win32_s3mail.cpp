#include <windows.h>

#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Global variables
static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static HGLRC g_hglrc = NULL;
static int g_window_width = 1200;
static int g_window_height = 800;
static int g_mouse_x = 0, g_mouse_y = 0;
static int g_mouse_down = 0;

// Global font data for stb_truetype
static unsigned char font_buffer[1024*1024];
static stbtt_fontinfo font;
static unsigned char* font_bitmap;
static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
static GLuint font_texture_id;

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
static UIButton compose_button = {10, 10, 100, 30, "Compose", 0, 0};
static UIButton send_button = {120, 10, 80, 30, "Send", 0, 0};
static UIList folder_list = {10, 50, 200, 300, {"Inbox", "Sent", "Drafts", "Trash"}, 4, 0};
static UIList email_list = {220, 50, 400, 300, {"Email 1", "Email 2", "Email 3"}, 3, -1};

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

/*
void DrawText(const char* text, float x, float y) {
    // For now, just draw a placeholder rectangle for each character
    int len = (int)strlen(text);
    SetColor(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < len; i++) {
        DrawRect(x + i * 8, y, 6, 12);
    }
}
*/

void DrawText(const char* text, float x, float y) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    
    float current_x = x;
    
    glBegin(GL_QUADS);
    for (int i = 0; text[i]; i++) {
        if (text[i] >= 32 && text[i] < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512, 512, text[i] - 32, &current_x, &y, &q, 1);
            
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
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
        if (send_button.is_pressed) {
            MessageBox(hwnd, "Send clicked!", "S3Mail", MB_OK);
        }
        
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
        
        case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        return 0;
        
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
    
    // Update UI elements
    UpdateButton(&compose_button, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateButton(&send_button, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateList(&folder_list, g_mouse_x, g_mouse_y, g_mouse_down);
    UpdateList(&email_list, g_mouse_x, g_mouse_y, g_mouse_down);
    
    // Render UI elements
    RenderButton(&compose_button);
    RenderButton(&send_button);
    RenderList(&folder_list);
    RenderList(&email_list);
    
    // Draw email preview area
    SetColor(1.0f, 1.0f, 1.0f);
    DrawRect(630, 50, 550, 300);
    SetColor(0.0f, 0.0f, 0.0f);
    DrawRectOutline(630, 50, 550, 300);
    
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
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          g_window_width, g_window_height,
                          NULL, NULL, hInstance, NULL
                          );
    
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