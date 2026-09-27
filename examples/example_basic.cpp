// Exemplo básico: janela + formas geométricas + texto + contador de FPS.
#include "Engine2D.hpp"

using namespace Engine2D;

int main() {
    InitWindow(800, 450, "Engine2D - Exemplo Basico");
    SetTargetFPS(60);

    float angle = 0.0f;

    while (!WindowShouldClose()) {
        angle += GetFrameTime() * 90.0f; // 90 graus por segundo

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("Engine2D - motor 2D estilo raylib", 20, 20, 18, DARKGRAY);
            DrawText("Pressione ESC para sair", 20, 46, 16, GRAY);

            DrawRectangle(60, 120, 120, 80, RED);
            DrawRectangleLines(60, 120, 120, 80, MAROON);

            DrawCircle(300, 160, 40, BLUE);
            DrawCircleLines(300, 160, 50, DARKBLUE);

            DrawTriangle({ 420.0f, 200.0f }, { 460.0f, 120.0f }, { 500.0f, 200.0f }, GREEN);

            DrawPoly({ 620.0f, 160.0f }, 6, 45.0f, angle, ORANGE);
            DrawPolyLines({ 620.0f, 160.0f }, 6, 55.0f, angle, GOLD);

            DrawRectangleRounded({ 60.0f, 260.0f, 200.0f, 90.0f }, 0.4f, 12, PURPLE);

            DrawFPS(690, 410);
        EndDrawing();

        if (IsKeyPressed(KeyboardKey::KEY_ESCAPE)) SetWindowShouldClose(true);
    }

    CloseWindow();
    return 0;
}
