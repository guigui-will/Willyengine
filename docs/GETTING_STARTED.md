# Primeiros passos

Este guia parte do zero: instalar dependências, compilar e criar seu primeiro
programa com a Engine2D.

## 1. Instale as dependências

Veja a seção *Dependências* do [README](../README.md) para o seu sistema
operacional (Ubuntu/Debian, macOS ou Windows).

## 2. Compile a engine e os exemplos

```bash
git clone <url-do-seu-repositorio>
cd Engine2D
mkdir build && cd build
cmake ..
make -j
```

Isso vai gerar a biblioteca `libEngine2D.a` e três executáveis de exemplo:

- `example_basic` — formas geométricas, texto e contador de FPS
- `example_input` — mover um retângulo com teclado, clique do mouse e colisão
- `example_texture_camera` — textura procedural, `Camera2D` (zoom/scroll) e áudio

Rode qualquer um deles a partir da pasta `build`:
```bash
./example_basic
```

## 3. Crie seu próprio jogo

Crie um arquivo `meu_jogo.cpp` na raiz do projeto:

```cpp
#include "Engine2D.hpp"
using namespace Engine2D;

int main() {
    InitWindow(800, 450, "Meu Jogo");
    SetTargetFPS(60);

    Vector2 playerPos = { 400.0f, 225.0f };
    const float speed = 200.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KeyboardKey::KEY_RIGHT)) playerPos.x += speed * dt;
        if (IsKeyDown(KeyboardKey::KEY_LEFT))  playerPos.x -= speed * dt;
        if (IsKeyDown(KeyboardKey::KEY_DOWN))  playerPos.y += speed * dt;
        if (IsKeyDown(KeyboardKey::KEY_UP))    playerPos.y -= speed * dt;

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawCircleV(playerPos, 25.0f, BLUE);
            DrawText("Use as setas para mover o circulo", 10, 10, 18, DARKGRAY);
            DrawFPS(10, 420);
        EndDrawing();

        if (IsKeyPressed(KeyboardKey::KEY_ESCAPE)) SetWindowShouldClose(true);
    }

    CloseWindow();
    return 0;
}
```

E adicione ao `CMakeLists.txt` (logo após `add_library(Engine2D ...)`):

```cmake
add_executable(meu_jogo meu_jogo.cpp)
target_link_libraries(meu_jogo PRIVATE Engine2D)
```

Recompile (`cmake .. && make -j`) e rode `./meu_jogo`.

## 4. Próximos passos

- Leia a [referência completa da API](API.md) para conhecer todas as funções disponíveis.
- Estude os três exemplos em `examples/` — eles cobrem os casos de uso mais comuns.
- Quer sons? Chame `InitAudioDevice()` uma vez no início e use `LoadSound` / `PlaySound`.
- Quer rolagem de câmera (scroll, zoom, mundo maior que a tela)? Veja `Camera2D`
  no `example_texture_camera.cpp` e na referência da API.
