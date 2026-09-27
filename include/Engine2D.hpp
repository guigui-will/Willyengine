// =====================================================================================
//  Engine2D.hpp  -  Motor de jogos 2D estilo raylib, escrito em C++ puro sobre SDL2
// =====================================================================================
//
//  Objetivo: oferecer uma API simples, direta e "modo imediato" (immediate mode),
//  muito parecida com a raylib, para criar jogos 2D em C++.
//
//  Como usar (exemplo mínimo):
//
//      #include "Engine2D.hpp"
//      using namespace Engine2D;
//
//      int main() {
//          InitWindow(800, 450, "Minha Janela");
//          SetTargetFPS(60);
//
//          while (!WindowShouldClose()) {
//              BeginDrawing();
//                  ClearBackground(RAYWHITE);
//                  DrawText("Ola, mundo!", 190, 200, 20, DARKGRAY);
//              EndDrawing();
//          }
//
//          CloseWindow();
//          return 0;
//      }
//
//  Dependências: apenas SDL2 (biblioteca amplamente disponível em Windows/Linux/macOS).
//  Não é necessário SDL_image, SDL_ttf ou SDL_mixer: texturas (.bmp) e texto (fonte
//  bitmap embutida) são resolvidos internamente para minimizar dependências externas.
//
// =====================================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace Engine2D {

// =====================================================================================
//  TIPOS BÁSICOS
// =====================================================================================

struct Color {
    unsigned char r, g, b, a;
};

struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    Vector2() = default;
    Vector2(float x_, float y_) : x(x_), y(y_) {}

    Vector2 operator+(const Vector2& o) const { return { x + o.x, y + o.y }; }
    Vector2 operator-(const Vector2& o) const { return { x - o.x, y - o.y }; }
    Vector2 operator*(float s)         const { return { x * s,   y * s }; }
};

struct Rectangle {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// Câmera 2D no estilo raylib: aplica offset, rotação e zoom em torno de um alvo.
struct Camera2D {
    Vector2 offset   = { 0.0f, 0.0f }; // deslocamento na tela (geralmente o centro da janela)
    Vector2 target   = { 0.0f, 0.0f }; // ponto do "mundo" que fica sob o offset
    float   rotation = 0.0f;           // graus
    float   zoom     = 1.0f;           // 1.0f = sem zoom
};

// Textura carregada em VRAM (wrapper opaco em torno de SDL_Texture).
struct Texture2D {
    void*   handle = nullptr; // SDL_Texture*
    int     width  = 0;
    int     height = 0;
    bool IsValid() const { return handle != nullptr; }
};

// Imagem em memória de CPU (pixels RGBA8, antes de virar Texture2D).
struct Image {
    std::vector<unsigned char> pixels; // RGBA8, width*height*4 bytes
    int width  = 0;
    int height = 0;
};

// Som curto carregado totalmente em memória (formato WAV via SDL_AudioLoadWAV).
struct Sound {
    void*    handle = nullptr; // Engine2D::internal::SoundData*
    bool IsValid() const { return handle != nullptr; }
};

enum class KeyboardKey : int; // definido em Keys.hpp (incluído abaixo)
enum class MouseButton : int {
    LEFT   = 0,
    RIGHT  = 1,
    MIDDLE = 2,
};

// =====================================================================================
//  CORES PRÉ-DEFINIDAS (mesma paleta popularizada pela raylib)
// =====================================================================================

inline constexpr Color LIGHTGRAY  = { 200, 200, 200, 255 };
inline constexpr Color GRAY       = { 130, 130, 130, 255 };
inline constexpr Color DARKGRAY   = { 80,  80,  80,  255 };
inline constexpr Color YELLOW     = { 253, 249, 0,   255 };
inline constexpr Color GOLD       = { 255, 203, 0,   255 };
inline constexpr Color ORANGE     = { 255, 161, 0,   255 };
inline constexpr Color PINK       = { 255, 109, 194, 255 };
inline constexpr Color RED        = { 230, 41,  55,  255 };
inline constexpr Color MAROON     = { 190, 33,  55,  255 };
inline constexpr Color GREEN      = { 0,   228, 48,  255 };
inline constexpr Color LIME       = { 0,   158, 47,  255 };
inline constexpr Color DARKGREEN  = { 0,   117, 44,  255 };
inline constexpr Color SKYBLUE    = { 102, 191, 255, 255 };
inline constexpr Color BLUE       = { 0,   121, 241, 255 };
inline constexpr Color DARKBLUE   = { 0,   82,  172, 255 };
inline constexpr Color PURPLE     = { 200, 122, 255, 255 };
inline constexpr Color VIOLET     = { 135, 60,  190, 255 };
inline constexpr Color DARKPURPLE = { 112, 31,  126, 255 };
inline constexpr Color BEIGE      = { 211, 176, 131, 255 };
inline constexpr Color BROWN      = { 127, 106, 79,  255 };
inline constexpr Color DARKBROWN  = { 76,  63,  47,  255 };
inline constexpr Color WHITE      = { 255, 255, 255, 255 };
inline constexpr Color BLACK      = { 0,   0,   0,   255 };
inline constexpr Color BLANK      = { 0,   0,   0,   0   };
inline constexpr Color MAGENTA    = { 255, 0,   255, 255 };
inline constexpr Color RAYWHITE   = { 245, 245, 245, 255 };

// =====================================================================================
//  JANELA / CICLO DE VIDA
// =====================================================================================

// Cria a janela e o contexto de renderização. Deve ser a primeira chamada da engine.
bool InitWindow(int width, int height, const std::string& title);

// Fecha a janela e libera todos os recursos internos (texturas, sons, etc. que
// ainda não tenham sido descarregados manualmente).
void CloseWindow();

// Retorna true quando o usuário pediu para fechar a janela (botão "X", Alt+F4, ESC
// se configurado) ou quando SetWindowShouldClose(true) foi chamado manualmente.
bool WindowShouldClose();

// Redimensiona/renomeia a janela em tempo de execução.
void SetWindowSize(int width, int height);
void SetWindowTitle(const std::string& title);
int  GetScreenWidth();
int  GetScreenHeight();

// Alterna modo tela cheia.
void ToggleFullscreen();
bool IsWindowFullscreen();

// Força o encerramento do laço principal na próxima verificação de WindowShouldClose().
void SetWindowShouldClose(bool value);

// =====================================================================================
//  TEMPO / FPS
// =====================================================================================

void   SetTargetFPS(int fps);     // 0 = sem limite
int    GetFPS();                  // FPS instantâneo (médio nos últimos frames)
float  GetFrameTime();            // Delta time do último frame, em segundos
double GetTime();                 // Segundos desde InitWindow()

// =====================================================================================
//  DESENHO (MODO IMEDIATO)
// =====================================================================================

// Prepara o backbuffer para receber comandos de desenho. Deve ser chamado uma vez
// por frame, antes de qualquer Draw*().
void BeginDrawing();

// Envia o backbuffer para a tela (equivalente a um "SDL_RenderPresent") e também
// calcula o delta time / limita o FPS caso SetTargetFPS tenha sido usado.
void EndDrawing();

// Limpa toda a tela com uma cor sólida. Deve ser a primeira chamada dentro de
// BeginDrawing()/EndDrawing().
void ClearBackground(Color color);

// Ativa um sistema de coordenadas 2D com câmera (permite rolagem, zoom e rotação
// de cena). Tudo desenhado entre Begin/EndMode2D é afetado pela câmera.
void BeginMode2D(const Camera2D& camera);
void EndMode2D();

// ---- Formas geométricas -------------------------------------------------------------

void DrawPixel(int posX, int posY, Color color);
void DrawLine(int startX, int startY, int endX, int endY, Color color);
void DrawLineEx(Vector2 start, Vector2 end, float thickness, Color color);

void DrawCircle(int centerX, int centerY, float radius, Color color);
void DrawCircleLines(int centerX, int centerY, float radius, Color color);
void DrawCircleV(Vector2 center, float radius, Color color);

void DrawRectangle(int posX, int posY, int width, int height, Color color);
void DrawRectangleV(Vector2 position, Vector2 size, Color color);
void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
// Retângulo com cantos arredondados (roundness de 0.0 a 1.0).
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);

void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);

// Polígono regular (ex.: DrawPoly(centro, 6, raio, 0, cor) desenha um hexágono).
void DrawPoly(Vector2 center, int sides, float radius, float rotationDeg, Color color);
void DrawPolyLines(Vector2 center, int sides, float radius, float rotationDeg, Color color);

// ---- Texto ---------------------------------------------------------------------------
// A engine possui uma fonte bitmap 8x8 embutida (sem dependências externas).
// fontSize é a altura desejada em pixels; a fonte é escalada proporcionalmente.

void DrawText(const std::string& text, int posX, int posY, int fontSize, Color color);
int  MeasureText(const std::string& text, int fontSize); // largura em pixels
void DrawFPS(int posX, int posY); // atalho para exibir o contador de FPS atual

// ---- Texturas --------------------------------------------------------------------------

Texture2D LoadTexture(const std::string& filePath);      // carrega um .bmp do disco
Texture2D LoadTextureFromImage(const Image& image);       // envia pixels da CPU para a GPU
void      UnloadTexture(Texture2D texture);

void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
void DrawTextureEx(Texture2D texture, Vector2 position, float rotationDeg, float scale, Color tint);
// Desenha uma sub-região "source" da textura dentro do retângulo "dest", rotacionando
// em torno de "origin" (relativo a dest). Equivalente ao DrawTexturePro da raylib.
void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin,
                     float rotationDeg, Color tint);

// ---- Imagens (CPU) ---------------------------------------------------------------------

Image GenImageColor(int width, int height, Color color);
void  ImageDrawPixel(Image& image, int x, int y, Color color);
void  UnloadImage(Image& image);

// =====================================================================================
//  ENTRADA: TECLADO
// =====================================================================================

bool IsKeyPressed(KeyboardKey key);   // true apenas no frame em que foi pressionada
bool IsKeyDown(KeyboardKey key);      // true enquanto estiver segurada
bool IsKeyReleased(KeyboardKey key);  // true apenas no frame em que foi solta
bool IsKeyUp(KeyboardKey key);        // true enquanto não estiver pressionada

// =====================================================================================
//  ENTRADA: MOUSE
// =====================================================================================

int     GetMouseX();
int     GetMouseY();
Vector2 GetMousePosition();
float   GetMouseWheelMove();

bool IsMouseButtonPressed(MouseButton button);
bool IsMouseButtonDown(MouseButton button);
bool IsMouseButtonReleased(MouseButton button);
bool IsMouseButtonUp(MouseButton button);

// =====================================================================================
//  ÁUDIO
// =====================================================================================

bool InitAudioDevice();
void CloseAudioDevice();
bool IsAudioDeviceReady();

Sound LoadSound(const std::string& filePath); // carrega um .wav
void  UnloadSound(Sound sound);
void  PlaySound(Sound sound);
void  StopSound(Sound sound);
void  SetSoundVolume(Sound sound, float volume); // 0.0f .. 1.0f

// =====================================================================================
//  COLISÕES / MATEMÁTICA AUXILIAR
// =====================================================================================

bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);

float Vector2Distance(Vector2 v1, Vector2 v2);
float Vector2Length(Vector2 v);
Vector2 Vector2Normalize(Vector2 v);
float Vector2Angle(Vector2 v1, Vector2 v2); // em radianos

Color Fade(Color color, float alpha); // alpha de 0.0f a 1.0f

// =====================================================================================
//  ALEATORIEDADE
// =====================================================================================

void SetRandomSeed(unsigned int seed);
int  GetRandomValue(int min, int max); // inclusive nos dois extremos

} // namespace Engine2D

#include "Keys.hpp"
