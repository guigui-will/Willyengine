# Engine2D

Uma engine 2D em C++ com API "modo imediato", inspirada na [raylib](https://www.raylib.com/),
construída sobre SDL2 (+ SDL2_image e SDL2_mixer). Texto é desenhado com uma fonte
bitmap 8x8 embutida no próprio binário — não é preciso nenhum arquivo de fonte externo.

📖 **[Guia de primeiros passos](docs/GETTING_STARTED.md)** · 📚 **[Referência completa da API](docs/API.md)**

## Recursos

- Janela, loop principal, controle de FPS (`InitWindow`, `WindowShouldClose`, `SetTargetFPS`, `GetFrameTime`)
- Formas geométricas: retângulo (com cantos arredondados), círculo, linha, triângulo, polígono regular
- Texto com fonte embutida, **incluindo acentos do português** (`DrawText`, `MeasureText`, `DrawFPS`)
- Quebra automática de linha (`WrapText`, `DrawTextBoxed`)
- Texturas: carregar PNG/JPG/BMP do disco ou gerar em memória (`LoadTexture`, `GenImageColor`, `DrawTexturePro`), com suporte a **espelhamento (flip)**
- **Render-to-texture** (`RenderTexture2D`, `BeginTextureMode`/`EndTextureMode`)
- **Sistema de partículas** (`ParticleEmitter`, `CreateParticleEmitter`, `UpdateParticleEmitter`, `EmitParticleBurst`)
- `Camera2D` com posição, zoom e rotação (`BeginMode2D` / `EndMode2D`)
- Teclado, mouse, **toque (touch)** e **gamepad/controle** (até 4 simultâneos)
- **Entrada de texto** para caixas de digitação (`StartTextInput`, `GetCharPressed`)
- Áudio: efeitos curtos via `Sound` (`.wav`) e **música em streaming** via `Music` (`.wav`/`.ogg`/`.mp3`)
- Colisões (AABB, círculo, ponto) e utilitários de `Vector2`
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
    ├── example_basic.cpp            # formas, texto, FPS
    ├── example_input.cpp            # teclado, mouse, colisão
    ├── example_texture_camera.cpp   # textura procedural, Camera2D, áudio
    └── example_advanced.cpp         # partículas, render-to-texture, flip, texto acentuado, gamepad
```

## Notas e limitações conhecidas

- Áudio de efeitos (`Sound`) suporta `.wav`; música (`Music`) suporta `.wav`/`.ogg`/`.mp3`
  dependendo dos decoders com que o SDL2_mixer da sua distro foi compilado. O SDL2_mixer
  só toca **uma música por vez** (limitação da própria biblioteca) — use vários `Sound`
  para efeitos simultâneos.
- A fonte embutida é uma bitmap 8x8 de baixa resolução (estilo retrô), cobrindo ASCII
  básico + acentos comuns do português; para texto suave em alta resolução ou outros
  idiomas, é possível estender a engine com SDL2_ttf.
- Sem sistema de cenas, ECS ou gerenciador de assets — a engine é a camada de
  desenho/input/áudio (como a própria raylib), você organiza o "jogo" em cima dela.
- Sem networking e sem sistema de jobs/threads embutido.
- Testado em Linux (Ubuntu 24.04) com GCC 13. Deve compilar em Windows/macOS sem mudanças,
  desde que as dependências acima estejam instaladas.

## Licença

MIT — veja [LICENSE](LICENSE). Informações sobre as bibliotecas e a fonte de
terceiros usadas estão em [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
