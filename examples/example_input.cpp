// Exemplo de entrada: mover um retângulo com WASD/setas, detectar cliques do mouse
// e checar colisão entre o retângulo do jogador e um círculo "alvo".
#include "Engine2D.hpp"

using namespace Engine2D;

int main() {
    InitWindow(800, 450, "Engine2D - Exemplo de Entrada");
    SetTargetFPS(60);

    Rectangle player{ 375.0f, 200.0f, 50.0f, 50.0f };
    Vector2   target{ 600.0f, 120.0f };
    float     targetRadius = 30.0f;
    float     speed = 250.0f;

    int clicks = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KeyboardKey::KEY_RIGHT) || IsKeyDown(KeyboardKey::KEY_D)) player.x += speed * dt;
        if (IsKeyDown(KeyboardKey::KEY_LEFT)  || IsKeyDown(KeyboardKey::KEY_A)) player.x -= speed * dt;
        if (IsKeyDown(KeyboardKey::KEY_DOWN)  || IsKeyDown(KeyboardKey::KEY_S)) player.y += speed * dt;
        if (IsKeyDown(KeyboardKey::KEY_UP)    || IsKeyDown(KeyboardKey::KEY_W)) player.y -= speed * dt;

        if (IsMouseButtonPressed(MouseButton::LEFT)) clicks++;

        bool colliding = CheckCollisionCircleRec(target, targetRadius, player);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("WASD ou setas para mover o quadrado", 10, 10, 18, DARKGRAY);
            DrawText("Cliques do mouse: " + std::to_string(clicks), 10, 34, 18, DARKGRAY);

            DrawCircleV(target, targetRadius, colliding ? GREEN : SKYBLUE);
            DrawRectangleRec(player, colliding ? DARKGREEN : MAROON);

            DrawCircle(GetMouseX(), GetMouseY(), 4, BLACK);
            DrawFPS(700, 10);
        EndDrawing();

        if (IsKeyPressed(KeyboardKey::KEY_ESCAPE)) SetWindowShouldClose(true);
    }

    CloseWindow();
    return 0;
}
