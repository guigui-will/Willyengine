// Exemplo de: (1) textura gerada em memória (checkerboard), (2) Camera2D com
// rolagem/zoom/rotação e (3) áudio (toca um som se um arquivo "click.wav" existir
// ao lado do executável - caso não exista, o exemplo continua normalmente).
#include "Engine2D.hpp"
#include <cmath>

using namespace Engine2D;

static Image GenCheckerboard(int size, int cell, Color a, Color b) {
    Image img = GenImageColor(size, size, a);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            bool alt = ((x / cell) + (y / cell)) % 2 == 0;
            ImageDrawPixel(img, x, y, alt ? a : b);
        }
    }
    return img;
}

int main() {
    InitWindow(800, 450, "Engine2D - Textura + Camera2D + Audio");
    SetTargetFPS(60);

    Image checkerImg = GenCheckerboard(128, 16, LIGHTGRAY, DARKGRAY);
    Texture2D checker = LoadTextureFromImage(checkerImg);
    UnloadImage(checkerImg);

    InitAudioDevice();
    Sound click = LoadSound("click.wav"); // opcional: pode falhar silenciosamente

    Camera2D camera;
    camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.target = { 0.0f, 0.0f };
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KeyboardKey::KEY_RIGHT)) camera.target.x += 200.0f * dt;
        if (IsKeyDown(KeyboardKey::KEY_LEFT))  camera.target.x -= 200.0f * dt;
        if (IsKeyDown(KeyboardKey::KEY_UP))    camera.target.y -= 200.0f * dt;
        if (IsKeyDown(KeyboardKey::KEY_DOWN))  camera.target.y += 200.0f * dt;

        camera.zoom += GetMouseWheelMove() * 0.1f;
        if (camera.zoom < 0.2f) camera.zoom = 0.2f;
        if (camera.zoom > 3.0f) camera.zoom = 3.0f;

        if (IsKeyPressed(KeyboardKey::KEY_SPACE)) {
            if (click.IsValid()) PlaySound(click);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
                DrawTexture(checker, -64, -64, WHITE);
                DrawRectangleLines(-64, -64, 128, 128, RED);
                DrawCircle(0, 0, 6, RED); // origem do mundo
            EndMode2D();

            DrawText("Setas: move a camera | Roda do mouse: zoom | ESPACO: som", 10, 10, 16, DARKGRAY);
            DrawFPS(700, 10);
        EndDrawing();

        if (IsKeyPressed(KeyboardKey::KEY_ESCAPE)) SetWindowShouldClose(true);
    }

    UnloadSound(click);
    CloseAudioDevice();
    UnloadTexture(checker);
    CloseWindow();
    return 0;
}
