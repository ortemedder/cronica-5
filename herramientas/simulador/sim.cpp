// Simulador de La Cronica en el PC: monta y pinta la pantalla con los datos de una copia local de cdsn-datos
// y guarda imagenes .ppm (800x480). Uso: ./sim ruta/a/cdsn-datos carpeta_salida
#define LGFX_LINUX_FB
#include "Arduino.h"
#include <LovyanGFX.hpp>
#include <chrono>
#include <fstream>
#include <sstream>
SerialFalso Serial;
uint32_t millis() { using namespace std::chrono; static auto t0 = steady_clock::now(); return duration_cast<milliseconds>(steady_clock::now() - t0).count(); }
LGFX_Sprite tft;   // hace de pantalla
#include "../../cronica/config.h"
#include "../../cronica/texto.h"
#include "../../cronica/modelo.h"
#include "../../cronica/maqueta.h"
#include "../../cronica/dibujo.h"

std::string lee(const std::string &r) { std::ifstream f(r, std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str(); }
void foto(const std::string &nom) {
  std::ofstream f(nom, std::ios::binary); f << "P6\n800 480\n255\n";
  for (int y = 0; y < 480; y++) for (int x = 0; x < 800; x++) { auto c = tft.readPixelRGB(x, y); f.put(c.r); f.put(c.g); f.put(c.b); }
}
void pantalla(int32_t scroll, const std::string &nom) { pintaCabecera(); pintaTeletipo(0); pintaContenido(scroll); foto(nom); }

int main(int argc, char **argv) {
  std::string dir = argc > 1 ? argv[1] : ".", out = argc > 2 ? argv[2] : ".";
  setenv("TZ", ZONA_HORARIA, 1); tzset();
  tft.setColorDepth(16); tft.createSprite(800, 480);
  cargaFuentes(); iniciaMaqueta();
  for (auto *s : {&lienzo, &cabS, &teleS}) s->setColorDepth(16);
  lienzo.createSprite(ANCHO, CON_H); cabS.createSprite(ANCHO, CAB_H); teleS.createSprite(ANCHO, TEL_H);
  presentacion(); foto(out + "/0_presentacion.ppm");
  char err[60];
  std::string fa = lee(dir + "/faccion.json"), pa = lee(dir + "/cronica_ia.json"), fi = lee(dir + "/noticia_fija.txt");
  dFac = parseaFaccion(fa.data(), fa.size(), err, sizeof err); if (!dFac) printf("faccion: %s\n", err);
  dParte = parseaParte(pa.data(), pa.size()); dFija = parseaFija(fi.c_str());
  tUltOk = ahora() - 180; wifiOk = true;
  uint32_t t0 = millis(); maqueta(); printf("maqueta en %u ms\n", millis() - t0); preparaTeletipo();
  int32_t pasos[] = {0, 360, 720, 1100};
  for (int i = 0; i < 4; i++) pantalla(pasos[i], out + "/1_todas_" + std::to_string(i) + ".ppm");
  pantalla(max<int32_t>(0, altoContenido - CON_H), out + "/2_final.ppm");
  // abre el primer comunicado de la hemeroteca y la seccion conflictos
  expHeme[0] = true; maqueta(); pantalla(max<int32_t>(0, altoContenido - CON_H - 300), out + "/3_hemeroteca.ppm");
  seccion = 2; maqueta(); pantalla(0, out + "/4_conflictos.ppm");
  seccion = 4; maqueta(); pantalla(0, out + "/5_vecindario.ppm");
  return 0;
}
