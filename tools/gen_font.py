#!/usr/bin/env python3
"""Gera include/Font8x8.hpp: uma fonte bitmap 8x8 embutida na engine, rasterizada a
partir da fonte DejaVu Sans Bold (licenca Bitstream Vera, permissiva), para que
Engine2D nao precise de SDL_ttf nem de arquivos de fonte externos em runtime.

Cobre o ASCII imprimivel (32..126) em uma tabela contigua, mais um conjunto extra
de letras acentuadas comuns do portugues (e alguns simbolos uteis), em uma tabela
paralela indexada por code point Unicode.
"""
from PIL import Image, ImageDraw, ImageFont
import os

FONT_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
CELL = 8          # grade final 8x8 por glifo
SUPERSAMPLE = 4    # renderiza maior e faz downsample para suavizar antes do threshold

first_char = 32
last_char = 126

# Letras acentuadas comuns do portugues + alguns simbolos extra uteis.
extra_chars = list("áàâãéêíóôõúüçÁÀÂÃÉÊÍÓÔÕÚÜÇñÑ°ºª¿¡€")

font = ImageFont.truetype(FONT_PATH, size=CELL * SUPERSAMPLE - 4)

def rasterize(ch):
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
    return rows

glyphs = [rasterize(chr(code)) for code in range(first_char, last_char + 1)]
extra_glyphs = [(ord(ch), rasterize(ch)) for ch in extra_chars]

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
lines.append(f"constexpr int FONT8x8_EXTRA_COUNT = {len(extra_glyphs)};")
lines.append("")
lines.append(f"inline constexpr unsigned int FONT8x8_EXTRA_CODEPOINTS[{len(extra_glyphs)}] = {{")
lines.append("    " + ", ".join(f"0x{cp:04X}u" for cp, _ in extra_glyphs))
lines.append("};")
lines.append("")
lines.append(f"inline constexpr unsigned char FONT8x8_EXTRA[{len(extra_glyphs)}][8] = {{")
for cp, rows in extra_glyphs:
    row_str = ", ".join(f"0x{b:02X}" for b in rows)
    lines.append(f"    {{ {row_str} }}, // U+{cp:04X} '{chr(cp)}'")
lines.append("};")
lines.append("")
lines.append("// Retorna o glifo (8 bytes) para um code point Unicode, ou nullptr se não suportado.")
lines.append("inline const unsigned char* FindGlyph(unsigned int codepoint) {")
lines.append("    if (codepoint >= (unsigned int)FONT8x8_FIRST_CHAR && codepoint <= (unsigned int)FONT8x8_LAST_CHAR)")
lines.append("        return FONT8x8[codepoint - FONT8x8_FIRST_CHAR];")
lines.append("    for (int i = 0; i < FONT8x8_EXTRA_COUNT; i++)")
lines.append("        if (FONT8x8_EXTRA_CODEPOINTS[i] == codepoint) return FONT8x8_EXTRA[i];")
lines.append("    return nullptr;")
lines.append("}")
lines.append("")
lines.append("} } // namespace Engine2D::internal")
lines.append("")

with open(os.path.join(os.path.dirname(__file__), "Engine2D", "include", "Font8x8.hpp"), "w") as f:
    f.write("\n".join(lines))

print("Gerado com", len(glyphs), "glifos base +", len(extra_glyphs), "glifos extra.")
