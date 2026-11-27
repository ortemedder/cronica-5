// pantalla.h - placa ESP32-8048S050C: pantalla RGB 800x480 (ST7262), tactil GT911 y luz en GPIO 2
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_RGB     _panel;
  lgfx::Bus_RGB       _bus;
  lgfx::Light_PWM     _luz;
  lgfx::Touch_GT911   _tactil;
public:
  LGFX() {
    { auto c = _panel.config();
      c.memory_width = 800; c.memory_height = 480;
      c.panel_width  = 800; c.panel_height  = 480;
      c.offset_x = 0; c.offset_y = 0;
      _panel.config(c); }
    { auto c = _panel.config_detail();
      c.use_psram = 1;                       // la imagen de la pantalla, en la PSRAM
      _panel.config_detail(c); }
    { auto c = _bus.config();
      c.panel = &_panel;
      c.pin_d0  = 8;  c.pin_d1  = 3;  c.pin_d2  = 46; c.pin_d3  = 9;  c.pin_d4  = 1;    // azul
      c.pin_d5  = 5;  c.pin_d6  = 6;  c.pin_d7  = 7;  c.pin_d8  = 15; c.pin_d9  = 16; c.pin_d10 = 4;   // verde
      c.pin_d11 = 45; c.pin_d12 = 48; c.pin_d13 = 47; c.pin_d14 = 21; c.pin_d15 = 14;   // rojo
      c.pin_henable = 40; c.pin_vsync = 41; c.pin_hsync = 39; c.pin_pclk = 42;
      c.freq_write = 14000000;               // 14 MHz: deja aire a la PSRAM (con 16 a veces la imagen se desplaza)
      c.hsync_polarity = 0; c.hsync_front_porch = 8; c.hsync_pulse_width = 4; c.hsync_back_porch = 8;
      c.vsync_polarity = 0; c.vsync_front_porch = 8; c.vsync_pulse_width = 4; c.vsync_back_porch = 8;
      c.pclk_idle_high = 1;
      _bus.config(c); }
    _panel.setBus(&_bus);
    { auto c = _luz.config();
      c.pin_bl = 2; c.freq = 44100; c.pwm_channel = 7; c.invert = false;
      _luz.config(c); _panel.setLight(&_luz); }
    { auto c = _tactil.config();
      c.x_min = 0; c.x_max = 799; c.y_min = 0; c.y_max = 479;
      c.pin_int = -1; c.pin_rst = 38;
      c.bus_shared = false; c.offset_rotation = 0;
      c.i2c_port = 1; c.pin_sda = 19; c.pin_scl = 20; c.freq = 400000;
      c.i2c_addr = 0x14;                     // si no contesta, la libreria prueba 0x5D
      _tactil.config(c); _panel.setTouch(&_tactil); }
    setPanel(&_panel);
  }
};

LGFX tft;
