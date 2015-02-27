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
 * Filename    : alpha_neon.c
 * Description : neon simd alpha blend engine
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_alpha_neon_c__
#define __libaroma_alpha_neon_c__
#ifdef __ARM_HAVE_NEON

/* constants neon variables */
static byte __neon_const_initialized=0;
static uint16x8_t __neon_const_xalph;
static uint16x8_t __neon_const_zalph;
static uint16x8_t __neon_const_falph;
static uint16x8_t __neon_const_msk_r;
static uint16x8_t __neon_const_msk_g;
static uint16x8_t __neon_const_msk_b;

/* init constants */
inline void __neon_alpha_const_init(){
  if (!__neon_const_initialized){
    __neon_const_xalph = vdupq_n_u16(0xff);   /* max value */
    __neon_const_zalph = vdupq_n_u16(0);      /* zero value */
    __neon_const_falph = vdupq_n_u16(0x100);  /* reverse value */
    __neon_const_msk_r = vdupq_n_u16(0xf800); /* red mask */
    __neon_const_msk_g = vdupq_n_u16(0x07e0); /* green mask */
    __neon_const_msk_b = vdupq_n_u16(0x001f); /* blue mask */
    __neon_const_initialized=1;
  }
}

/* init dither table */
inline void __neon_dither_table(
    int y, uint16x8_t *r, uint16x8_t *g, uint16x8_t *b
){
  uint8x8_t tr,tg,tb;
  _libaroma_neon_dither_table(y, &tr, &tg, &tb);
  *r = vmovl_u8(tr);
  *g = vmovl_u8(tg);
  *b = vmovl_u8(tb);
}

/* dithered alpha blending */
void libaroma_alpha_px_line(
    int _Y, int n, wordp dst,
    wordp bottom, wordp top, bytep alpha){
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxb, pxt, rbl, gbl, bbl, rtl, gtl, btl, o, op, ro;
    
    /* dithering data */
    uint16x8_t table_r16,table_g16,table_b16;
    __neon_dither_table(
      _Y, &table_r16, &table_g16, &table_b16
    );
    for (i=0;i<n-left;i+=8) {
      /* prepare opacity and reversed opacity value */
      op = vmovl_u8(vld1_u8(alpha+i));
      ro = vsubq_u16(__neon_const_falph, op);
      
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
      pxt = vld1q_u16(top+i); /* top */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro);
      
      /* top blend */
      rtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_r),8),op);
      gtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_g),3),op);
      btl = vmulq_u16(vshlq_n_u16(vandq_u16(pxt,__neon_const_msk_b),3),op);
      
      /* dither blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 8);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 8);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 8);
      
      /* dithering */
      rbl = vminq_u16(vaddq_u16(rbl,table_r16),__neon_const_xalph);
      gbl = vminq_u16(vaddq_u16(gbl,table_g16),__neon_const_xalph);
      bbl = vminq_u16(vaddq_u16(bbl,table_b16),__neon_const_xalph);
      rbl = vqshlq_n_u16(vshrq_n_u16(rbl,3),11);
      gbl = vqshlq_n_u16(vshrq_n_u16(gbl,2),5);
      bbl = vshrq_n_u16(bbl,3);
      
      /* out value */
      o = vorrq_u16(vorrq_u16(rbl,gbl),bbl);
      
      /* comparison opaque/transparent */
      o = vbslq_u16(vceqq_u16(op,__neon_const_xalph),pxt,o); /*opacity*/
      o = vbslq_u16(vceqq_u16(op,__neon_const_zalph),pxb,o); /*transparent*/
      
      vst1q_u16(dst+i,o);
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_dither(
        i, _Y, libaroma_alpha32(bottom[i], top[i], alpha[i]));
    }
  }
}

/* non-dithered alpha blending */
void libaroma_alpha_px(
    int n, wordp dst,
    wordp bottom, wordp top, bytep alpha) {
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxb, pxt, rbl, gbl, bbl, rtl, gtl, btl, o, op, ro;
    for (i=0;i<n-left;i+=8) {
      /* prepare opacity and reversed opacity value */
      op = vmovl_u8(vld1_u8(alpha+i));
      ro = vsubq_u16(__neon_const_falph, op);
      
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
      pxt = vld1q_u16(top+i); /* top */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro);
      
      /* top blend */
      rtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_r),8),op);
      gtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_g),3),op);
      btl = vmulq_u16(vshlq_n_u16(vandq_u16(pxt,__neon_const_msk_b),3),op);
      
      /* non-dithering blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 11);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 10);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 11);
      rbl = vshlq_n_u16(rbl, 11);
      gbl = vshlq_n_u16(gbl, 5);
      
      /* out value */
      o = vorrq_u16(vorrq_u16(rbl,gbl),bbl);
      
      /* comparison opaque/transparent */
      o = vbslq_u16(vceqq_u16(op,__neon_const_xalph),pxt,o); /*opacity*/
      o = vbslq_u16(vceqq_u16(op,__neon_const_zalph),pxb,o); /*transparent*/
      
      vst1q_u16(dst+i,o);
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_alpha(bottom[i], top[i], alpha[i]);
    }
  }
}

/* dithered alpha blending - constant alpha */
void libaroma_alpha_const_line(
    int _Y, int n, wordp dst,
    wordp bottom, wordp top, byte alpha) {
  if (alpha==0xff){
    if (dst!=top){
      memcpy(dst,top,n*2);
    }
    return;
  }
  else if (alpha==0){
    if (dst!=bottom){
      memcpy(dst,bottom,n*2);
    }
    return;
  }
  
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxb, pxt, rbl, gbl, bbl, rtl, gtl, btl, op, ro;
    
    /* dithering data */
    uint16x8_t table_r16,table_g16,table_b16;
    __neon_dither_table(
      _Y, &table_r16, &table_g16, &table_b16
    );
    
    /* constant alpha */
    op = vdupq_n_u16(alpha);
    ro = vsubq_u16(__neon_const_falph, op);
    for (i=0;i<n-left;i+=8) {
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
      pxt = vld1q_u16(top+i); /* top */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro);
      
      /* top blend */
      rtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_r),8),op);
      gtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_g),3),op);
      btl = vmulq_u16(vshlq_n_u16(vandq_u16(pxt,__neon_const_msk_b),3),op);
      
      /* dither blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 8);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 8);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 8);
      
      /* dithering */
      rbl = vminq_u16(vaddq_u16(rbl,table_r16),__neon_const_xalph);
      gbl = vminq_u16(vaddq_u16(gbl,table_g16),__neon_const_xalph);
      bbl = vminq_u16(vaddq_u16(bbl,table_b16),__neon_const_xalph);
      rbl = vqshlq_n_u16(vshrq_n_u16(rbl,3),11);
      gbl = vqshlq_n_u16(vshrq_n_u16(gbl,2),5);
      bbl = vshrq_n_u16(bbl,3);
      
      /* out value */
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(rbl,gbl),bbl));
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_dither(
        i, _Y, libaroma_alpha32(bottom[i], top[i], alpha));
    }
  }
}

/* non-dithered alpha blending - constant alpha */
void libaroma_alpha_const(
    int n, wordp dst,
    wordp bottom, wordp top, byte alpha) {
  if (alpha==0xff){
    if (dst!=top){
      memcpy(dst,top,n*2);
    }
    return;
  }
  else if (alpha==0){
    if (dst!=bottom){
      memcpy(dst,bottom,n*2);
    }
    return;
  }
  
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxb, pxt, rbl, gbl, bbl, rtl, gtl, btl, op, ro;
    /* constant alpha */
    op = vdupq_n_u16(alpha);
    ro = vsubq_u16(__neon_const_falph, op);
    for (i=0;i<n-left;i+=8) {  
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
      pxt = vld1q_u16(top+i); /* top */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro);
      
      /* top blend */
      rtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_r),8),op);
      gtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_g),3),op);
      btl = vmulq_u16(vshlq_n_u16(vandq_u16(pxt,__neon_const_msk_b),3),op);
      
      /* non-dithering blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 11);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 10);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 11);
      rbl = vshlq_n_u16(rbl, 11);
      gbl = vshlq_n_u16(gbl, 5);
      
      /* out value */
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(rbl,gbl),bbl));
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_alpha(bottom[i], top[i], alpha);
    }
  }
}

/* alpha blending with black */
void libaroma_alpha_black(
    int n, wordp dst,
    wordp top, byte alpha) {
  if (alpha==0xff){
    if (dst!=top){
      memcpy(dst,top,n*2);
    }
    return;
  }
  else if (alpha==0){
    memset(dst,0,n*2);
    return;
  }
  
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxt, rtl, gtl, btl, op;
    /* constant alpha */
    op = vdupq_n_u16(alpha);
    for (i=0;i<n-left;i+=8) {  
      /* get 8 pixels data from top & bottom layer */
      pxt = vld1q_u16(top+i); /* top */
      /* top blend */
      rtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_r),8),op);
      gtl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxt,__neon_const_msk_g),3),op);
      btl = vmulq_u16(vshlq_n_u16(vandq_u16(pxt,__neon_const_msk_b),3),op);
      
      /* non-dithering blend result */
      rtl  = vshrq_n_u16(rtl, 11);
      gtl  = vshrq_n_u16(gtl, 10);
      btl  = vshrq_n_u16(btl, 11);
      rtl = vshlq_n_u16(rtl, 11);
      gtl = vshlq_n_u16(gtl, 5);
      
      /* out value */
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(rtl,gtl),btl));
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_alphab(top[i], alpha);
    }
  }
}

/* alphablending - constant top & alpha */
void libaroma_alpha_rgba_fill(
    int n, wordp dst,
    wordp bottom, word top, byte alpha) {
  if (alpha==0xff){
    libaroma_color_set(dst,top,n);
    return;
  }
  else if (alpha==0){
    if (dst!=bottom){
      memcpy(dst,bottom,n*2);
    }
    return;
  }
  
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxb, rbl, gbl, bbl, rtl, gtl, btl, ro;
    
    /* constant top & alpha */
    rtl = vdupq_n_u16(libaroma_color_r(top)*alpha);
    gtl = vdupq_n_u16(libaroma_color_g(top)*alpha);
    btl = vdupq_n_u16(libaroma_color_b(top)*alpha);
    ro  = vdupq_n_u16(0x100-alpha);
    for (i=0;i<n-left;i+=8) {
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro);
      
      /* non-dithering blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 11);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 10);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 11);
      rbl = vshlq_n_u16(rbl, 11);
      gbl = vshlq_n_u16(gbl, 5);
      
      /* out value */
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(rbl,gbl),bbl));
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_alpha(bottom[i], top, alpha);
    }
  }
}

/* alphablending - constant top dynamic alpha */
void libaroma_alpha_mono(int n, wordp dst, wordp bottom,
    word top, bytep alpha){
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint16x8_t pxb, pxt, rbl, gbl, bbl, rtl, gtl, btl, o, op, ro,rts,gts,bts;
    pxt = vdupq_n_u16(top);
    rts = vdupq_n_u16(libaroma_color_r(top));
    gts = vdupq_n_u16(libaroma_color_g(top));
    bts = vdupq_n_u16(libaroma_color_b(top));
    for (i=0;i<n-left;i+=8) {
      /* prepare opacity and reversed opacity value */
      op = vmovl_u8(vld1_u8(alpha+i));
      ro = vsubq_u16(__neon_const_falph, op);
      
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro);
      
      /* top blend */
      rtl = vmulq_u16(rts,op);
      gtl = vmulq_u16(gts,op);
      btl = vmulq_u16(bts,op);
      
      /* non-dithering blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 11);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 10);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 11);
      rbl = vshlq_n_u16(rbl, 11);
      gbl = vshlq_n_u16(gbl, 5);
      
      /* out value */
      o = vorrq_u16(vorrq_u16(rbl,gbl),bbl);
      
      /* comparison opaque/transparent */
      o = vbslq_u16(vceqq_u16(op,__neon_const_xalph),pxt,o); /*opacity*/
      o = vbslq_u16(vceqq_u16(op,__neon_const_zalph),pxb,o); /*transparent*/
      
      vst1q_u16(dst+i,o);
    }
  }
  /* leftover */
  if (left>0) {
    for (i=n-left;i<n;i++) {
      dst[i] = libaroma_alpha(bottom[i], top, alpha[i]);
    }
  }
}

/* subpixel alphablending  */
void libaroma_alpha_multi_line(int n, wordp dst, wordp bottom,
    word top, bytep alphargb) {
  int i,left=n%8;
  
  /* neon */
  if (n>=8){
    __neon_alpha_const_init();
    uint8x8x3_t alphabuf;
    uint16x8_t pxb, rbl, gbl, bbl, rtl, gtl, btl, rts, gts, bts,
      op_r,op_g,op_b,ro_r,ro_g,ro_b;
    rts = vdupq_n_u16(libaroma_color_r(top));
    gts = vdupq_n_u16(libaroma_color_g(top));
    bts = vdupq_n_u16(libaroma_color_b(top));
    for (i=0;i<n-left;i+=8) {
      /* 24bit opacity */
      alphabuf = vld3_u8(alphargb+(i*3));
      
      /* prepare opacity and reversed opacity value */
      op_r = vmovl_u8(alphabuf.val[0]);
      op_g = vmovl_u8(alphabuf.val[1]);
      op_b = vmovl_u8(alphabuf.val[2]);
      ro_r = vsubq_u16(__neon_const_falph, op_r);
      ro_g = vsubq_u16(__neon_const_falph, op_g);
      ro_b = vsubq_u16(__neon_const_falph, op_b);
      
      /* get 8 pixels data from top & bottom layer */
      pxb = vld1q_u16(bottom+i); /* bottom */
  
      /* bottom blend */
      rbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_r),8),ro_r);
      gbl = vmulq_u16(vrshrq_n_u16(vandq_u16(pxb,__neon_const_msk_g),3),ro_g);
      bbl = vmulq_u16(vshlq_n_u16(vandq_u16(pxb,__neon_const_msk_b),3),ro_b);
      
      /* top blend */
      rtl = vmulq_u16(rts,op_r);
      gtl = vmulq_u16(gts,op_g);
      btl = vmulq_u16(bts,op_b);
      
      /* non-dithering blend result */
      rbl  = vshrq_n_u16(vaddq_u16(rbl, rtl), 11);
      gbl  = vshrq_n_u16(vaddq_u16(gbl, gtl), 10);
      bbl  = vshrq_n_u16(vaddq_u16(bbl, btl), 11);
      rbl = vshlq_n_u16(rbl, 11);
      gbl = vshlq_n_u16(gbl, 5);
      
      /* out value */
      vst1q_u16(dst+i,vorrq_u16(vorrq_u16(rbl,gbl),bbl));
    }
  }
  /* leftover */
  if (left>0) {
    int j;
    for (i=n-left;i<n;i++) {
      j=i*3;
      dst[i] = libaroma_alpha_multi(
        bottom[i],
        top,
        alphargb[j],
        alphargb[j+1],
        alphargb[j+2]
      );
    }
  }
}

/* set available engine */
#define __engine_have_libaroma_alpha_black 1
#define __engine_have_libaroma_alpha_const 1
#define __engine_have_libaroma_alpha_const_line 1
#define __engine_have_libaroma_alpha_px 1
#define __engine_have_libaroma_alpha_px_line 1
#define __engine_have_libaroma_alpha_rgba_fill 1
#define __engine_have_libaroma_alpha_mono 1
#define __engine_have_libaroma_alpha_multi_line 1

#endif /* __ARM_HAVE_NEON */
#endif /* __libaroma_alpha_neon_c__ */
