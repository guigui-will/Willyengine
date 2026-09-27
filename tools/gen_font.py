#!/usr/bin/env python3
"""Gera include/Font8x8.hpp: uma fonte bitmap 8x8 (ASCII 32..126) embutida na engine,
rasterizada a partir da fonte DejaVu Sans Bold (licenca Bitstream Vera, permissiva),
para que Engine2D nao precise de SDL_ttf nem de arquivos de fonte externos em runtime.
"""
from PIL import Image, ImageDraw, ImageFont

FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
CELL = 8          # grade final 8x8 por glifo
SUPERSAMPLE = 4    # renderiza maior e faz downsample para suavizar antes do threshold

first_char = 32
last_char = 126

font = ImageFont.truetype(FONT_PATH, size=CELL * SUPERSAMPLE - 4)

glyphs = []
for code in range(first_char, last_char + 1):
    ch = chr(code)
    big = CELL * SUPERSAMPLE
    img = Image.new("L", (big, big), 0)
    draw = ImageDraw.Draw(img)
    try:
        bbox = draw.textbbox((0, 0), ch, font=font)
    except Exception:
        bbox = (0, 0, 0, 0)
    w = bbox[2] - bbox[0]
    h = bbox[3] - bbox[1]
    x = (big - w) // 2 - bbox[0]
    y = (big - h) // 2 - bbox[1]
    draw.text((x, y), ch, fill=255, font=font)
    small = img.resize((CELL, CELL), Image.LANCZOS)
    px = small.load()
    rows = []
    for row in range(CELL):
        byte = 0
        for col in range(CELL):
            val = px[col, row]
            bit = 1 if val > 90 else 0
            byte |= (bit << (7 - col))
        rows.append(byte)
    glyphs.append(rows)

import os

lines = []
lines.append("// =====================================================================================")
lines.append("//  Font8x8.hpp - Fonte bitmap 8x8 embutida (gerada automaticamente, gen_font.py)")
lines.append("//  Cobre os caracteres ASCII imprimiveis 32..126. Cada glifo ocupa 8 bytes (1 bit/pixel,")
lines.append("//  bit mais significativo = pixel mais a esquerda). Nao depende de arquivos externos.")
lines.append("// =====================================================================================")
lines.append("#pragma once")
lines.append("")
lines.append("namespace Engine2D { namespace internal {")
lines.append("")
lines.append(f"constexpr int FONT8x8_FIRST_CHAR = {first_char};")
lines.append(f"constexpr int FONT8x8_LAST_CHAR  = {last_char};")
lines.append(f"constexpr int FONT8x8_GLYPH_SIZE = {CELL};")
lines.append("")
lines.append(f"inline constexpr unsigned char FONT8x8[{len(glyphs)}][8] = {{")
for i, rows in enumerate(glyphs):
    code = first_char + i
    ch = chr(code)
    comment = ch if ch not in ('\\', '/', '*') else ('backslash' if ch == '\\' else ch)
    row_str = ", ".join(f"0x{b:02X}" for b in rows)
    lines.append(f"    {{ {row_str} }}, // {code} '{comment}'")
lines.append("};")
lines.append("")
lines.append("} } // namespace Engine2D::internal")
lines.append("")

with open(os.path.join(os.path.dirname(__file__), "..", "include", "Font8x8.hpp"), "w") as f:
    f.write("\n".join(lines))

print("Gerado com", len(glyphs), "glifos.")
