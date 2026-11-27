// config.h - version, direcciones de los datos, tiempos y colores de La Cronica
#pragma once

#define VERSION "v0.1.2"
#define TELETIPO 1   // 0 = teletipo quieto (para probar si el parpadeo viene de moverlo)

// Datos (los mismos que lee la web cdsn-web)
const char *URL_FACCION = "https://raw.githubusercontent.com/ortemedder/cdsn-datos/main/faccion.json";
const char *URL_FIJA    = "https://raw.githubusercontent.com/ortemedder/cdsn-datos/main/noticia_fija.txt";
const char *URL_PARTE   = "https://raw.githubusercontent.com/ortemedder/cdsn-datos/main/cronica_ia.json";

const uint32_t CADA_MS      = 5 * 60 * 1000UL;   // descarga cada 5 min (como la web)
const uint32_t REINTENTO_MS = 60 * 1000UL;       // si falla, otra vez al minuto
const uint32_t FRAME_MS     = 50;                 // 20 imagenes por segundo (menos trabajo para la PSRAM)
const uint32_t VUELTA_MS    = 5 * 60 * 1000UL;   // sin tocar 5 min: vuelve arriba sola
const char *ZONA_HORARIA = "CET-1CEST,M3.5.0,M10.5.0/3";   // hora de Espana peninsular

// Pantalla 800x480
const int ANCHO = 800, ALTO = 480;
const int CAB_H = 82;              // cabecera (titulo + banda HUD)
const int TEL_Y = CAB_H, TEL_H = 24;   // teletipo
const int CON_Y = TEL_Y + TEL_H;   // zona de noticias (se desplaza)
const int CON_H = ALTO - CON_Y;
const int MARGEN = 14;             // margen izquierdo y derecho del contenido

// Colores de la web (0xRRGGBB). Ojo: en LovyanGFX un numero suelto (int) es RGB565; uint32_t es RGB888
const uint32_t C_FONDO = 0x0d0904;
const uint32_t C_PANEL = 0x1a1208;
const uint32_t C_LINEA = 0x3a2a12;
const uint32_t C_NARANJA = 0xff8c00;
const uint32_t C_NAR_OSC = 0xb06a10;
const uint32_t C_NAR_CLA = 0xffb347;
const uint32_t C_VERDE = 0x5adc78;
const uint32_t C_VERDE2 = 0x4cd964;
const uint32_t C_AMARILLO = 0xffd228;
const uint32_t C_ROJO = 0xff4632;
const uint32_t C_AZUL = 0x5aaaff;
const uint32_t C_CIAN = 0x66cbff;
const uint32_t C_ROSA = 0xff6ec8;
const uint32_t C_TEXTO = 0xf2e6d6;
const uint32_t C_TEXTO2 = 0xe0d2bf;
const uint32_t C_CREMA = 0xffe7c4;
const uint32_t C_URG_TXT = 0xffb3a8;
