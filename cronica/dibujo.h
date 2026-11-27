// dibujo.h - pinta la cabecera (titulo + banda HUD), el teletipo y la ventana de noticias que se ve
#pragma once
#include "emblema.h"

LGFX_Sprite lienzo(&tft);   // zona de noticias (800 x 374), en la PSRAM
LGFX_Sprite cabS(&tft);     // cabecera (800 x 82)
LGFX_Sprite teleS(&tft);    // teletipo (800 x 24)

// ---------- texto de una linea, con negritas (\x01) y sistemas en naranja (\x02) ----------
void lineaTexto(LovyanGFX &g, int x, int y, const char *s, uint8_t est, uint8_t f, uint8_t fb, uint32_t c, uint32_t cb, uint32_t ch) {
  String seg; int cx = x;
  auto sale = [&]() {
    if (!seg.length()) return;
    g.setFont(&FU[(est & 1) ? fb : f].f);
    g.setTextColor((uint32_t)((est & 2) ? ch : (est & 1) ? cb : c));
    g.drawString(seg, cx, y); cx += g.textWidth(seg); seg = "";
  };
  for (const char *p = s; *p; p++) {
    if (*p == '\x01') { sale(); est ^= 1; }
    else if (*p == '\x02') { sale(); est ^= 2; }
    else seg += *p;
  }
  sale();
}

// ---------- emblema con transparencia ----------
uint16_t mezcla565(uint16_t a, uint16_t b, uint8_t al) {   // b sobre a con alfa al
  uint32_t ar = (a >> 11) & 31, ag = (a >> 5) & 63, ab = a & 31, br = (b >> 11) & 31, bg = (b >> 5) & 63, bb = b & 31;
  uint32_t r = (ar * (255 - al) + br * al) / 255, g = (ag * (255 - al) + bg * al) / 255, bl = (ab * (255 - al) + bb * al) / 255;
  return (r << 11) | (g << 5) | bl;
}
void emblema(LGFX_Sprite &s, int x0, int y0, const uint16_t *col, const uint8_t *alf, int px, bool espejo) {
  for (int y = 0; y < px; y++) for (int x = 0; x < px; x++) {
    int i = y * px + (espejo ? px - 1 - x : x);
    uint8_t al = pgm_read_byte(&alf[i]); if (!al) continue;
    uint16_t c = pgm_read_word(&col[i]);
    if (al < 250) { uint16_t f = s.readPixel(x0 + x, y0 + y); c = mezcla565(f, c, al); }
    s.drawPixel(x0 + x, y0 + y, c);
  }
}

// ---------- dibujos de cada tipo de noticia (los de la web, a 40x28) ----------
inline int S(float v) { return (int)(v * 0.4f + 0.5f); }
void icono(LovyanGFX &g, int k, int x, int y) {
  auto L = [&](float x1, float y1, float x2, float y2, uint32_t c, float w = 2.5f) { g.drawWideLine(x + x1 * .4f, y + y1 * .4f, x + x2 * .4f, y + y2 * .4f, w * .45f, c); };
  auto T = [&](float a, float b, float c, float d, float e, float f, uint32_t col) { g.fillTriangle(x + S(a), y + S(b), x + S(c), y + S(d), x + S(e), y + S(f), col); };
  auto O = [&](float cx, float cy, float r, uint32_t c) { g.fillCircle(x + S(cx), y + S(cy), max(1, S(r)), c); };
  auto Oc = [&](float cx, float cy, float r, uint32_t c) { g.drawCircle(x + S(cx), y + S(cy), max(1, S(r)), c); };
  switch (k) {
    case I_FIN: L(30, 10, 30, 62, (uint32_t)0xf2e6d6); g.fillRect(x + S(32), y + S(10), S(36), S(22), (uint32_t)0xf2e6d6);
      for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) if ((i + j) & 1) g.fillRect(x + S(32 + i * 9), y + S(10 + j * 5.5f), S(9), S(5.5f) + 1, (uint32_t)0x1a1208); break;
    case I_GUERRA: T(14, 35, 40, 20, 40, 50, (uint32_t)0x5aaaff); T(86, 35, 60, 20, 60, 50, (uint32_t)0xff4632); L(42, 30, 58, 26, (uint32_t)0xffd228); L(42, 40, 58, 44, (uint32_t)0xffd228); O(50, 35, 4, (uint32_t)0xffd228); break;
    case I_ELEC: g.fillRoundRect(x + S(30), y + S(34), S(40), S(28), 2, (uint32_t)0x7f77dd); g.fillRect(x + S(40), y + S(30), S(20), S(5), (uint32_t)0x3c3489);
      g.fillRect(x + S(43), y + S(10), S(14), S(22), (uint32_t)0xf2e6d6); Oc(50, 48, 6, (uint32_t)0xf2e6d6); break;
    case I_LLEGA: g.drawEllipse(x + S(80), y + S(35), S(11), S(22), (uint32_t)0x5adc78); g.drawEllipse(x + S(80), y + S(35), S(11) - 1, S(22) - 1, (uint32_t)0x5adc78);
      T(14, 24, 34, 29, 14, 34, (uint32_t)0xff8c00); T(24, 38, 44, 43, 24, 48, (uint32_t)0xff8c00); T(42, 28, 64, 34, 42, 40, (uint32_t)0xffb366); break;
    case I_VECIN: Oc(30, 35, 20, (uint32_t)0x3a2a12); O(30, 35, 6, (uint32_t)0xff8c00); L(36, 33, 70, 18, (uint32_t)0xff8c00, 1.5f); L(36, 37, 72, 50, (uint32_t)0xff8c00, 1.5f); O(76, 16, 5, (uint32_t)0x5adc78); O(78, 52, 5, (uint32_t)0x5adc78); break;
    case I_NUESTRA: L(34, 12, 34, 62, (uint32_t)0xf2e6d6); T(36, 12, 72, 20, 36, 30, (uint32_t)0xff8c00); O(34, 62, 4, (uint32_t)0x5adc78); L(50, 52, 56, 46, (uint32_t)0x5adc78); L(56, 46, 62, 52, (uint32_t)0x5adc78); break;
    case I_HUIDA: T(64, 26, 84, 31, 64, 36, (uint32_t)0xb06a10); T(54, 40, 74, 45, 54, 50, (uint32_t)0xb06a10); L(16, 31, 56, 31, (uint32_t)0x3a2a12, 1.5f); L(16, 45, 46, 45, (uint32_t)0x3a2a12, 1.5f); break;
    case I_OBRA: Oc(56, 38, 20, (uint32_t)0x888780); Oc(56, 38, 18, (uint32_t)0x888780); g.fillRect(x + S(51), y + S(33), S(10), S(10), (uint32_t)0xb4b2a9);
      L(20, 10, 20, 62, (uint32_t)0xffd228); L(20, 12, 44, 12, (uint32_t)0xffd228); L(42, 12, 42, 22, (uint32_t)0x888780, 1); break;
    case I_ESTACION: Oc(50, 35, 22, (uint32_t)0x5aaaff); Oc(50, 35, 21, (uint32_t)0x5aaaff); O(50, 35, 9, (uint32_t)0x5aaaff); L(28, 35, 72, 35, (uint32_t)0x5aaaff, 2); L(50, 13, 50, 57, (uint32_t)0x5aaaff, 2); break;
    case I_RETIRADA: { const float P[][2] = {{50, 8}, {72, 17}, {70, 40}, {62, 54}, {50, 63}, {38, 54}, {30, 40}, {28, 17}, {50, 8}};
      for (int i = 0; i < 8; i++) L(P[i][0], P[i][1], P[i + 1][0], P[i + 1][1], (uint32_t)0xff4632);
      L(51, 12, 45, 28, (uint32_t)0xff4632, 2); L(45, 28, 54, 34, (uint32_t)0xff4632, 2); L(54, 34, 44, 48, (uint32_t)0xff4632, 2); L(44, 48, 50, 61, (uint32_t)0xff4632, 2); } break;
    case I_POBL: O(36, 26, 7, (uint32_t)0xff6ec8); T(24, 54, 36, 36, 48, 54, (uint32_t)0xff6ec8); O(62, 24, 8, (uint32_t)0xffb3e0); T(48, 56, 62, 34, 76, 56, (uint32_t)0xffb3e0); break;
    case I_ACTIV: { const float P[][2] = {{10, 45}, {28, 45}, {36, 22}, {46, 58}, {56, 30}, {64, 45}, {90, 45}};
      for (int i = 0; i < 6; i++) L(P[i][0], P[i][1], P[i + 1][0], P[i + 1][1], (uint32_t)0x5adc78, 3); } break;
    case I_RIVAL: { const float P[][2] = {{14, 54}, {34, 44}, {52, 48}, {70, 26}, {86, 18}};
      for (int i = 0; i < 4; i++) L(P[i][0], P[i][1], P[i + 1][0], P[i + 1][1], (uint32_t)0xff4632, 3); T(80, 14, 90, 16, 86, 25, (uint32_t)0xff4632); } break;
    case I_RESUMEN: L(18, 60, 84, 60, (uint32_t)0x3a2a12, 1); g.fillRect(x + S(24), y + S(38), S(10), S(22), (uint32_t)0x5adc78); g.fillRect(x + S(40), y + S(26), S(10), S(34), (uint32_t)0x2fae55);
      g.fillRect(x + S(56), y + S(32), S(10), S(28), (uint32_t)0x5adc78); g.fillRect(x + S(72), y + S(14), S(10), S(46), (uint32_t)0x1d8a42); break;
    case I_ALMIR: case I_THARG: {
      const float P[][2] = {{50, 10}, {56, 26}, {73, 26}, {59, 36}, {64, 53}, {50, 43}, {36, 53}, {41, 36}, {27, 26}, {44, 26}};
      uint32_t c = k == I_ALMIR ? (uint32_t)0xffd228 : (uint32_t)0x5adc78;
      if (k == I_ALMIR) { for (int i = 0; i < 10; i += 2) T(50, 33, P[i][0], P[i][1], P[(i + 1) % 10][0], P[(i + 1) % 10][1], c);
                          for (int i = 1; i < 10; i += 2) T(50, 33, P[i][0], P[i][1], P[(i + 1) % 10][0], P[(i + 1) % 10][1], c);
                          g.fillRoundRect(x + S(30), y + S(56), S(40), S(6), 1, (uint32_t)0xb06a10); }
      else { for (int i = 0; i < 10; i++) L(P[i][0], P[i][1], P[(i + 1) % 10][0], P[(i + 1) % 10][1], c); O(50, 36, 6, c); } } break;
    default: Oc(50, 35, 16, (uint32_t)0xb06a10); Oc(50, 35, 15, (uint32_t)0xb06a10); O(50, 35, 4, (uint32_t)0xff8c00);
  }
}

// ---------- una pieza ----------
void pinta(LGFX_Sprite &g, const Prim &p, int y, int32_t scroll) {
  const char *t = arena + p.txt;
  switch (p.tipo) {
    case P_TEXTO: {
      int x = p.x;
      if (p.est & 0x80) x = p.x + (p.w - anchoLinea(t, 0, p.f, p.fb)) / 2;
      lineaTexto(g, x, y, t, p.est & 3, p.f, p.fb, p.c1, p.c2, p.c3); } break;
    case P_RECT: g.fillRect(p.x, y, p.w, p.h, p.c1); break;
    case P_MARCO: {
      g.fillRect(p.x, y, p.w, p.h, p.c1);
      for (uint32_t i = 0; i < max<uint32_t>(1, p.c3); i++) g.drawRect(p.x + i, y + i, p.w - 2 * i, p.h - 2 * i, p.c2);
      if (p.est & 1) g.fillRect(p.x, y, 5, p.h, p.c2); } break;
    case P_LINEA: g.fillRect(p.x, y, p.w, p.h, p.c1); break;
    case P_PUNTOS: for (int x = p.x; x < p.x + p.w; x += 4) g.drawFastHLine(x, y, 2, p.c1); break;
    case P_ICONO: icono(g, p.f, p.x, y); break;
    case P_SEMAF: {
      const uint32_t on[3] = {(uint32_t)0x4cd964, (uint32_t)0xffd228, (uint32_t)0xff4632};
      for (int i = 0; i < 3; i++) {
        int cx = p.x + 9 + i * 22, cy = y + 11;
        if (i == p.f) { g.fillCircle(cx, cy, 10, mezcla(C_FONDO, on[i], 0.35f)); g.fillCircle(cx, cy, 7, on[i]); }
        else g.fillCircle(cx, cy, 7, (uint32_t)0x2a2a2a);
      }
      g.setFont(&FU[MONOB].f); g.setTextColor((uint32_t)C_TEXTO2); g.drawString(t, p.x + 72, y + 4); } break;
    case P_INTEL: {
      g.fillRect(p.x, y, p.w, p.h, (uint32_t)0x06121d);
      for (int yy = max(y, 0), b = min(y + p.h, (int)CON_H); yy < b; yy++) if (((yy - y) & 3) >= 2) g.drawFastHLine(p.x, yy, p.w, (uint32_t)0x071725);   // lineas de monitor
      g.drawRect(p.x, y, p.w, p.h, p.c1);
      g.drawRect(p.x - 1, y - 1, p.w + 2, p.h + 2, (uint32_t)0x000000);
      for (int x = p.x + 6; x < p.x + p.w - 6; x += 6) { g.drawFastHLine(x, y + 6, 3, (uint32_t)0x204359); g.drawFastHLine(x, y + p.h - 7, 3, (uint32_t)0x204359); }
      for (int yy = y + 6; yy < y + p.h - 6; yy += 6) { g.drawFastVLine(p.x + 6, yy, 3, (uint32_t)0x204359); g.drawFastVLine(p.x + p.w - 7, yy, 3, (uint32_t)0x204359); } } break;
    case P_ACCESO:
      g.drawRect(p.x, y, p.w, p.h, C_ROJO);
      g.setFont(&FU[MONOB].f); g.setTextColor((uint32_t)C_ROJO); g.drawString(t, p.x + (p.w - g.textWidth(t)) / 2, y + 3); break;
    case P_DIA: {
      g.setFont(&FU[PEQB].f); int tw = g.textWidth(t), cx = p.x + p.w / 2;
      g.setTextColor((uint32_t)p.c1); g.drawString(t, cx - tw / 2, y + 5);
      int l1 = cx - tw / 2 - 14, l2 = cx + tw / 2 + 14, ly = y + 13;
      for (int x = p.x; x < l1; x++) g.drawPixel(x, ly, mezcla(C_FONDO, C_NARANJA, 0.7f * (x - p.x) / (float)max(1, l1 - p.x)));
      for (int x = l2; x < p.x + p.w; x++) g.drawPixel(x, ly, mezcla(C_FONDO, C_NARANJA, 0.7f * (p.x + p.w - x) / (float)max(1, p.x + p.w - l2)));
      g.fillTriangle(l1 + 4, ly, l1 + 8, ly - 4, l1 + 8, ly + 4, C_NAR_OSC); g.fillTriangle(l2 - 4, ly, l2 - 8, ly - 4, l2 - 8, ly + 4, C_NAR_OSC); } break;
    case P_BOTON: {
      bool on = p.est;
      g.fillRoundRect(p.x, y, p.w, p.h, 8, on ? C_AZUL : C_PANEL);
      g.drawRoundRect(p.x, y, p.w, p.h, 8, on ? C_AZUL : C_LINEA);
      g.setFont(&FU[BOT].f); g.setTextColor((uint32_t)(on ? (uint32_t)0x000000 : C_VERDE));
      g.drawString(t, p.x + (p.w - g.textWidth(t)) / 2, y + (p.h - FU[BOT].alto) / 2); } break;
    case P_DOBLE: g.drawFastHLine(p.x, y, p.w, p.c1); g.drawFastHLine(p.x, y + 3, p.w, p.c1); break;
    case P_SELLO:
      g.drawRect(p.x, y, p.w, p.h, p.c1); g.drawRect(p.x + 1, y + 1, p.w - 2, p.h - 2, p.c1);
      g.setFont(&FU[MONOB].f); g.setTextColor((uint32_t)p.c1); g.drawString("URGENTE", p.x + (p.w - g.textWidth("URGENTE")) / 2, y + 4); break;
    case P_CINTA:
      g.fillRect(p.x, y, p.w, p.h, (uint32_t)0xe8dcc0);
      for (int x = p.x; x < p.x + p.w; x += 8) { g.drawPixel(x, y, C_FONDO); g.drawPixel(x + 4, y + p.h - 1, C_FONDO); }
      g.setFont(&FU[MONO].f); g.setTextColor((uint32_t)0x3a2a12); g.drawString(t, p.x + (p.w - g.textWidth(t)) / 2, y + 4); break;
    case P_HOSTIL: {
      bool parpa = (millis() / 800) & 1;
      g.fillRect(p.x, y, p.w, p.h, (uint32_t)0xc40000); g.drawRect(p.x, y, p.w, p.h, (uint32_t)0xffe600); g.drawRect(p.x + 1, y + 1, p.w - 2, p.h - 2, (uint32_t)0xffe600);
      g.setTextColor((uint32_t)(parpa ? (uint32_t)0xffe600 : (uint32_t)0xffffff));
      g.setFont(&FU[MONOB].f); g.drawString(t, p.x + (p.w - g.textWidth(t)) / 2, y + 8);
      const char *t2 = arena + p.c1; g.setFont(&FU[MONO].f); g.drawString(t2, p.x + (p.w - g.textWidth(t2)) / 2, y + 28); } break;
  }
}

// ---------- la ventana de noticias ----------
bool moviendo = false;
void pintaContenido(int32_t scroll) {
  lienzo.fillScreen(C_FONDO);
  for (size_t i = 0; i < nPrims; i++) {
    const Prim &p = prims[i];
    int y = p.y - scroll;
    if (y > CON_H || y + max<int>(p.h, 30) < 0) continue;   // fuera de la ventana
    pinta(lienzo, p, y, scroll);
  }
  if (altoContenido > CON_H) {   // barra de desplazamiento
    int th = max<int>(30, (int64_t)CON_H * CON_H / altoContenido);
    int ty = (int64_t)scroll * (CON_H - th) / (altoContenido - CON_H);
    lienzo.fillRoundRect(ANCHO - 6, ty, 4, th, 2, moviendo ? C_AMARILLO : C_NAR_OSC);
  }
  lienzo.pushSprite(0, CON_Y);
}

// ---------- cabecera: emblemas, titulo, banda HUD y estado ----------
void pintaCabecera() {
  LGFX_Sprite &g = cabS;
  for (int y = 0; y < CAB_H; y++) g.drawFastHLine(0, y, ANCHO, mezcla((uint32_t)0x1a0f02, C_FONDO, y / (float)CAB_H));
  uint32_t sem = 7;   // estrellas fijas
  for (int i = 0; i < 70; i++) {
    sem = sem * 1103515245 + 12345; int x = (sem >> 8) % ANCHO; sem = sem * 1103515245 + 12345; int y = (sem >> 8) % 56;
    sem = sem * 1103515245 + 12345; g.drawPixel(x, y, mezcla(C_FONDO, (sem >> 20) & 1 ? (uint32_t)0xffffff : (uint32_t)0xffd9a0, 0.25f + ((sem >> 9) % 50) / 100.0f));
  }
  emblema(g, 6, 0, EMB_P_COL, EMB_P_ALF, EMB_P_PX, false);
  emblema(g, ANCHO - 6 - EMB_P_PX, 0, EMB_P_COL, EMB_P_ALF, EMB_P_PX, true);
  // titulo con espaciado y brillo naranja
  const char *tit = "LA CRÓNICA DE SION";
  g.setFont(&FU[CABF].f);
  int tw = 0, n = 0; { const char *p = tit; while (*p) { const char *q = p; sigCP(q); tw += g.textWidth(String(p).substring(0, q - p)); p = q; n++; } }
  tw += 4 * (n - 1);
  auto escribe = [&](int dx, int dy, uint32_t c) {
    g.setTextColor(c); int x = (ANCHO - tw) / 2 + dx; const char *p = tit;
    while (*p) { const char *q = p; sigCP(q); String ch = String(p).substring(0, q - p); g.drawString(ch, x, 4 + dy); x += g.textWidth(ch) + 4; p = q; }
  };
  for (int dx = -2; dx <= 2; dx++) for (int dy = -2; dy <= 2; dy++) if (dx || dy) escribe(dx, dy, mezcla((uint32_t)0x1a0f02, C_NARANJA, (abs(dx) + abs(dy) > 2) ? 0.18f : 0.35f));
  escribe(0, 0, C_CREMA);
  g.setFont(&FU[PEQ].f); g.setTextColor((uint32_t)C_NAR_CLA);
  const char *sub = "Diario del BGS de Caballeros de Sion";
  g.drawString(sub, (ANCHO - g.textWidth(sub)) / 2, 41);
  // estado de los datos (a la derecha del subtitulo)
  time_t a = ahora(); String est, edad; uint32_t ce = C_ROSA;
  if (tUltOk && a) { long m = (a - tUltOk) / 60; edad = m < 1 ? String("datos al día") : m < 120 ? "datos de hace " + String(m) + " min" : "datos de hace " + String(m / 60) + " h"; if (m > 60) ce = C_ROJO; }
  if (estadoRed[0]) { est = String(estadoRed); if (edad.length()) est += " · " + edad; ce = C_ROJO; }   // error de red
  else if (edad.length()) est = edad;
  else est = tUltOk ? "datos guardados" : "conectando…";
  g.setFont(&FU[MONO].f); g.setTextColor(ce); g.drawString(est, ANCHO - 80 - g.textWidth(est), 44);
  // banda HUD
  const int y0 = 60, h = 20, x0 = 10, x1 = ANCHO - 10;
  for (int x = x0; x < x1; x++) {
    float t = fabsf((x - ANCHO / 2.0f) / ((x1 - x0) / 2.0f));
    int borde = min(x - x0, x1 - 1 - x); int hh = borde < 10 ? h * borde / 10 : h;
    g.drawFastVLine(x, y0 + (h - hh) / 2, max(1, hh), mezcla(C_NAR_CLA, C_NARANJA, t));
  }
  std::vector<String> I;
  I.push_back("◆ " + (a ? mayusculas(fechaJuego(a)) : String("FECHA GALÁCTICA DESCONOCIDA")));
  if (dParte && !(*dParte)["inicio"].isNull() && a) {   // AÑO y N.º de la web (si el parte trae "inicio")
    String ini = (*dParte)["inicio"] | ""; struct tm ti = {}; ti.tm_year = ini.substring(0, 4).toInt() - 1900; ti.tm_mon = ini.substring(5, 7).toInt() - 1; ti.tm_mday = ini.substring(8, 10).toInt(); ti.tm_hour = 12;
    struct tm hoy = local(a); hoy.tm_hour = 12; hoy.tm_min = hoy.tm_sec = 0;
    long num = lround((mktime(&hoy) - mktime(&ti)) / 86400.0) + 1;
    I.push_back(String("AÑO ") + (hoy.tm_year + 1900 + 1286 - 3311) + " · N.º " + num);
  }
  int hoyN = 0; if (a) { String d = diaISO(a); for (const Not &x : NOT) if (diaISO((time_t)(x.n[0] | 0.0)) == d) hoyN++; }
  I.push_back(String(hoyN) + (hoyN == 1 ? " NOTICIA HOY" : " NOTICIAS HOY"));
  I.push_back("HORA GALÁCTICA " + hora(a));
  if (dFac && a) { long tg = (*dFac)["tick_global"] | 0L; if (tg) I.push_back("TICK EN ~" + String(max(0L, lround((tg + 86400 - a) / 3600.0))) + " H"); }
  g.setFont(&FU[MONOB].f); g.setTextColor((uint32_t)0x140b00);
  int total = 0; for (auto &s : I) total += g.textWidth(s);
  int hueco = (x1 - x0 - 36 - total) / max<int>(1, I.size() - 1), x = x0 + 18;
  for (auto &s : I) { g.drawString(s, x, y0 + 4); x += g.textWidth(s) + hueco; }
  g.pushSprite(0, 0);
}

// ---------- teletipo con los titulares ----------
std::vector<String> TT; std::vector<int> TTw; int ttTotal = 0; float ttX = 0;
void preparaTeletipo() {
  TT.clear(); TTw.clear(); ttTotal = 0;
  int n = 0;
  teleS.setFont(&FU[MONO].f);
  for (const Not &x : NOT) { if (n++ >= 12) break; String t = hastaPunto(sinMarcas(x.n[1] | "")); TT.push_back(t); TTw.push_back(teleS.textWidth(t)); }
  for (int w : TTw) ttTotal += w + 34;   // 34 = separador ◆
}
void pintaTeletipo(float dt) {
  LGFX_Sprite &g = teleS;
  g.fillScreen((uint32_t)0x0b0700);
  uint32_t borde = mezcla((uint32_t)0x0b0700, C_NARANJA, 0.4f);
  g.drawFastHLine(0, 0, ANCHO, borde); g.drawFastHLine(0, TEL_H - 1, ANCHO, borde);
  if (ttTotal) {
    ttX -= dt * 60.0f;   // 60 px por segundo
    if (ttX < -ttTotal) ttX += ttTotal;
    g.setFont(&FU[MONO].f);
    for (int vuelta = 0; vuelta < 2; vuelta++) {
      int x = (int)ttX + vuelta * ttTotal;
      for (size_t i = 0; i < TT.size(); i++) {
        if (x > ANCHO) break;
        if (x + TTw[i] + 34 > 0) {
          g.setTextColor((uint32_t)0xffcf8a); g.drawString(TT[i], x, 5);
          g.setTextColor((uint32_t)C_ROJO); g.drawString("◆", x + TTw[i] + 11, 5);
        }
        x += TTw[i] + 34;
      }
    }
  }
  g.pushSprite(0, TEL_Y);
}

// ---------- presentacion al encender ----------
void presentacion() {
  LGFX_Sprite s(&tft); s.setPsram(true); s.setColorDepth(16);
  if (!s.createSprite(ANCHO, ALTO)) { tft.fillScreen(C_FONDO); return; }
  s.fillScreen(C_FONDO);
  emblema(s, (ANCHO - EMB_G_PX) / 2, 40, EMB_G_COL, EMB_G_ALF, EMB_G_PX, false);
  s.setFont(&FU[CABF].f); s.setTextColor((uint32_t)C_CREMA);
  const char *t = "LA CRÓNICA DE SION"; s.drawString(t, (ANCHO - s.textWidth(t)) / 2, 230);
  s.setFont(&FU[TXT].f); s.setTextColor((uint32_t)C_NAR_CLA);
  t = "by Cmdr Ortem Ed Der"; s.drawString(t, (ANCHO - s.textWidth(t)) / 2, 280);
  s.setFont(&FU[MONO].f); s.setTextColor((uint32_t)C_NAR_OSC);
  String v = String("Pantalla de 5\" · ") + VERSION + " · conectando al WiFi…"; s.drawString(v, (ANCHO - s.textWidth(v)) / 2, 330);
  s.pushSprite(0, 0); s.deleteSprite();
}
