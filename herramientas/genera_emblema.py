#!/usr/bin/env python3
"""Genera cronica/emblema.h: el emblema de CDSN (icon-192.png de cdsn-web) en RGB565 + alfa, en dos tamanos.
Uso: python3 herramientas/genera_emblema.py ruta/a/icon-192.png"""
import os, sys
from PIL import Image, ImageDraw

def conv(im, px, nom):
    a = im.resize((px * 4, px * 4), Image.LANCZOS)
    m = Image.new("L", a.size, 0); ImageDraw.Draw(m).ellipse((2, 2, a.size[0] - 3, a.size[1] - 3), fill=255)
    a.putalpha(m); a = a.resize((px, px), Image.LANCZOS)
    col, alf = [], []
    for r, g, b, al in a.getdata():
        col.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)); alf.append(al)
    s = [f"const uint16_t {nom}_COL[] PROGMEM = {{"]
    s += ["  " + ",".join(f"0x{v:04x}" for v in col[i:i + 16]) + "," for i in range(0, len(col), 16)]
    s += ["};", f"const uint8_t {nom}_ALF[] PROGMEM = {{"]
    s += ["  " + ",".join(str(v) for v in alf[i:i + 32]) + "," for i in range(0, len(alf), 32)]
    s += ["};", f"const int {nom}_PX = {px};", ""]
    return s

im = Image.open(sys.argv[1]).convert("RGBA")
out = ["// emblema.h - GENERADO por herramientas/genera_emblema.py (emblema de Caballeros de Sion)", "#pragma once", "#include <pgmspace.h>", ""]
out += conv(im, 64, "EMB_P") + conv(im, 170, "EMB_G")
open(os.path.join(os.path.dirname(__file__), "..", "cronica", "emblema.h"), "w").write("\n".join(out))
print("emblema.h generado")
