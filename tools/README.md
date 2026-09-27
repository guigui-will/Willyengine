# tools/

## `gen_font.py`

Script usado para gerar `include/Font8x8.hpp`: rasteriza a fonte DejaVu Sans
Bold (instalada no sistema, normalmente em
`/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf` no Ubuntu/Debian) em
uma grade 8x8 por caractere, produzindo uma fonte bitmap embutida que a engine
usa para desenhar texto sem depender de SDL2_ttf nem de arquivos de fonte
externos em tempo de execução.

Requer Python 3 com Pillow (`pip install Pillow`).

Para regenerar (por exemplo, se quiser trocar a fonte-fonte usada ou ajustar
o limiar de binarização):

```bash
sudo apt-get install -y fonts-dejavu-core   # se ainda não tiver a fonte
pip install --break-system-packages Pillow
python3 tools/gen_font.py
```

Isso sobrescreve `include/Font8x8.hpp`.
