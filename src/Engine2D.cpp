// =====================================================================================
//  Engine2D.cpp  -  Implementação do motor (backend SDL2 / SDL2_image / SDL2_mixer)
// =====================================================================================
#include "Engine2D.hpp"
#include "Font8x8.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unordered_map>

namespace Engine2D {

// =====================================================================================
//  ESTADO GLOBAL INTERNO
// =====================================================================================
namespace internal {

struct SoundData {
    Mix_Chunk* chunk = nullptr;
    int        channel = -1;
};

// Estrutura central com todo o estado "escondido" da engine (equivalente ao
// CoreData interno da raylib). Fica isolada num único objeto estático para
// evitar variáveis globais soltas.
struct State {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;

    int screenWidth  = 0;
    int screenHeight = 0;
    bool fullscreen  = false;

    bool shouldClose = false;

    // ---- Tempo ----
    Uint64 perfFreq       = 0;
    Uint64 lastCounter    = 0;
    double frameTime      = 0.0;
    double totalTime      = 0.0;
    int    targetFPS      = 0;      // 0 = ilimitado
    double targetFrameSec = 0.0;
    int    currentFPS     = 0;
    double fpsAccumTime   = 0.0;
    int    fpsAccumFrames = 0;

    // ---- Teclado ----
    static constexpr int MAX_KEYS = 512;
    bool keyCurrent[MAX_KEYS]  = { false };
    bool keyPrevious[MAX_KEYS] = { false };

    // ---- Mouse ----
    static constexpr int MAX_MOUSE_BUTTONS = 5;
    bool mouseCurrent[MAX_MOUSE_BUTTONS]  = { false };
    bool mousePrevious[MAX_MOUSE_BUTTONS] = { false };
    int  mouseX = 0;
    int  mouseY = 0;
    float mouseWheel = 0.0f;

    // ---- Câmera 2D ----
    bool     cameraActive = false;
    Camera2D activeCamera;

    // ---- Áudio ----
    bool audioReady = false;

    // ---- Random ----
    bool randomSeeded = false;
};

static State S;

// Converte um SDL_Scancode para o código de tecla usado pela engine (compatível
// com os valores numéricos da GLFW/raylib, para ASCII e algumas teclas especiais).
static int ScancodeToEngineKey(SDL_Scancode sc) {
    switch (sc) {
        case SDL_SCANCODE_SPACE: return 32;
        case SDL_SCANCODE_APOSTROPHE: return 39;
        case SDL_SCANCODE_COMMA: return 44;
        case SDL_SCANCODE_MINUS: return 45;
        case SDL_SCANCODE_PERIOD: return 46;
        case SDL_SCANCODE_SLASH: return 47;
        case SDL_SCANCODE_0: return 48; case SDL_SCANCODE_1: return 49;
        case SDL_SCANCODE_2: return 50; case SDL_SCANCODE_3: return 51;
        case SDL_SCANCODE_4: return 52; case SDL_SCANCODE_5: return 53;
        case SDL_SCANCODE_6: return 54; case SDL_SCANCODE_7: return 55;
        case SDL_SCANCODE_8: return 56; case SDL_SCANCODE_9: return 57;
        case SDL_SCANCODE_SEMICOLON: return 59;
        case SDL_SCANCODE_EQUALS: return 61;
        case SDL_SCANCODE_A: return 65; case SDL_SCANCODE_B: return 66;
        case SDL_SCANCODE_C: return 67; case SDL_SCANCODE_D: return 68;
        case SDL_SCANCODE_E: return 69; case SDL_SCANCODE_F: return 70;
        case SDL_SCANCODE_G: return 71; case SDL_SCANCODE_H: return 72;
        case SDL_SCANCODE_I: return 73; case SDL_SCANCODE_J: return 74;
        case SDL_SCANCODE_K: return 75; case SDL_SCANCODE_L: return 76;
        case SDL_SCANCODE_M: return 77; case SDL_SCANCODE_N: return 78;
        case SDL_SCANCODE_O: return 79; case SDL_SCANCODE_P: return 80;
        case SDL_SCANCODE_Q: return 81; case SDL_SCANCODE_R: return 82;
        case SDL_SCANCODE_S: return 83; case SDL_SCANCODE_T: return 84;
        case SDL_SCANCODE_U: return 85; case SDL_SCANCODE_V: return 86;
        case SDL_SCANCODE_W: return 87; case SDL_SCANCODE_X: return 88;
        case SDL_SCANCODE_Y: return 89; case SDL_SCANCODE_Z: return 90;
        case SDL_SCANCODE_ESCAPE: return 256;
        case SDL_SCANCODE_RETURN: return 257;
        case SDL_SCANCODE_TAB: return 258;
        case SDL_SCANCODE_BACKSPACE: return 259;
        case SDL_SCANCODE_INSERT: return 260;
        case SDL_SCANCODE_DELETE: return 261;
        case SDL_SCANCODE_RIGHT: return 262;
        case SDL_SCANCODE_LEFT: return 263;
        case SDL_SCANCODE_DOWN: return 264;
        case SDL_SCANCODE_UP: return 265;
        case SDL_SCANCODE_PAGEUP: return 266;
        case SDL_SCANCODE_PAGEDOWN: return 267;
        case SDL_SCANCODE_HOME: return 268;
        case SDL_SCANCODE_END: return 269;
        case SDL_SCANCODE_LSHIFT: return 340;
        case SDL_SCANCODE_LCTRL: return 341;
        case SDL_SCANCODE_LALT: return 342;
        case SDL_SCANCODE_RSHIFT: return 344;
        case SDL_SCANCODE_RCTRL: return 345;
        case SDL_SCANCODE_RALT: return 346;
        case SDL_SCANCODE_F1: return 290; case SDL_SCANCODE_F2: return 291;
        case SDL_SCANCODE_F3: return 292; case SDL_SCANCODE_F4: return 293;
        case SDL_SCANCODE_F5: return 294; case SDL_SCANCODE_F6: return 295;
        case SDL_SCANCODE_F7: return 296; case SDL_SCANCODE_F8: return 297;
        case SDL_SCANCODE_F9: return 298; case SDL_SCANCODE_F10: return 299;
        case SDL_SCANCODE_F11: return 300; case SDL_SCANCODE_F12: return 301;
        default: return -1;
    }
}

// Aplica a câmera 2D ativa (se houver) a um ponto do "mundo", retornando a posição
// correspondente em coordenadas de tela.
static Vector2 WorldToScreen(Vector2 p) {
    if (!S.cameraActive) return p;
    const Camera2D& cam = S.activeCamera;

    // 1) desloca para a origem do alvo
    float x = p.x - cam.target.x;
    float y = p.y - cam.target.y;

    // 2) rotação
    float rad = cam.rotation * 3.14159265358979323846f / 180.0f;
    float cosr = std::cos(rad);
    float sinr = std::sin(rad);
    float rx = x * cosr - y * sinr;
    float ry = x * sinr + y * cosr;

    // 3) zoom
    rx *= cam.zoom;
    ry *= cam.zoom;

    // 4) desloca para o offset (posição na tela)
    return { rx + cam.offset.x, ry + cam.offset.y };
}

static float WorldToScreenScale() {
    return S.cameraActive ? S.activeCamera.zoom : 1.0f;
}

} // namespace internal

using namespace internal;

// =====================================================================================
//  JANELA / CICLO DE VIDA
// =====================================================================================

bool InitWindow(int width, int height, const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        std::fprintf(stderr, "Engine2D: falha ao iniciar SDL: %s\n", SDL_GetError());
        return false;
    }

    // Inicializa o SDL_image com suporte a PNG e JPG (formatos mais comuns além do BMP).
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) == 0) {
        // Não é fatal: ainda é possível carregar .bmp nativamente via SDL2.
        std::fprintf(stderr, "Engine2D: aviso - SDL_image sem suporte completo a PNG/JPG (%s)\n", IMG_GetError());
    }

    S.window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!S.window) {
        std::fprintf(stderr, "Engine2D: falha ao criar janela: %s\n", SDL_GetError());
        return false;
    }

    S.renderer = SDL_CreateRenderer(
        S.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!S.renderer) {
        // fallback para renderer por software, caso não haja aceleração disponível
        S.renderer = SDL_CreateRenderer(S.window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!S.renderer) {
        std::fprintf(stderr, "Engine2D: falha ao criar renderer: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(S.renderer, SDL_BLENDMODE_BLEND);

    S.screenWidth  = width;
    S.screenHeight = height;
    S.shouldClose  = false;

    S.perfFreq    = SDL_GetPerformanceFrequency();
    S.lastCounter = SDL_GetPerformanceCounter();
    S.totalTime   = 0.0;
    S.frameTime   = 0.0;

    std::fill(std::begin(S.keyCurrent), std::end(S.keyCurrent), false);
    std::fill(std::begin(S.keyPrevious), std::end(S.keyPrevious), false);
    std::fill(std::begin(S.mouseCurrent), std::end(S.mouseCurrent), false);
    std::fill(std::begin(S.mousePrevious), std::end(S.mousePrevious), false);

    return true;
}

void CloseWindow() {
    if (S.audioReady) {
        CloseAudioDevice();
    }
    if (S.renderer) { SDL_DestroyRenderer(S.renderer); S.renderer = nullptr; }
    if (S.window)   { SDL_DestroyWindow(S.window);     S.window   = nullptr; }
    IMG_Quit();
    SDL_Quit();
}

static void PollEvents() {
    // Copia estado atual para "anterior" antes de processar novos eventos.
    std::copy(std::begin(S.keyCurrent), std::end(S.keyCurrent), std::begin(S.keyPrevious));
    std::copy(std::begin(S.mouseCurrent), std::end(S.mouseCurrent), std::begin(S.mousePrevious));
    S.mouseWheel = 0.0f;

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                S.shouldClose = true;
                break;
            case SDL_KEYDOWN: {
                int k = ScancodeToEngineKey(ev.key.keysym.scancode);
                if (k >= 0 && k < State::MAX_KEYS) S.keyCurrent[k] = true;
                break;
            }
            case SDL_KEYUP: {
                int k = ScancodeToEngineKey(ev.key.keysym.scancode);
                if (k >= 0 && k < State::MAX_KEYS) S.keyCurrent[k] = false;
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                int b = ev.button.button - 1; // SDL: 1=left,2=middle,3=right -> ajustado abaixo
                if (ev.button.button == SDL_BUTTON_LEFT) S.mouseCurrent[0] = true;
                else if (ev.button.button == SDL_BUTTON_RIGHT) S.mouseCurrent[1] = true;
                else if (ev.button.button == SDL_BUTTON_MIDDLE) S.mouseCurrent[2] = true;
                (void)b;
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if (ev.button.button == SDL_BUTTON_LEFT) S.mouseCurrent[0] = false;
                else if (ev.button.button == SDL_BUTTON_RIGHT) S.mouseCurrent[1] = false;
                else if (ev.button.button == SDL_BUTTON_MIDDLE) S.mouseCurrent[2] = false;
                break;
            }
            case SDL_MOUSEWHEEL:
                S.mouseWheel += static_cast<float>(ev.wheel.y);
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    S.screenWidth  = ev.window.data1;
                    S.screenHeight = ev.window.data2;
                }
                break;
            default: break;
        }
    }

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    S.mouseX = mx;
    S.mouseY = my;
}

bool WindowShouldClose() {
    PollEvents();
    return S.shouldClose;
}

void SetWindowShouldClose(bool value) { S.shouldClose = value; }

void SetWindowSize(int width, int height) {
    if (S.window) SDL_SetWindowSize(S.window, width, height);
    S.screenWidth  = width;
    S.screenHeight = height;
}

void SetWindowTitle(const std::string& title) {
    if (S.window) SDL_SetWindowTitle(S.window, title.c_str());
}

int GetScreenWidth()  { return S.screenWidth; }
int GetScreenHeight() { return S.screenHeight; }

void ToggleFullscreen() {
    if (!S.window) return;
    S.fullscreen = !S.fullscreen;
    SDL_SetWindowFullscreen(S.window, S.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool IsWindowFullscreen() { return S.fullscreen; }

// =====================================================================================
//  TEMPO / FPS
// =====================================================================================

void SetTargetFPS(int fps) {
    S.targetFPS = fps;
    S.targetFrameSec = (fps > 0) ? (1.0 / static_cast<double>(fps)) : 0.0;
}

int GetFPS() { return S.currentFPS; }
float GetFrameTime() { return static_cast<float>(S.frameTime); }
double GetTime() { return S.totalTime; }

// =====================================================================================
//  DESENHO
// =====================================================================================

void BeginDrawing() {
    // início de frame: nada de especial além de garantir que o renderer existe
}

void EndDrawing() {
    SDL_RenderPresent(S.renderer);

    // ---- limitação de FPS (quando não há vsync suficiente ou o alvo é menor) ----
    Uint64 now = SDL_GetPerformanceCounter();
    double elapsed = static_cast<double>(now - S.lastCounter) / static_cast<double>(S.perfFreq);

    if (S.targetFrameSec > 0.0 && elapsed < S.targetFrameSec) {
        double remaining = S.targetFrameSec - elapsed;
        SDL_Delay(static_cast<Uint32>(remaining * 1000.0));
        now = SDL_GetPerformanceCounter();
        elapsed = static_cast<double>(now - S.lastCounter) / static_cast<double>(S.perfFreq);
    }

    S.frameTime = elapsed;
    S.totalTime += elapsed;
    S.lastCounter = now;

    // FPS médio, atualizado a cada ~0.25s para não "tremer" na tela
    S.fpsAccumTime += elapsed;
    S.fpsAccumFrames += 1;
    if (S.fpsAccumTime >= 0.25) {
        S.currentFPS = static_cast<int>(std::round(S.fpsAccumFrames / S.fpsAccumTime));
        S.fpsAccumTime = 0.0;
        S.fpsAccumFrames = 0;
    }
}

void ClearBackground(Color color) {
    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(S.renderer);
}

void BeginMode2D(const Camera2D& camera) {
    S.cameraActive = true;
    S.activeCamera = camera;
}

void EndMode2D() {
    S.cameraActive = false;
}

// ---- Formas geométricas --------------------------------------------------------------

void DrawPixel(int posX, int posY, Color color) {
    Vector2 p = WorldToScreen({ (float)posX, (float)posY });
    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(S.renderer, (int)p.x, (int)p.y);
}

void DrawLine(int startX, int startY, int endX, int endY, Color color) {
    Vector2 a = WorldToScreen({ (float)startX, (float)startY });
    Vector2 b = WorldToScreen({ (float)endX, (float)endY });
    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(S.renderer, (int)a.x, (int)a.y, (int)b.x, (int)b.y);
}

void DrawLineEx(Vector2 start, Vector2 end, float thickness, Color color) {
    Vector2 a = WorldToScreen(start);
    Vector2 b = WorldToScreen(end);
    float scale = WorldToScreenScale();
    thickness = std::max(1.0f, thickness * scale);

    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.0001f) { DrawPixel((int)start.x, (int)start.y, color); return; }
    float nx = -dy / len * (thickness * 0.5f);
    float ny =  dx / len * (thickness * 0.5f);

    SDL_Vertex verts[4];
    SDL_Color sc{ color.r, color.g, color.b, color.a };
    verts[0].position = { a.x + nx, a.y + ny }; verts[0].color = sc; verts[0].tex_coord = {0,0};
    verts[1].position = { a.x - nx, a.y - ny }; verts[1].color = sc; verts[1].tex_coord = {0,0};
    verts[2].position = { b.x - nx, b.y - ny }; verts[2].color = sc; verts[2].tex_coord = {0,0};
    verts[3].position = { b.x + nx, b.y + ny }; verts[3].color = sc; verts[3].tex_coord = {0,0};
    int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(S.renderer, nullptr, verts, 4, idx, 6);
}

void DrawCircle(int centerX, int centerY, float radius, Color color) {
    Vector2 c = WorldToScreen({ (float)centerX, (float)centerY });
    float r = radius * WorldToScreenScale();
    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);

    int ir = (int)std::ceil(r);
    for (int dy = -ir; dy <= ir; dy++) {
        float dxMax = std::sqrt(std::max(0.0f, r * r - (float)(dy * dy)));
        int x0 = (int)std::round(c.x - dxMax);
        int x1 = (int)std::round(c.x + dxMax);
        SDL_RenderDrawLine(S.renderer, x0, (int)c.y + dy, x1, (int)c.y + dy);
    }
}

void DrawCircleV(Vector2 center, float radius, Color color) {
    DrawCircle((int)center.x, (int)center.y, radius, color);
}

void DrawCircleLines(int centerX, int centerY, float radius, Color color) {
    Vector2 c = WorldToScreen({ (float)centerX, (float)centerY });
    float r = radius * WorldToScreenScale();
    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);

    const int segments = std::max(12, (int)(r * 0.5f));
    float prevX = c.x + r, prevY = c.y;
    for (int i = 1; i <= segments; i++) {
        float ang = (float)i / (float)segments * 2.0f * 3.14159265358979323846f;
        float x = c.x + std::cos(ang) * r;
        float y = c.y + std::sin(ang) * r;
        SDL_RenderDrawLine(S.renderer, (int)prevX, (int)prevY, (int)x, (int)y);
        prevX = x; prevY = y;
    }
}

void DrawRectangle(int posX, int posY, int width, int height, Color color) {
    DrawRectangleRec({ (float)posX, (float)posY, (float)width, (float)height }, color);
}

void DrawRectangleV(Vector2 position, Vector2 size, Color color) {
    DrawRectangleRec({ position.x, position.y, size.x, size.y }, color);
}

void DrawRectangleRec(Rectangle rec, Color color) {
    if (!S.cameraActive) {
        SDL_FRect r{ rec.x, rec.y, rec.width, rec.height };
        SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRectF(S.renderer, &r);
        return;
    }
    // Com câmera ativa, o retângulo pode rotacionar/escalar: desenhamos como polígono.
    Vector2 p0 = WorldToScreen({ rec.x, rec.y });
    Vector2 p1 = WorldToScreen({ rec.x + rec.width, rec.y });
    Vector2 p2 = WorldToScreen({ rec.x + rec.width, rec.y + rec.height });
    Vector2 p3 = WorldToScreen({ rec.x, rec.y + rec.height });
    SDL_Vertex verts[4];
    SDL_Color sc{ color.r, color.g, color.b, color.a };
    verts[0].position = { p0.x, p0.y }; verts[0].color = sc; verts[0].tex_coord = {0,0};
    verts[1].position = { p1.x, p1.y }; verts[1].color = sc; verts[1].tex_coord = {0,0};
    verts[2].position = { p2.x, p2.y }; verts[2].color = sc; verts[2].tex_coord = {0,0};
    verts[3].position = { p3.x, p3.y }; verts[3].color = sc; verts[3].tex_coord = {0,0};
    int idx[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(S.renderer, nullptr, verts, 4, idx, 6);
}

void DrawRectangleLines(int posX, int posY, int width, int height, Color color) {
    Vector2 p0 = WorldToScreen({ (float)posX, (float)posY });
    Vector2 p1 = WorldToScreen({ (float)(posX + width), (float)posY });
    Vector2 p2 = WorldToScreen({ (float)(posX + width), (float)(posY + height) });
    Vector2 p3 = WorldToScreen({ (float)posX, (float)(posY + height) });
    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(S.renderer, (int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y);
    SDL_RenderDrawLine(S.renderer, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);
    SDL_RenderDrawLine(S.renderer, (int)p2.x, (int)p2.y, (int)p3.x, (int)p3.y);
    SDL_RenderDrawLine(S.renderer, (int)p3.x, (int)p3.y, (int)p0.x, (int)p0.y);
}

void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color) {
    DrawLineEx({ rec.x, rec.y }, { rec.x + rec.width, rec.y }, lineThick, color);
    DrawLineEx({ rec.x + rec.width, rec.y }, { rec.x + rec.width, rec.y + rec.height }, lineThick, color);
    DrawLineEx({ rec.x + rec.width, rec.y + rec.height }, { rec.x, rec.y + rec.height }, lineThick, color);
    DrawLineEx({ rec.x, rec.y + rec.height }, { rec.x, rec.y }, lineThick, color);
}

void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color) {
    roundness = std::clamp(roundness, 0.0f, 1.0f);
    float radius = roundness * std::min(rec.width, rec.height) * 0.5f;
    if (radius <= 0.01f) { DrawRectangleRec(rec, color); return; }
    segments = std::max(4, segments);

    // corpo central (cruz) + quatro cantos arredondados
    DrawRectangleRec({ rec.x + radius, rec.y, rec.width - 2 * radius, rec.height }, color);
    DrawRectangleRec({ rec.x, rec.y + radius, radius, rec.height - 2 * radius }, color);
    DrawRectangleRec({ rec.x + rec.width - radius, rec.y + radius, radius, rec.height - 2 * radius }, color);

    struct CornerCenter { float cx, cy, startDeg; };
    CornerCenter corners[4] = {
        { rec.x + radius,               rec.y + radius,                180.0f }, // sup-esq
        { rec.x + rec.width - radius,   rec.y + radius,                270.0f }, // sup-dir
        { rec.x + rec.width - radius,   rec.y + rec.height - radius,   0.0f   }, // inf-dir
        { rec.x + radius,               rec.y + rec.height - radius,   90.0f  }, // inf-esq
    };
    for (auto& corner : corners) {
        for (int i = 0; i < segments; i++) {
            float a0 = (corner.startDeg + (float)i / segments * 90.0f) * 3.14159265f / 180.0f;
            float a1 = (corner.startDeg + (float)(i + 1) / segments * 90.0f) * 3.14159265f / 180.0f;
            Vector2 p0 = WorldToScreen({ corner.cx, corner.cy });
            Vector2 p1 = WorldToScreen({ corner.cx + std::cos(a0) * radius, corner.cy + std::sin(a0) * radius });
            Vector2 p2 = WorldToScreen({ corner.cx + std::cos(a1) * radius, corner.cy + std::sin(a1) * radius });
            SDL_Vertex verts[3];
            SDL_Color sc{ color.r, color.g, color.b, color.a };
            verts[0].position = { p0.x, p0.y }; verts[0].color = sc; verts[0].tex_coord = {0,0};
            verts[1].position = { p1.x, p1.y }; verts[1].color = sc; verts[1].tex_coord = {0,0};
            verts[2].position = { p2.x, p2.y }; verts[2].color = sc; verts[2].tex_coord = {0,0};
            SDL_RenderGeometry(S.renderer, nullptr, verts, 3, nullptr, 0);
        }
    }
}

void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
    Vector2 p1 = WorldToScreen(v1);
    Vector2 p2 = WorldToScreen(v2);
    Vector2 p3 = WorldToScreen(v3);
    SDL_Vertex verts[3];
    SDL_Color sc{ color.r, color.g, color.b, color.a };
    verts[0].position = { p1.x, p1.y }; verts[0].color = sc; verts[0].tex_coord = {0,0};
    verts[1].position = { p2.x, p2.y }; verts[1].color = sc; verts[1].tex_coord = {0,0};
    verts[2].position = { p3.x, p3.y }; verts[2].color = sc; verts[2].tex_coord = {0,0};
    SDL_RenderGeometry(S.renderer, nullptr, verts, 3, nullptr, 0);
}

void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color) {
    DrawLine((int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, color);
    DrawLine((int)v2.x, (int)v2.y, (int)v3.x, (int)v3.y, color);
    DrawLine((int)v3.x, (int)v3.y, (int)v1.x, (int)v1.y, color);
}

void DrawPoly(Vector2 center, int sides, float radius, float rotationDeg, Color color) {
    sides = std::max(3, sides);
    float rot = rotationDeg * 3.14159265f / 180.0f;
    for (int i = 0; i < sides; i++) {
        float a0 = rot + (float)i / sides * 2.0f * 3.14159265f;
        float a1 = rot + (float)(i + 1) / sides * 2.0f * 3.14159265f;
        Vector2 v1 = center;
        Vector2 v2 = { center.x + std::cos(a0) * radius, center.y + std::sin(a0) * radius };
        Vector2 v3 = { center.x + std::cos(a1) * radius, center.y + std::sin(a1) * radius };
        DrawTriangle(v1, v2, v3, color);
    }
}

void DrawPolyLines(Vector2 center, int sides, float radius, float rotationDeg, Color color) {
    sides = std::max(3, sides);
    float rot = rotationDeg * 3.14159265f / 180.0f;
    Vector2 prev = { center.x + std::cos(rot) * radius, center.y + std::sin(rot) * radius };
    for (int i = 1; i <= sides; i++) {
        float a = rot + (float)i / sides * 2.0f * 3.14159265f;
        Vector2 cur = { center.x + std::cos(a) * radius, center.y + std::sin(a) * radius };
        DrawLine((int)prev.x, (int)prev.y, (int)cur.x, (int)cur.y, color);
        prev = cur;
    }
}

// ---- Texto (fonte bitmap 8x8 embutida) ------------------------------------------------

int MeasureText(const std::string& text, int fontSize) {
    float scale = (float)fontSize / (float)internal::FONT8x8_GLYPH_SIZE;
    return (int)std::round(text.size() * internal::FONT8x8_GLYPH_SIZE * scale);
}

void DrawText(const std::string& text, int posX, int posY, int fontSize, Color color) {
    if (fontSize <= 0) return;
    float scale = (float)fontSize / (float)internal::FONT8x8_GLYPH_SIZE;
    int glyphSize = internal::FONT8x8_GLYPH_SIZE;

    SDL_SetRenderDrawColor(S.renderer, color.r, color.g, color.b, color.a);

    int cursorX = posX;
    for (unsigned char ch : text) {
        if (ch == '\n') { posY += fontSize + 2; cursorX = posX; continue; }
        int idx = (int)ch - internal::FONT8x8_FIRST_CHAR;
        if (idx < 0 || idx >= (internal::FONT8x8_LAST_CHAR - internal::FONT8x8_FIRST_CHAR + 1)) {
            cursorX += fontSize; // caractere fora do intervalo suportado: apenas avança
            continue;
        }
        const unsigned char* glyph = internal::FONT8x8[idx];
        for (int row = 0; row < glyphSize; row++) {
            unsigned char bits = glyph[row];
            for (int col = 0; col < glyphSize; col++) {
                if (bits & (1 << (7 - col))) {
                    float px = cursorX + col * scale;
                    float py = (float)posY + row * scale;
                    Vector2 sp = WorldToScreen({ px, py });
                    float pixelScale = std::max(1.0f, scale * WorldToScreenScale());
                    SDL_FRect r{ sp.x, sp.y, pixelScale, pixelScale };
                    SDL_RenderFillRectF(S.renderer, &r);
                }
            }
        }
        cursorX += (int)std::round(glyphSize * scale);
    }
}

void DrawFPS(int posX, int posY) {
    std::string text = "FPS: " + std::to_string(GetFPS());
    Color color = (S.currentFPS >= 50) ? LIME : (S.currentFPS >= 25 ? ORANGE : RED);
    DrawText(text, posX, posY, 20, color);
}

// ---- Texturas ---------------------------------------------------------------------------

Texture2D LoadTexture(const std::string& filePath) {
    Texture2D tex;
    SDL_Surface* surf = IMG_Load(filePath.c_str());
    if (!surf) {
        std::fprintf(stderr, "Engine2D: falha ao carregar textura '%s': %s\n", filePath.c_str(), IMG_GetError());
        return tex;
    }
    SDL_Texture* sdlTex = SDL_CreateTextureFromSurface(S.renderer, surf);
    tex.width  = surf->w;
    tex.height = surf->h;
    SDL_FreeSurface(surf);
    if (!sdlTex) {
        std::fprintf(stderr, "Engine2D: falha ao criar SDL_Texture: %s\n", SDL_GetError());
        return tex;
    }
    SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
    tex.handle = sdlTex;
    return tex;
}

Texture2D LoadTextureFromImage(const Image& image) {
    Texture2D tex;
    if (image.width <= 0 || image.height <= 0) return tex;

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        (void*)image.pixels.data(), image.width, image.height, 32, image.width * 4,
        SDL_PIXELFORMAT_RGBA32
    );
    if (!surf) {
        std::fprintf(stderr, "Engine2D: falha ao criar surface a partir da imagem: %s\n", SDL_GetError());
        return tex;
    }
    SDL_Texture* sdlTex = SDL_CreateTextureFromSurface(S.renderer, surf);
    SDL_FreeSurface(surf);
    if (!sdlTex) {
        std::fprintf(stderr, "Engine2D: falha ao criar SDL_Texture a partir da imagem: %s\n", SDL_GetError());
        return tex;
    }
    SDL_SetTextureBlendMode(sdlTex, SDL_BLENDMODE_BLEND);
    tex.handle = sdlTex;
    tex.width  = image.width;
    tex.height = image.height;
    return tex;
}

void UnloadTexture(Texture2D texture) {
    if (texture.handle) SDL_DestroyTexture(reinterpret_cast<SDL_Texture*>(texture.handle));
}

void DrawTexture(Texture2D texture, int posX, int posY, Color tint) {
    DrawTextureEx(texture, { (float)posX, (float)posY }, 0.0f, 1.0f, tint);
}

void DrawTextureV(Texture2D texture, Vector2 position, Color tint) {
    DrawTextureEx(texture, position, 0.0f, 1.0f, tint);
}

void DrawTextureEx(Texture2D texture, Vector2 position, float rotationDeg, float scale, Color tint) {
    if (!texture.IsValid()) return;
    Rectangle source{ 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest{ position.x, position.y, texture.width * scale, texture.height * scale };
    DrawTexturePro(texture, source, dest, { 0, 0 }, rotationDeg, tint);
}

void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint) {
    Rectangle dest{ position.x, position.y, source.width, source.height };
    DrawTexturePro(texture, source, dest, { 0, 0 }, 0.0f, tint);
}

void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin,
                     float rotationDeg, Color tint) {
    if (!texture.IsValid()) return;
    SDL_Texture* sdlTex = reinterpret_cast<SDL_Texture*>(texture.handle);
    SDL_SetTextureColorMod(sdlTex, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(sdlTex, tint.a);

    SDL_Rect srcRect{ (int)source.x, (int)source.y, (int)source.width, (int)source.height };

    Vector2 screenPos = WorldToScreen({ dest.x, dest.y });
    float scale = WorldToScreenScale();

    SDL_FRect dstRect{
        screenPos.x, screenPos.y,
        dest.width * scale, dest.height * scale
    };
    SDL_FPoint center{ origin.x * scale, origin.y * scale };

    double totalRotation = rotationDeg + (S.cameraActive ? S.activeCamera.rotation : 0.0f);

    SDL_RenderCopyExF(S.renderer, sdlTex, &srcRect, &dstRect, totalRotation, &center, SDL_FLIP_NONE);
}

// ---- Imagens (CPU) ----------------------------------------------------------------------

Image GenImageColor(int width, int height, Color color) {
    Image img;
    img.width = width;
    img.height = height;
    img.pixels.resize((size_t)width * height * 4);
    for (int i = 0; i < width * height; i++) {
        img.pixels[i * 4 + 0] = color.r;
        img.pixels[i * 4 + 1] = color.g;
        img.pixels[i * 4 + 2] = color.b;
        img.pixels[i * 4 + 3] = color.a;
    }
    return img;
}

void ImageDrawPixel(Image& image, int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= image.width || y >= image.height) return;
    size_t i = ((size_t)y * image.width + x) * 4;
    image.pixels[i + 0] = color.r;
    image.pixels[i + 1] = color.g;
    image.pixels[i + 2] = color.b;
    image.pixels[i + 3] = color.a;
}

void UnloadImage(Image& image) {
    image.pixels.clear();
    image.width = 0;
    image.height = 0;
}

// =====================================================================================
//  ENTRADA: TECLADO
// =====================================================================================

static inline int KeyIndex(KeyboardKey key) { return static_cast<int>(key); }

bool IsKeyDown(KeyboardKey key) {
    int k = KeyIndex(key);
    return (k >= 0 && k < State::MAX_KEYS) ? S.keyCurrent[k] : false;
}

bool IsKeyUp(KeyboardKey key) { return !IsKeyDown(key); }

bool IsKeyPressed(KeyboardKey key) {
    int k = KeyIndex(key);
    if (k < 0 || k >= State::MAX_KEYS) return false;
    return S.keyCurrent[k] && !S.keyPrevious[k];
}

bool IsKeyReleased(KeyboardKey key) {
    int k = KeyIndex(key);
    if (k < 0 || k >= State::MAX_KEYS) return false;
    return !S.keyCurrent[k] && S.keyPrevious[k];
}

// =====================================================================================
//  ENTRADA: MOUSE
// =====================================================================================

int GetMouseX() { return S.mouseX; }
int GetMouseY() { return S.mouseY; }
Vector2 GetMousePosition() { return { (float)S.mouseX, (float)S.mouseY }; }
float GetMouseWheelMove() { return S.mouseWheel; }

static inline int MouseIndex(MouseButton b) { return static_cast<int>(b); }

bool IsMouseButtonDown(MouseButton button) {
    int b = MouseIndex(button);
    return (b >= 0 && b < State::MAX_MOUSE_BUTTONS) ? S.mouseCurrent[b] : false;
}
bool IsMouseButtonUp(MouseButton button) { return !IsMouseButtonDown(button); }

bool IsMouseButtonPressed(MouseButton button) {
    int b = MouseIndex(button);
    if (b < 0 || b >= State::MAX_MOUSE_BUTTONS) return false;
    return S.mouseCurrent[b] && !S.mousePrevious[b];
}

bool IsMouseButtonReleased(MouseButton button) {
    int b = MouseIndex(button);
    if (b < 0 || b >= State::MAX_MOUSE_BUTTONS) return false;
    return !S.mouseCurrent[b] && S.mousePrevious[b];
}

// =====================================================================================
//  ÁUDIO
// =====================================================================================

bool InitAudioDevice() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        std::fprintf(stderr, "Engine2D: falha ao iniciar áudio: %s\n", Mix_GetError());
        S.audioReady = false;
        return false;
    }
    Mix_AllocateChannels(32);
    S.audioReady = true;
    return true;
}

void CloseAudioDevice() {
    if (S.audioReady) {
        Mix_CloseAudio();
        S.audioReady = false;
    }
}

bool IsAudioDeviceReady() { return S.audioReady; }

Sound LoadSound(const std::string& filePath) {
    Sound snd;
    if (!S.audioReady) {
        std::fprintf(stderr, "Engine2D: InitAudioDevice() não foi chamado antes de LoadSound.\n");
        return snd;
    }
    Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
    if (!chunk) {
        std::fprintf(stderr, "Engine2D: falha ao carregar som '%s': %s\n", filePath.c_str(), Mix_GetError());
        return snd;
    }
    SoundData* data = new SoundData();
    data->chunk = chunk;
    snd.handle = data;
    return snd;
}

void UnloadSound(Sound sound) {
    if (!sound.handle) return;
    SoundData* data = reinterpret_cast<SoundData*>(sound.handle);
    if (data->chunk) Mix_FreeChunk(data->chunk);
    delete data;
}

void PlaySound(Sound sound) {
    if (!sound.handle) return;
    SoundData* data = reinterpret_cast<SoundData*>(sound.handle);
    data->channel = Mix_PlayChannel(-1, data->chunk, 0);
}

void StopSound(Sound sound) {
    if (!sound.handle) return;
    SoundData* data = reinterpret_cast<SoundData*>(sound.handle);
    if (data->channel >= 0) Mix_HaltChannel(data->channel);
}

void SetSoundVolume(Sound sound, float volume) {
    if (!sound.handle) return;
    SoundData* data = reinterpret_cast<SoundData*>(sound.handle);
    int vol = (int)(std::clamp(volume, 0.0f, 1.0f) * MIX_MAX_VOLUME);
    Mix_VolumeChunk(data->chunk, vol);
}

// =====================================================================================
//  COLISÕES / MATEMÁTICA
// =====================================================================================

bool CheckCollisionRecs(Rectangle a, Rectangle b) {
    return (a.x < b.x + b.width) && (a.x + a.width > b.x) &&
           (a.y < b.y + b.height) && (a.y + a.height > b.y);
}

bool CheckCollisionCircles(Vector2 c1, float r1, Vector2 c2, float r2) {
    float dx = c2.x - c1.x, dy = c2.y - c1.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = r1 + r2;
    return distSq <= (radiusSum * radiusSum);
}

bool CheckCollisionPointRec(Vector2 point, Rectangle rec) {
    return point.x >= rec.x && point.x <= rec.x + rec.width &&
           point.y >= rec.y && point.y <= rec.y + rec.height;
}

bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) {
    float closestX = std::clamp(center.x, rec.x, rec.x + rec.width);
    float closestY = std::clamp(center.y, rec.y, rec.y + rec.height);
    float dx = center.x - closestX;
    float dy = center.y - closestY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

float Vector2Distance(Vector2 v1, Vector2 v2) {
    float dx = v2.x - v1.x, dy = v2.y - v1.y;
    return std::sqrt(dx * dx + dy * dy);
}

float Vector2Length(Vector2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }

Vector2 Vector2Normalize(Vector2 v) {
    float len = Vector2Length(v);
    if (len < 0.00001f) return { 0.0f, 0.0f };
    return { v.x / len, v.y / len };
}

float Vector2Angle(Vector2 v1, Vector2 v2) {
    return std::atan2(v2.y - v1.y, v2.x - v1.x);
}

Color Fade(Color color, float alpha) {
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    color.a = static_cast<unsigned char>(255.0f * alpha);
    return color;
}

// =====================================================================================
//  ALEATORIEDADE
// =====================================================================================

void SetRandomSeed(unsigned int seed) {
    std::srand(seed);
    S.randomSeeded = true;
}

int GetRandomValue(int min, int max) {
    if (!S.randomSeeded) {
        std::srand((unsigned int)std::time(nullptr));
        S.randomSeeded = true;
    }
    if (min > max) std::swap(min, max);
    return min + (std::rand() % (max - min + 1));
}

} // namespace Engine2D
