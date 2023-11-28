// Aseprite Document Library
// Copyright (c) 2001-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "base/ints.h"
#include <ctime>

namespace doc {

  // The greatest int type to storage a color for an image in the
  // available pixel formats.
  typedef uint64_t color_t;

  //////////////////////////////////////////////////////////////////////
  // RGBA

  const uint32_t rgba_r_shift = 0;
  const uint32_t rgba_g_shift = 8;
  const uint32_t rgba_b_shift = 16;
  const uint32_t rgba_a_shift = 24;

  const uint32_t rgba_r_mask = 0x000000ff;
  const uint32_t rgba_g_mask = 0x0000ff00;
  const uint32_t rgba_b_mask = 0x00ff0000;
  const uint32_t rgba_rgb_mask = 0x00ffffff;
  const uint32_t rgba_a_mask = 0xff000000;

  inline uint8_t rgba_getr(uint32_t c) {
    return (c >> rgba_r_shift) & 0xff;
  }

  inline uint8_t rgba_getg(uint32_t c) {
    return (c >> rgba_g_shift) & 0xff;
  }

  inline uint8_t rgba_getb(uint32_t c) {
    return (c >> rgba_b_shift) & 0xff;
  }

  inline uint8_t rgba_geta(uint32_t c) {
    return (c >> rgba_a_shift) & 0xff;
  }

  inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((r << rgba_r_shift) |
            (g << rgba_g_shift) |
            (b << rgba_b_shift) |
            (a << rgba_a_shift));
  }

  inline int rgb_luma(int r, int g, int b) {
    return (r*2126 + g*7152 + b*722) / 10000;
  }

  inline uint8_t rgba_luma(uint32_t c) {
    return rgb_luma(rgba_getr(c), rgba_getg(c), rgba_getb(c));
  }

  //////////////////////////////////////////////////////////////////////
  // Grayscale

  const uint16_t graya_v_shift = 0;
  const uint16_t graya_a_shift = 8;

  const uint16_t graya_v_mask = 0x00ff;
  const uint16_t graya_a_mask = 0xff00;

  inline uint8_t graya_getv(uint16_t c) {
    return (c >> graya_v_shift) & 0xff;
  }

  inline uint8_t graya_geta(uint16_t c) {
    return (c >> graya_a_shift) & 0xff;
  }

  inline uint16_t graya(uint8_t v, uint8_t a) {
    return ((v << graya_v_shift) |
            (a << graya_a_shift));
  }

  inline uint16_t gray(uint8_t v) {
    return graya(v, 255);
  }

  //////////////////////////////////////////////////////////////////////
  // TRGBA

  const uint64_t trgba_r_shift = 0;
  const uint64_t trgba_g_shift = 8;
  const uint64_t trgba_b_shift = 16;
  const uint64_t trgba_a_shift = 24;
  const uint64_t trgba_t_shift = 32;

  const uint64_t trgba_r_mask =    0x00000000000000ff;
  const uint64_t trgba_g_mask =    0x000000000000ff00;
  const uint64_t trgba_b_mask =    0x0000000000ff0000;
  const uint64_t trgba_a_mask =    0x00000000ff000000;
  const uint64_t trgba_rgb_mask =  0x0000000000ffffff;
  const uint64_t trgba_rgba_mask = 0x00000000ffffffff;
  const uint64_t trgba_t_mask =    0xffffffff00000000;

  inline uint8_t trgba_getr(uint64_t c) {
    return (c >> trgba_r_shift) & 0xff;
  }

  inline uint8_t trgba_getg(uint64_t c) {
    return (c >> trgba_g_shift) & 0xff;
  }

  inline uint8_t trgba_getb(uint64_t c) {
    return (c >> trgba_b_shift) & 0xff;
  }

  inline uint8_t trgba_geta(uint64_t c) {
    return (c >> trgba_a_shift) & 0xff;
  }

  inline uint32_t trgba_gett(uint64_t c) {
    return (c >> trgba_t_shift) & 0xffffffff;
  }

  inline uint32_t trgba_get_current_t() {
    // Normalize since our epoch, shift 4 so we can count up to 16 changes per second
    // Note this timestamp method will overflow in 2031, need to fix before then by putting
    // per-file epoch in the file format that gets updated based on the lowest timestamp across
    // all the layers periodically.
    return (time(NULL) - 0x65652b00) << 4;
  }

  inline uint64_t trgba_with_adjusted_t(uint64_t cur, uint64_t val, uint64_t min_t = trgba_get_current_t()) {
    // Take the max timestamp between the current pixel and the new one, adding to the current one if the color changed.
    uint64_t cur_t = trgba_gett(cur) + ((val & trgba_rgba_mask) != (cur & trgba_rgba_mask));
    uint64_t new_t = cur_t > min_t ? cur_t : min_t;
    return (val & ~trgba_t_mask) | (new_t << trgba_t_shift);
  }

  inline uint64_t trgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint32_t t) {
    return ((r << trgba_r_shift) |
            (g << trgba_g_shift) |
            (b << trgba_b_shift) |
            ((uint64_t)a << trgba_a_shift) |
            ((uint64_t)t << trgba_t_shift));
  }

  inline uint8_t trgba_luma(uint64_t c) {
    return rgb_luma(trgba_getr(c), trgba_getg(c), trgba_getb(c));
  }

  // Takes target and applies max t from src or target to it
  inline uint64_t rtgba_tmerge(uint64_t src, uint64_t target) {
    return target > src ? target : (src & trgba_t_mask) | (target & trgba_rgba_mask);
  }
} // namespace doc