#!/usr/bin/env python3
"""Genera cronica/fuentes.h: fuentes VLW suavizadas (DejaVu) con acentos y simbolos.

Uso:  python3 herramientas/genera_fuentes.py
Necesita: pip install freetype-py  y las fuentes DejaVu (vienen en casi todos los Linux).
Formato VLW (el de Processing / TFT_eSPI / LovyanGFX), con los glifos ordenados por Unicode.
"""
import os, struct, sys
import freetype

DIR = "/usr/share/fonts/truetype/dejavu/"
SALIDA = os.path.join(os.path.dirname(__file__), "..", "cronica", "fuentes.h")

# ASCII, Latin-1 y los simbolos que usan las noticias y la web
CHARS = list(range(0x21, 0x7F)) + list(range(0xA1, 0x100)) + [ord(c) for c in
        "—–‘’“”…•€→←↑↓▲▼▸▾◆■●○★✓✗⚔⚠✉≈≥≤−‰"]
CHARS = sorted(set(CHARS))

# (nombre, archivo, tamano en px)
FUENTES = [
    ("F_TXT",   "DejaVuSerif.ttf",          17),   # texto de las noticias
    ("F_TXTB",  "DejaVuSerif-Bold.ttf",     17),   # **negritas**
    ("F_PEQ",   "DejaVuSerif.ttf",          14),   # entradillas pequenas, pies
    ("F_PEQB",  "DejaVuSerif-Bold.ttf",     14),
    ("F_TIT",   "DejaVuSerif-Bold.ttf",     22),   # titulares
    ("F_CAB",   "DejaVuSerif-Bold.ttf",     34),   # LA CRONICA DE SION
    ("F_MONO",  "DejaVuSansMono.ttf",       13),   # horas, banda HUD
    ("F_MONOB", "DejaVuSansMono-Bold.ttf",  13),
    ("F_BOT",   "DejaVuSans-Bold.ttf",      14),   # botones
]
RESERVA = "DejaVuSans.ttf"   # para los simbolos que no tenga la fuente principal


def glifo(cara, reserva, cp, px):
    for f in (cara, reserva):
        if f.get_char_index(cp):
            f.set_pixel_sizes(0, px)
            f.load_char(cp, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_NORMAL)
            g = f.glyph
            b = g.bitmap
            filas = [bytes(b.buffer[r * b.pitch:r * b.pitch + b.width]) for r in range(b.rows)]
            return dict(cp=cp, w=b.width, h=b.rows, adv=(g.advance.x + 32) >> 6,
                        dy=g.bitmap_top, dx=g.bitmap_left, bmp=b"".join(filas))
    return None


def vlw(archivo, px):
    cara = freetype.Face(DIR + archivo)
    reserva = freetype.Face(DIR + RESERVA)
    cara.set_pixel_sizes(0, px)
    asc = (cara.size.ascender + 32) >> 6
    desc = (-cara.size.descender + 32) >> 6
    G = [g for g in (glifo(cara, reserva, cp, px) for cp in CHARS) if g]
    cab = struct.pack(">6i", len(G), 11, px, 0, asc, desc)
    tabla = b"".join(struct.pack(">7i", g["cp"], g["h"], g["w"], g["adv"], g["dy"], g["dx"], 0) for g in G)
    return cab + tabla + b"".join(g["bmp"] for g in G), len(G)


def main():
    out = ["// fuentes.h - GENERADO por herramientas/genera_fuentes.py (no editar a mano)",
           "// Fuentes DejaVu (licencia libre, ver https://dejavu-fonts.github.io/License.html) en formato VLW.",
           "#pragma once", "#include <pgmspace.h>", ""]
    total = 0
    for nom, arch, px in FUENTES:
        datos, n = vlw(arch, px)
        total += len(datos)
        out.append(f"// {arch} {px} px, {n} glifos, {len(datos)} bytes")
        out.append(f"const uint8_t {nom}[] PROGMEM = {{")
        for i in range(0, len(datos), 24):
            out.append("  " + ",".join(str(x) for x in datos[i:i + 24]) + ",")
        out.append("};\n")
    with open(SALIDA, "w") as f:
        f.write("\n".join(out))
    print(f"{SALIDA}: {len(FUENTES)} fuentes, {total} bytes")


if __name__ == "__main__":
    sys.exit(main())
