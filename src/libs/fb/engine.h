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
 * Filename    : engine.h
 * Description : graph engine
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 19/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_engine_h__
#define __libaroma_engine_h__

/* Color Engine */
#ifdef RGB
#undef RGB
#endif
#define RGB(X) libaroma_rgb_to16(0x##X)
word libaroma_rgb_from_string(const char * c);
byte libaroma_color_r(word rgb);
byte libaroma_color_g(word rgb);
byte libaroma_color_b(word rgb);
byte libaroma_color_hi_r(byte v);
byte libaroma_color_hi_g(byte v);
#define libaroma_color_hi_b(v) libaroma_color_hi_r(v)
byte libaroma_color_luminance(word rgb);
byte libaroma_color_isdark(word rgb);
byte libaroma_color_r32(dword rgb);
byte libaroma_color_g32(dword rgb);
byte libaroma_color_b32(dword rgb);
byte libaroma_color_a32(dword rgb);
byte libaroma_color_close_r(byte c);
byte libaroma_color_close_g(byte c);
#define libaroma_color_close_b libaroma_color_close_r
byte libaroma_color_left(byte r, byte g, byte b);
dword libaroma_color_merge(word color, byte hicolor);
byte libaroma_color_merge_r(word color, byte hicolor);
byte libaroma_color_merge_g(word color, byte hicolor);
byte libaroma_color_merge_b(word color, byte hicolor);
word libaroma_rgb(byte r, byte g, byte b);
dword libaroma_rgba(byte r, byte g, byte b, byte a);
dword libaroma_rgb32(byte r, byte g, byte b);
word libaroma_rgb_to16(dword rgb);
dword libaroma_rgb_to32(word rgb);
dword libaroma_rgb_to_rgba(word rgb, byte alpha);
void libaroma_color_set(wordp dst, word color, int n);
void libaroma_color_set32(dwordp dst, dword color, int n);
void libaroma_color_copy32(dwordp dst, wordp src, int n, bytep rgb_pos);
void libaroma_color_copy16(wordp dst, dwordp src, int n, bytep rgb_pos);

/* Blt Engine */
void libaroma_btl16(int n, wordp dst, const dwordp src);
void libaroma_btl32(int n, dwordp dst, const wordp src);

/* Dither Engine */
byte libaroma_dither_table_pos(int x, int y);
byte libaroma_dither_r(byte p);
byte libaroma_dither_g(byte p);
byte libaroma_dither_b(byte p);
word libaroma_dither_rgb(int x, int y, byte sr, byte sg, byte sb);
word libaroma_dither_mono_rgb(int x, int y, byte sr, byte sg, byte sb);
word libaroma_dither_mono(int x, int y, dword col);
word libaroma_dither(int x, int y, dword col);
void libaroma_dither_line(int y, int w, wordp dst, const dwordp src);
void libaroma_dither_line_const(int y, int w, wordp dst, dword src);

/* Alphablend Engine */
word libaroma_alpha(word dcl, word scl, byte l);
dword libaroma_alpha32(word dcl, word scl, byte l);
word libaroma_alpha_multi(word dcl, word scl, byte lr, byte lg, byte lb);
word libaroma_alphab(word scl, byte l);
void libaroma_alpha_black(int n, wordp dst, wordp top, byte alpha);
void libaroma_alpha_const(int n, wordp dst,
    wordp bottom, wordp top, byte alpha);
void libaroma_alpha_const_line(int _Y, int n, wordp dst,
    wordp bottom, wordp top, byte alpha);
void libaroma_alpha_px(int n, wordp dst, wordp bottom,
    wordp top, bytep alpha);
void libaroma_alpha_px_line(int _Y, int n, wordp dst,
    wordp bottom, wordp top, bytep alpha);
void libaroma_alpha_rgba_fill(int n, wordp dst, wordp bottom,
    word top, byte alpha);
void libaroma_alpha_mono(int n, wordp dst, wordp bottom,
    word top, bytep alpha);
void libaroma_alpha_multi_line(int n, wordp dst, wordp bottom,
    word top, bytep alphargb);

/* Aligned blit Engine */
void libaroma_blt_align16(wordp dst, wordp src,
    int w, int h, int dst_stride, int src_stride);
void libaroma_blt_align32_to16(wordp dst, dwordp src,
    int w, int h, int dst_stride, int src_stride);
void libaroma_blt_align16_to32(dwordp dst, wordp src,
    int w, int h, int dst_stride, int src_stride);
void libaroma_blt_align32(dwordp dst, dwordp src,
    int w, int h, int dst_stride, int src_stride);
void libaroma_blt_align_to32_pos(dwordp dst, wordp src,
    int w, int h, int dst_stride, int src_stride,
    bytep rgb_pos);
void libaroma_blt_align_to16_pos(wordp dst, dwordp src,
    int w, int h, int dst_stride, int src_stride,
    bytep rgb_pos);

#endif /* __libaroma_engine_h__ */
