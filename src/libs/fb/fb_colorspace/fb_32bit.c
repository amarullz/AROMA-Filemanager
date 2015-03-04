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
 * Filename    : fb_32bit.c
 * Description : linux framebuffer driver for 32bit framebuffer
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb32bit_driver_c__
#define __libaroma_linux_fb32bit_driver_c__

/*
 * function : set framebuffer rgb position
 */
void LINUXFBDR_setrgbpos(LIBAROMA_FBP me, byte r, byte g, byte b) {
  if (me == NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  /* save color position */
  mi->rgb_pos[0] = r;
  mi->rgb_pos[1] = g;
  mi->rgb_pos[2] = b;
  mi->rgb_pos[3] = r >> 3;
  mi->rgb_pos[4] = g >> 3;
  mi->rgb_pos[5] = b >> 3;
}

/*
 * function : init framebuffer colorspace
 */
void LINUXFBDR_init_32bit(LIBAROMA_FBP me) {
  if (me == NULL) {
    return;
  }
  ALOGS("LINUXFBDR Init 32bit Colorspace");
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  
  mi->stride = mi->line - (me->w * mi->pixsz); /* calculate stride size */

  /* gralloc framebuffer subpixel position style */
  if (mi->var.transp.offset){
    LINUXFBDR_setrgbpos(me,16,8,0);
  }
  else{
    LINUXFBDR_setrgbpos(me,0,8,16);
  }
}

/*
 * function : save display canvas into framebuffer
 */
byte LINUXFBDR_sync_32bit(
    LIBAROMA_FBP me,
    wordp src,
    int x,
    int y,
    int w,
    int h) {
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  pthread_mutex_lock(&mi->mutex);
  if ((w > 0) && (h > 0) && (!mi->double_buffering)) {
    int copy_stride = me->w - w;
    dwordp copy_dst =
      (dwordp) (((bytep) mi->current_buffer) + (mi->line * y) + (x * mi->pixsz));
    wordp copy_src =
      (wordp) (src + (me->w * y) + x);
    libaroma_blt_align_to32_pos(
      copy_dst, copy_src,
      w, h,
      mi->stride + (copy_stride * mi->pixsz), copy_stride * 2,
      mi->rgb_pos
    );
  }
  else {
    libaroma_blt_align_to32_pos(
      (dwordp) mi->current_buffer, src,
      me->w, me->h, mi->stride, 0,
      mi->rgb_pos);
  }
  pthread_cond_signal(&mi->cond);
  pthread_mutex_unlock(&mi->mutex);
  return 1;
}

/*
 * function : save framebuffer into display canvas
 */
byte LINUXFBDR_snapshoot_32bit(LIBAROMA_FBP me, wordp dst) {
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  libaroma_blt_align_to16_pos(
    dst, (dwordp) mi->buffer, me->w, me->h,
    0, mi->stride, mi->rgb_pos);
  return 1;
}

#endif /* __libaroma_linux_fb32bit_driver_c__ */ 
