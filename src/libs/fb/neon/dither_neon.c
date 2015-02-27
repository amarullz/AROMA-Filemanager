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
 * Filename    : dither_neon.c
 * Description : neon simd dither engine
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_dither_neon_c__
#define __libaroma_dither_neon_c__
#ifdef __ARM_HAVE_NEON

/* Get Dither Table */
void _libaroma_neon_dither_table(
    int y,
    uint8x8_t * table_r,
    uint8x8_t * table_g,
    uint8x8_t * table_b) {
  byte table_p = ((y & 7) << 3);
  /* gather dither table */
  *table_r = vld1_u8(libaroma_dither_tresshold_r + table_p);
  *table_g = vld1_u8(libaroma_dither_tresshold_g + table_p);
  *table_b = vld1_u8(libaroma_dither_tresshold_b + table_p);
}

/* NEON SIMD Line Dither */
void libaroma_dither_line(int y, int w, wordp dst, const dwordp src) {
  int i,left=w%8;
  
  /* neon */
  if (w>=8){
    uint8x8_t table_r, table_g, table_b;
    _libaroma_neon_dither_table(y,&table_r,&table_g,&table_b);
    uint16x8_t ff = vmovq_n_u16(255);
    uint8_t * nsrc = (uint8_t *) src;
    uint8x8x4_t rgba;
    uint16x8_t r,g,b;
    for (i=0;i<w-left;i+=8) {
      rgba = vld4_u8(nsrc + i*4);
      r = vminq_u16(vaddl_u8(rgba.val[2], table_r),ff);
      g = vminq_u16(vaddl_u8(rgba.val[1], table_g),ff);
      b = vminq_u16(vaddl_u8(rgba.val[0], table_b),ff);
      r = vqshlq_n_u16(vshrq_n_u16(r,3),11);
      g = vqshlq_n_u16(vshrq_n_u16(g,2),5);
      b = vshrq_n_u16(b, 3);
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(r,g),b));
    }
  }
  /* leftover */
  if (left>0){
    for (i=w-left;i<w;i++) {
      dst[i] = libaroma_dither(i, y, src[i]);
    }
  }
}

/* NEON SIMD Constant Line Dither */
void libaroma_dither_line_const(int y, int w, wordp dst, const dword src) {
  int i,left=w%8;
  
  /* neon */
  if (w>=8){
    uint8x8_t table_r, table_g, table_b;
    _libaroma_neon_dither_table(y,&table_r,&table_g,&table_b);
    uint16x8_t ff = vmovq_n_u16(255);
    uint8x8_t src_r = vmov_n_u8(libaroma_color_r32(src));
    uint8x8_t src_g = vmov_n_u8(libaroma_color_g32(src));
    uint8x8_t src_b = vmov_n_u8(libaroma_color_b32(src));
    uint16x8_t r,g,b;
    for (i=0;i<w-left;i+=8) {
      r = vminq_u16(vaddl_u8(src_r, table_r),ff);
      g = vminq_u16(vaddl_u8(src_g, table_g),ff);
      b = vminq_u16(vaddl_u8(src_b, table_b),ff);
      r = vqshlq_n_u16(vshrq_n_u16(r,3),11);
      g = vqshlq_n_u16(vshrq_n_u16(g,2),5);
      b = vshrq_n_u16(b, 3);
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(r,g),b));
    }
  }
  /* leftover */
  if (left>0){
    for (i=w-left;i<w;i++) {
      dst[i] = libaroma_dither(i,y,src);
    }
  }
}

/* set available engine */
#define __engine_have_libaroma_dither_line 1
#define __engine_have_libaroma_dither_line_const 1

#endif /* __ARM_HAVE_NEON */
#endif /* __libaroma_dither_neon_c__ */
