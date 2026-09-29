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

- Sistema de cenas / gerenciador de estados de jogo (menu, jogando, pausado, game over).
- Suporte a spritesheets com animação automática (um "AnimatedSprite" pronto).
- Suporte a joystick genérico além de gamepad (`SDL_Joystick` para controles não mapeados
  como `SDL_GameController`).
- Fonte TTF opcional via SDL2_ttf, como alternativa de alta resolução à fonte bitmap embutida.
- Múltiplas músicas simultâneas (o SDL2_mixer só toca uma por vez nativamente — precisaria
  de um mixer próprio por cima do `Mix_Chunk`).
