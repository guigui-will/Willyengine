# Referência da API — Engine2D

> Todos os símbolos vivem dentro do namespace `Engine2D`. Os exemplos abaixo
> assumem `using namespace Engine2D;`.

Índice:

- [Tipos básicos](#tipos-básicos)
- [Cores pré-definidas](#cores-pré-definidas)
- [Janela e ciclo de vida](#janela-e-ciclo-de-vida)
- [Tempo / FPS](#tempo--fps)
- [Desenho](#desenho)
- [Texto](#texto)
- [Texturas e imagens](#texturas-e-imagens)
- [Câmera 2D](#câmera-2d)
- [Entrada: teclado](#entrada-teclado)
- [Entrada: mouse](#entrada-mouse)
- [Áudio](#áudio)
- [Colisões e matemática](#colisões-e-matemática)
- [Aleatoriedade](#aleatoriedade)

---

## Tipos básicos

### `Color`
```cpp
struct Color { unsigned char r, g, b, a; };
```
Componentes de 0 a 255.

### `Vector2`
```cpp
struct Vector2 {
    float x, y;
    Vector2 operator+(const Vector2&) const;
    Vector2 operator-(const Vector2&) const;
    Vector2 operator*(float) const;
};
```

### `Rectangle`
```cpp
struct Rectangle { float x, y, width, height; };
```

### `Camera2D`
```cpp
struct Camera2D {
    Vector2 offset;   // posição na tela que corresponde ao "target" no mundo
    Vector2 target;   // ponto do mundo focado pela câmera
    float   rotation; // graus
    float   zoom;     // 1.0 = sem zoom
};
```

### `Texture2D`
```cpp
struct Texture2D {
    void* handle; int width; int height;
    bool IsValid() const;
};
```
Wrapper opaco em torno de um `SDL_Texture*` (VRAM). Criado por `LoadTexture`
ou `LoadTextureFromImage`; deve ser liberado com `UnloadTexture`.

### `Image`
```cpp
struct Image { std::vector<unsigned char> pixels; int width; int height; };
```
Imagem em memória de CPU, pixels RGBA8 (4 bytes por pixel). Use para gerar
texturas proceduralmente antes de enviá-las à GPU com `LoadTextureFromImage`.

### `Sound`
```cpp
struct Sound { void* handle; bool IsValid() const; };
```
Som curto totalmente carregado em memória (arquivos `.wav`).

### `KeyboardKey` / `MouseButton`
Enums com os códigos de tecla (ver `include/Keys.hpp`) e botões do mouse
(`LEFT`, `RIGHT`, `MIDDLE`).

---

## Cores pré-definidas

`LIGHTGRAY, GRAY, DARKGRAY, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN,
LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE, PURPLE, VIOLET, DARKPURPLE, BEIGE,
BROWN, DARKBROWN, WHITE, BLACK, BLANK, MAGENTA, RAYWHITE`

---

## Janela e ciclo de vida

| Função | Descrição |
|---|---|
| `bool InitWindow(int width, int height, const std::string& title)` | Cria a janela e o renderer. Deve ser a primeira chamada. Retorna `false` em caso de falha. |
| `void CloseWindow()` | Fecha a janela e libera recursos internos (áudio, renderer, etc.). |
| `bool WindowShouldClose()` | Processa eventos do SO e retorna `true` quando a janela deve fechar (botão "X", `SetWindowShouldClose(true)`, etc.). Chame uma vez por frame, tipicamente na condição do `while`. |
| `void SetWindowShouldClose(bool value)` | Força o fim do laço principal na próxima verificação. |
| `void SetWindowSize(int width, int height)` | Redimensiona a janela em tempo de execução. |
| `void SetWindowTitle(const std::string& title)` | Altera o título da janela. |
| `int GetScreenWidth()` / `int GetScreenHeight()` | Dimensões atuais da janela. |
| `void ToggleFullscreen()` | Alterna entre janela e tela cheia (modo "desktop fullscreen"). |
| `bool IsWindowFullscreen()` | Estado atual do modo tela cheia. |

---

## Tempo / FPS

| Função | Descrição |
|---|---|
| `void SetTargetFPS(int fps)` | Limita o laço principal a `fps` quadros por segundo. `0` = sem limite (além do vsync). |
| `int GetFPS()` | FPS médio atual (atualizado a cada ~0.25s). |
| `float GetFrameTime()` | Delta time do último quadro, em segundos — use para animações independentes de FPS. |
| `double GetTime()` | Segundos decorridos desde `InitWindow()`. |

---

## Desenho

O padrão de uso é sempre:
```cpp
BeginDrawing();
    ClearBackground(RAYWHITE);
    // ... chamadas Draw*() ...
EndDrawing();
```

| Função | Descrição |
|---|---|
| `void BeginDrawing()` | Início do quadro. |
| `void EndDrawing()` | Apresenta o quadro na tela, calcula `GetFrameTime()`/`GetFPS()` e aplica o limite de `SetTargetFPS`. |
| `void ClearBackground(Color color)` | Limpa a tela inteira com uma cor sólida. |
| `void DrawPixel(int x, int y, Color color)` | Um pixel. |
| `void DrawLine(int x1, int y1, int x2, int y2, Color color)` | Linha de 1px. |
| `void DrawLineEx(Vector2 start, Vector2 end, float thickness, Color color)` | Linha com espessura customizável. |
| `void DrawCircle / DrawCircleV(..., float radius, Color color)` | Círculo preenchido. |
| `void DrawCircleLines(int x, int y, float radius, Color color)` | Contorno de círculo. |
| `void DrawRectangle / DrawRectangleV / DrawRectangleRec(...)` | Retângulo preenchido (várias assinaturas equivalentes). |
| `void DrawRectangleLines / DrawRectangleLinesEx(...)` | Contorno de retângulo (com espessura em `Ex`). |
| `void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color)` | Retângulo com cantos arredondados. `roundness` de `0.0` a `1.0`. |
| `void DrawTriangle / DrawTriangleLines(Vector2 v1, v2, v3, Color color)` | Triângulo preenchido / contorno. |
| `void DrawPoly / DrawPolyLines(Vector2 center, int sides, float radius, float rotationDeg, Color color)` | Polígono regular preenchido / contorno (ex.: hexágono com `sides=6`). |

---

## Texto

A engine usa uma **fonte bitmap 8x8 embutida** (sem dependências externas —
veja `include/Font8x8.hpp`), cobrindo os caracteres ASCII imprimíveis (32–126).

| Função | Descrição |
|---|---|
| `void DrawText(const std::string& text, int posX, int posY, int fontSize, Color color)` | Desenha texto. `fontSize` é a altura em pixels (a fonte é escalada proporcionalmente a partir da grade 8x8). Suporta `\n` para quebra de linha. |
| `int MeasureText(const std::string& text, int fontSize)` | Largura em pixels que o texto ocupará — útil para centralizar. |
| `void DrawFPS(int posX, int posY)` | Atalho para exibir o FPS atual (verde ≥50, laranja ≥25, vermelho abaixo disso). |

> Por ser uma fonte bitmap de baixa resolução (estilo retrô), tamanhos grandes
> (`fontSize` alto) ficam com aspecto "quadriculado". Para texto suave em alta
> resolução, considere estender a engine com SDL2_ttf.

---

## Texturas e imagens

| Função | Descrição |
|---|---|
| `Texture2D LoadTexture(const std::string& filePath)` | Carrega uma imagem do disco (PNG, JPG ou BMP) para a GPU. |
| `Texture2D LoadTextureFromImage(const Image& image)` | Envia uma `Image` (CPU) para a GPU. |
| `void UnloadTexture(Texture2D texture)` | Libera a textura da GPU. Sempre chame ao terminar de usar. |
| `void DrawTexture(Texture2D, int x, int y, Color tint)` | Desenha a textura na posição, sem rotação/escala. |
| `void DrawTextureV(Texture2D, Vector2 position, Color tint)` | Igual, recebendo `Vector2`. |
| `void DrawTextureEx(Texture2D, Vector2 position, float rotationDeg, float scale, Color tint)` | Com rotação e escala uniforme. |
| `void DrawTextureRec(Texture2D, Rectangle source, Vector2 position, Color tint)` | Desenha apenas uma sub-região `source` da textura (útil para spritesheets). |
| `void DrawTexturePro(Texture2D, Rectangle source, Rectangle dest, Vector2 origin, float rotationDeg, Color tint)` | Controle total: recorte de origem, retângulo de destino, pivô de rotação (`origin`, relativo ao destino) e rotação. |
| `Image GenImageColor(int width, int height, Color color)` | Cria uma imagem sólida em memória. |
| `void ImageDrawPixel(Image& image, int x, int y, Color color)` | Pinta um pixel de uma `Image` (CPU) — útil para gerar texturas proceduralmente (ruído, checkerboards, etc.) antes de subir para a GPU. |
| `void UnloadImage(Image& image)` | Libera a memória de CPU da imagem (a `Texture2D` gerada a partir dela continua válida). |

`tint` em branco (`WHITE`) desenha a textura com as cores originais; qualquer
outra cor multiplica (module) os pixels da textura, e o canal alfa de `tint`
controla a transparência geral.

---

## Câmera 2D

```cpp
Camera2D camera;
camera.offset = { GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
camera.target = { 0.0f, 0.0f };
camera.zoom   = 1.0f;

BeginMode2D(camera);
    // tudo desenhado aqui é afetado por posição/zoom/rotação da câmera
    DrawRectangle(-25, -25, 50, 50, RED);
EndMode2D();
```

| Função | Descrição |
|---|---|
| `void BeginMode2D(const Camera2D& camera)` | Ativa a câmera: todo desenho seguinte é transformado por ela. |
| `void EndMode2D()` | Desativa a câmera; volta ao sistema de coordenadas da tela. |

---

## Entrada: teclado

| Função | Descrição |
|---|---|
| `bool IsKeyDown(KeyboardKey key)` | `true` enquanto a tecla estiver pressionada. |
| `bool IsKeyUp(KeyboardKey key)` | `true` enquanto **não** estiver pressionada. |
| `bool IsKeyPressed(KeyboardKey key)` | `true` apenas no primeiro quadro em que foi pressionada. |
| `bool IsKeyReleased(KeyboardKey key)` | `true` apenas no primeiro quadro em que foi solta. |

Use `KeyboardKey::KEY_W`, `KeyboardKey::KEY_SPACE`, `KeyboardKey::KEY_ESCAPE`,
etc. — veja a lista completa em `include/Keys.hpp`.

---

## Entrada: mouse

| Função | Descrição |
|---|---|
| `int GetMouseX()` / `int GetMouseY()` | Posição do cursor em pixels da janela. |
| `Vector2 GetMousePosition()` | Idem, como `Vector2`. |
| `float GetMouseWheelMove()` | Quantidade de "scroll" desde o último quadro (positivo = para cima). |
| `bool IsMouseButtonDown/Up/Pressed/Released(MouseButton button)` | Mesma semântica do teclado, para `MouseButton::LEFT / RIGHT / MIDDLE`. |

---

## Áudio

| Função | Descrição |
|---|---|
| `bool InitAudioDevice()` | Inicializa o dispositivo de áudio. Chame antes de carregar sons. |
| `void CloseAudioDevice()` | Libera o dispositivo de áudio (também chamado automaticamente por `CloseWindow`). |
| `bool IsAudioDeviceReady()` | Se o áudio foi inicializado com sucesso. |
| `Sound LoadSound(const std::string& filePath)` | Carrega um `.wav`. Se `InitAudioDevice` não foi chamado, falha graciosamente (retorna um `Sound` inválido). |
| `void UnloadSound(Sound sound)` | Libera o som. |
| `void PlaySound(Sound sound)` | Toca o som (não bloqueia). |
| `void StopSound(Sound sound)` | Interrompe a reprodução, se estiver tocando. |
| `void SetSoundVolume(Sound sound, float volume)` | Volume de `0.0` a `1.0`. |

> Sempre verifique `sound.IsValid()` antes de tocar um som que pode não ter
> sido encontrado no disco — assim seu jogo não quebra por um asset faltando.

---

## Colisões e matemática

| Função | Descrição |
|---|---|
| `bool CheckCollisionRecs(Rectangle a, Rectangle b)` | Colisão AABB entre dois retângulos. |
| `bool CheckCollisionCircles(Vector2 c1, float r1, Vector2 c2, float r2)` | Colisão entre dois círculos. |
| `bool CheckCollisionPointRec(Vector2 point, Rectangle rec)` | Se um ponto está dentro de um retângulo. |
| `bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec)` | Colisão entre círculo e retângulo. |
| `float Vector2Distance(Vector2 a, Vector2 b)` | Distância euclidiana. |
| `float Vector2Length(Vector2 v)` | Magnitude do vetor. |
| `Vector2 Vector2Normalize(Vector2 v)` | Vetor unitário na mesma direção. |
| `float Vector2Angle(Vector2 a, Vector2 b)` | Ângulo (radianos) do vetor `b - a`. |
| `Color Fade(Color color, float alpha)` | Retorna a cor com o canal alfa multiplicado por `alpha` (`0.0`–`1.0`). |

---

## Aleatoriedade

| Função | Descrição |
|---|---|
| `void SetRandomSeed(unsigned int seed)` | Define a semente do gerador. |
| `int GetRandomValue(int min, int max)` | Inteiro aleatório no intervalo `[min, max]` (inclusive). Semeado automaticamente com o relógio na primeira chamada, se `SetRandomSeed` não tiver sido usado. |
