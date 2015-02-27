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
 * Filename    : fb_16bit.c
 * Description : linux framebuffer driver for 16bit framebuffer
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb16bit_driver_c__
#define __libaroma_linux_fb16bit_driver_c__

/*
 * function : init framebuffer colorspace
 */
void LINUXFBDR_init_16bit(LIBAROMA_FBP me) {
  if (me == NULL) {
    return;
  }
  LOGS("LINUXFBDR Init 16bit Colorspace\n");
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  
  /* fix not standard 16bit framebuffer */
  if (mi->line / 4 == me->w) {
    mi->line = mi->line / 2;
  }
  mi->stride = mi->line - (me->w * 2); /* calculate stride size */
}

/*
 * function : save display canvas into framebuffer
 */
byte LINUXFBDR_sync_16bit(
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
  
  if ((w > 0) && (h > 0)) {
    /* defined area only */
    int copy_stride = me->w-w;
    wordp copy_dst =
      (wordp)  (((bytep) mi->buffer) + (mi->line * y) + (x * mi->pixsz));
    wordp copy_src =
      (wordp) (src + (me->w * y) + x);
    libaroma_blt_align16(copy_dst, copy_src, w, h,
      mi->stride + (copy_stride * mi->pixsz),
      copy_stride * 2
    );
  }
  else {
    /* whole screen */
    libaroma_blt_align16((wordp) mi->buffer,
      (wordp) src,
      me->w, me->h, mi->stride, 0
    );
  }
  return 1;
}

/*
 * function : save framebuffer into display canvas
 */
byte LINUXFBDR_snapshoot_16bit(LIBAROMA_FBP me, wordp dst) {
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  libaroma_blt_align16(
    dst, (wordp) mi->buffer, me->w, me->h, 0, mi->stride);
  return 1;
}

#endif /* __libaroma_linux_fb16bit_driver_c__ */
