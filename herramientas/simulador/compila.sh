#!/bin/sh
# Compila el simulador en Linux (g++) y saca capturas. Uso: herramientas/simulador/compila.sh ruta/a/cdsn-datos [salida]
# Necesita LovyanGFX y ArduinoJson en ~/Arduino/libraries.
set -e
L=~/Arduino/libraries/LovyanGFX/src; J=~/Arduino/libraries/ArduinoJson/src; D=$(dirname "$0"); O=/tmp/simobj
mkdir -p $O "${2:-/tmp/fotos}"
for f in $(find $L/lgfx -name "*.cpp" -o -name "*.c" | grep -v /platforms/) $L/lgfx/v1/platforms/framebuffer/common.cpp $L/lgfx/v1/platforms/framebuffer/Panel_fb.cpp; do
  o=$O/$(echo $f | md5sum | cut -c1-8).o; [ -f $o ] || g++ -x c++ -std=c++17 -O1 -DLGFX_LINUX_FB -I$D -I$L -c $f -o $o
done
g++ -std=c++17 -O1 -DLGFX_LINUX_FB -I$D -I$L -I$J $D/sim.cpp $O/*.o -o /tmp/sim
/tmp/sim "$1" "${2:-/tmp/fotos}"
