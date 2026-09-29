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
- [Render-to-texture](#render-to-texture)
- [Partículas](#partículas)
- [Câmera 2D](#câmera-2d)
- [Entrada: teclado](#entrada-teclado)
- [Entrada de texto](#entrada-de-texto)
- [Entrada: mouse](#entrada-mouse)
- [Entrada: toque](#entrada-toque)
- [Entrada: gamepad](#entrada-gamepad)
- [Áudio](#áudio)
- [Música em streaming](#música-em-streaming)
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

### `Music`
```cpp
struct Music { void* handle; bool IsValid() const; };
```
Música em streaming (não carrega tudo em memória de uma vez) — veja [Música em streaming](#música-em-streaming).

### `RenderTexture2D`
```cpp
struct RenderTexture2D { void* handle; Texture2D texture; bool IsValid() const; };
```
Textura de destino para renderização ("render-to-texture") — veja [Render-to-texture](#render-to-texture).

### `Particle` / `ParticleEmitter`
Veja a seção [Partículas](#partículas).

### `KeyboardKey` / `MouseButton` / `GamepadButton` / `GamepadAxis`
Enums com os códigos de tecla (ver `include/Keys.hpp`), botões do mouse
(`LEFT`, `RIGHT`, `MIDDLE`), botões de gamepad (`A, B, X, Y, LEFT_BUMPER,
RIGHT_BUMPER, BACK, START, GUIDE, LEFT_THUMB, RIGHT_THUMB, DPAD_UP/DOWN/LEFT/RIGHT`)
e eixos de gamepad (`LEFT_X, LEFT_Y, RIGHT_X, RIGHT_Y, LEFT_TRIGGER, RIGHT_TRIGGER`).

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
veja `include/Font8x8.hpp`), cobrindo os caracteres ASCII imprimíveis (32–126)
**mais as letras acentuadas mais comuns do português** (á à â ã é ê í ó ô õ ú
ü ç, maiúsculas incluídas, além de ñ/Ñ e alguns símbolos como °, ª, ¿, ¡, €).
Strings devem estar em **UTF-8** (padrão dos editores/compiladores atuais) —
a engine decodifica UTF-8 internamente para casar com a tabela de glifos.

| Função | Descrição |
|---|---|
| `void DrawText(const std::string& text, int posX, int posY, int fontSize, Color color)` | Desenha texto. `fontSize` é a altura em pixels (a fonte é escalada proporcionalmente a partir da grade 8x8). Suporta `\n` para quebra de linha manual. |
| `int MeasureText(const std::string& text, int fontSize)` | Largura em pixels que o texto ocupará (da linha mais longa, se houver `\n`) — útil para centralizar. |
| `void DrawFPS(int posX, int posY)` | Atalho para exibir o FPS atual (verde ≥50, laranja ≥25, vermelho abaixo disso). |
| `std::vector<std::string> WrapText(const std::string& text, int fontSize, int maxWidth)` | Quebra o texto em linhas que cabem em `maxWidth` pixels, respeitando palavras inteiras. |
| `void DrawTextBoxed(const std::string& text, Rectangle rec, int fontSize, Color color)` | Desenha texto já quebrado automaticamente para caber dentro de `rec` (não deixa vazar verticalmente). |

> Por ser uma fonte bitmap de baixa resolução (estilo retrô), tamanhos grandes
> (`fontSize` alto) ficam com aspecto "quadriculado", e caracteres acentuados
> ficam mais apertados que as letras simples. Para texto suave em alta
> resolução, considere estender a engine com SDL2_ttf. Caracteres fora do
> conjunto suportado (ASCII básico + acentos listados acima) são ignorados
> (avançam o cursor sem desenhar nada).

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
| `void DrawTexturePro(Texture2D, Rectangle source, Rectangle dest, Vector2 origin, float rotationDeg, Color tint)` | Controle total: recorte de origem, retângulo de destino, pivô de rotação (`origin`, relativo ao destino) e rotação. **Espelhamento:** passe `source.width` e/ou `source.height` **negativos** para espelhar horizontal/verticalmente. |
| `Image GenImageColor(int width, int height, Color color)` | Cria uma imagem sólida em memória. |
| `void ImageDrawPixel(Image& image, int x, int y, Color color)` | Pinta um pixel de uma `Image` (CPU) — útil para gerar texturas proceduralmente (ruído, checkerboards, etc.) antes de subir para a GPU. |
| `void UnloadImage(Image& image)` | Libera a memória de CPU da imagem (a `Texture2D` gerada a partir dela continua válida). |

`tint` em branco (`WHITE`) desenha a textura com as cores originais; qualquer
outra cor multiplica (module) os pixels da textura, e o canal alfa de `tint`
controla a transparência geral.

---

## Render-to-texture

Permite desenhar em uma textura em vez de desenhar direto na tela — útil para
pós-processamento, minimapas, composição de camadas, ou "carimbar" algo uma
vez e reutilizar sem redesenhar todo frame (como no `example_advanced.cpp`).

```cpp
RenderTexture2D target = LoadRenderTexture(320, 180);

BeginTextureMode(target);
    ClearBackground(BLANK); // BLANK preserva a transparência
    DrawCircle(160, 90, 40, RED);
EndTextureMode();

// mais tarde, em qualquer BeginDrawing()/EndDrawing():
DrawTexture(target.texture, 0, 0, WHITE);

UnloadRenderTexture(target);
```

| Função | Descrição |
|---|---|
| `RenderTexture2D LoadRenderTexture(int width, int height)` | Cria uma textura de destino de renderização. |
| `void UnloadRenderTexture(RenderTexture2D target)` | Libera a textura. |
| `void BeginTextureMode(RenderTexture2D target)` | A partir daqui, todo desenho vai para `target` em vez da tela. |
| `void EndTextureMode()` | Volta a desenhar na tela normalmente. |

---

## Partículas

Sistema de partículas simples de uso geral (fogo, fumaça, explosões, faíscas...).
Um `ParticleEmitter` mantém um "pool" fixo de partículas (sem alocação em
runtime); cor e tamanho são interpolados de `startColor`/`startSize` até
`endColor`/`endSize` conforme a partícula envelhece.

```cpp
ParticleEmitter fire = CreateParticleEmitter({ 400.0f, 300.0f }, 300);
fire.emissionRate = 80.0f;   // partículas/segundo
fire.particleLife = 1.2f;    // segundos de vida de cada uma
fire.velocityMin  = { -20.0f, -140.0f };
fire.velocityMax  = {  20.0f,  -60.0f };
fire.startColor   = { 255, 220, 60, 255 };
fire.endColor     = { 200, 30,  0,   0 };

// no loop principal:
UpdateParticleEmitter(fire, GetFrameTime());
// dentro de BeginDrawing()/EndDrawing():
DrawParticleEmitter(fire);
```

| Função | Descrição |
|---|---|
| `ParticleEmitter CreateParticleEmitter(Vector2 position, int maxParticles)` | Cria um emissor com capacidade máxima `maxParticles` (partículas além disso são ignoradas até uma antiga expirar). |
| `void UpdateParticleEmitter(ParticleEmitter& emitter, float dt)` | Avança a simulação: spawna novas partículas conforme `emissionRate` e atualiza posição/vida das existentes. Chame uma vez por frame. |
| `void EmitParticleBurst(ParticleEmitter& emitter, int count)` | Dispara `count` partículas de uma vez, independente de `emissionRate` (ótimo para explosões). |
| `void DrawParticleEmitter(const ParticleEmitter& emitter)` | Desenha todas as partículas ativas (como círculos, cor/tamanho interpolados pela vida restante). |

Os campos de `ParticleEmitter` (`position`, `velocityMin/Max`, `emissionRate`,
`particleLife`, `startSize`/`endSize`, `startColor`/`endColor`) são públicos e
podem ser ajustados a qualquer momento, inclusive em tempo real.

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

## Entrada de texto

Para caixas de digitação (nome do jogador, chat, etc.), capturando texto já
decodificado de UTF-8 para Unicode:

```cpp
StartTextInput();
std::string nome;
// a cada frame:
int c;
while ((c = GetCharPressed()) != 0) {
    if (c == 8 && !nome.empty()) nome.pop_back(); // backspace (tratado à parte)
    else if (c >= 32) nome += (char)c; // simplificado: só ASCII aqui
}
```

| Função | Descrição |
|---|---|
| `void StartTextInput()` | Ativa a captura de texto (em mobile pode abrir o teclado virtual). |
| `void StopTextInput()` | Desativa a captura. |
| `int GetCharPressed()` | Retorna o próximo caractere Unicode digitado desde a última chamada, ou `0` se não houver nenhum — chame em loop a cada frame até esvaziar a fila. |

---

## Entrada: mouse

| Função | Descrição |
|---|---|
| `int GetMouseX()` / `int GetMouseY()` | Posição do cursor em pixels da janela. |
| `Vector2 GetMousePosition()` | Idem, como `Vector2`. |
| `float GetMouseWheelMove()` | Quantidade de "scroll" desde o último quadro (positivo = para cima). |
| `bool IsMouseButtonDown/Up/Pressed/Released(MouseButton button)` | Mesma semântica do teclado, para `MouseButton::LEFT / RIGHT / MIDDLE`. |

---

## Entrada: toque

Suporte básico a telas sensíveis ao toque, além do mouse (em desktop normalmente
fica sempre com `GetTouchPointCount() == 0`).

| Função | Descrição |
|---|---|
| `int GetTouchPointCount()` | Número de dedos tocando a tela agora (até 8). |
| `Vector2 GetTouchPosition(int index)` | Posição do toque de índice `0` a `GetTouchPointCount()-1`. |

---

## Entrada: gamepad

Suporte a até 4 controles simultâneos (Xbox, PlayStation e a maioria dos
genéricos compatíveis, já remapeados pela SDL2 para um layout lógico único).
Controles conectados antes ou depois do `InitWindow()` são detectados
automaticamente.

```cpp
if (IsGamepadAvailable(0)) {
    float x = GetGamepadAxisMovement(0, GamepadAxis::LEFT_X);
    float y = GetGamepadAxisMovement(0, GamepadAxis::LEFT_Y);
    if (IsGamepadButtonPressed(0, GamepadButton::A)) { /* pular */ }
}
```

| Função | Descrição |
|---|---|
| `bool IsGamepadAvailable(int gamepad)` | `gamepad`: índice de 0 a 3. |
| `bool IsGamepadButtonDown/Pressed/Released(int gamepad, GamepadButton button)` | Mesma semântica do teclado/mouse. |
| `float GetGamepadAxisMovement(int gamepad, GamepadAxis axis)` | Sticks: -1.0 a 1.0 (com pequena zona morta para evitar drift). Gatilhos (`LEFT_TRIGGER`/`RIGHT_TRIGGER`): 0.0 a 1.0. |

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

## Música em streaming

Diferente de `Sound` (que carrega tudo em memória), `Music` toca em streaming
via SDL2_mixer — ideal para trilhas sonoras longas (`.wav`, `.ogg` ou `.mp3`,
dependendo dos decoders habilitados na instalação do SDL2_mixer do sistema).

```cpp
InitAudioDevice();
Music trilha = LoadMusicStream("trilha.ogg");
PlayMusicStream(trilha);

while (!WindowShouldClose()) {
    UpdateMusicStream(trilha); // chame sempre, mesmo que pareça não fazer nada
    // ...
}

UnloadMusicStream(trilha);
```

| Função | Descrição |
|---|---|
| `Music LoadMusicStream(const std::string& filePath)` | Carrega (abre) um arquivo de música. |
| `void UnloadMusicStream(Music music)` | Libera. |
| `void PlayMusicStream(Music music)` | Toca em loop. |
| `void UpdateMusicStream(Music music)` | Chame a cada frame (existe por compatibilidade de padrão de uso; o streaming em si já roda internamente no SDL2_mixer). |
| `void StopMusicStream(Music music)` / `PauseMusicStream` / `ResumeMusicStream` | Controle de reprodução. |
| `bool IsMusicStreamPlaying(Music music)` | Se está tocando (e não pausada). |
| `void SetMusicVolume(Music music, float volume)` | Volume de `0.0` a `1.0`. |

> **Limitação:** o SDL2_mixer só reproduz **uma música por vez** (é uma
> limitação da própria biblioteca, não desta engine) — use `Sound`/`PlaySound`
> para múltiplos efeitos simultâneos, e `Music` apenas para a trilha de fundo.

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
