/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename    : engine.c
 * Description : engine header
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 27/02/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_engine_c__
#define __libaroma_engine_c__

#ifdef __ARM_HAVE_NEON
  #include <arm_neon.h> /* include arm_neon.h */
  #define LIBAROMA_CONFIG_ENGINE_ALPHA \
    "neon/alpha_neon.c"
  #define LIBAROMA_CONFIG_ENGINE_BLT \
    "neon/blt_neon.c"
  #define LIBAROMA_CONFIG_ENGINE_COLOR \
    "neon/color_neon.c"
  #define LIBAROMA_CONFIG_ENGINE_DITHER \
    "neon/dither_neon.c"
#endif /* __ARM_HAVE_NEON */

/*
 * Variable    : libaroma_dither_tresshold_r
 * Type        : const
 * Descriptions: dither tresshold for red channel
 */
static const byte libaroma_dither_tresshold_r[64] = {
  1, 7, 3, 5, 0, 8, 2, 6,
  7, 1, 5, 3, 8, 0, 6, 2,
  3, 5, 0, 8, 2, 6, 1, 7,
  5, 3, 8, 0, 6, 2, 7, 1,
  0, 8, 2, 6, 1, 7, 3, 5,
  8, 0, 6, 2, 7, 1, 5, 3,
  2, 6, 1, 7, 3, 5, 0, 8,
  6, 2, 7, 1, 5, 3, 8, 0
};
/*
 * Variable    : libaroma_dither_tresshold_g
 * Type        : const
 * Descriptions: dither tresshold for green channel
 */
static const byte libaroma_dither_tresshold_g[64] = {
  1, 3, 2, 2, 3, 1, 2, 2,
  2, 2, 0, 4, 2, 2, 4, 0,
  3, 1, 2, 2, 1, 3, 2, 2,
  2, 2, 4, 0, 2, 2, 0, 4,
  1, 3, 2, 2, 3, 1, 2, 2,
  2, 2, 0, 4, 2, 2, 4, 0,
  3, 1, 2, 2, 1, 3, 2, 2,
  2, 2, 4, 0, 2, 2, 0, 4
};
/*
 * Variable    : libaroma_dither_tresshold_b
 * Type        : const
 * Descriptions: dither tresshold for blue channel
 */
static const byte libaroma_dither_tresshold_b[64] = {
  5, 3, 8, 0, 6, 2, 7, 1,
  3, 5, 0, 8, 2, 6, 1, 7,
  8, 0, 6, 2, 7, 1, 5, 3,
  0, 8, 2, 6, 1, 7, 3, 5,
  6, 2, 7, 1, 5, 3, 8, 0,
  2, 6, 1, 7, 3, 5, 0, 8,
  7, 1, 5, 3, 8, 0, 6, 2,
  1, 7, 3, 5, 0, 8, 2, 6
};



word libaroma_rgb_from_string(const char * c) {
  if (c[0] != '#') {
    return 0;
  }
  char out[9] = {'0', 'x'};
  int  i;
  if (strlen(c) == 7) {
    for (i = 1; i < 7; i++) {
      out[i + 1] = c[i];
    }
  }
  else if (strlen(c) == 4) {
    for (i = 0; i < 3; i++) {
      out[(i * 2) + 2] = c[i + 1];
      out[(i * 2) + 3] = c[i + 1];
    }
  }
  else {
    return 0;
  }
  out[8] = 0;
  return libaroma_rgb_to16(strtoul(out, NULL, 0));
}

/* 16bit color channel */
byte libaroma_color_r(word rgb) {
  return ((byte) (((((word)(rgb)) & 0xF800)) >> 8));
}
byte libaroma_color_g(word rgb) {
  return ((byte) (((((word)(rgb)) & 0x07E0)) >> 3));
}
byte libaroma_color_b(word rgb) {
  return ((byte) (((((word)(rgb)) & 0x001F)) << 3));
}

/* hi color shifted */
byte libaroma_color_hi_r(byte v){
  return (v | (v >> 5));
}
byte libaroma_color_hi_g(byte v){
  return (v | (v >> 6));
}

/* calculate color luminance */
byte libaroma_color_luminance(word rgb){
  return (byte) MIN(0xff,MAX(0,
    ((
      libaroma_color_r(rgb)*306+
      libaroma_color_g(rgb)*602+
      libaroma_color_b(rgb)*116
    )>>10)
  ));
}

/* is dark color */
byte libaroma_color_isdark(word rgb){
  if (libaroma_color_luminance(rgb)>128){
    return 0;
  }
  return 1;
}

/* 32bit color channel */
byte libaroma_color_r32(dword rgb) {
  return (byte) ((rgb >> 16) & 0xff);
}
byte libaroma_color_g32(dword rgb) {
  return (byte) ((rgb >> 8) & 0xff);
}
byte libaroma_color_b32(dword rgb) {
  return (byte) (rgb & 0xff);
}
byte libaroma_color_a32(dword rgb) {
  return (byte) ((rgb >> 24) & 0xff);
}

/* 16bit closest color channel */
byte libaroma_color_close_r(byte c) { /* red & blue */
  return (((byte) c) >> 3 << 3);
}
byte libaroma_color_close_g(byte c) {
  return (((byte) c) >> 2 << 2);
}

/* hi color left */
byte libaroma_color_left(byte r, byte g, byte b) {
  return (
    (((r - libaroma_color_close_r(r)) & 7) << 5) |
    (((g - libaroma_color_close_g(g)) & 3) << 3) |
    ((b - libaroma_color_close_b(b)) & 7)
  );
}

/* merge hi color & main color */
dword libaroma_color_merge(word color, byte hicolor) {
  return libaroma_rgba(
    libaroma_color_r(color) + (hicolor >> 5),
    libaroma_color_g(color) + ((hicolor >> 3) & 3),
    libaroma_color_b(color) + (hicolor & 7),
    0xff
  );
}
byte libaroma_color_merge_r(word color, byte hicolor) {
  return ((byte) (((((word)(color)) & 0xF800)) >> 8)) + (hicolor >> 5);
}
byte libaroma_color_merge_g(word color, byte hicolor) {
  return ((byte) (((((word)(color)) & 0x07E0)) >> 3)) + ((hicolor >> 3) & 3);
}
byte libaroma_color_merge_b(word color, byte hicolor) {
  return ((byte) (((((word)(color)) & 0x001F)) << 3)) + (hicolor & 7);
}

/* 16bit rgb */
word libaroma_rgb(byte r, byte g, byte b) {
  return ((word)((r >> 3) << 11)|((g >> 2) << 5) | (b >> 3));
}
/* 32bit rgba */
dword libaroma_rgba(byte r, byte g, byte b, byte a) {
  return (dword)
  (
    ((r & 0xff) << 16) |
    ((g & 0xff) << 8) |
    (b & 0xff) |
    ((a & 0xff) << 24)
  );
}
/* 32bit rgb */
dword libaroma_rgb32(byte r, byte g, byte b) {
  return libaroma_rgba(r, g, b, 0xff);
}

/* Convert 32bit color to 16bit color */
word libaroma_rgb_to16(dword rgb) {
  return libaroma_rgb(
    libaroma_color_r32(rgb), 
    libaroma_color_g32(rgb), 
    libaroma_color_b32(rgb));
}

/* Convert 16bit color to 32bit color */
dword libaroma_rgb_to32(word rgb) {
#ifdef LIBAROMA_CONFIG_USE_HICOLOR_BIT
  return libaroma_rgb32(
    libaroma_color_hi_r(libaroma_color_r(rgb)), 
    libaroma_color_hi_g(libaroma_color_g(rgb)), 
    libaroma_color_hi_b(libaroma_color_b(rgb))
  );
#else
return libaroma_rgb32(
    libaroma_color_r(rgb), 
    libaroma_color_g(rgb), 
    libaroma_color_b(rgb)
  );
#endif
}

/* Convert 16bit color to RGBA */
dword libaroma_rgb_to_rgba(word rgb, byte alpha) {
#ifdef LIBAROMA_CONFIG_USE_HICOLOR_BIT
  return libaroma_rgba(
    libaroma_color_hi_r(libaroma_color_r(rgb)),
    libaroma_color_hi_g(libaroma_color_g(rgb)),
    libaroma_color_hi_b(libaroma_color_b(rgb)),
    alpha
  );
#else
return libaroma_rgba(
    libaroma_color_r(rgb), 
    libaroma_color_g(rgb), 
    libaroma_color_b(rgb),
    alpha
  );
#endif
}

#ifdef LIBAROMA_CONFIG_ENGINE_COLOR
#include LIBAROMA_CONFIG_ENGINE_COLOR
#endif

#ifndef __engine_have_libaroma_color_set
void libaroma_color_set(wordp dst, word color, int n) {
#ifdef libaroma_memset16
  libaroma_memset16(dst,color,n);
#else
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = color;
  }
#endif
}
#endif

#ifndef __engine_have_libaroma_color_set32
void libaroma_color_set32(dwordp dst, dword color, int n) {
#ifdef libaroma_memcpy32
  libaroma_memcpy32(dst,color,n);
#else
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = color;
  }
#endif
}
#endif

#ifndef __engine_have_libaroma_color_copy32
void libaroma_color_copy32(dwordp dst, wordp src, int n, bytep rgb_pos) {
  int i;
  for (i = 0; i < n; i++) {
    word cl = src[i];
#ifdef LIBAROMA_CONFIG_USE_HICOLOR_BIT
    dst[i] = (
       ((libaroma_color_hi_r(libaroma_color_r(cl)) & 0xff) << rgb_pos[0]) |
       ((libaroma_color_hi_g(libaroma_color_g(cl)) & 0xff) << rgb_pos[1]) |
       ((libaroma_color_hi_b(libaroma_color_b(cl)) & 0xff) << rgb_pos[2])
     );
#else
    dst[i] = (
       ((libaroma_color_r(cl) & 0xff) << rgb_pos[0]) |
       ((libaroma_color_g(cl) & 0xff) << rgb_pos[1]) |
       ((libaroma_color_b(cl) & 0xff) << rgb_pos[2])
     );
#endif
  }
}
#endif

#ifndef __engine_have_libaroma_color_copy16
void libaroma_color_copy16(wordp dst, dwordp src, int n, bytep rgb_pos) {
  int i;
  for (i = 0; i < n; i++) {
    dword cl = src[i];
    dst[i] = libaroma_rgb(
       (byte) ((cl >> rgb_pos[0]) & 0xff),
       (byte) ((cl >> rgb_pos[1]) & 0xff),
       (byte) ((cl >> rgb_pos[2]) & 0xff)
     );
  }
}
#endif


#ifdef LIBAROMA_CONFIG_ENGINE_BLT
#include LIBAROMA_CONFIG_ENGINE_BLT
#endif

#ifndef __engine_have_libaroma_btl16
void libaroma_btl16(int n, wordp dst, const dwordp src) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_rgb_to16(src[i]);
  }
}
#endif

#ifndef __engine_have_libaroma_btl32
void libaroma_btl32(int n, dwordp dst, const wordp src) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_rgb_to32(src[i]);
  }
}
#endif


byte libaroma_dither_table_pos(int x, int y) {
  return ((y & 7) << 3) + (x & 7);
}
byte libaroma_dither_r(byte p) {
  return libaroma_dither_tresshold_r[p];
}
byte libaroma_dither_g(byte p) {
  return libaroma_dither_tresshold_g[p];
}
byte libaroma_dither_b(byte p) {
  return libaroma_dither_tresshold_b[p];
}
word libaroma_dither_rgb(int x, int y, byte sr, byte sg, byte sb) {
  byte dither_xy = ((y & 7) << 3) + (x & 7);
  byte r = libaroma_color_close_r(MIN(sr +
                    libaroma_dither_tresshold_r[dither_xy], 0xff));
  byte g = libaroma_color_close_g(MIN(sg +
                    libaroma_dither_tresshold_g[dither_xy], 0xff));
  byte b = libaroma_color_close_b(MIN(sb +
                    libaroma_dither_tresshold_b[dither_xy], 0xff));
  return libaroma_rgb(r, g, b);
}
word libaroma_dither_mono_rgb(int x, int y, byte sr, byte sg, byte sb) {
  byte dither_xy = libaroma_dither_tresshold_g[((y & 7) << 3) + (x & 7)];
  byte dither_xyrb = dither_xy * 2;
  byte r = libaroma_color_close_r(MIN(sr + dither_xyrb, 0xff));
  byte g = libaroma_color_close_g(MIN(sg + dither_xy, 0xff));
  byte b = libaroma_color_close_b(MIN(sb + dither_xyrb, 0xff));
  return libaroma_rgb(r, g, b);
}
word libaroma_dither_mono(int x, int y, dword col) {
  return libaroma_dither_mono_rgb(x, y, 
    libaroma_color_r32(col), 
    libaroma_color_g32(col), 
    libaroma_color_b32(col));
}
word libaroma_dither(int x, int y, dword col) {
  return libaroma_dither_rgb(x, y, 
    libaroma_color_r32(col), 
    libaroma_color_g32(col), 
    libaroma_color_b32(col));
}

#ifdef LIBAROMA_CONFIG_ENGINE_DITHER
#include LIBAROMA_CONFIG_ENGINE_DITHER
#endif

#ifndef __engine_have_libaroma_dither_line
void libaroma_dither_line(int y, int w, wordp dst, const dwordp src) {
  int i;

  for (i = 0; i < w; i++) {
    dst[i] = libaroma_dither(i, y, src[i]);
  }
}
#endif

#ifndef __engine_have_libaroma_dither_line_const
void libaroma_dither_line_const(int y, int w, wordp dst, dword src) {
  int i;
  for (i = 0; i < w; i++) {
    dst[i] = libaroma_dither(i, y, src);
  }
}
#endif


word libaroma_alpha(word dcl, word scl, byte l) {
  if (scl == dcl) {
    return scl;
  }
  else if (l == 0) {
    return dcl;
  }
  else if (l == 0xff) {
    return scl;
  }
  word na = l;
  word fa = 256 - na;
  return
    (word) (
      (((libaroma_color_r(dcl) * fa) + 
        (libaroma_color_r(scl) * na)) >> 11 << 11) |
      (((libaroma_color_g(dcl) * fa) + 
        (libaroma_color_g(scl) * na)) >> 10 << 5) |
      (((libaroma_color_b(dcl) * fa) + 
        (libaroma_color_b(scl) * na)) >> 11)
    );
}
dword libaroma_alpha32(word dcl, word scl, byte l) {
  if (scl == dcl) {
    return libaroma_rgb_to32(scl);
  }
  else if (l == 0) {
    return libaroma_rgb_to32(dcl);
  }
  else if (l == 0xff) {
    return libaroma_rgb_to32(scl);
  }
  word na = l;
  word fa = 256 - na;
  return
    (dword) (
      (((libaroma_color_r(dcl) * fa) + 
        (libaroma_color_r(scl) * na)) >> 8 << 16) |
      (((libaroma_color_g(dcl) * fa) + 
        (libaroma_color_g(scl) * na)) >> 8 << 8) |
      (((libaroma_color_b(dcl) * fa) + 
        (libaroma_color_b(scl) * na)) >> 8) |
      (0xff << 24)
    );
}
word libaroma_alpha_multi(word dcl, word scl, byte lr, byte lg, byte lb) {
  if (scl == dcl) {
    return scl;
  }
  else if (lr + lg + lb == 0) {
    return dcl;
  }
  else if (lr + lg + lb == 765) {
    return scl;
  }
  word  rr = 256 - lr;
  word  rg = 256 - lg;
  word  rb = 256 - lb;
  return
    (word) (
      (((libaroma_color_r(dcl) * rr) + 
        (libaroma_color_r(scl) * lr)) >> 11 << 11) |
      (((libaroma_color_g(dcl) * rg) + 
        (libaroma_color_g(scl) * lg)) >> 10 << 5) |
      (((libaroma_color_b(dcl) * rb) + 
        (libaroma_color_b(scl) * lb)) >> 11)
    );
}
word libaroma_alphab(word scl, byte l) {
  if (l == 0) {
    return 0;
  }
  else if (l == 255) {
    return scl;
  }
  word na = l;
  return
    (word) (
      ((libaroma_color_r(scl) * na) >> 11 << 11) |
      ((libaroma_color_g(scl) * na) >> 10 << 5) |
      ((libaroma_color_b(scl) * na) >> 11)
    );
}

#ifdef LIBAROMA_CONFIG_ENGINE_ALPHA
#include LIBAROMA_CONFIG_ENGINE_ALPHA
#endif

#ifndef __engine_have_libaroma_alpha_black
void libaroma_alpha_black(int n, wordp dst, wordp top, byte alpha) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_alphab(top[i], alpha);
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_const
void libaroma_alpha_const(int n, wordp dst,
    wordp bottom, wordp top, byte alpha) {
  int i;

  for (i = 0; i < n; i++) {
    dst[i] = libaroma_alpha(bottom[i], top[i], alpha);
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_const_line
void libaroma_alpha_const_line(int _Y, int n, wordp dst,
    wordp bottom, wordp top, byte alpha) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_dither(i, _Y, libaroma_alpha32(bottom[i], top[i], alpha));
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_px
void libaroma_alpha_px(int n, wordp dst, wordp bottom,
    wordp top, bytep alpha) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_alpha(bottom[i], top[i], alpha[i]);
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_px_line
void libaroma_alpha_px_line(int _Y, int n, wordp dst,
    wordp bottom, wordp top, bytep alpha) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_dither(i, _Y, 
      libaroma_alpha32(bottom[i], top[i], alpha[i]));
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_rgba_fill
void libaroma_alpha_rgba_fill(int n, wordp dst, wordp bottom,
    word top, byte alpha) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_alpha(bottom[i], top, alpha);
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_mono
void libaroma_alpha_mono(int n, wordp dst, wordp bottom,
    word top, bytep alpha) {
  int i;
  for (i = 0; i < n; i++) {
    dst[i] = libaroma_alpha(bottom[i], top, alpha[i]);
  }
}
#endif

#ifndef __engine_have_libaroma_alpha_multi_line
void libaroma_alpha_multi_line(int n, wordp dst, wordp bottom,
    word top, bytep alphargb) {
  int i;

  for (i = 0; i < n; i++) {
    int j = i * 3;
    dst[i] = libaroma_alpha_multi(bottom[i], top, alphargb[j],
      alphargb[j + 1],  alphargb[j + 2]);
  }
}
#endif



void libaroma_blt_align16(wordp dst, wordp src,
    int w, int h, int dst_stride, int src_stride) {
  int i;
  int w2 = w<<1;
  int ds = w2 + dst_stride;
  int ss = w2 + src_stride;
  bytep d = (bytep) dst;
  bytep s = (bytep) src;
  for (i = 0; i < h; i++) {
    memcpy(
      d+ds*i, s+ss*i, w2
    );
  }
}
void libaroma_blt_align32_to16(wordp dst, dwordp src,
    int w, int h, int dst_stride, int src_stride) {
  int i;
  int dline = w+(dst_stride>>1);
  int sline = w+(src_stride>>2);
  for (i = 0; i < h; i++) {
    libaroma_dither_line(
      i, w, dst+dline*i, src+sline*i
    );
  }
}
void libaroma_blt_align16_to32(dwordp dst, wordp src,
    int w, int h, int dst_stride, int src_stride) {
  int i;
  int dline = w+(dst_stride>>2);
  int sline = w+(src_stride>>1);
  for (i = 0; i < h; i++) {
    libaroma_btl32(
      w,dst+dline*i,src+sline*i
    );
  }
}
void libaroma_blt_align32(dwordp dst, dwordp src,
    int w, int h, int dst_stride, int src_stride) {
  int i;
  int w4 = w<<2;
  int ds = w4 + dst_stride;
  int ss = w4 + src_stride;
  bytep d = (bytep) dst;
  bytep s = (bytep) src;
  for (i = 0; i < h; i++) {
    memcpy(
      d+ds*i, s+ss*i, w4
    );
  }
}
void libaroma_blt_align_to32_pos(dwordp dst, wordp src,
    int w, int h, int dst_stride, int src_stride,
    bytep rgb_pos) {
  int i;
  int dline = w+(dst_stride>>2);
  int sline = w+(src_stride>>1);
  for (i = 0; i < h; i++) {
    libaroma_color_copy32(
      dst+dline*i, src+sline*i, w, rgb_pos
    );
  }
}
void libaroma_blt_align_to16_pos(wordp dst, dwordp src,
    int w, int h, int dst_stride, int src_stride,
    bytep rgb_pos) {
  int i;
  int dline = w+(dst_stride>>1);
  int sline = w+(src_stride>>2);
  for (i = 0; i < h; i++) {
    libaroma_color_copy16(
      dst+dline*i, src+sline*i, w, rgb_pos
    );
  }
}


#endif /* __libaroma_engine_c__ */
