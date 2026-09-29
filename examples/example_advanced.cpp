// Exemplo dos recursos "avançados": sistema de partículas, render-to-texture,
// flip de textura, texto acentuado com quebra automática de linha, e gamepad
// (se houver um conectado). Música/áudio ficam comentados pois dependem de um
// arquivo externo — veja as instruções no código abaixo.
#include "Engine2D.hpp"
#include <cmath>

using namespace Engine2D;

static Image GenCheckerboard(int size, int cell, Color a, Color b) {
    Image img = GenImageColor(size, size, a);
    for (int y = 0; y < size; y++)
        for (int x = 0; x < size; x++)
            ImageDrawPixel(img, x, y, (((x / cell) + (y / cell)) % 2 == 0) ? a : b);
    return img;
}

int main() {
    InitWindow(900, 520, "Engine2D - Particulas, RenderTexture, Flip, Acentos e Gamepad");
    SetTargetFPS(60);

    // ---- Textura de exemplo (para demonstrar o flip) ----
    Image checkerImg = GenCheckerboard(64, 8, ORANGE, MAROON);
    Texture2D checker = LoadTextureFromImage(checkerImg);
    UnloadImage(checkerImg);

    // ---- Emissor de partículas (estilo "fogueira") ----
    ParticleEmitter fire = CreateParticleEmitter({ 150.0f, 420.0f }, 300);
    fire.emissionRate = 80.0f;
    fire.particleLife = 1.2f;
    fire.velocityMin  = { -20.0f, -140.0f };
    fire.velocityMax  = {  20.0f,  -60.0f };
    fire.startSize    = 10.0f;
    fire.endSize      = 0.0f;
    fire.startColor   = { 255, 220, 60, 255 };
    fire.endColor     = { 200, 30,  0,   0 };

    // ---- Render texture: desenhamos um pequeno "selo" uma vez e reutilizamos ----
    RenderTexture2D stamp = LoadRenderTexture(80, 80);
    BeginTextureMode(stamp);
        ClearBackground(BLANK);
        DrawCircle(40, 40, 38, SKYBLUE);
        DrawCircleLines(40, 40, 38, DARKBLUE);
        DrawText("2D", 24, 28, 24, WHITE);
    EndTextureMode();

    const std::string paragrafo =
        "Este texto usa acentuação (ção, não, é, ã) e quebra automaticamente "
        "de linha dentro da caixa, graças a WrapText/DrawTextBoxed.";

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        UpdateParticleEmitter(fire, dt);

        // Move o emissor de partículas com o mouse, se estiver pressionado.
        if (IsMouseButtonDown(MouseButton::LEFT)) {
            fire.position = GetMousePosition();
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("Segure o botao esquerdo do mouse para mover a fogueira", 10, 10, 16, DARKGRAY);

            // Textura normal vs. espelhada (flip horizontal via source.width negativo)
            DrawTexture(checker, 650, 20, WHITE);
            DrawText("normal", 650, 90, 14, GRAY);
            {
                Rectangle src{ 0, 0, -(float)checker.width, (float)checker.height }; // flip H
                DrawTexturePro(checker, src, { 750.0f, 20.0f, (float)checker.width, (float)checker.height },
                               { 0, 0 }, 0.0f, WHITE);
            }
            DrawText("espelhada", 750, 90, 14, GRAY);

            // Render texture reutilizada (o "selo" foi desenhado só uma vez, fora do loop)
            DrawTexture(stamp.texture, 650, 130, WHITE);
            DrawTexture(stamp.texture, 740, 160, Fade(WHITE, 0.6f));

            // Texto longo com quebra automática dentro de uma caixa
            DrawTextBoxed(paragrafo, { 20.0f, 60.0f, 260.0f, 140.0f }, 16, DARKGRAY);

            // Partículas
            DrawParticleEmitter(fire);

            // Gamepad (mostra estado do primeiro controle, se houver)
            if (IsGamepadAvailable(0)) {
                DrawText("Gamepad conectado!", 20, 460, 16, DARKGREEN);
                float lx = GetGamepadAxisMovement(0, GamepadAxis::LEFT_X);
                float ly = GetGamepadAxisMovement(0, GamepadAxis::LEFT_Y);
                DrawCircle(500, 460, 30, LIGHTGRAY);
                DrawCircle((int)(500 + lx * 25), (int)(460 + ly * 25), 8, RED);
                if (IsGamepadButtonDown(0, GamepadButton::A)) DrawText("A", 560, 452, 20, GREEN);
            } else {
                DrawText("Nenhum gamepad conectado (opcional)", 20, 460, 16, GRAY);
            }

            DrawFPS(820, 490);
        EndDrawing();

        if (IsKeyPressed(KeyboardKey::KEY_ESCAPE)) SetWindowShouldClose(true);
    }

    UnloadRenderTexture(stamp);
    UnloadTexture(checker);
    CloseWindow();
    return 0;
}
