// La Cronica de Sion en la pantalla de 5" (placa ESP32-8048S050C: ESP32-S3 N16R8, 800x480, tactil GT911)
// Lee de GitHub ortemedder/cdsn-datos: faccion.json (noticias, noticia fija, hemeroteca), noticia_fija.txt y cronica_ia.json.
// Placa en el IDE: "ESP32S3 Dev Module", paquete esp32 2.0.17, PSRAM "OPI PSRAM", Flash 16MB,
// Partition Scheme "16M Flash (3MB APP/9.9MB FATFS)". Librerias: LovyanGFX y ArduinoJson 7.
// Cmdr Ortem Ed Der (ortemedder@gmail.com) y Claude.

#if __has_include("mi_wifi.h")
#include "mi_wifi.h"
#else
#error "Falta mi_wifi.h: copia mi_wifi.h.ejemplo como mi_wifi.h y pon tu WiFi"
#endif
#include "config.h"
#include "pantalla.h"
#include "texto.h"
#include "modelo.h"
#include "red.h"
#include "maqueta.h"
#include "dibujo.h"

int32_t scroll = 0;          // cuanto se ha bajado (px)
float vel = 0;               // inercia al soltar (px/s)
bool repinta = true;
uint32_t tToque = 0, tCab = 0, tFrame = 0, tTele = 0, tIner = 0;

int32_t maxScroll() { return max<int32_t>(0, altoContenido - CON_H); }
void limita() { if (scroll < 0) { scroll = 0; vel = 0; } if (scroll > maxScroll()) { scroll = maxScroll(); vel = 0; } }

void rehace() {   // vuelve a montar la Cronica (datos nuevos, otra seccion, abrir/cerrar algo)
  maqueta(); preparaTeletipo(); limita(); repinta = true;
}

// ---------- tactil: arrastrar para desplazar, tocar para elegir ----------
bool tocando = false, arrastra = false;
int32_t tx0, ty0, scroll0, yAnt; uint32_t tAnt;
void toca(int x, int y) {   // un toque corto en (x, y)
  if (y < CON_Y) {   // cabecera o teletipo: arriba del todo
    if (scroll > 0) vel = -max(1500.0f, scroll * 3.0f);
    return;
  }
  int32_t cy = scroll + (y - CON_Y);
  for (size_t i = 0; i < nZonas; i++) {
    const Zona &z = zonas[i];
    if (cy < z.y0 || cy > z.y1 || x < z.x0 || x > z.x1) continue;
    switch (z.acc) {
      case A_SECCION: seccion = z.id; scroll = 0; break;
      case A_RESUMEN: expRes[z.id] = !expRes[z.id]; break;
      case A_HEME:    expHeme[z.id] = !expHeme[z.id]; break;
      case A_VERANT:  verAnt = !verAnt; antSel = -1; break;
      case A_ANT:     antSel = antSel == z.id ? -1 : z.id; break;
    }
    rehace(); return;
  }
}
void miraTactil() {
  int32_t x, y;
  bool t = tft.getTouch(&x, &y);
  uint32_t ms = millis();
  if (t) {
    tToque = ms;
    if (!tocando) { tocando = true; arrastra = false; tx0 = x; ty0 = y; scroll0 = scroll; yAnt = y; tAnt = ms; vel = 0; return; }
    if (!arrastra && abs(y - ty0) > 10) arrastra = true;
    if (arrastra) {
      scroll = scroll0 - (y - ty0); limita();
      uint32_t dt = ms - tAnt;
      if (dt >= 15) { vel = 0.6f * vel + 0.4f * (-(y - yAnt) * 1000.0f / dt); yAnt = y; tAnt = ms; }
      moviendo = true; repinta = true;
    }
  } else if (tocando) {
    tocando = false;
    if (!arrastra) { vel = 0; toca(tx0, ty0); }
    else if (ms - tAnt > 120) vel = 0;   // se paro antes de soltar
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nLa Cronica de Sion %s - PSRAM %u, flash %u\n", VERSION, ESP.getPsramSize(), ESP.getFlashChipSize());
  setenv("TZ", ZONA_HORARIA, 1); tzset();
  tft.init();
  tft.setBrightness(200);
  tft.fillScreen(TFT_BLACK);
  cargaFuentes();
  iniciaMaqueta();
  presentacion();
  for (auto *s : {&lienzo, &cabS, &teleS}) { s->setPsram(true); s->setColorDepth(16); }
  if (!lienzo.createSprite(ANCHO, CON_H) || !cabS.createSprite(ANCHO, CAB_H) || !teleS.createSprite(ANCHO, TEL_H))
    Serial.println("ERROR: sin memoria para la pantalla (¿PSRAM activada como OPI?)");
  if (FFat.begin(true)) cargaCache(); else Serial.println("FFat no disponible (¿Partition Scheme con FATFS?)");
  xTaskCreatePinnedToCore(tareaRed, "red", 16384, nullptr, 1, nullptr, 0);
  delay(1200);   // que se vea la presentacion
  tft.fillScreen(C_FONDO);
  rehace();
  pintaCabecera();
  pintaTeletipo(0);
  tTele = millis();
}

void loop() {
  uint32_t ms = millis();
  if (recogeNuevos()) { rehace(); pintaCabecera(); pintaTeletipo(0); tCab = ms; }
  miraTactil();
  if (!tocando && vel != 0) {   // inercia
    float dt = (ms - tIner) / 1000.0f; if (dt > 0.1f) dt = 0.1f;
    scroll += (int32_t)(vel * dt); vel *= powf(0.05f, dt);   // frena
    if (fabsf(vel) < 20) vel = 0;
    limita(); moviendo = true; repinta = true;
  }
  tIner = ms;
  if (!tocando && vel == 0 && moviendo) { moviendo = false; repinta = true; }
  if (scroll > 0 && !tocando && vel == 0 && ms - tToque > VUELTA_MS) vel = -max(1500.0f, scroll * 3.0f);   // vuelve arriba sola
  if (ms - tCab > 30000) { pintaCabecera(); tCab = ms; }   // hora y estado
  if (ms - tFrame >= FRAME_MS) {
    if (TELETIPO) { pintaTeletipo((ms - tTele) / 1000.0f); tTele = ms; }
    if (repinta) { pintaContenido(scroll); repinta = false; }
    tFrame = ms;
  }
  delay(1);
}
