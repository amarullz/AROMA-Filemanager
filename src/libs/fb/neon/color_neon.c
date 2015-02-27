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
 * Filename    : color_neon.c
 * Description : neon simd color engine
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_color_neon_c__
#define __libaroma_color_neon_c__
#ifdef __ARM_HAVE_NEON

/* set color buffer */
/* 512 & 256bit vector */
void libaroma_color_set(wordp dst, word color, int n) {
#ifdef libaroma_memset16
  libaroma_memset16(dst,color,n);
#else
  int i,left=n%32;
  if (n>=32){
    /* use 512bit vector */
    uint16x8x4_t t_clr;
    t_clr.val[0]= t_clr.val[1]= t_clr.val[2]= t_clr.val[3]=vdupq_n_u16(color);
    for (i=0;i<n-left;i+=32) {
      vst4q_u16(dst+i, t_clr);
    }
  }
  if (left>0){
    for (i=n-left;i<n;i++) {
      dst[i]=color;
    }
  }
#endif
}

/* 16bit to 32bit */
void libaroma_color_copy32(dwordp dst, wordp src, int n, bytep rgb_pos) {
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    uint16x8_t msk_r = vdupq_n_u16(0xF800); /* Red Mask */
    uint16x8_t msk_g = vdupq_n_u16(0x07E0); /* Green Mask */
    uint16x8_t msk_b = vdupq_n_u16(0x001F); /* Blue Mask */
    
    /* vars */
    uint16x8_t psrc;
    uint8x8x4_t n_dst;
#ifdef LIBAROMA_CONFIG_USE_HICOLOR_BIT
    uint8x8_t r, g, b;
#endif

    for (i=0;i<n-left;i+=8) {
      /* load source color */
      psrc = vld1q_u16(src+i);
#ifdef LIBAROMA_CONFIG_USE_HICOLOR_BIT
      /* get subpixels of source color */
      r = vshrn_n_u16(vandq_u16(psrc,msk_r),8);
      g = vshrn_n_u16(vandq_u16(psrc,msk_g),3);
      b = vmovn_u16(vshlq_n_u16(vandq_u16(psrc,msk_b),3));
      
      /* small byte left : 11111xxx 111111xx 11111xxx */
      n_dst.val[rgb_pos[3]] = vorr_u8(r,vshr_n_u8(r,5));
      n_dst.val[rgb_pos[4]] = vorr_u8(g,vshr_n_u8(g,6));
      n_dst.val[rgb_pos[5]] = vorr_u8(b,vshr_n_u8(b,5));
#else
      n_dst.val[rgb_pos[3]] = vshrn_n_u16(vandq_u16(psrc,msk_r),8);
      n_dst.val[rgb_pos[4]] = vshrn_n_u16(vandq_u16(psrc,msk_g),3);
      n_dst.val[rgb_pos[5]] = vmovn_u16(vshlq_n_u16(vandq_u16(psrc,msk_b),3));
#endif
      /* dump it */
      vst4_u8((uint8_t *) (dst+i), n_dst);
    }
  }
  
  /* leftover */
  if (left>0){
    word cl;
    for (i=n-left;i<n;i++) {
      cl = src[i];
#ifdef LIBAROMA_CONFIG_USE_HICOLOR_BIT
      dst[i] = (
        (libaroma_color_hi_r(libaroma_color_r(cl)) << rgb_pos[0]) |
        (libaroma_color_hi_g(libaroma_color_g(cl)) << rgb_pos[1]) |
        (libaroma_color_hi_b(libaroma_color_b(cl)) << rgb_pos[2])
      );
#else
      dst[i] = (
        (libaroma_color_r(cl) << rgb_pos[0]) |
        (libaroma_color_g(cl) << rgb_pos[1]) |
        (libaroma_color_b(cl) << rgb_pos[2])
      );
#endif
    }
  }
}


/* 32bit to 26bit */
void libaroma_color_copy16(wordp dst, dwordp src, int n, bytep rgb_pos) {
  int i,left=n%8;
  /* neon */
  if (n>=8) {
    uint8x8x4_t psrc;
    for (i=0;i<n-left;i+=8) {
      psrc = vld4_u8((uint8_t *) (src+i));
      vst1q_u16(
        dst+i, 
        vorrq_u16(
          vorrq_u16(
            vshlq_n_u16(vmovl_u8(vshr_n_u8(psrc.val[rgb_pos[3]], 3)), 11),
            vshlq_n_u16(vmovl_u8(vshr_n_u8(psrc.val[rgb_pos[4]], 2)), 5)
          ),
          vmovl_u8(vshr_n_u8(psrc.val[rgb_pos[5]], 3))
        )
      );
    }
  }
  
  /* leftover */
  if (left>0){
    dword cl;
    for (i=n-left;i<n;i++) {
      cl = src[i];
      dst[i] = libaroma_rgb(
        (byte) ((cl>>rgb_pos[0])&0xff),
        (byte) ((cl>>rgb_pos[1])&0xff),
        (byte) ((cl>>rgb_pos[2])&0xff)
      );
    }
  }
}

/* set available engine */
#define __engine_have_libaroma_color_set 1
#define __engine_have_libaroma_color_copy32 1
#define __engine_have_libaroma_color_copy16 1

/* TODO: __engine_have_libaroma_color_set32 */

#endif /* __ARM_HAVE_NEON */
#endif /* __libaroma_color_neon_c__ */
