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
 * Filename    : fb_driver.c
 * Description : linux framebuffer driver
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_driver_c__
#define __libaroma_linux_fb_driver_c__

/* include fb_driver.h */
#include "fb_driver.h"
#include "fb_colorspace/fb_16bit.c" /* 16 bit */
#include "fb_colorspace/fb_32bit.c" /* 32 bit */
#include "fb_qcom/fb_qcom.c" /* qcom overlay */

/*
 * Function    : LINUXFBDR_init
 * Return Value: byte
 * Descriptions: init framebuffer
 */
byte LINUXFBDR_init(LIBAROMA_FBP me) {
  ALOGV("LINUXFBDR initialized internal data");
  
  /* allocating internal data */
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP)
                      malloc(sizeof(LINUXFBDR_INTERNAL));
  if (!mi) {
    ALOGE("LINUXFBDR malloc internal data - memory error");
    return 0;
  }

  /* cleanup */
  memset(mi, 0, sizeof(LINUXFBDR_INTERNAL));
  
  /* set internal address */
  me->internal = (voidp) mi;
  
  /* set release callback */
  me->release = &LINUXFBDR_release;
  
  /* init mutex & cond */
  pthread_mutex_init(&mi->mutex,NULL);
  pthread_cond_init(&mi->cond,NULL);

  /* open framebuffer device */
  mi->fb = open(LINUXFBDR_DEVICE, O_RDWR, 0);
  if (mi->fb < 1) {
    /* if not works, try non android standard device path */
    mi->fb = open(LINUXFBDR_DEVICE_NON_ANDROID, O_RDWR, 0);
  }
  if (mi->fb < 1) {
    /* cannot find device */
    ALOGE("LINUXFBDR no framebuffer device");
    goto error; /* exit if error */
  }
  
  /* get framebuffer var & fix data */
  ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix); /* fix info */
  ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var); /* var info */
  
  /* set libaroma framebuffer instance values */
  me->w        = mi->var.xres;  /* width */
  me->h        = mi->var.yres;  /* height */
  me->sz       = me->w*me->h;   /* width x height */
  
  if (QCOMFB_init(me)){
    /* qcom fb */
    me->sync = &QCOMFB_sync;
    me->snapshoot=NULL;
    ALOGI("using qcom framebuffer driver");
  }
  else{
    /* it's not qcom */
    ALOGI("not using qcom framebuffer driver");
    
    if ((mi->var.bits_per_pixel != 32) && (mi->var.bits_per_pixel != 16)) {
      /* non 32/16bit colorspace is not supported */
      ALOGE("LINUXFBDR bits_per_pixel=%i not supported",
        mi->var.bits_per_pixel);
      goto error;
    }
    
    /* init features - double buffer, vsync */
    LINUXFBDR_init_features(me);
    
    /* set internal useful data */
    mi->line      = mi->fix.line_length;      /* line memory size */
    mi->depth     = mi->var.bits_per_pixel;   /* color depth */
    mi->pixsz     = mi->depth >> 3;           /* pixel size per byte */
    mi->fb_sz     = (mi->var.xres_virtual * mi->var.yres_virtual * mi->pixsz);
    
    if (mi->fix.smem_len<(dword) mi->fb_sz){
      /* smem_len is invalid */
      ALOGE("LINUXFBDR smem_len(%i) < fb_sz(%i)", mi->fix.smem_len, mi->fb_sz);
      goto error;
    }
    
    /* map buffer */
    ALOGV("LINUXFBDR mmap Framebuffer Memory");
    mi->buffer  = (voidp) mmap(
                    0, mi->fix.smem_len,
                    PROT_READ | PROT_WRITE, MAP_SHARED,
                    mi->fb, 0
                  );
  
    if (!mi->buffer) {
      ALOGE("LINUXFBDR mmap framebuffer memory error");
      goto error;
    }
    
    /* swap buffer now */
    LINUXFBDR_swap_buffer(mi);
    LINUXFBDR_flush(me);
    
    if (mi->pixsz == 2) {
      /* not 32bit depth */
      mi->is32 = 0;
      /* init colorspace */
      LINUXFBDR_init_16bit(me);
      /* set sync callbacks */
      me->sync     = &LINUXFBDR_sync_16bit;
      me->snapshoot = &LINUXFBDR_snapshoot_16bit;
    }
    else {
      mi->is32 = 1;
      /* init colorspace */
      LINUXFBDR_init_32bit(me);
      /* set sync callbacks */
      me->sync     = &LINUXFBDR_sync_32bit;
      me->snapshoot = &LINUXFBDR_snapshoot_32bit;
    }
  }
  
  /* set dpi */
  LINUXFBDP_set_dpi(me);
  
  /* start flush receiver */
  mi->active=1;
  pthread_create(&mi->thread,NULL,LINUXFBDR_flush_receiver,(void *) me);
  
  /* dump info */
  LINUXFBDR_dump(mi);
  
  /* ok */
  goto ok;
  /* return */
error:
  free(mi);
  return 0;
ok:
  return 1;
} /* End of LINUXFBDR_init */

/*
 * Function    : LINUXFBDR_release
 * Return Value: void
 * Descriptions: release framebuffer driver
 */
void LINUXFBDR_release(LIBAROMA_FBP me) {
  if (me==NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  if (mi==NULL){
    return;
  }
  
  /* terminate flush thread */
  mi->active=0;
  pthread_mutex_lock(&mi->mutex);
  pthread_cond_signal(&mi->cond);
  pthread_mutex_unlock(&mi->mutex);
  pthread_join(mi->thread,NULL);
  
  if (mi->qcom!=NULL){
    /* release qcom overlay driver */
    QCOMFB_release(me);
  }
  
  /* unmap */
  if (mi->buffer!=NULL){
    ALOGV("LINUXFBDR munmap buffer");
    munmap(mi->buffer, mi->fix.smem_len);
  }
  
  /* close fb */
  ALOGV("LINUXFBDR close fb-fd");
  close(mi->fb);
  
  /* destroy mutex & cond */
  pthread_cond_destroy(&mi->cond);
  pthread_mutex_destroy(&mi->mutex);
  
  /* free internal data */
  ALOGV("LINUXFBDR free internal data");
  free(me->internal);
} /* End of LINUXFBDR_release */

/*
 * Function    : LINUXFBDR_swap_buffer
 * Return Value: void
 * Descriptions: swap back buffer
 */
void LINUXFBDR_swap_buffer(LINUXFBDR_INTERNALP mi){
  mi->current_buffer = mi->buffer + (mi->var.yoffset * mi->fix.line_length);
  if ((mi->double_buffering)&&(mi->var.yoffset==0)){
    mi->var.yoffset = mi->var.yres;
  }
  else{
    mi->var.yoffset=0;
  }
} /* End of LINUXFBDR_swap_buffer */

/*
 * Function    : LINUXFBDR_flush
 * Return Value: byte
 * Descriptions: flush content into display & wait for vsync
 */
byte LINUXFBDR_flush(LIBAROMA_FBP me) {
  if (me == NULL) {
    return 0;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  
  if (mi->is_omap){
    /* omap wait for vsync */
    int s=0;
    mi->var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
    ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
    ioctl(mi->fb, OMAPFB_WAITFORVSYNC, &s);
  }
  else{
    /* refresh display */
    mi->var.activate = FB_ACTIVATE_VBL;
    if (ioctl(mi->fb, FBIOPAN_DISPLAY, &mi->var)!=0){
      ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
    }
  }
  return 1;
} /* End of LINUXFBDR_flush */

/*
 * Function    : LINUXFBDR_flush_receiver
 * Return Value: static void *
 * Descriptions: flush signal receiver
 */
static void * LINUXFBDR_flush_receiver(void * cookie){
  LIBAROMA_FBP me=(LIBAROMA_FBP) cookie;
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  if (mi->qcom!=NULL){
    /* using qcom fluser */
    QCOMFB_flush_receiver(me,mi);
  }
  else{
    while (mi->active){
      pthread_mutex_lock(&mi->mutex);
      pthread_cond_wait(&mi->cond, &mi->mutex);
      LINUXFBDR_swap_buffer(mi);
      pthread_mutex_unlock(&mi->mutex);
      LINUXFBDR_flush(me);
    }
  }
  return NULL;
} /* End of LINUXFBDR_flush_receiver */

/*
 * Function    : LINUXFBDR_init_features
 * Return Value: void
 * Descriptions: init framebuffer features
 */
void LINUXFBDR_init_features(LIBAROMA_FBP me) {
  if (me==NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  if (mi==NULL){
    return;
  }
  int res=0;
  
  /* set non interlanced */
  mi->var.vmode = FB_VMODE_NONINTERLACED;
  
  /* request double buffer */
  mi->double_buffering=0;
  if (mi->var.yres_virtual<mi->var.yres*2){
    mi->var.yres_virtual=mi->var.yres*2;
    mi->var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
    ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
    
    /* update vars */
    ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix);
    ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var);
    if (mi->var.yres_virtual>=mi->var.yres*2){
      /* support double buffering */
      mi->double_buffering=1;
    }
  }
  else{
    mi->double_buffering=1;
  }
  ALOGV("LINUXFBDR Double Buffering = %s",mi->double_buffering?"yes":"no");
  
  /* activate vsync - universal */
  mi->var.sync=FB_SYNC_VERT_HIGH_ACT;
  mi->var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
  res = ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
  ALOGV("LINUXFBDR FB_SYNC_VERT_HIGH_ACT = %i",res);
  
  /* update vars */
  ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix);
  ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var);
  
  /* enable omapfb vsync */
  mi->is_omap=0;
  if (mi->var.sync!=FB_SYNC_VERT_HIGH_ACT){
    if (!strncmp(mi->fix.id, "omapfb", strlen("omapfb"))){
      int state=1;
      res=ioctl(mi->fb, OMAPFB_ENABLEVSYNC, &state);
      ALOGV("LINUXFBDR OMAPFB_ENABLEVSYNC = %i",res);
      if (res==0){
        mi->is_omap=1;
      }
    }
  }
} /* End of LINUXFBDR_init_features */

/*
 * Function    : LINUXFBDP_set_dpi
 * Return Value: void
 * Descriptions: set dpi
 */
void LINUXFBDP_set_dpi(LIBAROMA_FBP me) {
  if (me==NULL) {
    return;
  }
  LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
  me->dpi = 0;
  int dpi_fallback = floor(MIN(mi->var.xres,mi->var.yres)/160) * 80;
  if ((mi->var.width<= 0)||(mi->var.height <= 0)) {
    /* phone dpi */
    me->dpi = dpi_fallback;
  }
  else{
    /* calculate dpi */
    me->dpi = round(mi->var.xres / (mi->var.width * 0.039370) / 80) * 80;
  }
  if ((me->dpi<160)||(me->dpi>960)){
    me->dpi = dpi_fallback;
  }
} /* End of LINUXFBDP_set_dpi */

/*
 * Function    : LINUXFBDR_dump
 * Return Value: void
 * Descriptions: dump framebuffer informations
 */
void LINUXFBDR_dump(LINUXFBDR_INTERNALP mi) {
  ALOGI("FRAMEBUFFER INFORMATIONS:");
  ALOGI("VAR");
  ALOGI(" xres           : %i", mi->var.xres);
  ALOGI(" yres           : %i", mi->var.yres);
  ALOGV(" xres_virtual   : %i", mi->var.xres_virtual);
  ALOGV(" yres_virtual   : %i", mi->var.yres_virtual);
  ALOGV(" xoffset        : %i", mi->var.xoffset);
  ALOGV(" yoffset        : %i", mi->var.yoffset);
  ALOGI(" bits_per_pixel : %i", mi->var.bits_per_pixel);
  ALOGV(" grayscale      : %i", mi->var.grayscale);
  ALOGI(" red            : %i, %i, %i", 
    mi->var.red.offset, mi->var.red.length, mi->var.red.msb_right);
  ALOGI(" green          : %i, %i, %i", 
    mi->var.green.offset, mi->var.green.length, mi->var.red.msb_right);
  ALOGI(" blue           : %i, %i, %i", 
    mi->var.blue.offset, mi->var.blue.length, mi->var.red.msb_right);
  ALOGV(" transp         : %i, %i, %i", 
    mi->var.transp.offset, mi->var.transp.length, mi->var.red.msb_right);
  ALOGV(" nonstd         : %i", mi->var.nonstd);
  ALOGV(" activate       : %i", mi->var.activate);
  ALOGV(" height         : %i", mi->var.height);
  ALOGV(" width          : %i", mi->var.width);
  ALOGV(" accel_flags    : %i", mi->var.accel_flags);
  ALOGV(" pixclock       : %i", mi->var.pixclock);
  ALOGV(" left_margin    : %i", mi->var.left_margin);
  ALOGV(" right_margin   : %i", mi->var.right_margin);
  ALOGV(" upper_margin   : %i", mi->var.upper_margin);
  ALOGV(" lower_margin   : %i", mi->var.lower_margin);
  ALOGV(" hsync_len      : %i", mi->var.hsync_len);
  ALOGV(" vsync_len      : %i", mi->var.vsync_len);
  ALOGV(" sync           : %i", mi->var.sync);
  ALOGV(" rotate         : %i", mi->var.rotate);
  
  ALOGI("FIX");
  ALOGI(" id             : %s", mi->fix.id);
  ALOGI(" smem_len       : %i", mi->fix.smem_len);
  ALOGV(" type           : %i", mi->fix.type);
  ALOGV(" type_aux       : %i", mi->fix.type_aux);
  ALOGV(" visual         : %i", mi->fix.visual);
  ALOGV(" xpanstep       : %i", mi->fix.xpanstep);
  ALOGV(" ypanstep       : %i", mi->fix.ypanstep);
  ALOGV(" ywrapstep      : %i", mi->fix.ywrapstep);
  ALOGI(" line_length    : %i", mi->fix.line_length);
  ALOGV(" accel          : %i", mi->fix.accel);
} /* End of LINUXFBDR_dump */

/*
 * Function    : __linux_fb_driver_init
 * Return Value: byte
 * Descriptions: init function for libaroma fb
 */
byte __linux_fb_driver_init(LIBAROMA_FBP me) {
  return LINUXFBDR_init(me);
} /* End of __linux_fb_driver_init */

#endif /* __libaroma_linux_fb_driver_c__ */
