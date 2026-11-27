// modelo.h - los datos de La Cronica en memoria (faccion.json filtrado, noticia_fija.txt y cronica_ia.json)
#pragma once
#include <ArduinoJson.h>

// ArduinoJson en la PSRAM (8 MB): asi cabe todo y no se come la RAM interna
struct PsramAlloc : ArduinoJson::Allocator {
  void *allocate(size_t n) override { return heap_caps_malloc(n, MALLOC_CAP_SPIRAM); }
  void deallocate(void *p) override { heap_caps_free(p); }
  void *reallocate(void *p, size_t n) override { return heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM); }
} psram;

// Datos en uso (solo los toca loop) y los recien bajados (los deja la tarea de red)
JsonDocument *dFac = nullptr, *dParte = nullptr, *dFija = nullptr;
JsonDocument *nFac = nullptr, *nParte = nullptr, *nFija = nullptr;
volatile bool hayNuevo = false;      // la tarea de red dejo datos nuevos
volatile bool pideDescarga = false;  // forzar descarga ya
volatile bool wifiOk = false;
volatile time_t tUltOk = 0;          // ultima descarga buena
char estadoRed[60] = "";            // ultimo error de red ("" = todo bien)

time_t ahora() { time_t t = time(nullptr); return t > 1600000000 ? t : 0; }

// ---------- noticia_fija.txt -> [[titular, texto], ...] (igual que la web) ----------
JsonDocument *parseaFija(const char *txt) {
  JsonDocument *d = new JsonDocument(&psram);
  JsonArray A = d->to<JsonArray>();
  String bloque;
  auto cierra = [&]() {
    bloque.trim();
    if (bloque.length()) {
      int i = bloque.indexOf('\n');
      String tit = i < 0 ? bloque : bloque.substring(0, i), resto = i < 0 ? "" : bloque.substring(i + 1);
      tit.trim(); resto.trim();
      JsonArray n = A.add<JsonArray>(); n.add(tit); n.add(resto);
    }
    bloque = "";
  };
  const char *p = txt;
  while (*p) {
    const char *f = strchr(p, '\n'); size_t len = f ? f - p : strlen(p);
    String lin(p, len); lin.replace("\r", "");
    String t = lin; t.trim();
    if (t.startsWith("---") && t.length() == 3) cierra();
    else if (!t.startsWith("#")) { bloque += lin; bloque += '\n'; }
    p = f ? f + 1 : p + len;
  }
  cierra();
  return d;
}

// lo que se guarda de faccion.json (solo lo de La Cronica; el resto son ~1 MB de sistemas)
JsonDocument *parseaFaccion(const char *buf, size_t len, char *err, size_t nerr) {
  JsonDocument filtro;
  filtro["t"] = true; filtro["tick_global"] = true;
  filtro["noticias"] = true; filtro["fija"] = true; filtro["hemeroteca"] = true;
  JsonDocument *d = new JsonDocument(&psram);
  DeserializationError e = deserializeJson(*d, buf, len, DeserializationOption::Filter(filtro));
  if (e || !(*d)["noticias"].is<JsonArray>()) { snprintf(err, nerr, "JSON: %s", e ? e.c_str() : "sin noticias"); delete d; return nullptr; }
  return d;
}
JsonDocument *parseaParte(const char *buf, size_t len) {
  JsonDocument *d = new JsonDocument(&psram);
  if (deserializeJson(*d, buf, len) || !(*d)["partes"].is<JsonArray>()) { delete d; return nullptr; }
  return d;
}

// Llamar desde loop: cambia a los datos nuevos. Devuelve true si hay que rehacer la maqueta.
bool recogeNuevos() {
  if (!hayNuevo) return false;
  if (nFac) { delete dFac; dFac = nFac; }
  if (nParte) { delete dParte; dParte = nParte; }
  if (nFija) { delete dFija; dFija = nFija; }
  nFac = nParte = nFija = nullptr;
  hayNuevo = false;
  return true;
}
