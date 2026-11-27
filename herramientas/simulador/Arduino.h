// Arduino.h de mentira para compilar La Cronica en el PC (simulador). Solo lo que usa el programa.
#pragma once
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <type_traits>
using std::max; using std::min;
#define PROGMEM
#define MALLOC_CAP_SPIRAM 0
inline uint8_t pgm_read_byte(const void *p) { return *(const uint8_t *)p; }
inline uint16_t pgm_read_word(const void *p) { return *(const uint16_t *)p; }
inline void *ps_malloc(size_t n) { return malloc(n); }
inline void *heap_caps_malloc(size_t n, int) { return malloc(n); }
inline void *heap_caps_realloc(void *p, size_t n, int) { return realloc(p, n); }
inline void heap_caps_free(void *p) { free(p); }
uint32_t millis();
inline void delay(uint32_t) {}
struct SerialFalso { template <class... A> void printf(const char *f, A... a) { ::printf(f, a...); } void println(const char *s) { puts(s); } };
extern SerialFalso Serial;

class String {
  std::string s;
public:
  String() {}
  String(const char *c) : s(c ? c : "") {}
  String(const char *c, size_t n) : s(c, n) {}
  String(const std::string &x) : s(x) {}
  String(char c) : s(1, c) {}
  template <class T, typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, char>::value, int>::type = 0>
  String(T v) : s(std::to_string(v)) {}
  operator const char *() const { return s.c_str(); }
  const char *c_str() const { return s.c_str(); }
  unsigned length() const { return s.size(); }
  void reserve(size_t n) { s.reserve(n); }
  char operator[](unsigned i) const { return i < s.size() ? s[i] : 0; }
  void setCharAt(unsigned i, char c) { if (i < s.size()) s[i] = c; }
  String substring(unsigned a, unsigned b) const { if (a > s.size()) a = s.size(); if (b > s.size()) b = s.size(); return b > a ? String(s.substr(a, b - a)) : String(); }
  String substring(unsigned a) const { return substring(a, s.size()); }
  int indexOf(const char *x) const { auto p = s.find(x); return p == std::string::npos ? -1 : (int)p; }
  int indexOf(char x) const { auto p = s.find(x); return p == std::string::npos ? -1 : (int)p; }
  bool startsWith(const char *x) const { return s.compare(0, strlen(x), x) == 0; }
  void trim() { size_t a = s.find_first_not_of(" \t\r\n"); if (a == std::string::npos) { s.clear(); return; } size_t b = s.find_last_not_of(" \t\r\n"); s = s.substr(a, b - a + 1); }
  void replace(const char *a, const char *b) { size_t p = 0, la = strlen(a), lb = strlen(b); while ((p = s.find(a, p)) != std::string::npos) { s.replace(p, la, b); p += lb; } }
  bool equalsIgnoreCase(const char *x) const { return strcasecmp(s.c_str(), x) == 0; }
  long toInt() const { return atol(s.c_str()); }
  String &operator+=(const String &x) { s += x.s; return *this; }
  String &operator+=(const char *x) { s += x; return *this; }
  String &operator+=(char c) { s += c; return *this; }
  friend String operator+(const String &a, const String &b) { return String(a.s + b.s); }
  friend String operator+(const String &a, const char *b) { return String(a.s + b); }
  friend String operator+(const char *a, const String &b) { return String(a + b.s); }
  friend String operator+(const String &a, char c) { return String(a.s + c); }
  template <class T, typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, char>::value, int>::type = 0>
  friend String operator+(const String &a, T v) { return String(a.s + std::to_string(v)); }
  friend bool operator==(const String &a, const String &b) { return a.s == b.s; }
  friend bool operator==(const String &a, const char *b) { return a.s == b; }
  friend bool operator!=(const String &a, const String &b) { return a.s != b.s; }
  friend bool operator!=(const String &a, const char *b) { return a.s != b; }
  friend bool operator>(const String &a, const String &b) { return a.s > b.s; }
  friend bool operator<(const String &a, const String &b) { return a.s < b.s; }
};
