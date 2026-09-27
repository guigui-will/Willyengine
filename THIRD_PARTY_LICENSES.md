# Licenças de terceiros

Este projeto depende de bibliotecas e dados de terceiros. Nenhum código deles é
distribuído dentro deste repositório (são dependências que você instala
separadamente), exceto a fonte bitmap embutida descrita abaixo.

## SDL2, SDL2_image, SDL2_mixer

Licenciadas sob a **zlib license**. Você precisa instalá-las no seu sistema
(veja o README.md); elas não estão incluídas neste repositório.

- https://www.libsdl.org/
- https://github.com/libsdl-org/SDL_image
- https://github.com/libsdl-org/SDL_mixer

## Fonte bitmap embutida (include/Font8x8.hpp)

O arquivo `include/Font8x8.hpp` contém uma fonte bitmap 8x8, gerada
automaticamente (script `gen_font.py`) a partir da rasterização da fonte
**DejaVu Sans Bold** em baixa resolução.

A família DejaVu é derivada da fonte Bitstream Vera e é distribuída sob uma
licença permissiva (Bitstream Vera License + DejaVu changes), que permite uso,
modificação e redistribuição, inclusive de versões modificadas/derivadas como
esta rasterização em baixa resolução, desde que o aviso de licença abaixo seja
preservado.

Texto da licença: https://dejavu-fonts.github.io/License.html

Resumo: a fonte pode ser usada livremente, inclusive embutida e modificada,
sem restrições que afetem a licença do restante do projeto (não é "copyleft").
