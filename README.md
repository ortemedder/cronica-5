# cronica-5 · La Crónica de Sion en la pantalla de 5"

> **IMPORTANTE:** paquete de placas **esp32 2.0.17** (NO la 3.x), como en radar-cdsn. Si el IDE ofrece actualizar el paquete esp32, decir que no.

La pestaña **Crónica** de la web cdsn-web, en la placa **ESP32-8048S050C** (ESP32-S3 N16R8, 800×480, táctil GT911):
banda HUD, teletipo, ÚLTIMA HORA, informe de inteligencia (parte del día), últimas noticias por secciones,
reportaje (noticia fija) y hemeroteca. Fondo oscuro, bordes dorados y alertas en rojo. Se desplaza con el dedo.

Lee de GitHub `ortemedder/cdsn-datos` cada 5 min: `faccion.json` (noticias, fija, hemeroteca), `noticia_fija.txt` y `cronica_ia.json`.
Guarda una copia en la placa, así que al encender sin WiFi enseña lo último que bajó.

## Arduino IDE
- Placa: **ESP32S3 Dev Module** (paquete esp32 **2.0.17**).
- **PSRAM: OPI PSRAM** · **Flash Size: 16MB** · **Partition Scheme: 16M Flash (3MB APP/9.9MB FATFS)**.
- Librerías (Gestor de librerías): **LovyanGFX** (de lovyan03, 1.2.x) y **ArduinoJson** (de Benoit Blanchon, versión 7).
- Copia `cronica/mi_wifi.h.ejemplo` como `cronica/mi_wifi.h` y pon tu WiFi (2,4 GHz). `mi_wifi.h` no se sube a GitHub.
- Abre `cronica/cronica.ino` y súbelo. En el Monitor Serie (115200) sale cómo va la descarga.

## Manejo
- Arrastrar arriba/abajo: desplazar (con inercia). Tocar la cabecera: volver arriba.
- Botones de sección: Todas, Almirante, Conflictos, Expansión, Vecindario… («Todas» = lo nuestro; lo de otras facciones, en Vecindario).
- Tocar una noticia con «▸ ver resumen», un comunicado de la hemeroteca o «Partes anteriores»: se abre o se cierra.
- Sin tocar 5 min, vuelve arriba sola.

## Archivos
- `cronica/cronica.ino`: arranque, táctil y bucle. `config.h`: versión, direcciones, tiempos y colores.
- `pantalla.h`: pines de la placa (RGB, GT911 en 19/20 con reset en 38, luz en GPIO 2).
- `modelo.h`: los datos en la PSRAM. `red.h`: WiFi, descarga (en el núcleo 0), copia en FFat y hora.
- `maqueta.h`: monta La Crónica (igual que la web). `dibujo.h`: cabecera, teletipo, dibujos y la ventana que se ve.
- `fuentes.h` y `emblema.h`: generados por `herramientas/` (fuentes DejaVu con acentos; emblema de CDSN de cdsn-web).
- `herramientas/simulador/`: compila La Crónica en el PC y saca capturas con los datos de una copia de cdsn-datos.

Capturas del simulador: `imagenes/`.

## Pendiente (fase 2)
- Botón **🔊 Escuchar parte**: un MP3 que genera la Orange Pi del pueblo y suena en el altavoz de la placa.
- Las imágenes con datos de la web (marcador de la guerra, influencia de 7 días) necesitan `sistemas` de faccion.json: de momento no salen.

## Versiones
- v0.1.2 (27/11/2026): contra el parpadeo: imagen entera en la PSRAM y 20 imágenes por segundo (TELETIPO 0 en config.h para dejarlo quieto).
- v0.1.1 (27/11/2026): arreglada la pantalla en blanco (reloj de la pantalla).
- v0.1 (27/11/2026): primera versión (probada en el simulador; falta probarla en la placa).

Creado por Cmdr Ortem Ed Der (ortemedder@gmail.com) y Claude.
