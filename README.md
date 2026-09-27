# Engine2D

Uma engine 2D em C++ com API "modo imediato", inspirada na [raylib](https://www.raylib.com/),
construída sobre SDL2 (+ SDL2_image e SDL2_mixer). Texto é desenhado com uma fonte
bitmap 8x8 embutida no próprio binário — não é preciso nenhum arquivo de fonte externo.

📖 **[Guia de primeiros passos](docs/GETTING_STARTED.md)** · 📚 **[Referência completa da API](docs/API.md)**

## Recursos

- Janela, loop principal, controle de FPS (`InitWindow`, `WindowShouldClose`, `SetTargetFPS`, `GetFrameTime`)
- Formas geométricas: retângulo (com cantos arredondados), círculo, linha, triângulo, polígono regular
- Texto com fonte embutida (`DrawText`, `MeasureText`, `DrawFPS`)
- Texturas: carregar PNG/JPG/BMP do disco ou gerar em memória (`LoadTexture`, `GenImageColor`, `DrawTexturePro`)
- `Camera2D` com posição, zoom e rotação (`BeginMode2D` / `EndMode2D`)
- Teclado e mouse (`IsKeyDown`, `IsKeyPressed`, `IsMouseButtonPressed`, `GetMousePosition`, ...)
- Áudio: carregar e tocar sons `.wav` (`InitAudioDevice`, `LoadSound`, `PlaySound`)
- Colisões (AABB, círculo-círculo, círculo-retângulo, ponto-retângulo) e utilitários de `Vector2`
- Números aleatórios (`GetRandomValue`)

## Dependências

Apenas bibliotecas C/C++ amplamente disponíveis:

- **SDL2** (obrigatório)
- **SDL2_image** (carregar texturas PNG/JPG além de BMP)
- **SDL2_mixer** (áudio)
- CMake ≥ 3.10 e um compilador C++17

### Instalar no Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake \
    libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

### Instalar no macOS (Homebrew)
```bash
brew install cmake sdl2 sdl2_image sdl2_mixer
```

### Instalar no Windows
A forma mais simples é usar [vcpkg](https://github.com/microsoft/vcpkg):
```bat
vcpkg install sdl2 sdl2-image sdl2-mixer
```
e depois configurar o CMake apontando para o toolchain do vcpkg:
```bat
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[caminho-do-vcpkg]/scripts/buildsystems/vcpkg.cmake
```

## Compilando

```bash
mkdir build && cd build
cmake ..
make -j
```

Isso gera:
- `libEngine2D.a` — a biblioteca estática da engine
- `example_basic`, `example_input`, `example_texture_camera` — três programas de exemplo

Para usar em outro projeto, basta:
```cmake
add_subdirectory(Engine2D)
target_link_libraries(meu_jogo PRIVATE Engine2D)
```

## Exemplo mínimo

```cpp
#include "Engine2D.hpp"
using namespace Engine2D;

int main() {
    InitWindow(800, 450, "Minha Janela");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Ola, mundo!", 190, 200, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

## Estrutura do projeto

```
Engine2D/
├── CMakeLists.txt
├── LICENSE
├── THIRD_PARTY_LICENSES.md
├── docs/
│   ├── GETTING_STARTED.md
│   └── API.md
├── include/
│   ├── Engine2D.hpp   # API pública
│   ├── Keys.hpp       # enum de teclas
│   └── Font8x8.hpp    # fonte bitmap embutida (gerada por gen_font.py)
├── src/
│   └── Engine2D.cpp   # implementação (backend SDL2)
└── examples/
    ├── example_basic.cpp           # formas, texto, FPS
    ├── example_input.cpp           # teclado, mouse, colisão
    └── example_texture_camera.cpp  # textura procedural, Camera2D, áudio
```

## Notas e limitações conhecidas

- Áudio suporta o formato `.wav` (via SDL2_mixer, que também consegue tocar OGG/MP3 se
  a distribuição SDL2_mixer tiver esses decoders habilitados).
- A fonte embutida é uma bitmap 8x8 de baixa resolução (estilo retrô); para texto mais
  refinado, é possível estender a engine com SDL2_ttf.
- `DrawTexturePro` não implementa espelhamento (flip) por enquanto; use `source.width`/`height`
  negativos como próxima extensão, se precisar.
- Testado em Linux (Ubuntu 24.04) com GCC 13. Deve compilar em Windows/macOS sem mudanças,
  desde que as dependências acima estejam instaladas.

## Licença

MIT — veja [LICENSE](LICENSE). Informações sobre as bibliotecas e a fonte de
terceiros usadas estão em [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
