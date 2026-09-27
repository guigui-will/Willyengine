# Contribuindo

Contribuições são bem-vindas! Algumas orientações rápidas:

1. Abra uma *issue* descrevendo o bug ou a funcionalidade antes de um PR grande,
   para alinhar a abordagem.
2. Mantenha o estilo do código existente (nomes em `PascalCase` para funções e
   tipos, seguindo a convenção da raylib) e comente em português, como o resto
   do projeto.
3. Compile com `-Wall -Wextra` e garanta que não há warnings novos:
   ```bash
   cmake -B build -S . && cmake --build build
   ```
4. Se adicionar uma função pública em `include/Engine2D.hpp`, documente-a
   também em `docs/API.md`.
5. Sempre que possível, teste manualmente rodando os exemplos antes de abrir o PR.

## Ideias de extensão

- Suporte a flip horizontal/vertical em `DrawTexturePro` (via `source.width`/`height` negativos).
- `RenderTexture2D` (renderizar para uma textura, não só para a tela).
- Sistema de partículas simples.
- Suporte a joystick/gamepad (SDL2 já oferece a API de base).
- Fonte TTF opcional via SDL2_ttf, como alternativa à fonte bitmap embutida.
