// texto.h - fuentes, UTF-8 (acentos), limpieza de emojis y corte de lineas
#pragma once
#include "fuentes.h"

// ---------- fuentes VLW (suavizadas) cargadas desde la memoria flash ----------
struct Fuente {
  lgfx::PointerWrapper datos;
  lgfx::VLWfont f;
  int alto = 0;
  void carga(const uint8_t *arr) { datos.set(arr); f.loadFont(&datos); }
};
enum { TXT, TXTB, PEQ, PEQB, TIT, CABF, MONO, MONOB, BOT, NFUENTES };
Fuente FU[NFUENTES];

void cargaFuentes() {
  const uint8_t *arr[NFUENTES] = {F_TXT, F_TXTB, F_PEQ, F_PEQB, F_TIT, F_CAB, F_MONO, F_MONOB, F_BOT};
  for (int i = 0; i < NFUENTES; i++) {
    FU[i].carga(arr[i]);
    tft.setFont(&FU[i].f); FU[i].alto = tft.fontHeight();
  }
}

// color de la web (0xRRGGBB) para LovyanGFX
inline uint32_t C(uint32_t rgb) { return lgfx::color888((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255); }
inline uint16_t C16(uint32_t rgb) { return lgfx::color565((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255); }
uint32_t mezcla(uint32_t a, uint32_t b, float t) {   // de a hacia b
  int r = ((a >> 16) & 255) + (((int)((b >> 16) & 255) - (int)((a >> 16) & 255)) * t);
  int g = ((a >> 8) & 255) + (((int)((b >> 8) & 255) - (int)((a >> 8) & 255)) * t);
  int bl = (a & 255) + (((int)(b & 255) - (int)(a & 255)) * t);
  return (r << 16) | (g << 8) | bl;
}

// ---------- UTF-8 ----------
uint32_t sigCP(const char *&p) {   // lee un caracter y avanza
  uint8_t c = *p++;
  if (c < 0x80) return c;
  int n = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : (c >= 0xC0) ? 1 : 0;
  uint32_t cp = c & (0x3F >> n);
  while (n-- && (*p & 0xC0) == 0x80) cp = (cp << 6) | (*p++ & 0x3F);
  return cp;
}
void ponCP(String &s, uint32_t cp) {
  char b[5];
  if (cp < 0x80) { b[0] = cp; b[1] = 0; }
  else if (cp < 0x800) { b[0] = 0xC0 | (cp >> 6); b[1] = 0x80 | (cp & 63); b[2] = 0; }
  else if (cp < 0x10000) { b[0] = 0xE0 | (cp >> 12); b[1] = 0x80 | ((cp >> 6) & 63); b[2] = 0x80 | (cp & 63); b[3] = 0; }
  else { b[0] = 0xF0 | (cp >> 18); b[1] = 0x80 | ((cp >> 12) & 63); b[2] = 0x80 | ((cp >> 6) & 63); b[3] = 0x80 | (cp & 63); b[4] = 0; }
  s += b;
}
const char *SIMBOLOS = "—–‘’“”…•€→←↑↓▲▼▸▾◆■●○★✓✗⚔⚠✉≈≥≤−‰";
bool tieneGlifo(uint32_t cp) {
  if (cp >= 0x20 && cp < 0x7F) return true;
  if (cp >= 0xA0 && cp <= 0xFF) return true;
  const char *p = SIMBOLOS;
  while (*p) if (sigCP(p) == cp) return true;
  return false;
}
uint32_t traduceEmoji(uint32_t cp) {   // emojis que tienen un simbolo parecido en la fuente
  switch (cp) {
    case 0x2B06: case 0x1F4C8: return 0x2191;   // ⬆ 📈 -> ↑
    case 0x2B07: case 0x1F4C9: case 0x1F53B: return 0x25BC;   // ⬇ 📉 🔻 -> ▼
    case 0x2796: return 0x2212;                  // ➖ -> −
    case 0x2705: case 0x2714: return 0x2713;     // ✅ ✔ -> ✓
    case 0x274C: return 0x2717;                  // ❌ -> ✗
    case 0x2B50: case 0x1F31F: return 0x2605;    // ⭐ -> ★
    case 0x1F534: return 0x25CF;                 // 🔴 -> ●
  }
  return 0;
}
// Prepara un texto para pintarlo: quita los emojis que no tiene la fuente,
// **negrita** -> marca \x01 y [[Sistema]] -> marca \x02 (naranja). Si quitaIcono, quita el emoji del principio.
String limpia(const char *t, bool quitaIcono = false) {
  String s; if (!t) return s;
  s.reserve(strlen(t) + 4);
  const char *p = t;
  if (quitaIcono) {   // salta todo lo que no sea letra, numero, * o [ al principio
    while (*p) {
      const char *q = p; uint32_t cp = sigCP(q);
      if (cp == '*' || cp == '[' || cp == '"' || cp == 0xA1 || cp == 0xBF || cp == 0xAB || (cp < 128 && isalnum(cp)) || (cp >= 0xC0 && cp <= 0xFF)) break;
      p = q;
    }
  }
  while (*p) {
    if (p[0] == '*' && p[1] == '*') { s += '\x01'; p += 2; continue; }
    if (p[0] == '[' && p[1] == '[') { s += '\x02'; p += 2; continue; }
    if (p[0] == ']' && p[1] == ']') { s += '\x02'; p += 2; continue; }
    uint32_t cp = sigCP(p);
    if (cp == '\r' || cp == '\t') cp = ' ';
    if (cp == '\n') { s += '\n'; continue; }
    if (cp < 0x20) continue;
    if (!tieneGlifo(cp)) { cp = traduceEmoji(cp); if (!cp) continue; }
    ponCP(s, cp);
  }
  // quita espacios dobles y del principio que dejan los emojis quitados
  String r; r.reserve(s.length());
  char ant = '\n';
  for (unsigned i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == ' ' && (ant == ' ' || ant == '\n')) continue;
    r += c; ant = c;
  }
  r.trim();
  return r;
}
String sinMarcas(const char *t) {   // como la web: sin negritas ni el icono del principio
  String s = limpia(t, true), r;
  for (unsigned i = 0; i < s.length(); i++) if (s[i] != '\x01' && s[i] != '\x02') r += s[i];
  return r;
}
String mayusculas(const String &s) {   // tambien las vocales con tilde y la ñ
  String r; const char *p = s.c_str();
  while (*p) {
    uint32_t cp = sigCP(p);
    if (cp >= 'a' && cp <= 'z') cp -= 32;
    else if (cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) cp -= 0x20;
    ponCP(r, cp);
  }
  return r;
}
String hastaPunto(const String &s) {   // la parte antes de " · " (el titular corto)
  int i = s.indexOf(" \xC2\xB7 "); return i > 0 ? s.substring(0, i) : s;
}
