// maqueta.h - monta La Cronica como una lista de piezas (cajas, lineas de texto, iconos) con su altura.
// Luego dibujo.h pinta solo las piezas que caen en la ventana que se ve, segun el desplazamiento.
#pragma once
#include <vector>

// ---------- piezas ----------
enum : uint8_t { P_TEXTO, P_RECT, P_MARCO, P_RMARCO, P_LINEA, P_PUNTOS, P_ICONO, P_SEMAF, P_INTEL, P_DIA, P_BOTON,
                 P_DOBLE, P_SELLO, P_CINTA, P_HOSTIL, P_ACCESO };
struct Prim {
  int32_t y; int16_t x, w, h;
  uint8_t tipo, f, fb, est;
  uint32_t c1, c2, c3;
  uint32_t txt;   // posicion del texto en la arena
};
// zonas que se pueden tocar
enum : uint8_t { A_SECCION, A_RESUMEN, A_HEME, A_VERANT, A_ANT };
struct Zona { int32_t y0, y1; int16_t x0, x1; uint8_t acc; int16_t id; };

const size_t MAX_PRIMS = 12000, MAX_ZONAS = 1500, ARENA = 600 * 1024;
Prim *prims; size_t nPrims = 0;
Zona *zonas; size_t nZonas = 0;
char *arena; size_t nArena = 0;
int32_t altoContenido = 0;

// estado de lo que se ve
int seccion = 0;                     // 0 = Todas
bool expRes[320];                    // noticias con el resumen abierto
bool expHeme[100];                   // comunicados de la hemeroteca abiertos
bool verAnt = false; int antSel = -1;   // partes anteriores

const char *SECS[][2] = {{"", "Todas"}, {"cronica", "Almirante"}, {"conflictos", "Conflictos"}, {"expansion", "Expansión"},
  {"vecindario", "Vecindario"}, {"obras", "Obras"}, {"estaciones", "Estaciones"}, {"poblacion", "Población"},
  {"actividad", "Actividad"}, {"thargoides", "Thargoides"}, {"resumen", "Resúmenes"}};
const int NSECS = sizeof(SECS) / sizeof(SECS[0]);

void iniciaMaqueta() {
  prims = (Prim *)ps_malloc(MAX_PRIMS * sizeof(Prim));
  zonas = (Zona *)ps_malloc(MAX_ZONAS * sizeof(Zona));
  arena = (char *)ps_malloc(ARENA);
}
uint32_t guardaTxt(const String &s) {
  size_t n = s.length() + 1;
  if (nArena + n > ARENA) return 0;   // la posicion 0 es siempre "" (ver abajo)
  memcpy(arena + nArena, s.c_str(), n); uint32_t r = nArena; nArena += n; return r;
}
Prim &pieza(uint8_t tipo, int x, int32_t y, int w, int h, uint32_t c1 = 0, uint32_t c2 = 0, uint32_t c3 = 0) {
  static Prim basura;
  if (nPrims >= MAX_PRIMS) return basura;
  Prim &p = prims[nPrims++];
  p = Prim{y, (int16_t)x, (int16_t)w, (int16_t)h, tipo, 0, 0, 0, c1, c2, c3, 0};
  return p;
}
void zona(int x0, int32_t y0, int x1, int32_t y1, uint8_t acc, int id) {
  if (nZonas < MAX_ZONAS) zonas[nZonas++] = Zona{y0, y1, (int16_t)x0, (int16_t)x1, acc, (int16_t)id};
}

// ---------- medir y cortar texto (\x01 = negrita, \x02 = sistema en naranja) ----------
int anchoLinea(const char *s, uint8_t est, uint8_t f, uint8_t fb) {
  int w = 0; String seg;
  auto suma = [&]() { if (seg.length()) { tft.setFont(&FU[(est & 1) ? fb : f].f); w += tft.textWidth(seg); seg = ""; } };
  for (const char *p = s; *p; p++) {
    if (*p == '\x01') { suma(); est ^= 1; }
    else if (*p == '\x02') { suma(); est ^= 2; }
    else seg += *p;
  }
  suma(); return w;
}
uint8_t estadoTras(const String &s, uint8_t est) {
  for (unsigned i = 0; i < s.length(); i++) { if (s[i] == '\x01') est ^= 1; else if (s[i] == '\x02') est ^= 2; }
  return est;
}
// Escribe el texto cortado en lineas de ancho w. Devuelve la y de debajo.
int32_t texto(int x, int32_t y, int w, uint8_t f, uint8_t fb, uint32_t c, uint32_t cb, uint32_t ch, const String &s, int extra = 3) {
  int lh = max(FU[f].alto, FU[fb].alto) + extra;
  uint8_t est = 0, estLin = 0;
  String lin;
  auto suelta = [&]() {
    Prim &p = pieza(P_TEXTO, x, y, w, lh, c, cb, ch);
    p.f = f; p.fb = fb; p.est = estLin; p.txt = guardaTxt(lin);
    y += lh; lin = "";
  };
  const char *p = s.c_str();
  while (true) {
    const char *q = p; while (*q && *q != ' ' && *q != '\n') q++;
    String pal(p, q - p);
    if (pal.length()) {
      String cand = lin.length() ? lin + " " + pal : pal;
      if (lin.length() && anchoLinea(cand.c_str(), estLin, f, fb) > w) {
        suelta(); estLin = est; lin = pal;
      } else {
        if (!lin.length()) estLin = est;
        lin = cand;
      }
      est = estadoTras(pal, est);
    }
    if (*q == '\n') {
      if (lin.length()) suelta(); else y += lh / 2;   // linea en blanco = medio renglon
      estLin = est; p = q + 1;
    } else if (*q == ' ') p = q + 1;
    else { if (lin.length()) suelta(); break; }
  }
  return y;
}
int anchoTxt(uint8_t f, const String &s) { tft.setFont(&FU[f].f); return tft.textWidth(s); }

// ---------- fechas (hora de Espana) ----------
const char *DIAS[] = {"domingo", "lunes", "martes", "miércoles", "jueves", "viernes", "sábado"};
const char *MESES[] = {"enero", "febrero", "marzo", "abril", "mayo", "junio", "julio", "agosto", "septiembre", "octubre", "noviembre", "diciembre"};
struct tm local(time_t t) { struct tm r; localtime_r(&t, &r); return r; }
String hora(time_t t) { if (!t) return "—"; struct tm l = local(t); char b[8]; snprintf(b, 8, "%02d:%02d", l.tm_hour, l.tm_min); return b; }
String fechaCorta(time_t t) { struct tm l = local(t); char b[12]; snprintf(b, 12, "%02d/%02d/%02d", l.tm_mday, l.tm_mon + 1, l.tm_year % 100); return b; }
String diaISO(time_t t) { struct tm l = local(t); char b[12]; snprintf(b, 12, "%04d-%02d-%02d", l.tm_year + 1900, l.tm_mon + 1, l.tm_mday); return b; }
String diaLargo(time_t t) { struct tm l = local(t); return String(DIAS[l.tm_wday]) + ", " + l.tm_mday + " de " + MESES[l.tm_mon]; }
String fechaJuego(time_t t) { struct tm l = local(t); return diaLargo(t) + " de " + (l.tm_year + 1900 + 1286); }
String diaTit(time_t t) {
  time_t a = ahora();
  if (a) { String d = diaISO(t); if (d == diaISO(a)) return "Hoy"; if (d == diaISO(a - 86400)) return "Ayer"; }
  String s = diaLargo(t); s.setCharAt(0, toupper(s[0])); return s;
}

// ---------- noticias (igual que la web) ----------
struct Not { JsonArray n; const char *sec; bool urg; int idx; };
std::vector<Not> NOT;   // la mas nueva primero
bool empiezaPor(const char *t, const char *u) { return t && strncmp(t, u, strlen(u)) == 0; }
void leeNoticias() {
  NOT.clear(); if (!dFac) return;
  JsonArray A = (*dFac)["noticias"];
  int i = 0;
  for (JsonArray n : A) {
    const char *sec = n[2] | "", *t = n[1] | "";
    if (!strcmp(sec, "escucha")) { i++; continue; }
    bool urg = (n[3] | 0) != 0;
    if (!strcmp(sec, "expansion") && empiezaPor(t, "\xF0\x9F\x9B\xB8")) { sec = "vecindario"; urg = false; }   // 🛸 = de otras facciones
    NOT.push_back(Not{n, sec, urg, i++});
  }
  std::reverse(NOT.begin(), NOT.end());
}
uint32_t primerCP(const char *t) { const char *p = t; return (t && *t) ? sigCP(p) : 0; }
// tipo de dibujo de cada noticia (tipoNot de la web)
enum { I_FIN, I_GUERRA, I_ELEC, I_LLEGA, I_VECIN, I_NUESTRA, I_HUIDA, I_OBRA, I_ESTACION, I_RETIRADA, I_POBL, I_ACTIV, I_RIVAL, I_RESUMEN, I_ALMIR, I_THARG, I_OTRA };
int tipoNot(const Not &x) {
  const char *t = x.n[1] | ""; const char *s = x.sec; uint32_t i = primerCP(t);
  if (!strcmp(s, "cronica")) return I_ALMIR;
  if (!strcmp(s, "resumen")) return I_RESUMEN;
  if (!strcmp(s, "vecindario")) return I_VECIN;
  if (!strcmp(s, "thargoides")) return I_THARG;
  if (i == 0x1F6EC) return I_LLEGA;
  if (i == 0x1F3C1) return I_FIN;
  if (i == 0x2709) return I_ELEC;
  if ((i == 0x1F6A8 && strstr(t, "Retirada")) || strstr(t, "ya no está") || strstr(t, "Riesgo de retirada")) return I_RETIRADA;
  if (!strcmp(s, "conflictos") && i != 0x1F4C9 && i != 0x2B06 && i != 0x1F53B && i != 0x1F575) return strstr(t, "Elecciones") ? I_ELEC : I_GUERRA;
  if (i == 0x1F195 || (i == 0x1F4E3 && strstr(t, "Expansión"))) return I_NUESTRA;
  if (i == 0x1F3C3) return I_HUIDA;
  if (!strcmp(s, "obras")) return I_OBRA;
  if (!strcmp(s, "estaciones")) return I_ESTACION;
  if (!strcmp(s, "poblacion")) return I_POBL;
  if (!strcmp(s, "actividad")) return I_ACTIV;
  if (!strcmp(s, "conflictos")) return I_RIVAL;
  if (!strcmp(s, "expansion")) return I_NUESTRA;
  return I_OTRA;
}
uint32_t colorSec(const char *s) {
  if (!strcmp(s, "conflictos")) return 0xff4632; if (!strcmp(s, "expansion")) return 0x4cd964;
  if (!strcmp(s, "obras")) return 0xffd228;      if (!strcmp(s, "estaciones")) return 0x66cbff;
  if (!strcmp(s, "poblacion")) return 0xc98bff;  if (!strcmp(s, "actividad")) return 0xff8c00;
  if (!strcmp(s, "vecindario")) return 0x3b82f6; if (!strcmp(s, "resumen")) return 0xe0d2bf;
  if (!strcmp(s, "thargoides")) return 0x2ef2a0; if (!strcmp(s, "cronica")) return 0xffd228;
  return C_LINEA;
}
String accion(const Not &x, int k) {   // que hacer para ayudar (la web, sin la ficha del sistema)
  String s = x.n[4].is<const char *>() ? String("**") + (const char *)(x.n[4] | "") + "**" : String("el sistema");
  switch (k) {
    case I_GUERRA:   return "⚔ Hoy cuenta: bonos de combate en las zonas de conflicto de " + s;
    case I_ELEC:     return "✉ Hoy cuenta: misiones, comercio y datos para CDSN en " + s + " (sin combate)";
    case I_RETIRADA: return "Necesita ayuda: misiones de CDSN, comercio con beneficio y datos de exploración en " + s;
    case I_LLEGA:    return "Defiende " + s + ": misiones de CDSN para no perder terreno frente al recién llegado";
    case I_VECIN:    return "Atentos: si llega a un sistema nuestro, toca reforzarlo";
    case I_RESUMEN:  return "Mira en la web (pestaña Consejos) dónde hace más falta hoy";
  }
  return "";
}

// ---------- parte del dia (IA) ----------
struct ParteHoy { JsonObject sel; String fecha; bool hay = false; };
bool visible(JsonObject p) { time_t a = ahora(); return !a || (time_t)(p["publicar_t"] | 0.0) <= a; }
ParteHoy parteHoy() {
  ParteHoy r; if (!dParte) return r;
  JsonArray P = (*dParte)["partes"]; const char *mod = (*dParte)["modelo"] | "";
  for (JsonObject p : P) if (visible(p)) { String f = p["fecha"] | ""; if (f > r.fecha) r.fecha = f; }
  if (!r.fecha.length()) return r;
  JsonObject primero;
  for (JsonObject p : P) if (visible(p) && r.fecha == (p["fecha"] | "")) {
    if (primero.isNull()) primero = p;
    if (!strcmp(p["modelo"] | "", mod)) { r.sel = p; break; }
  }
  if (r.sel.isNull()) r.sel = primero;
  r.hay = true; return r;
}
bool parteRoto(JsonObject p) {
  if (p["roto"] | false) return true;
  if (p["perdido"] | false) return false;
  const char *e = p["entradilla"] | "";
  while (*e == ' ' || *e == '\n') e++;
  return *e == '{' || (!p["puntos"].as<JsonArray>().size() && !p["hacer"].as<JsonArray>().size());
}
const char *nombreModelo(const char *m) {
  if (!strcmp(m, "haiku")) return "Claude Haiku 5.5"; if (!strcmp(m, "sonnet")) return "Claude Sonnet 5.5";
  if (!strcmp(m, "opus")) return "Claude Opus 5.5"; return m;
}
String miles(long v) {   // 47192 -> 47.192
  String s = String(v), r; int n = s.length();
  for (int i = 0; i < n; i++) { r += s[i]; if ((n - i - 1) % 3 == 0 && i < n - 1) r += '.'; }
  return r;
}
String ruido(const String &t) {   // el parte roto se ve "interferido"
  String r; uint32_t sem = 12345;
  for (unsigned i = 0; i < t.length(); i++) {
    char c = t[i]; sem = sem * 1103515245 + 12345;
    if (strchr("aeiosrnt", c) && ((sem >> 16) % 10) == 0) c = "#%&@$"[(sem >> 8) % 5];
    r += c;
  }
  return r;
}
int32_t cuerpoParte(int x, int32_t y, int w, JsonObject p) {
  const char *mod = p["modelo"] | "";
  String pie = String((p["perdido"] | false) ? "Sin respuesta de " : "Recibido de ") + nombreModelo(mod) + " · ";
  if (!p["al_dia"].isNull()) pie += String("sistemas con dato nuevo tras el tick: ") + (int)(p["al_dia"] | 0) + " de " + (int)(p["ctrl"] | 0) + " · ";
  pie += miles(p["tokens_in"] | 0L) + " + " + miles(p["tokens_out"] | 0L) + " tokens";
  JsonArray dud = p["dudosos"];
  if (dud.size()) { pie += " · ⚠ sistemas dudosos: "; bool pr = true; for (const char *d : dud) { if (!pr) pie += ", "; pie += d; pr = false; } }
  auto ponPie = [&]() { return texto(x, y + 4, w, PEQ, PEQ, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, limpia(pie.c_str())); };
  auto hostil = [&](const char *a, const char *b) {
    Prim &h = pieza(P_HOSTIL, x, y, w, 50); h.txt = guardaTxt(a); h.f = 0; h.c1 = guardaTxt(b); y += 58;
  };
  if (p["perdido"] | false) { hostil("TRANSMISIÓN PERDIDA", "NO SE RECIBIÓ EL PARTE DE HOY"); y = ponPie(); return y; }
  if (parteRoto(p)) {
    hostil("⚠ TRANSMISIÓN INTERCEPTADA POR FUERZAS HOSTILES ⚠", "POSIBLES DATOS ERRÓNEOS");
    String tit = p["titular"] | "", ent = p["entradilla"] | "";
    const char *e = ent.c_str(); while (*e == ' ') e++;
    if (tit.length() && !tit.equalsIgnoreCase("parte del día") && *e != '{') {
      y = texto(x, y, w, TIT, TIT, C_TEXTO, C_TEXTO, C_NARANJA, mayusculas(limpia(tit.c_str()))) + 2;
      y = texto(x, y, w, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, limpia(ent.c_str())) + 4;
    }
    String bruto = p["bruto"] | ent.c_str(); if (bruto.length() > 420) bruto = bruto.substring(0, 420);
    y = texto(x, y, w, MONO, MONO, 0x99ffee, 0x99ffee, 0x99ffee, ruido(limpia(bruto.c_str())) + "…[señal perdida]") + 6;
    Prim &c = pieza(P_CINTA, x, y, w, 22); c.txt = guardaTxt("Esta transcripción puede tener errores de desencriptado · STOP"); y += 26;
    return ponPie();
  }
  y = texto(x, y, w, TIT, TIT, C_TEXTO, C_TEXTO, C_NARANJA, mayusculas(limpia(p["titular"] | ""))) + 2;
  const char *r = p["riesgo"] | "";
  if (*r) { Prim &s = pieza(P_SEMAF, x, y, 240, 22); s.f = !strcmp(r, "alto") ? 2 : !strcmp(r, "medio") ? 1 : 0; s.txt = guardaTxt(mayusculas(String("RIESGO ") + r)); y += 26; }
  y = texto(x, y, w, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, limpia(p["entradilla"] | "")) + 6;
  for (JsonObject q : p["puntos"].as<JsonArray>()) {
    y = texto(x, y, w, TXTB, TXTB, C_AMARILLO, C_AMARILLO, C_NARANJA, "▸ " + limpia(q["titulo"] | ""));
    y = texto(x + 14, y, w - 14, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, limpia(q["texto"] | "")) + 6;
  }
  JsonArray hac = p["hacer"];
  if (hac.size()) {
    y = texto(x, y + 2, w, TXTB, TXTB, C_VERDE2, C_VERDE2, C_VERDE2, "QUÉ HACER HOY");
    int k = 1;
    for (const char *h : hac) y = texto(x + 14, y, w - 14, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, String(k++) + ". " + limpia(h)) + 2;
    y += 4;
  }
  if (p["oportunidad"].is<const char *>()) {
    y = texto(x, y + 2, w, TXTB, TXTB, C_CIAN, C_CIAN, C_CIAN, "OPORTUNIDAD");
    y = texto(x, y, w, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, limpia(p["oportunidad"] | "")) + 6;
  }
  Prim &c = pieza(P_CINTA, x, y, w, 22); c.txt = guardaTxt("Esta transcripción puede tener errores de desencriptado · STOP"); y += 26;
  return ponPie();
}
int32_t informe(int X, int32_t y, int W) {
  ParteHoy H = parteHoy(); if (!H.hay) return y;
  JsonObject s = H.sel;
  time_t pub = (time_t)(s["publicar_t"] | 0.0), a = ahora();
  bool tarde = a && H.fecha < diaISO(a) && a > pub + 86400;
  size_t ic = nPrims; Prim &caja = pieza(P_INTEL, X, y, W, 0, C_CIAN);
  int x = X + 14, w = W - 28; int32_t y0 = y; y += 10;
  String cab = "INFORME DE INTELIGENCIA · " + mayusculas(diaTit(pub));
  if (!s["numero"].isNull()) cab += String(" · N.º ") + (int)(s["numero"] | 0);
  Prim &ac = pieza(P_ACCESO, X + W - 14 - 190, y - 1, 190, 20); ac.txt = guardaTxt("ACCESO: ALMIRANTAZGO");
  y = texto(x, y, w - 200, MONOB, MONOB, C_CIAN, C_CIAN, C_CIAN, cab);
  if (tarde) y = texto(x, y, w, MONOB, MONOB, C_AMARILLO, C_AMARILLO, C_AMARILLO, "EL DE HOY LLEGA CON RETRASO");
  pieza(P_LINEA, x, y + 3, w, 1, 0x2b5670); y += 10;
  y = cuerpoParte(x, y, w, s);
  // partes anteriores (hasta 14 dias)
  std::vector<String> ant;
  for (JsonObject p : (*dParte)["partes"].as<JsonArray>()) {
    if (!visible(p)) continue; String f = p["fecha"] | "";
    if (f != H.fecha && std::find(ant.begin(), ant.end(), f) == ant.end()) ant.push_back(f);
  }
  std::sort(ant.begin(), ant.end(), [](const String &a, const String &b) { return a > b; });
  if (ant.size() > 14) ant.resize(14);
  if (ant.size()) {
    int32_t yz = y + 4;
    y = texto(x, yz, w, PEQB, PEQB, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, String(verAnt ? "▾" : "▸") + " Partes anteriores (" + ant.size() + ")");
    zona(X, yz - 4, X + W, y + 4, A_VERANT, 0);
    if (verAnt) for (size_t i = 0; i < ant.size(); i++) {
      JsonObject p; const char *mod = (*dParte)["modelo"] | "";
      for (JsonObject q : (*dParte)["partes"].as<JsonArray>()) if (visible(q) && ant[i] == (q["fecha"] | "")) { if (p.isNull()) p = q; if (!strcmp(q["modelo"] | "", mod)) { p = q; break; } }
      String f = ant[i]; String fe = f.substring(8, 10) + "/" + f.substring(5, 7) + "/" + f.substring(0, 4);
      int32_t y1 = y + 4;
      y = texto(x + 8, y1, w - 8, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, String(antSel == (int)i ? "▾ " : "▸ ") + "\x01" + fe + "\x01 · " + limpia(p["titular"] | ""));
      zona(X, y1 - 2, X + W, y + 2, A_ANT, i);
      if (antSel == (int)i) y = cuerpoParte(x + 16, y + 6, w - 16, p) + 4;
    }
  }
  y += 12;
  prims[ic].h = y - y0;
  (void)caja;
  return y + 10;
}

// ---------- la maqueta entera ----------
int32_t noticia(const Not &x, int X, int32_t y, int W) {
  int k = tipoNot(x);
  bool gr = k == I_GUERRA || k == I_ELEC || k == I_RETIRADA || k == I_LLEGA || k == I_VECIN;
  size_t ib = nPrims; pieza(P_RECT, X, y, 3, 0, colorSec(x.sec));
  int32_t y0 = y;
  Prim &h = pieza(P_TEXTO, X + 10, y + 2, 60, 18, C_ROSA, C_ROSA, C_ROSA); h.f = h.fb = MONO; h.txt = guardaTxt(hora((time_t)(x.n[0] | 0.0)));
  Prim &ic = pieza(P_ICONO, X + 10, y + 20, 40, 28); ic.f = k;
  int tx = X + 76, tw = W - 76 - (x.urg ? 96 : 4);
  if (x.urg) pieza(P_SELLO, X + W - 92, y + 2, 86, 20, C_ROJO);
  uint32_t col = x.urg ? C_URG_TXT : C_TEXTO;
  int32_t yt = texto(tx, y, tw, TXT, TXTB, col, col, C_NARANJA, limpia(x.n[1] | "", true));
  if (gr || k == I_RESUMEN) { String a = accion(x, k); if (a.length()) yt = texto(tx, yt + 2, W - 76 - 4, PEQ, PEQB, C_VERDE, C_VERDE, C_VERDE, limpia(a.c_str())); }
  if (x.n[5].is<const char *>() && x.idx < 320) {
    int32_t yz = yt + 2;
    yt = texto(tx, yz, W - 80, PEQB, PEQB, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, expRes[x.idx] ? "▾ ocultar resumen" : "▸ ver resumen");
    if (expRes[x.idx]) yt = texto(tx, yt + 2, W - 80, PEQ, PEQB, C_TEXTO2, C_TEXTO2, C_NARANJA, limpia(x.n[5] | "")) + 2;
    zona(X, y0, X + W, yt + 4, A_RESUMEN, x.idx);
  }
  y = max(yt, y0 + 50) + 6;
  prims[ib].h = y - y0 - 4;
  return y + 6;
}

struct Fija { String tit, txt; };
std::vector<Fija> fijas() {   // noticia_fija.txt; si no se pudo leer, la que trae faccion.json
  std::vector<Fija> F;
  if (dFija) for (JsonArray n : dFija->as<JsonArray>()) F.push_back(Fija{n[0] | "", n[1] | ""});
  else if (dFac) for (JsonArray n : (*dFac)["fija"]["n"].as<JsonArray>()) F.push_back(Fija{n[0] | "", n[1] | ""});
  return F;
}

void maqueta() {
  nPrims = nZonas = 0; nArena = 0; guardaTxt("");
  const int X = MARGEN, W = ANCHO - 2 * MARGEN - 6;
  int32_t y = 10;
  if (!dFac) {
    y = texto(X, 120, W, TIT, TIT, C_NAR_CLA, C_NAR_CLA, C_NAR_CLA, "Descargando La Crónica…");
    y = texto(X, y + 6, W, TXT, TXTB, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, String(estadoRed)) + 20;
    altoContenido = y; return;
  }
  leeNoticias();
  time_t a = ahora();
  const char *cs = SECS[seccion][0];
  if (!seccion) {
    // ULTIMA HORA: urgentes de las ultimas 24 h (no del vecindario), como mucho 2
    int u = 0;
    for (const Not &x : NOT) {
      if (u >= 2) break;
      if (!x.urg || !strcmp(x.sec, "vecindario") || (a && a - (time_t)(x.n[0] | 0.0) >= 86400)) continue;
      size_t ib = nPrims; pieza(P_MARCO, X, y, W, 0, 0x2a0503, C_ROJO, 1); prims[ib].est = 1;   // est 1 = barra gruesa a la izquierda
      String t = "\x01ÚLTIMA HORA\x01  " + hora((time_t)(x.n[0] | 0.0)) + "  " + hastaPunto(sinMarcas(x.n[1] | ""));
      if (x.n[4].is<const char *>()) t += "  \x02· " + String((const char *)(x.n[4] | "")) + "\x02";
      int32_t y2 = texto(X + 14, y + 6, W - 24, TXT, TXTB, C_TEXTO, C_ROJO, C_NAR_OSC, t) + 4;
      prims[ib].h = y2 - y; y = y2 + 8; u++;
    }
    y = informe(X, y, W);
  }
  // botones de seccion
  { int bx = X; int32_t by = y;
    for (int i = 0; i < NSECS; i++) {
      int bw = anchoTxt(BOT, SECS[i][1]) + 26;
      if (bx + bw > X + W) { bx = X; by += 38; }
      Prim &b = pieza(P_BOTON, bx, by, bw, 32); b.est = (i == seccion); b.txt = guardaTxt(SECS[i][1]);
      zona(bx, by - 3, bx + bw, by + 35, A_SECCION, i);
      bx += bw + 6;
    }
    y = by + 32 + 12; }
  // lista de noticias por dias
  String dia; int n = 0;
  for (const Not &x : NOT) {
    if (*cs ? strcmp(x.sec, cs) : !strcmp(x.sec, "vecindario")) continue;   // "Todas" = lo nuestro
    String d = diaTit((time_t)(x.n[0] | 0.0));
    if (d != dia) { dia = d; Prim &p = pieza(P_DIA, X, y, W, 26, C_NAR_CLA); p.txt = guardaTxt(d); y += 32; }
    y = noticia(x, X, y, W); n++;
  }
  if (!n) y = texto(X, y + 10, W, TXT, TXTB, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, "Sin noticias en esta sección.") + 16;
  // reportaje (la noticia fija), al final como en la web
  std::vector<Fija> F = fijas();
  if (!seccion) for (Fija &f : F) {
    size_t ib = nPrims; pieza(P_MARCO, X, y, W, 0, 0x1d1606, C_AMARILLO, 2);
    int32_t y0 = y; y += 12;
    y = texto(X + 16, y, W - 32, MONOB, MONOB, C_AMARILLO, C_AMARILLO, C_AMARILLO, "◆ REPORTAJE") + 2;
    y = texto(X + 16, y, W - 32, TIT, TIT, C_TEXTO, C_TEXTO, C_NARANJA, mayusculas(limpia(f.tit.c_str()))) + 4;
    if (f.txt.length()) y = texto(X + 16, y, W - 32, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, limpia(f.txt.c_str()));
    y += 12; prims[ib].h = y - y0; y += 14;
  }
  // hemeroteca
  y += 8; pieza(P_DOBLE, X, y, W, 4, C_NAR_OSC); y += 12;
  { Prim &t = pieza(P_TEXTO, X, y, W, 20, C_AMARILLO, C_AMARILLO, C_AMARILLO); t.f = t.fb = MONOB; t.est = 0x80;   // 0x80 = centrado
    t.txt = guardaTxt("HEMEROTECA · Comunicados del Almirante"); y += 28; }
  time_t desde = dFac ? (time_t)((*dFac)["fija"]["desde"] | 0.0) : 0;
  for (Fija &f : F) {
    int32_t y0 = y;
    texto(X, y, 120, MONO, MONO, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, desde ? "AHORA\ndesde " + fechaCorta(desde) : String("AHORA"));
    y = texto(X + 128, y, W - 128, TXTB, TXTB, C_TEXTO, C_TEXTO, C_NARANJA, limpia(f.tit.c_str()));
    y = max(y, y0 + 34) + 6; pieza(P_PUNTOS, X, y, W, 1, 0x3a2a12); y += 8;
  }
  int k = 0;
  for (JsonObject h : (*dFac)["hemeroteca"].as<JsonArray>()) {
    int32_t y0 = y;
    texto(X, y, 120, MONO, MONO, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, fechaCorta((time_t)(h["desde"] | 0.0)) + "\n– " + fechaCorta((time_t)(h["hasta"] | 0.0)));
    y = texto(X + 128, y, W - 128, TXTB, TXTB, C_TEXTO, C_TEXTO, C_NARANJA, limpia(h["tit"] | ""));
    bool ab = k < 100 && expHeme[k];
    y = texto(X + 128, y + 2, W - 128, PEQB, PEQB, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, ab ? "▾ cerrar" : "▸ leer");
    if (ab) { String t = limpia(h["txt"] | ""); y = texto(X + 128, y + 4, W - 128, TXT, TXTB, C_TEXTO2, C_TEXTO2, C_NARANJA, t.length() ? t : String("(sin texto)")); }
    y = max(y, y0 + 34) + 6;
    zona(X, y0 - 4, X + W, y, A_HEME, k);
    pieza(P_PUNTOS, X, y, W, 1, 0x3a2a12); y += 8; k++;
  }
  if (!F.size() && !k) y = texto(X, y, W, PEQ, PEQ, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC, "Aún no hay comunicados guardados.") + 6;
  // pie
  y += 16;
  { Prim &t = pieza(P_TEXTO, X, y, W, 18, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC); t.f = t.fb = MONO; t.est = 0x80;
    t.txt = guardaTxt(String("Las ") + (*dFac)["noticias"].as<JsonArray>().size() + " últimas noticias del escucha de EDDN"); y += 24; }
  pieza(P_DOBLE, X + W / 4, y, W / 2, 4, C_NAR_OSC); y += 10;
  { Prim &t = pieza(P_TEXTO, X, y, W, 18, C_NAR_CLA, C_NAR_CLA, C_NAR_CLA); t.f = t.fb = MONOB; t.est = 0x80;
    t.txt = guardaTxt("REDACCIÓN: ESCUCHAS DE SION · IMPRESO EN MELVIN CITY · NDABANGLUYA"); y += 22; }
  { Prim &t = pieza(P_TEXTO, X, y, W, 18, C_NAR_OSC, C_NAR_OSC, C_NAR_OSC); t.f = t.fb = MONO; t.est = 0x80;
    t.txt = guardaTxt("La Crónica en pantalla de 5\" " VERSION " · Cmdr Ortem Ed Der"); y += 30; }
  altoContenido = y;
  Serial.printf("Maqueta: %u piezas, %u zonas, %u bytes de texto, alto %d\n", (unsigned)nPrims, (unsigned)nZonas, (unsigned)nArena, (int)altoContenido);
}
