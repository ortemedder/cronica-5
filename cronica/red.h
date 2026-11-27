// red.h - descarga de cdsn-datos en segundo plano (nucleo 0), copia en la memoria de la placa (FFat)
// para arrancar sin WiFi, y la hora (NTP)
#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <FFat.h>

// ---------- descarga a un bufer en la PSRAM ----------
char *baja(const char *url, size_t &len, char *err, size_t nerr) {
  WiFiClientSecure cli; cli.setInsecure();
  HTTPClient http; http.useHTTP10(true); http.setTimeout(20000);
  String u = String(url) + "?t=" + String((uint32_t)millis());   // sin cache
  if (!http.begin(cli, u)) { snprintf(err, nerr, "URL"); return nullptr; }
  int cod = http.GET();
  if (cod != 200) { snprintf(err, nerr, "HTTP %d", cod); http.end(); return nullptr; }
  int total = http.getSize();
  size_t cap = total > 0 ? total + 1 : 3 * 1024 * 1024;
  char *buf = (char *)ps_malloc(cap);
  if (!buf) { snprintf(err, nerr, "sin memoria"); http.end(); return nullptr; }
  WiFiClient *s = http.getStreamPtr();
  size_t n = 0; uint32_t t = millis();
  while ((http.connected() || s->available()) && (total < 0 || n < (size_t)total) && n < cap - 1) {
    int a = s->available();
    if (a > 0) {
      if ((size_t)a > cap - 1 - n) a = cap - 1 - n;
      n += s->readBytes(buf + n, a); t = millis();
    } else {
      if (millis() - t > 15000) break;   // 15 s sin datos = corte
      delay(2);
    }
  }
  http.end();
  buf[n] = 0; len = n;
  if (total > 0 && n < (size_t)total) { snprintf(err, nerr, "cortado %u/%d", (unsigned)n, total); free(buf); return nullptr; }
  return buf;
}

void guarda(const char *ruta, const char *buf, size_t len) {
  File f = FFat.open(ruta, FILE_WRITE); if (!f) return;
  f.write((const uint8_t *)buf, len); f.close();
}
char *leeArchivo(const char *ruta, size_t &len) {
  File f = FFat.open(ruta, FILE_READ); if (!f) return nullptr;
  len = f.size(); char *b = (char *)ps_malloc(len + 1);
  if (b) { f.read((uint8_t *)b, len); b[len] = 0; }
  f.close(); return b;
}

// Al arrancar: lo ultimo que se guardo (por si no hay WiFi)
void cargaCache() {
  char err[40]; size_t n;
  char *b = leeArchivo("/faccion.json", n);
  if (b) { dFac = parseaFaccion(b, n, err, sizeof err); free(b); }
  b = leeArchivo("/parte.json", n);
  if (b) { dParte = parseaParte(b, n); free(b); }
  b = leeArchivo("/fija.txt", n);
  if (b) { dFija = parseaFija(b); free(b); }
  if (dFac) tUltOk = (time_t)((*dFac)["t"] | 0.0);   // la edad de los datos guardados
  Serial.printf("Cache: faccion %s, parte %s, fija %s\n", dFac ? "si" : "no", dParte ? "si" : "no", dFija ? "si" : "no");
}

bool conectaWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  WiFi.mode(WIFI_STA); WiFi.setSleep(false);
  WiFi.begin(WIFI_NOMBRE, WIFI_CLAVE);
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) delay(250);
  wifiOk = WiFi.status() == WL_CONNECTED;
  if (wifiOk) { configTzTime(ZONA_HORARIA, "pool.ntp.org", "time.google.com"); delay(300); }
  return wifiOk;
}

bool descargaTodo() {
  char err[48] = "";
  if (!conectaWifi()) { strcpy(estadoRed, "sin WiFi"); return false; }
  for (int i = 0; i < 40 && !ahora(); i++) delay(250);   // espera la hora (NTP) la primera vez
  size_t n;
  JsonDocument *fi = nullptr, *pa = nullptr, *fa = nullptr;
  char *b = baja(URL_FIJA, n, err, sizeof err);
  if (b) { fi = parseaFija(b); guarda("/fija.txt", b, n); free(b); }
  b = baja(URL_PARTE, n, err, sizeof err);
  if (b) { pa = parseaParte(b, n); if (pa) guarda("/parte.json", b, n); free(b); }
  uint32_t t0 = millis();
  b = baja(URL_FACCION, n, err, sizeof err);
  if (b) {
    fa = parseaFaccion(b, n, err, sizeof err); free(b);
    if (fa) {   // se guarda solo lo filtrado (~100 KB)
      size_t m = measureJson(*fa); char *s = (char *)ps_malloc(m + 1);
      if (s) { serializeJson(*fa, s, m + 1); guarda("/faccion.json", s, m); free(s); }
    }
  }
  Serial.printf("Descarga faccion.json: %s (%u ms), PSRAM libre %u, RAM libre %u\n", fa ? "OK" : err,
                (unsigned)(millis() - t0), ESP.getFreePsram(), ESP.getFreeHeap());
  if (!fa && !pa && !fi) { snprintf(estadoRed, sizeof estadoRed, "error: %s", err); return false; }
  while (hayNuevo) delay(50);   // que loop haya recogido lo anterior
  nFac = fa; nParte = pa; nFija = fi;
  hayNuevo = true;
  if (fa) { tUltOk = ahora(); estadoRed[0] = 0; }
  else snprintf(estadoRed, sizeof estadoRed, "faccion.json: %s", err);
  return fa != nullptr;
}

// Tarea de red, en el nucleo 0 (la pantalla y el tactil van en el 1, sin pararse)
void tareaRed(void *) {
  uint32_t proxima = 0;
  for (;;) {
    if (pideDescarga || (int32_t)(millis() - proxima) >= 0) {
      pideDescarga = false;
      bool ok = descargaTodo();
      proxima = millis() + (ok ? CADA_MS : REINTENTO_MS);
    }
    wifiOk = WiFi.status() == WL_CONNECTED;
    delay(500);
  }
}

